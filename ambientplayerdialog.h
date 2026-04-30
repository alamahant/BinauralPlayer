#ifndef AMBIENTPLAYERDIALOG_H
#define AMBIENTPLAYERDIALOG_H

#include "ambientplayer.h"

#include <QDialog>

#include <QMediaPlayer>

#include<QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>

#include <QFileDialog>
#include <QMessageBox>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>

#include <QCloseEvent>
#include <QTime>
#include <QDebug>
#include <QDir>

class AmbientPlayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AmbientPlayerDialog(AmbientPlayer* player, QWidget *parent = nullptr);
    ~AmbientPlayerDialog() = default;
    void setPlayer(AmbientPlayer* player);
    void loadPlayerData();

    QPushButton *playPauseButton() const;

    QPushButton *stopButton() const;
    QMediaPlayer::PlaybackState state;
    void updateUI();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
private slots:
    void onBrowseClicked();
    void onPlayPauseClicked();
    void onStopClicked();
    void onVolumeChanged(int value);
    void onNameChanged(const QString &text);
    void onEnabledToggled(bool checked);
    void onRepeatToggled(bool checked);
    void onApplyClicked();
    void onPlayerStateChanged();
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);

private:
    void applyChanges();

    AmbientPlayer* m_player;
    bool m_changesMade;

    QLineEdit* m_filePathEdit;
    QLineEdit* m_nameEdit;
    QPushButton* m_browseButton;
    QPushButton* m_playPauseButton;
    QPushButton* m_stopButton;
    QSlider* m_volumeSlider;
    QSlider* m_progressSlider;
    QLabel* m_volumeLabel;
    QLabel* m_durationLabel;
    QCheckBox* m_enabledCheck;
    QCheckBox* m_repeatCheck;
    QPushButton* m_applyButton;
    QPushButton* m_okButton;
    QPushButton* m_cancelButton;
    void setupLayout();
    void setupConnections();
    void updateTimeDisplay(qint64 position, qint64 duration);
    void seekAudio();
    QMediaPlayer::PlaybackState m_savedState = QMediaPlayer::StoppedState;


};

#endif // AMBIENTPLAYERDIALOG_H
