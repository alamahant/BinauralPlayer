#ifndef SESSIONDIALOG_H
#define SESSIONDIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QVector>
#include <QString>

// Forward declarations for implementation-only classes
class QVBoxLayout;
class QHBoxLayout;

struct Stage {
    int toneType;          // 0=BINAURAL, 1=ISOCHRONIC, 2=GENERATOR
    double leftFreq;
    double rightFreq;
    int waveform;          // 0=SINE, 1=SQUARE, 2=TRIANGLE, 3=SAWTOOTH
    int durationMinutes;
    double pulseFreq;    // NEW: For ISOCHRONIC pulse frequency
    double volumePercent; // NEW: Volume percentage
    // Helper methods
    int durationSeconds() const { return durationMinutes * 60; }
    double beatFreq() const { return rightFreq - leftFreq; }
    bool isIsochronic() const { return toneType == 1; }
};

class SessionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SessionDialog(QWidget *parent = nullptr);
    ~SessionDialog();

    // Public interface
    bool isSessionActive() const { return m_sessionActive; }
    void stopSession();

    // Configuration
    void setUnlimitedDuration(bool unlimited) { m_unlimitedDuration = unlimited; }

    QPushButton *pauseButton() const;
    void setPauseButton(QPushButton *newPauseButton);

    QPushButton *stopButton() const;

signals:
    void stageChanged(int toneType, double leftFreq, double rightFreq,
                        int waveform, double pulseFreq, double volumePercent);
    void sessionStarted(int totalSeconds);
    void sessionEnded();
    void sessionParseRequest();  // If need MainWindow validation
    void dialogHidden();
protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onParseClicked();
    void onLoadClicked();
    void onSaveClicked();
    void onClearClicked();
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();
    void onStageTimerTimeout();

private:
    // UI Components
    QTextEdit *m_textEdit;
    QLabel *m_statusLabel;
    QLabel *m_stageInfoLabel;
    QLabel *m_totalTimeLabel;
    QPushButton *m_parseButton;
    QPushButton *m_loadButton;
    QPushButton *m_saveButton;
    QPushButton *m_clearButton;
    QPushButton *m_playButton;
    QPushButton *m_pauseButton;
    QPushButton *m_stopButton;

    // Data
    QVector<Stage> m_stages;
    int m_currentStageIndex;
    int m_stageTimeRemainingSec;
    int m_totalTimeRemainingSec;

    // State
    bool m_sessionActive;
    bool m_paused;
    bool m_unlimitedDuration;  // Added

    // Timers
    QTimer *m_stageTimer;

    // Private methods
    void setupUI();
    bool parseStagesFromText();
    Stage parseLine(const QString &line, bool &ok, QString &error);
    bool validateStage(const Stage &stage, int lineNum, QString &error);

    void startSession();
    void pauseSession();
    void resumeSession();
    void endSession();
    void startStage(int index);
    void startTransition(int fromIndex, int toIndex);

    void updateUIFromState();
    void highlightCurrentStage();
    void calculateTotalTime();
    void setupConnections();
    void formatTimeString(int seconds, QString &timeStr);
signals:
    void fadeRequested(double targetVolume);
    void pauseRequested();
    void resumeRequested();
    void syncTimersRequested(int stageIndex, int totalTimeRemainingSec);
};

#endif // SESSIONDIALOG_H
