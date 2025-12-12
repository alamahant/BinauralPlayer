#include "ambientplayerdialog.h"


#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCloseEvent>
#include <QTime>
#include <QDebug>
#include"constants.h"
#include<QDialog>

AmbientPlayerDialog::AmbientPlayerDialog(AmbientPlayer* player, QWidget *parent)
    : QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint)
    , m_player(player)
    , m_changesMade(false)
{
    setAttribute(Qt::WA_DeleteOnClose, false);  // Important!
    setWindowTitle(tr("Ambient Player Settings"));
    setMinimumSize(500, 400);

    // Create widgets
    m_filePathEdit = new QLineEdit(this);
    m_filePathEdit->setReadOnly(true);

    m_browseButton = new QPushButton(tr("Browse..."), this);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setMaxLength(10);

    m_playPauseButton = new QPushButton(tr("Play"), this);
    m_stopButton = new QPushButton(tr("Stop"), this);

    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);

    m_progressSlider = new QSlider(Qt::Horizontal, this);
    m_progressSlider->setEnabled(true); // Display-only/allow interaction
    m_volumeLabel = new QLabel("50%", this);
    m_durationLabel = new QLabel("00:00 / 00:00", this);

    m_enabledCheck = new QCheckBox(tr("Enabled"), this);
    m_repeatCheck = new QCheckBox(tr("Auto-Repeat"), this);

    m_applyButton = new QPushButton(tr("Apply"), this);
    m_okButton = new QPushButton(tr("OK"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);

    setupLayout();
    setupConnections();
    loadPlayerData();

}

void AmbientPlayerDialog::setupLayout()
{
    // File section
    QHBoxLayout* fileLayout = new QHBoxLayout();
    fileLayout->addWidget(m_filePathEdit);
    fileLayout->addWidget(m_browseButton);

    // Playback controls
    QHBoxLayout* playbackLayout = new QHBoxLayout();
    playbackLayout->addWidget(m_playPauseButton);
    playbackLayout->addWidget(m_stopButton);
    playbackLayout->addStretch();
    playbackLayout->addWidget(m_durationLabel);

    // Progress slider
    QHBoxLayout* progressLayout = new QHBoxLayout();
    progressLayout->addWidget(m_progressSlider);

    // Volume section
    QHBoxLayout* volumeLayout = new QHBoxLayout();
    volumeLayout->addWidget(new QLabel(tr("Volume:")));
    volumeLayout->addWidget(m_volumeSlider);
    volumeLayout->addWidget(m_volumeLabel);

    // Settings checkboxes
    QHBoxLayout* settingsLayout = new QHBoxLayout();
    settingsLayout->addWidget(m_enabledCheck);
    settingsLayout->addWidget(m_repeatCheck);
    settingsLayout->addStretch();

    // Dialog buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_applyButton);
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);

    // Create group boxes
    QGroupBox* fileGroup = new QGroupBox(tr("Media File"), this);
    fileGroup->setLayout(fileLayout);

    QGroupBox* nameGroup = new QGroupBox(tr("Display Name"), this);
    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameLayout->addWidget(m_nameEdit);
    nameGroup->setLayout(nameLayout);

    QGroupBox* playbackGroup = new QGroupBox(tr("Playback Controls"), this);
    QVBoxLayout* playbackGroupLayout = new QVBoxLayout();
    playbackGroupLayout->addLayout(playbackLayout);
    playbackGroupLayout->addLayout(progressLayout);
    playbackGroup->setLayout(playbackGroupLayout);

    QGroupBox* settingsGroup = new QGroupBox(tr("Settings"), this);
    QVBoxLayout* settingsGroupLayout = new QVBoxLayout();
    settingsGroupLayout->addLayout(volumeLayout);
    settingsGroupLayout->addLayout(settingsLayout);
    settingsGroup->setLayout(settingsGroupLayout);

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(fileGroup);
    mainLayout->addWidget(nameGroup);
    mainLayout->addWidget(playbackGroup);
    mainLayout->addWidget(settingsGroup);
    mainLayout->addLayout(buttonLayout);
}

void AmbientPlayerDialog::setupConnections()
{
    // Button connections
    connect(m_browseButton, &QPushButton::clicked, this, &AmbientPlayerDialog::onBrowseClicked);
    connect(m_playPauseButton, &QPushButton::clicked, this, &AmbientPlayerDialog::onPlayPauseClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &AmbientPlayerDialog::onStopClicked);
    connect(m_applyButton, &QPushButton::clicked, this, &AmbientPlayerDialog::onApplyClicked);
    connect(m_okButton, &QPushButton::clicked, this, [this]() { applyChanges(); accept(); });
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // Slider connections
    connect(m_volumeSlider, &QSlider::valueChanged, this, &AmbientPlayerDialog::onVolumeChanged);

    // Edit connections
    connect(m_nameEdit, &QLineEdit::textChanged, this, &AmbientPlayerDialog::onNameChanged);
    connect(m_enabledCheck, &QCheckBox::toggled, this, &AmbientPlayerDialog::onEnabledToggled);
    connect(m_repeatCheck, &QCheckBox::toggled, this, &AmbientPlayerDialog::onRepeatToggled);

    // Connect to player signals if player exists
    if (m_player) {
        connect(m_player, &AmbientPlayer::stateChanged, this, &AmbientPlayerDialog::onPlayerStateChanged);
        connect(m_player, &AmbientPlayer::needsUpdate, this, &AmbientPlayerDialog::updateUI);

        // Connect to player's QMediaPlayer signals for progress updates
        //QMediaPlayer* mediaPlayer = m_player->button()->parent()->findChild<QMediaPlayer*>();
        QMediaPlayer* mediaPlayer = m_player->mediaPlayer();

        if (mediaPlayer) {
            connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &AmbientPlayerDialog::onPositionChanged);
            connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &AmbientPlayerDialog::onDurationChanged);
            connect(m_progressSlider, &QSlider::sliderReleased, this, &AmbientPlayerDialog::seekAudio);

        }
    }

    connect(m_okButton, &QPushButton::clicked, this, [this]() {
        applyChanges();
        hide();
    });

    // Cancel: just hide
    connect(m_cancelButton, &QPushButton::clicked, this, &QWidget::hide);

    // Apply: just apply
    connect(m_applyButton, &QPushButton::clicked, this, &AmbientPlayerDialog::onApplyClicked);
}

void AmbientPlayerDialog::loadPlayerData()
{
    if (!m_player) return;

    m_filePathEdit->setText(m_player->filePath());
    m_nameEdit->setText(m_player->name());
    m_volumeSlider->setValue(m_player->volume());
    m_enabledCheck->setChecked(m_player->isEnabled());
    m_repeatCheck->setChecked(m_player->autoRepeat());

    updateUI();
}

void AmbientPlayerDialog::updateUI()
{
    if (!m_player) return;

    // Update volume label
    m_volumeLabel->setText(QString("%1%").arg(m_player->volume()));  // ← FIXED!
    // Update play/pause button
    QMediaPlayer::PlaybackState state = m_player->playbackState();
    if (state == QMediaPlayer::PlayingState) {
        m_playPauseButton->setText(tr("Pause"));
    } else {
        m_playPauseButton->setText(tr("Play"));
    }

    // Update enabled state
    m_playPauseButton->setEnabled(m_player->hasAudio());
    m_stopButton->setEnabled(m_player->hasAudio());
}

void AmbientPlayerDialog::onBrowseClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("Select Audio File"),
        ConstantGlobals::ambientFilePath,
        tr("Audio Files (*.mp3 *.wav *.ogg *.flac *.m4a);;All Files (*.*)"));

    if (!filePath.isEmpty()) {
        m_filePathEdit->setText(filePath);
        m_changesMade = true;

        // Auto-enable if selecting a file
        if (!m_enabledCheck->isChecked()) {
            m_enabledCheck->setChecked(true);
        }
    }
}

void AmbientPlayerDialog::onPlayPauseClicked()
{
    if (!m_player) return;

    QMediaPlayer::PlaybackState state = m_player->playbackState();
    if (state == QMediaPlayer::PlayingState) {
        m_player->pause();
    } else {
        m_player->play();
    }
}

void AmbientPlayerDialog::onStopClicked()
{
    if (m_player) {
        m_player->stop();
    }
}

void AmbientPlayerDialog::onVolumeChanged(int value)
{
    m_volumeLabel->setText(QString("%1%").arg(value));
    m_changesMade = true;
    if (m_player) {
            m_player->setVolume(value);
        }
}

void AmbientPlayerDialog::onNameChanged(const QString &text)
{
    // Update button text in real-time for preview
    if (m_player) {
        m_player->setName(text);
    }
    m_changesMade = true;

}

void AmbientPlayerDialog::onEnabledToggled(bool checked)
{
    m_changesMade = true;
}

void AmbientPlayerDialog::onRepeatToggled(bool checked)
{
    m_changesMade = true;
}

void AmbientPlayerDialog::onApplyClicked()
{
    applyChanges();
    m_changesMade = false;
}

void AmbientPlayerDialog::applyChanges()
{
    if (!m_player || !m_changesMade) return;
    // Save current state BEFORE applying changes
    // Apply all changes to the player
    m_player->setFilePath(m_filePathEdit->text());
    m_player->setName(m_nameEdit->text());
    //m_player->setVolume(m_volumeSlider->value());
    m_player->setEnabled(m_enabledCheck->isChecked());
    m_player->setAutoRepeat(m_repeatCheck->isChecked());

    m_changesMade = false;
}

QPushButton *AmbientPlayerDialog::stopButton() const
{
    return m_stopButton;
}

QPushButton *AmbientPlayerDialog::playPauseButton() const
{
    return m_playPauseButton;
}

void AmbientPlayerDialog::onPlayerStateChanged()
{
    updateUI();
}

void AmbientPlayerDialog::onPositionChanged(qint64 position)
{

    if (!m_progressSlider->isSliderDown()) {
        m_progressSlider->setValue(position);
    }
    updateTimeDisplay(position, m_progressSlider->maximum());
}

void AmbientPlayerDialog::onDurationChanged(qint64 duration)
{


    m_progressSlider->setRange(0, duration);

    // Update display with current position
       qint64 currentPos = m_progressSlider->value();
       updateTimeDisplay(currentPos, duration);
}


void AmbientPlayerDialog::setPlayer(AmbientPlayer* player)
{
    if (m_player == player) return;


    // Save changes if any
    if (m_changesMade && m_player) {
        applyChanges();
    }

    // Switch to new player
    m_player = player;
    m_changesMade = false;

    // Reload UI with new player's data
    loadPlayerData();
    updateUI();

    // Update window title
    setWindowTitle(tr("Ambient Player - %1").arg(player->name()));
}

void AmbientPlayerDialog::closeEvent(QCloseEvent *event)
{
    hide();  // Just hide, don't close
    event->ignore();  // Don't process further

}

void AmbientPlayerDialog::updateTimeDisplay(qint64 position, qint64 duration)
{
    QTime currentTime = QTime(0, 0, 0).addMSecs(position);
    QTime totalTime = QTime(0, 0, 0).addMSecs(duration);

    m_durationLabel->setText(QString("%1 / %2")
        .arg(currentTime.toString("mm:ss"))
        .arg(totalTime.toString("mm:ss")));
}


void AmbientPlayerDialog::showEvent(QShowEvent *event)
{

    QDialog::showEvent(event);
    //correcting some weitd bug that dialog open causes auto play
    if(m_player->isEnabled() && state == QMediaPlayer::PlayingState){
        m_playPauseButton->click();
    }else if(m_player->isEnabled() && state == QMediaPlayer::PausedState){
        m_playPauseButton->click();

    }else if(m_player->isEnabled() && state == QMediaPlayer::StoppedState){
        m_stopButton->click();

    }


}

void AmbientPlayerDialog::hideEvent(QHideEvent *event)
{

   if (m_player->isEnabled()){
       state = m_player->playbackState();
   }


    QDialog::hideEvent(event);
}

void AmbientPlayerDialog::seekAudio()
{
    int position = m_progressSlider->value();
       QMediaPlayer* mediaPlayer = m_player->mediaPlayer();
       if (mediaPlayer && mediaPlayer->isSeekable()) {
           mediaPlayer->setPosition(position);
       }
}

