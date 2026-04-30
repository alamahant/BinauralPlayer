#pragma once

#include <QDialog>
#include <QColor>
#include "flickerwidget.h"

class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QSlider;
class QPushButton;
class QTextEdit;
class QSpinBox;
class QButtonGroup;

class VisStimDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VisStimDialog(FlickerWidget *flicker, QWidget *parent = nullptr);

    void syncFrequency(double hz);
    void syncWaveType(int type);   // 0=sine 1=square 2=sawtooth

    QPushButton *startStopBtn() const;

private slots:
    void onFreqOverrideToggled(bool checked);
    void onEnvOverrideToggled(bool checked);
    void onFreqSpinChanged(double hz);
    void onIntensityChanged(int value);
    void onPickOnColor();
    void onPickOffColor();
    void onPickTextColor();
    void onPickTextBgColor();
    void onStartStop();

private:
    void buildUi();
    void updateSyncBadges();
    void updateBandLabel(double hz);
    void applyToFlicker();
    QString bandName(double hz) const;
    void setSwatchColor(QPushButton *btn, const QColor &color);

    FlickerWidget *m_flicker;

    double m_syncedFreq = 7.83;
    int    m_syncedWave = 1;       // 0=sine 1=square 2=sawtooth

    QLabel         *m_freqSyncBadge   = nullptr;
    QCheckBox      *m_freqOverrideCb  = nullptr;
    QDoubleSpinBox *m_freqSpin        = nullptr;
    QLabel         *m_bandLabel       = nullptr;

    QLabel         *m_envSyncBadge    = nullptr;
    QCheckBox      *m_envOverrideCb   = nullptr;
    QButtonGroup   *m_envGroup        = nullptr;   // square/sine/sawtooth

    QSlider        *m_intensitySlider = nullptr;
    QLabel         *m_intensityLabel  = nullptr;

    QPushButton    *m_onColorBtn      = nullptr;
    QPushButton    *m_offColorBtn     = nullptr;
    QColor          m_onColor         = Qt::white;
    QColor          m_offColor        = Qt::black;

    QTextEdit      *m_textEdit        = nullptr;
    QButtonGroup   *m_displayGroup    = nullptr;   // off/flash/always
    QSpinBox       *m_fontSizeSpin    = nullptr;
    QPushButton    *m_textColorBtn    = nullptr;
    QPushButton    *m_textBgBtn       = nullptr;
    QColor          m_textColor       = Qt::white;
    QColor          m_textBgColor     = QColor(0, 0, 0, 0);

    QPushButton    *m_startStopBtn    = nullptr;
    bool            m_running         = false;

    QDoubleSpinBox *m_subliminalFactorSpin = nullptr;
private slots:
    void onReset();
};
