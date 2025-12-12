#ifndef AMBIENTPLAYER_H
#define AMBIENTPLAYER_H

#include <QObject>

#include <QMediaPlayer>
#include <QPushButton>
#include<QAudioOutput>

class AmbientPlayer : public QObject
{
    Q_OBJECT

public:
    // Constructor
    explicit AmbientPlayer(QObject *parent = nullptr);
    ~AmbientPlayer();
    QMediaPlayer* mediaPlayer() const { return m_player; }
    // ----- SIMPLE SETTERS/GETTERS (No need for complex ones) -----
    void setName(const QString &name);
    QString name() const { return m_name; }

    void setFilePath(const QString &path);
    QString filePath() const { return m_filePath; }

    void setVolume(int volume);  // 0-100
    int volume() const { return m_volume; }

    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

    void setAutoRepeat(bool repeat);
    bool autoRepeat() const { return m_autoRepeat; }

    // ----- PLAYBACK CONTROL -----
    void play();
    void pause();
    void stop();

    QMediaPlayer::PlaybackState playbackState() const;

    // ----- UI GETTER (MainWindow will use this) -----
    QPushButton* button() const { return m_button; }

    // ----- STATE CHECK -----
    bool hasAudio() const { return !m_filePath.isEmpty(); }

    int Volume() const;

signals:
    void nameChanged(const QString &newName);
    void stateChanged();
    void needsUpdate();  // Generic "something changed" signal

private slots:
    void updateButtonState();

private:
    // Core Data
    QString m_name;
    QString m_filePath;
    int m_volume;
    bool m_enabled;
    bool m_autoRepeat;

    // Audio Engine
    QMediaPlayer* m_player;

    // UI Element (One button in toolbar)
    QPushButton* m_button;

    void setupConnections();
    void updatePlayerSettings();
    QAudioOutput *m_audioOutput = nullptr;

    // master volume intergration
    int m_baseVolume;      // User's choice (0-100)
    float m_masterRatio;   // Master scaling (0.0-1.0, start at 1.0)
public:
    void applyMasterVolume(float ratio);

};
#endif // AMBIENTPLAYER_H
