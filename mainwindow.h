#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QObject>
#include <QMainWindow>
#include <QToolBar>
#include <QListWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "binauralengine.h"
#include<QSettings>
#include<QMediaPlayer>
#include<QSlider>
#include<QComboBox>
#include<QStandardItemModel>
#include"dynamicengine.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include<QAction>
#include<QMediaMetaData>
#include<QDialog>
#include<QTextBrowser>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    enum ToneType {

        BINAURAL,
        ISOCHRONIC,
        GENERATOR
    };

    // =================== TOOLBARS ===================
    QToolBar *createMediaToolbar();
    QToolBar *createBinauralToolbar();
    QToolBar *createBinauralToolbarExt();
    QToolBar *createNatureToolbar();

    // Media Player Toolbar widgets
    QToolBar *m_mediaToolbar;
    QPushButton *m_loadMusicButton;
    QPushButton *m_playMusicButton;
    QPushButton *m_pauseMusicButton;
    QPushButton *m_stopMusicButton;
    QPushButton *m_shuffleButton;
    QPushButton *m_repeatButton;
    QSlider *m_musicVolumeSlider;
    QLabel *m_musicVolumeLabel;

    // Binaural Toolbar widgets
    QToolBar *m_binauralToolbar;
    QToolBar *m_binauralToolbarExt;
    QPushButton *m_binauralPowerButton;
    QDoubleSpinBox *m_leftFreqInput;
    QDoubleSpinBox *m_rightFreqInput;
    QLabel *m_beatFreqLabel;
    QDoubleSpinBox *m_pulseFreqLabel;
    QLabel *isoPulseLabel;
    QComboBox *m_waveformCombo;
    QDoubleSpinBox *m_binauralVolumeInput;
    QPushButton *m_binauralPlayButton;
    QPushButton *m_binauralStopButton;

    // Nature Sounds Toolbar widgets
    QToolBar *m_natureToolbar;
    QPushButton *m_naturePowerButton;
    QPushButton *m_oceanButton;
    QPushButton *m_forestButton;
    QPushButton *m_rainButton;
    QPushButton *m_fireButton;
    QPushButton *m_streamButton;
    QDoubleSpinBox *m_natureVolumeInput;
    QPushButton *m_natureStopButton;

    // =================== MAIN AREA WIDGETS ===================
    //QListWidget *m_playlistWidget;
    QPushButton *m_addFilesButton;
    QPushButton *m_removeTrackButton;
    QPushButton *m_clearPlaylistButton;

    // =================== AUDIO ENGINES ===================
    //BinauralEngine *m_binauralEngine;
    DynamicEngine *m_binauralEngine;

    // =================== PRIVATE METHODS ===================
    void setupConnections();
    void setupLayout();
    void initializeAudioEngines();

    // UI styling methods
    void styleToolbar(QToolBar *toolbar, const QString &color);
    void updateBinauralBeatDisplay();

    // State management
    void updateBinauralPowerState(bool enabled);
    void updateNaturePowerState(bool enabled);

private slots:
    // Media player slots
    void onLoadMusicClicked();
    void onPlayMusicClicked();
    void onPauseMusicClicked();
    void onStopMusicClicked();
    void onMusicVolumeChanged(int value);

    // Binaural slots
    void onLeftFrequencyChanged(double value);
    void onRightFrequencyChanged(double value);
    void onWaveformChanged(int index);
    void onBinauralVolumeChanged(double value);
    void onBinauralPlayClicked();
    void onBinauralStopClicked();

    // Nature sounds slots
    void onOceanClicked();
    void onForestClicked();
    void onRainClicked();
    void onFireClicked();
    void onStreamClicked();
    void onNatureVolumeChanged(double value);
    void onNatureStopClicked();

    // Playlist slots
    void onAddFilesClicked();
    void onRemoveTrackClicked();
    void onClearPlaylistClicked();
    void onPlaylistItemDoubleClicked(QListWidgetItem *item);

    // Binaural engine signal handlers
    void onBinauralPlaybackStarted();
    void onBinauralPlaybackStopped();
    void onBinauralError(const QString &error);
    // Binaural slots
    void onBinauralPowerToggled(bool checked);

    // Nature sounds slots
    void onNaturePowerToggled(bool checked);

private:
    //warnings
    QSettings settings;
    void showFirstLaunchWarning();
    bool showBinauralWarning();
//timer
private:
    QTimer *m_autoStopTimer;
    int m_remainingSeconds;
    QSpinBox *m_brainwaveDuration;
    QLabel *m_countdownLabel;
    void startAutoStopTimer();
    void stopAutoStopTimer();
    void updateCountdownDisplay();
    void onAutoStopTimerTimeout();
private slots:
    void onBrainwaveDurationChanged(int minutes);
    //mediaplayer
    // Signal Handlers
      void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
      void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
      void onMediaPlayerError(QMediaPlayer::Error error, const QString &errorString);

      // Playlist Navigation
      void playNextTrack();
      void onPlaylistItemClicked(QListWidgetItem *item);
      void onDurationChanged(qint64 durationMs);
      void onPositionChanged(qint64 positionMs);
      void onSeekSliderMoved(int value);
      void playPreviousTrack();
      void onRepeatClicked();
      void onToneTypeComboIndexChanged(int index);
      void onMuteButtonClicked();
private:
    // Existing binaural engine...
    QMediaPlayer *m_mediaPlayer;
    QAudioOutput *m_audioOutput;
    QStringList m_musicFiles; // Store full file paths
    int m_currentTrackIndex = -1;
    bool m_isShuffle = false;
    bool m_isRepeat = false;
    QMap<int, QString> m_playlistMapping; // For shuffle mode
    bool m_waitingToPlay = false;
    QPushButton *m_nextButton;
    QPushButton *m_previousButton;

    QSlider *m_seekSlider;
    QLabel *m_currentTimeLabel;
    QLabel *m_totalTimeLabel;
    qint64 m_pausedPosition = 0;
    QPushButton *volumeIcon;
    QComboBox *toneTypeCombo = nullptr;
    QStandardItemModel *model;
    QStandardItem *squareWaveItem;
    qreal binEngineVolume = 1.0;
    void playRandomTrack();
    bool isShuffle = false;
//tabbedwidget
private:
    // Replace single playlist with tabbed playlists
    QTabWidget *m_playlistTabs;
    QListWidget *m_currentPlaylistWidget; // Keep for compatibility
    QMap<QString, QStringList> m_playlistFiles; // Playlist name -> list of file paths
    QString m_currentPlaylistName;
    // Helper functions
    QListWidget* currentPlaylistWidget() const;
    QString currentPlaylistName() const;
    void addNewPlaylist(const QString &name = "Default");
    void updateCurrentPlaylistReference();
    void updatePlaylistButtonsState();
    QPushButton *addPlaylistBtn;
    QPushButton *renamePlaylistBtn;
private slots:
    void onAddNewPlaylistClicked();
    void onRenamePlaylistClicked();
    void onClosePlaylistTab(int index);
    void onPlaylistTabChanged(int index);

//save-load
private:
    // Data structures
    struct BrainwavePreset {
        QString name;
        int toneType;           // 0=Binaural, 1=Isochronic, 2=Generator
        double leftFrequency;
        double rightFrequency;
        int waveform;           // 0=Sine, 1=Square, 2=Triangle, 3=Sawtooth
        double pulseFrequency;  // For isochronic
        double volume;          // 0-100%

        // JSON serialization
        QJsonObject toJson() const;
        static BrainwavePreset fromJson(const QJsonObject &json);
        bool isValid() const;
    };

    struct PlaylistTrack {
        QString filePath;
        QString title;          // Display name (filename or metadata)
        qint64 duration;        // milliseconds

        QJsonObject toJson() const;
        static PlaylistTrack fromJson(const QJsonObject &json);
    };

    // Helper methods
    QString generateDefaultPresetName() const;
    bool ensureDirectoryExists(const QString &path);

private slots:
    // Preset operations
    void onSavePresetClicked();
    void onLoadPresetClicked();
    void onManagePresetsClicked();

    // Playlist file operations
    void onOpenPlaylistClicked();
    void onSaveCurrentPlaylistClicked();
    void onSaveCurrentPlaylistAsClicked();
    void onSaveAllPlaylistsClicked();

    // Internal save/load methods
    bool savePresetToFile(const QString &filename, const BrainwavePreset &preset);
    BrainwavePreset loadPresetFromFile(const QString &filename);
    QList<BrainwavePreset> loadAllPresets();

    bool savePlaylistToFile(const QString &filename, const QString &playlistName);
    bool loadPlaylistFromFile(const QString &filename);
    void updatePlaylistFromCurrentTab(const QString &filename);
//actions
private:
    // Menu actions for presets
    QAction *savePresetAction;
    QAction *loadPresetAction;
    QAction *managePresetsAction;

    // Menu actions for playlists
    QAction *openPlaylistAction;
    QAction *saveCurrentPlaylistAction;
    QAction *saveCurrentPlaylistAsAction;
    QAction *saveAllPlaylistsAction;
    QAction *quitAction;

    void addActions();
//menubar
    void setupMenus();
    QPushButton *tbarOpenPresetButton;
    QPushButton *tbarSavePresetButton;
    //status update
    void updatePlayerStatus(const QString& filePath);
    QLabel *m_binauralStatusLabel;
    QString formatBinauralString();
    //stream
    void playRemoteStream(const QString &urlString);
    bool m_isStream = false;  // Add this flag
    QString m_currentStreamUrl = "";  // Store current stream URL
private slots:
    void onStreamFromUrl();
    //open with functionality
public slots:
    void onFileOpened(const QString &filePath);

    //track metadata
private:
    QMediaMetaData metaData;
    QString getTrackMetadata();
    QString currentTrackMetadata;
    QPushButton *m_trackInfoButton;
    QDialog *trackInfoDialog = nullptr;
    void createInfoDialog();
    QTextBrowser *metadataBrowser;
public slots:
    void handleMetaDataUpdated();

};
#endif // MAINWINDOW_H
