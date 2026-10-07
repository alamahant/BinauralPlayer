#include "subtitlemanager.h"

#include <QFile>
#include <QTextStream>
#include <QStringConverter>
#include <QStringList>
#include <algorithm>
#include<QRegularExpression>

SubtitleManager::SubtitleManager(QObject *parent) : QObject(parent) {}

qint64 SubtitleManager::parseSrtTime(const QString &s)
{
    // "00:05:06,181" -> 306181
    QString t = s.trimmed();
    t.replace(',', ':');
    const QStringList p = t.split(':');
    if (p.size() != 4) return -1;
    return (((p[0].toLongLong() * 60 + p[1].toLongLong()) * 60)
            + p[2].toLongLong()) * 1000 + p[3].toLongLong();
}

/*

bool SubtitleManager::loadSrt(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit error("Cannot open " + filePath);
        return false;
    }

    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    f.close();

    // Strip BOM
    if (!content.isEmpty() && content.at(0) == QChar(0xFEFF))
        content.remove(0, 1);

    // Normalize line endings
    content.replace("\r\n", "\n");
    content.replace('\r', '\n');

    // Split into blocks separated by blank lines
    const QStringList blocks = content.split(QRegularExpression("\n\\s*\n"),
                                             Qt::SkipEmptyParts);

    m_cues.clear();
    m_filePath = filePath;

    for (const QString &block : blocks) {
        const QStringList lines = block.split('\n');
        if (lines.size() < 2) continue;

        // Find the timestamp line (usually line 1, but tolerate variations)
        int tsIndex = -1;
        for (int i = 0; i < lines.size(); ++i) {
            if (lines.at(i).contains("-->")) { tsIndex = i; break; }
        }
        if (tsIndex < 0) continue;

        const QStringList times = lines.at(tsIndex).split("-->");
        if (times.size() != 2) continue;

        SubtitleCue cue;
        cue.startMs = parseSrtTime(times[0]);
        cue.endMs   = parseSrtTime(times[1]);
        if (cue.startMs < 0 || cue.endMs <= cue.startMs) continue;

        QStringList textLines;
        for (int i = tsIndex + 1; i < lines.size(); ++i)
            textLines << lines.at(i);
        cue.text = textLines.join('\n');

        m_cues.append(cue);
    }

    std::sort(m_cues.begin(), m_cues.end(),
              [](const SubtitleCue &a, const SubtitleCue &b) {
                  return a.startMs < b.startMs;
              });

    if (m_cues.isEmpty()) {
        emit error("No cues parsed from " + filePath);
        return false;
    }

    emit loaded(filePath, m_cues.size());
    return true;
}
*/

bool SubtitleManager::loadSrt(const QString &filePath)
{
    QFile f(filePath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit error("Cannot open " + filePath);
        return false;
    }

    QTextStream in(&f);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    f.close();

    // Strip BOM
    if (!content.isEmpty() && content.at(0) == QChar(0xFEFF))
        content.remove(0, 1);

    // Normalize line endings
    content.replace("\r\n", "\n");
    content.replace('\r', '\n');

    // Split into blocks separated by blank lines
    const QStringList blocks = content.split(QRegularExpression("\n[ \t]*\n"),
                                             Qt::SkipEmptyParts);

    m_cues.clear();
    m_filePath = filePath;

    // Regex to strip ASS/SSA override blocks like {\an8}, {\pos(10,20)}, {\i1}
    static const QRegularExpression kOverride(R"(\{\\[^}]*\})");
    // Regex to strip stray bare tags like \an8 (no braces)
    static const QRegularExpression kStrayTag(R"(\\an[1-9])");

    for (const QString &block : blocks) {
        const QStringList lines = block.split('\n');
        if (lines.size() < 2) continue;

        // Find the timestamp line
        int tsIndex = -1;
        for (int i = 0; i < lines.size(); ++i) {
            if (lines.at(i).contains("-->")) { tsIndex = i; break; }
        }
        if (tsIndex < 0) continue;

        const QStringList times = lines.at(tsIndex).split("-->");
        if (times.size() != 2) continue;

        SubtitleCue cue;
        cue.startMs = parseSrtTime(times[0]);
        cue.endMs   = parseSrtTime(times[1]);
        if (cue.startMs < 0 || cue.endMs <= cue.startMs) continue;

        QStringList textLines;
        for (int i = tsIndex + 1; i < lines.size(); ++i)
            textLines << lines.at(i);

        QString text = textLines.join('\n');

        // Strip ASS/SSA positioning and style tags
        text.remove(kOverride);
        text.remove(kStrayTag);
        text = text.trimmed();

        // Skip cues that were nothing but tags
        if (text.isEmpty()) continue;

        cue.text = text;
        m_cues.append(cue);
    }

    std::sort(m_cues.begin(), m_cues.end(),
              [](const SubtitleCue &a, const SubtitleCue &b) {
                  return a.startMs < b.startMs;
              });

    if (m_cues.isEmpty()) {
        emit error("No cues parsed from " + filePath);
        return false;
    }

    emit loaded(filePath, m_cues.size());
    return true;
}

void SubtitleManager::clear()
{
    m_cues.clear();
    m_filePath.clear();
    emit cleared();
}

QString SubtitleManager::textAt(qint64 positionMs) const
{
    const qint64 adjusted = positionMs + m_offsetMs;
    for (const auto &cue : m_cues) {
        if (adjusted >= cue.startMs && adjusted < cue.endMs)
            return cue.text;
    }
    return QString();
}

void SubtitleManager::setOffsetMs(qint64 offsetMs)
{
    m_offsetMs = offsetMs;
}
