#include "ambientplayer.h"

#include <QDebug>
#include<QAudioOutput>

AmbientPlayer::AmbientPlayer(QObject *parent)
    : QObject(parent)
    , m_name("Unnamed")
    , m_volume(50)
    , m_enabled(false)
    , m_autoRepeat(true)
    , m_baseVolume(50)
    , m_masterRatio(1.0f)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);

    m_button = new QPushButton(m_name);
    m_button->setMinimumWidth(80);
    m_button->setMaximumWidth(100);
    m_button->setCheckable(false);

    setupConnections();
    updatePlayerSettings();
}

AmbientPlayer::~AmbientPlayer()
{
}

void AmbientPlayer::setupConnections()
{
    connect(m_player, &QMediaPlayer::playbackStateChanged,
            this, &AmbientPlayer::updateButtonState);

    connect(m_button, &QPushButton::clicked, this, [this]() {
        if (m_player->playbackState() == QMediaPlayer::PlayingState) {
            pause();
        } else {
            play();
        }
    });

    connect(m_player, &QMediaPlayer::errorOccurred, this, [this]() {
        qWarning() << "AmbientPlayer error:" << m_player->errorString();
    });
}

void AmbientPlayer::updateButtonState()
{
    QString icon;
    switch (m_player->playbackState()) {
    case QMediaPlayer::PlayingState:
        icon = " ❚❚";  // Pause symbol
        m_button->setStyleSheet("QPushButton { color: green; }");
        break;
    case QMediaPlayer::PausedState:
        icon = " ▶";   // Play symbol
        m_button->setStyleSheet("QPushButton { color: orange; }");
        break;
    case QMediaPlayer::StoppedState:
        icon = " ■";   // Stop symbol
        m_button->setStyleSheet("");  // Default style
        break;
    }

    QString displayName = m_name;
    if (displayName.length() > 10) {
        displayName = displayName.left(8) + "..";
    }

    m_button->setText(displayName + icon);

    emit stateChanged();
    emit needsUpdate();
}

void AmbientPlayer::updatePlayerSettings()
{
    m_audioOutput->setVolume(m_volume);
    m_player->setLoops(m_autoRepeat ? QMediaPlayer::Infinite : 1);


    if (!m_enabled) {
        m_button->setStyleSheet("QPushButton { color: gray; }");
    }

    emit needsUpdate();
}

int AmbientPlayer::Volume() const
{
    return m_baseVolume;
}


void AmbientPlayer::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        updateButtonState();  // Update button text with new name
        emit nameChanged(m_name);
    }
}




void AmbientPlayer::setFilePath(const QString &path)
{
    if (m_filePath != path) {
        m_filePath = path;
        if (!path.isEmpty()) {
            m_player->setSource(QUrl::fromLocalFile(path));
        } else {
            m_player->setSource(QUrl());  // Clear source
        }
        emit needsUpdate();
    }
}


void AmbientPlayer::setVolume(int volume)  // volume is 0-100
{
    volume = qBound(0, volume, 100);
    if (m_volume != volume) {
        m_volume = volume;
        m_audioOutput->setVolume(m_volume / 100.0f);
        emit needsUpdate();
    }
}


void AmbientPlayer::applyMasterVolume(float ratio)
{
    m_masterRatio = ratio;

    float linear = m_baseVolume * m_masterRatio / 100.0f;
    float perceptual = qPow(linear, 0.5f);  // Square root curve

    m_audioOutput->setVolume(perceptual);
}

void AmbientPlayer::setEnabled(bool enabled)
{
    if (m_enabled != enabled) {
        m_enabled = enabled;

        if (!m_enabled && m_player->playbackState() == QMediaPlayer::PlayingState) {
            m_player->stop();
        }

        updatePlayerSettings();  // Update button appearance
        emit needsUpdate();
    }
}

void AmbientPlayer::setAutoRepeat(bool repeat)
{
    if (m_autoRepeat != repeat) {
        m_autoRepeat = repeat;
        m_player->setLoops(m_autoRepeat ? QMediaPlayer::Infinite : 1);
        emit needsUpdate();
    }
}


void AmbientPlayer::play()
{
    if (!m_enabled) {
        return;
    }

    if (m_filePath.isEmpty()) {
        return;
    }

    if (m_player->playbackState() == QMediaPlayer::StoppedState) {
        if (m_player->source().isEmpty() && !m_filePath.isEmpty()) {
            m_player->setSource(QUrl::fromLocalFile(m_filePath));
        }
    }

    m_player->play();
}

void AmbientPlayer::pause()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        m_player->pause();
    }
}

void AmbientPlayer::stop()
{
    if (m_player->playbackState() != QMediaPlayer::StoppedState) {
        m_player->stop();
    }
}

QMediaPlayer::PlaybackState AmbientPlayer::playbackState() const
{
    return m_player->playbackState();
}

/*
void AmbientPlayer::setVolume(int volume)
{
    volume = qBound(0, volume, 100);
    if (m_baseVolume != volume) {
        m_baseVolume = volume;
        applyMasterVolume(m_masterRatio);
        emit needsUpdate();
    }
}
*/
