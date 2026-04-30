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
#include"ambientplayerdialog.h"
#include"ambientplayer.h"
#include"sessiondialog.h"
#include<QTimer>
#include"cuesheetdialog.h"
#include<QtMultimediaWidgets/QVideoWidget>
#include<QPoint>
#include"vistimdialog.h"
#include"flickerwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
private:
    enum ToneType {

        BINAURAL,
        ISOCHRONIC,
        GENERATOR
    };

    QToolBar *createMediaToolbar();
    QToolBar *createBinauralToolbar();
    QToolBar *createBinauralToolbarExt();
    QToolBar *createNatureToolbar();

    QToolBar *m_mediaToolbar;
    QPushButton *m_loadMusicButton;
    QPushButton *m_playMusicButton;
    QPushButton *m_pauseMusicButton;
    QPushButton *m_stopMusicButton;
    QPushButton *m_shuffleButton;
    QPushButton *m_repeatButton;
    QSlider *m_musicVolumeSlider;
    QLabel *m_musicVolumeLabel;

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

    QToolBar *m_natureToolbar;
    QPushButton *m_naturePowerButton;


    QPushButton *m_addFilesButton;
    QPushButton *m_removeTrackButton;
    QPushButton *m_clearPlaylistButton;

    DynamicEngine *m_binauralEngine;

    void setupConnections();
    void setupLayout();
    void initializeAudioEngines();

    void styleToolbar(QToolBar *toolbar, const QString &color);
    void updateBinauralBeatDisplay();

    void updateBinauralPowerState(bool enabled);
    void updateNaturePowerState(bool enabled);

private slots:
    void onLoadMusicClicked();
    void onPlayMusicClicked();
    void onPauseMusicClicked();
    void onStopMusicClicked();
    void onMusicVolumeChanged(int value);

    void onLeftFrequencyChanged(double value);
    void onRightFrequencyChanged(double value);
    void onWaveformChanged(int index);
    void onBinauralVolumeChanged(double value);
    void onBinauralPlayClicked();
    void onBinauralPauseClicked();
    void onBinauralStopClicked();



    void onAddFilesClicked();
    void onRemoveTrackClicked();
    void onClearPlaylistClicked();
    void onPlaylistItemDoubleClicked(QListWidgetItem *item);

    void onBinauralPlaybackStarted();
    void onBinauralPlaybackStopped();
    void onBinauralError(const QString &error);
    void onBinauralPowerToggled(bool checked);

    void onNaturePowerToggled(bool checked);

private:
    QSettings settings;
    void showFirstLaunchWarning();
    bool showBinauralWarning();
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
      void onMediaStatusChanged(QMediaPlayer::MediaStatus status);
      void onPlaybackStateChanged(QMediaPlayer::PlaybackState state);
      void onMediaPlayerError(QMediaPlayer::Error error, const QString &errorString);

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
private:
    QTabWidget *m_playlistTabs;
    QListWidget *m_currentPlaylistWidget; // Keep for compatibility
    QMap<QString, QStringList> m_playlistFiles; // Playlist name -> list of file paths
    QString m_currentPlaylistName;
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

private:
    struct BrainwavePreset {
        QString name;
        int toneType;           // 0=Binaural, 1=Isochronic, 2=Generator
        double leftFrequency;
        double rightFrequency;
        int waveform;           // 0=Sine, 1=Square, 2=Triangle, 3=Sawtooth
        double pulseFrequency;  // For isochronic
        double volume;          // 0-100%

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

    QString generateDefaultPresetName() const;
    bool ensureDirectoryExists(const QString &path);

private slots:
    void onSavePresetClicked();
    void onLoadPresetClicked();
    void onManagePresetsClicked();

    void onOpenPlaylistClicked();
    void onSaveCurrentPlaylistClicked();
    void onSaveCurrentPlaylistAsClicked();
    void onSaveAllPlaylistsClicked();

    bool savePresetToFile(const QString &filename, const BrainwavePreset &preset);
    BrainwavePreset loadPresetFromFile(const QString &filename);
    QList<BrainwavePreset> loadAllPresets();

    bool savePlaylistToFile(const QString &filename, const QString &playlistName);
    bool loadPlaylistFromFile(const QString &filename);
    void updatePlaylistFromCurrentTab(const QString &filename);
private:
    QAction *savePresetAction;
    QAction *loadPresetAction;
    QAction *managePresetsAction;

    QAction *openPlaylistAction;
    QAction *saveCurrentPlaylistAction;
    QAction *saveCurrentPlaylistAsAction;
    QAction *saveAllPlaylistsAction;
    QAction *quitAction;

    void addActions();
    void setupMenus();
    QPushButton *tbarOpenPresetButton;
    QPushButton *tbarSavePresetButton;
    void updatePlayerStatus(const QString& filePath);
    QLabel *m_binauralStatusLabel;
    QString formatBinauralString();
    void playRemoteStream(const QString &urlString);
    bool m_isStream = false;
    QString m_currentStreamUrl = "";  // Store current stream URL
private slots:
    void onStreamFromUrl();
public slots:
    void onFileOpened(const QString &filePath);

private:
    QMediaMetaData metaData;
    QString getTrackMetadata();
    QString currentTrackMetadata;
    QPushButton *m_trackInfoButton;
    QDialog *trackInfoDialog = nullptr;
    void createInfoDialog();
    QTextBrowser *metadataBrowser;
    int m_playingTrackIndex = -1;  // Index of the track that's actually playing
public slots:
    void handleMetaDataUpdated();

private:
    QMap<QString, AmbientPlayer*> m_ambientPlayers;

    QPushButton* m_masterPlayButton;
    QPushButton* m_masterPauseButton;
    QPushButton* m_masterStopButton;
    QSlider* m_masterVolumeSlider;
    QLabel* m_masterVolumeLabel;
    QMap<QString, AmbientPlayerDialog*> m_playerDialogs;  // player1 → Dialog*
private slots:
    void onAmbientButtonClicked();
    void onMasterPlayClicked();
    void onMasterPauseClicked();
    void onMasterStopClicked();
    void onMasterVolumeChanged(int value);
    void setupAmbientPlayers();
    void saveAmbientPlayersSettings();
    void loadAmbientPlayersSettings();
    void saveAmbientPreset(const QString& presetName);
    void loadAmbientPreset(const QString& presetName);
    void resetAllPlayersToDefaults();
    void onSeekSliderReleased();
private:
    QStringList getAvailablePresets() const;
    QPushButton *openAmbientPresetButton;
    QPushButton *saveAmbientPresetButton;
    QPushButton *resetPlayersButton;

    void copyUserFiles();
    QPushButton *tbarResetBinauralSettingsButton;
    void mutePlayingAmbientPlayers(bool needMute);
    QPushButton *m_binauralPauseButton;
    bool m_engineIsPaused = false;
    QAction *unlimitedDurationAction;
    QPushButton *m_openSessionManagerButton;
    SessionDialog *m_sessionManagerDialog = nullptr;
private slots:
    void onSessionStageChanged(int toneType, double leftFreq, double rightFreq,
                                  int waveform, double pulseFreq, double volume);
    void onSessionStarted(int totalSeconds);
    void onSessionEnded();
private:
    QTimer m_fadeTimer;
    double m_fadeStartVolume;
    double m_fadeTargetVolume;
    int m_fadeSteps;
    double targetVolume = 0.0;
    QLabel *durationLabel;
    int currentStageIndex;
    int totalRemainigTime;

    QLineEdit *timeEdit;
    QPushButton *timeEditButton;
    qint64 parseTimeStringToMs(const QString &timeStr);
private slots:
    void onSeekTrack();
private:
    CueSheetDialog *m_cueDialog = nullptr;
    QPushButton *openCueButton;
private slots:
    void onCueTrackSelected(const QString &audioFile, qint64 startMs);
    void onCuePositionChanged(qint64 positionMs);
    void processDroppedFiles(const QStringList& filePaths);
private:

    QPushButton *tbarOpenPlaylistButton;
    QPushButton *tbarSavePlaylistButton;
    QPushButton *tbarSaveAllPlaylistsButton;

    QVideoWidget *videoWidget;
    QPushButton *openVideoButton;
    bool m_isVideoEnabled =false;
    QWidget* m_videoOriginalParent = nullptr;
    int m_videoOriginalTabIndex = -1;
    QListWidget *videoPlaylist;
private slots:
    void onVideoContextMenu(const QPoint &pos);

private:
    void setupVideoPlayer();

    QWidget *m_videoToolbar;
    QWidget *m_videoPlayerContainer;

    QPushButton *m_playButton;
    QPushButton *m_pauseButton;
    QPushButton *m_stopButton;
    QSlider *m_progressSlider;
    QPushButton *m_fullscreenButton;
    QLabel *m_timeLabel;
    void createVideoToolbar();
    void updateVideoTimeDisplay(qint64 position, qint64 duration);
    QPushButton *m_vnextButton;
    QPushButton *m_vpreviousButton;
    QSlider *m_vvolumeSlider;
    QLabel *m_volShowLabel;
   QString m_lastVideoPlaylistName;  // Which video playlist was last playing
   int m_lastVideoTrackIndex = -1;   // Track position in that playlist
   QListWidget *m_lastVideoPlaylist = nullptr;
private slots:
    void showVideoToolbar();
    bool eventFilter(QObject *watched, QEvent *event) override;
    void onPlayClicked();
    void onPauseClicked();
    void onStopClicked();
    void onFullscreenClicked();

    void onVideoPositionChanged(qint64 position);
    void onVideoSliderReleased();
    void onVideoDurationChanged(qint64 duration);
    void toggleFullScreen();
    void showFlickerTab();
    void setupFlickerTab();
private:
    QPushButton* m_loadVideoButton;
    VisStimDialog* m_visStimDialog = nullptr;
    QPushButton *m_visStimButton;
    FlickerWidget* m_flickerWidget = nullptr;
    QWidget *m_flickerContainer = nullptr;
    int m_flickerOriginalTabIndex = -1;
    double isoFreqValue = 7.83;
    bool isDarkTheme = false;
    void toggleTheme(bool enableDark);
    QAction* enableDarkThemeAction;
    QLabel *beatLabel;
    QLabel *volLabel;
    QLabel *volumeLabel;
    QLabel *searchLabel;
    QLineEdit *searchEdit;
    QPushButton *searchButton;
    QLabel *ambientTitleLabel;
    QHBoxLayout *playlistButtonLayout;
public slots:
    void toggleFlickerFullscreen();
};
#endif // MAINWINDOW_H
