#include "cuesheetdialog.h"


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>
#include"constants.h"

CueSheetDialog::CueSheetDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("CUE Sheet Import");
    setMinimumSize(500, 400);

    m_trackList = new QListWidget(this);
    m_trackList->setAlternatingRowColors(true);

    m_loadButton = new QPushButton("Load CUE File", this);
    m_playButton = new QPushButton("Play Track", this);
    m_prevButton = new QPushButton("◄", this);
    m_nextButton = new QPushButton("►", this);
    m_clearButton = new QPushButton("Clear", this);
    m_statusLabel = new QLabel("No CUE file loaded", this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_loadButton);
    buttonLayout->addWidget(m_prevButton);
    buttonLayout->addWidget(m_playButton);
    buttonLayout->addWidget(m_nextButton);
    buttonLayout->addWidget(m_clearButton);

    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_trackList);
    mainLayout->addWidget(m_statusLabel);

    connect(m_loadButton, &QPushButton::clicked, this, &CueSheetDialog::onLoadCue);
    connect(m_playButton, &QPushButton::clicked, this, &CueSheetDialog::onPlayTrack);
    connect(m_prevButton, &QPushButton::clicked, this, &CueSheetDialog::onPreviousTrack);
    connect(m_nextButton, &QPushButton::clicked, this, &CueSheetDialog::onNextTrack);
    connect(m_clearButton, &QPushButton::clicked, this, &CueSheetDialog::onClearAll);
    connect(m_trackList, &QListWidget::itemDoubleClicked,
            this, &CueSheetDialog::onTrackDoubleClicked);
}

CueSheetDialog::~CueSheetDialog() {}

void CueSheetDialog::closeEvent(QCloseEvent *event)
{
    emit hideRequested();
}

/*
void CueSheetDialog::onLoadCue()
{
    QString cuePath = QFileDialog::getOpenFileName(this,
            "Open CUE Sheet",
            ConstantGlobals::appDirPath,  // Opens at this directory
            "CUE files (*.cue);;All files (*.*)");


    if (!cuePath.isEmpty()) {
        parseCueFile(cuePath);
        updateTrackDisplay();
    }
}
*/

void CueSheetDialog::onLoadCue()
{
    QString initialDir = !ConstantGlobals::lastMusicDirPath.isEmpty()
                          ? ConstantGlobals::lastMusicDirPath
                          : ConstantGlobals::appDirPath;

    QString cuePath = QFileDialog::getOpenFileName(this,
        "Open CUE Sheet", initialDir,
        "CUE files (*.cue);;All files (*.*)");

    if (cuePath.isEmpty()) return;

    parseCueFile(cuePath);

    QMessageBox::StandardButton reply = QMessageBox::question(this,
        "Confirm CUE Loading",
        QString("Load CUE for:\n%1\n\nWith audio file:\n%2")
            .arg(QFileInfo(cuePath).fileName())
            .arg(QFileInfo(m_audioFilePath).fileName()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        updateTrackDisplay();
    } else {
        m_tracks.clear();
        m_audioFilePath.clear();
        m_trackList->clear();
    }
}


void CueSheetDialog::parseCueFile(const QString &cueFilePath)
{
    QFile file(cueFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_statusLabel->setText("Error opening CUE file");
        return;
    }

    m_tracks.clear();
    m_audioFilePath.clear();
    m_currentTrackIndex = -1;

    QTextStream stream(&file);
    QString line;
    CueTrack currentTrack;
    bool inTrack = false;

    QFileInfo cueInfo(cueFilePath);
    QString baseDir = cueInfo.absolutePath();

    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();

        if (line.startsWith("FILE ")) {
            int start = line.indexOf('"');
            int end = line.lastIndexOf('"');
            if (start != -1 && end != -1 && end > start + 1) {
                QString audioFile = line.mid(start + 1, end - start - 1);

                if (QFileInfo(audioFile).isRelative()) {
                    m_audioFilePath = baseDir + "/" + audioFile;
                } else {
                    m_audioFilePath = audioFile;
                }
            }
        }
        else if (line.startsWith("TRACK ") && line.contains(" AUDIO")) {
            if (inTrack) {
                m_tracks.append(currentTrack);
            }

            currentTrack = CueTrack();
            inTrack = true;

            QStringList parts = line.split(' ', Qt::SkipEmptyParts);
            if (parts.size() >= 2) {
                currentTrack.number = parts[1].toInt();
            }
        }
        else if (line.startsWith("TITLE ")) {
            if (inTrack) {
                int start = line.indexOf('"');
                int end = line.lastIndexOf('"');
                if (start != -1 && end != -1 && end > start + 1) {
                    currentTrack.title = line.mid(start + 1, end - start - 1);
                }
            }
        }
        else if (line.startsWith("PERFORMER ")) {
            if (inTrack) {
                int start = line.indexOf('"');
                int end = line.lastIndexOf('"');
                if (start != -1 && end != -1 && end > start + 1) {
                    currentTrack.performer = line.mid(start + 1, end - start - 1);
                }
            }
        }
        else if (line.startsWith("INDEX 01 ")) {
            if (inTrack) {
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() >= 3) {
                    currentTrack.timeCode = parts[2];
                    currentTrack.startMs = timeCodeToMs(parts[2]);
                }
            }
        }
    }

    if (inTrack) {
        m_tracks.append(currentTrack);
    }

    file.close();

    if (!m_tracks.isEmpty()) {
        m_currentTrackIndex = 0;
        m_statusLabel->setText(QString("Loaded %1 tracks from %2")
            .arg(m_tracks.size()).arg(QFileInfo(cueFilePath).fileName()));
    } else {
        m_statusLabel->setText("No tracks found in CUE file");
    }
}

qint64 CueSheetDialog::timeCodeToMs(const QString &timeCode)
{
    QStringList parts = timeCode.split(':');
    if (parts.size() != 3) return 0;

    int minutes = parts[0].toInt();
    int seconds = parts[1].toInt();
    int frames = parts[2].toInt();

    double frameMs = frames * (1000.0 / 75.0);

    return (minutes * 60000) + (seconds * 1000) + static_cast<qint64>(frameMs);
}

void CueSheetDialog::updateTrackDisplay()
{
    m_trackList->clear();

    for (const CueTrack &track : m_tracks) {
        QString displayText = QString("Track %1: %2")
            .arg(track.number, 2, 10, QChar('0'))
            .arg(track.title);

        if (!track.performer.isEmpty()) {
            displayText += QString(" - %1").arg(track.performer);
        }

        displayText += QString(" [%1]").arg(track.timeCode);

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, track.startMs);
        m_trackList->addItem(item);
    }

    if (m_currentTrackIndex >= 0 && m_currentTrackIndex < m_trackList->count()) {
        m_trackList->setCurrentRow(m_currentTrackIndex);
    }
}

void CueSheetDialog::onPlayTrack()
{
    /*
    if (m_currentTrackIndex < 0 || m_currentTrackIndex >= m_tracks.size()) return;
    if (m_audioFilePath.isEmpty()) return;

    const CueTrack &track = m_tracks[m_currentTrackIndex];

    qint64 startSeconds = track.startMs / 1000;

    emit trackSelected(m_audioFilePath, startSeconds);
    */


    QListWidgetItem *currentItem = m_trackList->currentItem();
    if (!currentItem) {
        if (m_currentTrackIndex >= 0 && m_currentTrackIndex < m_tracks.size()) {
            const CueTrack &track = m_tracks[m_currentTrackIndex];
            qint64 startSeconds = track.startMs / 1000;
            emit trackSelected(m_audioFilePath, startSeconds);
        }
        return;
    }

    int row = m_trackList->row(currentItem);
    if (row < 0 || row >= m_tracks.size()) return;

    m_currentTrackIndex = row;
    const CueTrack &track = m_tracks[row];
    qint64 startSeconds = track.startMs / 1000;
    emit trackSelected(m_audioFilePath, startSeconds);
}

void CueSheetDialog::onTrackDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;
    if (m_audioFilePath.isEmpty()) return;

    int row = m_trackList->row(item);
    if (row >= 0 && row < m_tracks.size()) {
        m_currentTrackIndex = row;
        const CueTrack &track = m_tracks[row];
        qint64 startSeconds = track.startMs / 1000;

        emit trackSelected(m_audioFilePath, startSeconds);
    }
}

void CueSheetDialog::onNextTrack()
{
    if (m_tracks.isEmpty()) return;

    m_currentTrackIndex = (m_currentTrackIndex + 1) % m_tracks.size();
    m_trackList->setCurrentRow(m_currentTrackIndex);

    if (ConstantGlobals::playbackState == QMediaPlayer::PlayingState){
    const CueTrack &track = m_tracks[m_currentTrackIndex];

    qint64 startSeconds = track.startMs / 1000;
    emit trackSelected(m_audioFilePath, startSeconds);
    }
}

void CueSheetDialog::onPreviousTrack()
{
    if (m_tracks.isEmpty()) return;

    m_currentTrackIndex = (m_currentTrackIndex - 1 + m_tracks.size()) % m_tracks.size();
    m_trackList->setCurrentRow(m_currentTrackIndex);

    if (ConstantGlobals::playbackState == QMediaPlayer::PlayingState){

    const CueTrack &track = m_tracks[m_currentTrackIndex];

    qint64 startSeconds = track.startMs / 1000;
    emit trackSelected(m_audioFilePath, startSeconds);
    }
}

void CueSheetDialog::onClearAll()
{
    m_tracks.clear();
    m_audioFilePath.clear();
    m_currentTrackIndex = -1;
    m_trackList->clear();
    m_statusLabel->setText("Cleared all CUE data");
}
