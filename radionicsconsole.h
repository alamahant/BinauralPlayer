#ifndef RADIONICSCONSOLE_H
#define RADIONICSCONSOLE_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDial>
#include <QSlider>
#include <QPixmap>
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include <cmath>
#include<QRandomGenerator>
#include<QTextEdit>
#include<QProgressBar>

class SpinKnob : public QDial
{
    Q_OBJECT
public:
    explicit SpinKnob(const QString &label, QWidget *parent = nullptr);
    int getLockedValue() const { return m_lockedValue; }
    bool isLocked() const { return m_isLocked; }
    void reset();

signals:
    void locked(int value);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void spin();

private:
    QTimer m_timer;
    bool m_isSpinning;
    bool m_isLocked;
    int m_lockedValue;
    QString m_label;
    QLabel *m_valueLabel;
    QRandomGenerator m_gen;
    int m_currentRandomValue;
};

class RadionicsConsole : public QDialog
{
    Q_OBJECT

public:
    explicit RadionicsConsole(QWidget *parent = nullptr);
    ~RadionicsConsole();

    double getCombinedSeed() const { return m_combinedSeed; }
    double getOffset() const { return m_offset; }
    double getLeftFrequency() const { return m_leftFrequency; }
    double getRightFrequency() const { return m_rightFrequency; }
    QString getTrend() const { return trendEdit->toPlainText().trimmed(); }
    QString getTarget() const { return targetEdit->toPlainText().trimmed(); }

protected:
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
signals:
    void structuralLinkCaptured(double seed, double leftFreq, double rightFreq,
                                double baseFreq, double offset,
                                const QString &trend, const QString &target);
    void playRequested(double leftFreq, double rightFreq);
    void stopRequested();
    void closeRequested();

private slots:
    void onUploadTrendImage();
    void onUploadTargetImage();
    void onBaseFrequencyChanged(int value);
    void onKnobLocked(int value);
    void onPlay();
    void onStop();
    void onSave();
    void onResetKnobs();

public slots:

    void onExternalStopRequested();
private:
    void setupUI();
    void updateCombinedSeed();
    void updateFrequencyDisplay();
    void resetAll();

    // ============================================================
    // UI Elements
    // ============================================================
    QLabel *m_trendLabel;
    QTextEdit *trendEdit;
    QLabel *m_trendImagePreview;
    QPushButton *m_trendUploadBtn;

    QLabel *m_targetLabel;
    QTextEdit *targetEdit;
    QLabel *m_targetImagePreview;
    QPushButton *m_targetUploadBtn;

    QLabel *m_baseFreqLabel;
    QSlider *m_baseFreqSlider;
    QLabel *m_baseFreqValue;

    SpinKnob *m_knob1;
    SpinKnob *m_knob2;
    SpinKnob *m_knob3;

    QLabel *m_seedLabel;
    QLabel *m_offsetLabel;
    QLabel *m_leftFreqLabel;
    QLabel *m_rightFreqLabel;

    QPushButton *m_playBtn;
    QPushButton *m_stopBtn;
    QPushButton *m_saveBtn;
    QPushButton *m_resetBtn;

    // ============================================================
    // Data
    // ============================================================
    double m_baseFrequency;
    double m_combinedSeed;
    double m_offset;
    double m_leftFrequency;
    double m_rightFrequency;

    int m_knob1Seed;
    int m_knob2Seed;
    int m_knob3Seed;

    bool m_isLocked;
    //duration slider
    QLabel *m_durationLabel;
    QSlider *m_durationSlider;
    QLabel *m_durationValue;
    int m_durationMinutes;
private slots:
    void onDurationChanged(int value);
    void loadSession();

signals:
    void durationChanged(int minutes);
private:
    QPixmap m_originalTargetPixmap;
    QPixmap m_originalTrendPixmap;
    QLabel *m_zoomPopup{nullptr};  // Custom popup for zoomed image
    void showZoomedImage(const QPixmap &pixmap, QLabel *sourceLabel);
    void hideZoomedImage();
    void showMessageBox(const QString &title, const QString &text);
    void saveSession(const QString &filePath);
    QString m_currentSessionPath;
    QString m_lastTargetImagePath;
    QString m_lastTrendImagePath;
    QProgressBar *bogusProgress;
    QTimer *m_bogusTimer;
};

#endif // RADIONICSCONSOLE_H
