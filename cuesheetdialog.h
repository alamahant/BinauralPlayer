#ifndef CUESHEETDIALOG_H
#define CUESHEETDIALOG_H

#include <QDialog>
#include <QList>
#include<QListWidget>
#include<QListWidgetItem>
#include<QLabel>

// Track data structure
struct CueTrack {
    int number;
    QString title;
    QString performer;
    qint64 startMs;  // Start time in milliseconds
    QString timeCode; // Original MM:SS:FF
};

class CueSheetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CueSheetDialog(QWidget *parent = nullptr);
    ~CueSheetDialog();
protected:
    void closeEvent(QCloseEvent *event) override;
signals:
    // Only 2 signals to communicate with MainWindow
    void trackSelected(const QString &audioFilePath, qint64 startPositionMs);
    void needAudioFileLoaded(const QString &audioFilePath);
    void hideRequested();
    void trackPositionChanged(qint64 positionMs);
private slots:
    void onLoadCue();
    void onPlayTrack();
    void onNextTrack();
    void onPreviousTrack();
    void onTrackDoubleClicked(QListWidgetItem *item);
    void onClearAll();

private:
    void parseCueFile(const QString &cueFilePath);
    qint64 timeCodeToMs(const QString &timeCode); // MM:SS:FF -> ms
    void updateTrackDisplay();

    // UI
    QListWidget *m_trackList;
    QPushButton *m_loadButton;
    QPushButton *m_playButton;
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;
    QLabel *m_statusLabel;

    // Data
    QString m_audioFilePath; // From "FILE" command in CUE
    QList<CueTrack> m_tracks;
    int m_currentTrackIndex = -1;
    QPushButton *m_clearButton;
};

#endif // CUESHEETDIALOG_H
