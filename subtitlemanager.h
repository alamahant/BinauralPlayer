#pragma once

#include <QObject>
#include <QString>
#include <QList>

struct SubtitleCue {
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString text;
};

class SubtitleManager : public QObject
{
    Q_OBJECT
public:
    explicit SubtitleManager(QObject *parent = nullptr);

    // Load an SRT file from disk. Returns true on success.
    bool loadSrt(const QString &filePath);

    // Clear the loaded cues.
    void clear();

    // Look up subtitle text for a given playback position (ms).
    // Returns empty string if no cue covers this position.
    QString textAt(qint64 positionMs) const;

    // Shift all cues by an offset (ms). Useful for sync correction.
    void setOffsetMs(qint64 offsetMs);
    qint64 offsetMs() const { return m_offsetMs; }

    bool isEmpty() const { return m_cues.isEmpty(); }
    int cueCount() const { return m_cues.size(); }
    QString currentFile() const { return m_filePath; }

signals:
    void loaded(const QString &filePath, int cueCount);
    void cleared();
    void error(const QString &message);

private:
    QList<SubtitleCue> m_cues;
    QString m_filePath;
    qint64 m_offsetMs = 0;

    static qint64 parseSrtTime(const QString &s);
};