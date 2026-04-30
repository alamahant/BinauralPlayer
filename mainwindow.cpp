#include "mainwindow.h"

#include "constants.h"
#include "donationdialog.h"
#include "helpmenudialog.h"
#include <QApplication>
#include <QAudioOutput>
#include <QAudioSink>
#include <QCheckBox>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QStatusBar>
#include <QTableWidget>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoWidget>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_binauralEngine(new DynamicEngine(this))
    ,
      m_mediaPlayer(nullptr), m_audioOutput(nullptr), m_currentTrackIndex(-1),
      m_isShuffle(false), m_isRepeat(false), m_autoStopTimer(nullptr),
      m_remainingSeconds(0), m_seekSlider(new QSlider(Qt::Horizontal, this)),
      m_currentTimeLabel(new QLabel("00:00", this)),
      m_totalTimeLabel(new QLabel("00:00", this)), m_playlistTabs(nullptr),
      m_currentPlaylistWidget(nullptr), m_currentPlaylistName(""),
      m_isStream(false), m_currentStreamUrl(""), m_masterPlayButton(nullptr),
      m_masterPauseButton(nullptr), m_masterStopButton(nullptr),
      m_masterVolumeSlider(nullptr), m_masterVolumeLabel(nullptr),
      m_naturePowerButton(nullptr),
      m_sessionManagerDialog(new SessionDialog(this)),
      m_cueDialog(new CueSheetDialog(this)),
      videoWidget(new QVideoWidget(this))
{
    setWindowTitle("Binaural Media Player");
    setMinimumSize(900, 700);
    setWindowIcon(QIcon(":/favicon/android-chrome-512x512.png"));
    setAcceptDrops(true); // ENABLE DRAG & DROP

    setupAmbientPlayers();

    m_mediaToolbar = createMediaToolbar();
    m_binauralToolbar = createBinauralToolbar();
    m_binauralToolbarExt = createBinauralToolbarExt();
    m_natureToolbar = createNatureToolbar();
    addToolBar(Qt::TopToolBarArea, m_mediaToolbar);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, m_binauralToolbar);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, m_binauralToolbarExt);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, m_natureToolbar);

    setupLayout();


    styleToolbar(m_mediaToolbar, "#4A90E2");       // Blue
    styleToolbar(m_binauralToolbar, "#7B68EE");    // Purple
    styleToolbar(m_binauralToolbarExt, "#7B68EE"); // Purple
    styleToolbar(m_natureToolbar, "#32CD32");      // Green

    updateBinauralPowerState(false);
    updateNaturePowerState(false);

    initializeAudioEngines();
    addActions();

    setupMenus();
    createInfoDialog();
    setupConnections();
    model = qobject_cast<QStandardItemModel *>(m_waveformCombo->model());
    squareWaveItem = model->item(1);

    connect(volumeIcon, &QPushButton::clicked, this,
            &MainWindow::onMuteButtonClicked);

    m_binauralStatusLabel = new QLabel(this);
    m_binauralStatusLabel->setAlignment(Qt::AlignRight);
    m_binauralStatusLabel->setMinimumWidth(150);
    m_binauralStatusLabel->setMaximumWidth(250);

    m_binauralStatusLabel->setText("Binaural engine disabled");

    statusBar()->addPermanentWidget(m_binauralStatusLabel);

    statusBar()->showMessage("Ready to play");
    showFirstLaunchWarning();
    createInfoDialog();
    onNaturePowerToggled(false);
    copyUserFiles();
    bool unlimited = settings.value("binaural/unlimitedDuration", false).toBool();
    m_sessionManagerDialog->setUnlimitedDuration(unlimited);
    m_fadeTimer.setInterval(50);

    setupVideoPlayer();

    isDarkTheme = settings.value("isdarktheme", false).toBool();
    blockSignals(true);
    enableDarkThemeAction->setChecked(isDarkTheme);
    blockSignals(false);
    toggleTheme(isDarkTheme);
}

MainWindow::~MainWindow() {
    if (m_binauralEngine && m_binauralEngine->isPlaying()) {
        m_binauralEngine->stop();
    }
    if (m_mediaPlayer && m_mediaPlayer->isPlaying()) {
        m_mediaPlayer->stop();
    }
    qDeleteAll(m_playerDialogs);
    m_playerDialogs.clear();

    m_ambientPlayers.clear();
}


QToolBar *MainWindow::createMediaToolbar() {
    QToolBar *toolbar = new QToolBar("Media Player", this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    QLabel *titleLabel = new QLabel("🎵 MEDIA PLAYER", toolbar);
    toolbar->addWidget(titleLabel);
    toolbar->addSeparator();

    m_loadMusicButton = new QPushButton(toolbar);
    m_loadMusicButton->setMinimumWidth(60);
    m_loadMusicButton->setIcon(QIcon(":icons/music.svg"));
    m_loadMusicButton->setToolTip("Load music files");
    toolbar->addWidget(m_loadMusicButton);

    tbarOpenPlaylistButton = new QPushButton(toolbar);
    tbarOpenPlaylistButton->setIcon(QIcon(":/icons/folder.svg"));
    tbarOpenPlaylistButton->setToolTip("Load Playlist");
    connect(tbarOpenPlaylistButton, &QPushButton::clicked, this,
            &MainWindow::onOpenPlaylistClicked);
    toolbar->addWidget(tbarOpenPlaylistButton);

    tbarSavePlaylistButton = new QPushButton(toolbar);
    tbarSavePlaylistButton->setToolTip("Save Playlist As");
    tbarSavePlaylistButton->setIcon(QIcon(":/icons/save.svg"));

    connect(tbarSavePlaylistButton, &QPushButton::clicked, this,
            &MainWindow::onSaveCurrentPlaylistAsClicked);
    toolbar->addWidget(tbarSavePlaylistButton);

    tbarSaveAllPlaylistsButton = new QPushButton(toolbar);
    tbarSaveAllPlaylistsButton->setToolTip("Save All Playlists");
    tbarSaveAllPlaylistsButton->setIcon(QIcon(":/icons/copy.svg"));

    connect(tbarSaveAllPlaylistsButton, &QPushButton::clicked, this,
            &MainWindow::onSaveAllPlaylistsClicked);
    toolbar->addWidget(tbarSaveAllPlaylistsButton);

    toolbar->addSeparator();
    m_previousButton = new QPushButton(toolbar);
    m_previousButton->setIcon(QIcon(":icons/skip-back.svg"));
    m_previousButton->setToolTip("Previous Track");

    toolbar->addWidget(m_previousButton);
    m_playMusicButton = new QPushButton(toolbar);
    m_playMusicButton->setMinimumWidth(80);
    m_playMusicButton->setIcon(QIcon(":icons/play.svg"));

    m_playMusicButton->setToolTip("Play selected track");
    toolbar->addWidget(m_playMusicButton);

    m_pauseMusicButton = new QPushButton(toolbar);
    m_pauseMusicButton->setMinimumWidth(50);
    m_pauseMusicButton->setIcon(QIcon(":icons/pause.svg"));

    m_pauseMusicButton->setToolTip("Pause playback");
    toolbar->addWidget(m_pauseMusicButton);

    m_stopMusicButton = new QPushButton(toolbar);
    m_stopMusicButton->setMinimumWidth(50);
    m_stopMusicButton->setIcon(QIcon(":icons/square.svg"));

    m_stopMusicButton->setToolTip("Stop playback");
    toolbar->addWidget(m_stopMusicButton);

    m_nextButton = new QPushButton(toolbar);
    m_nextButton->setIcon(QIcon(":icons/skip-forward.svg"));
    m_nextButton->setToolTip("Next Track");

    toolbar->addWidget(m_nextButton);
    toolbar->addSeparator();

    m_shuffleButton = new QPushButton(toolbar);
    m_shuffleButton->setIcon(QIcon(":icons/shuffle.svg"));

    m_shuffleButton->setCheckable(true);
    m_shuffleButton->setToolTip("Shuffle playlist");
    toolbar->addWidget(m_shuffleButton);

    m_repeatButton = new QPushButton(toolbar);
    m_repeatButton->setIcon(QIcon(":icons/repeat.svg"));

    m_repeatButton->setCheckable(true);
    m_repeatButton->setToolTip("Repeat current track");
    toolbar->addWidget(m_repeatButton);

    toolbar->addSeparator();

    volumeIcon = new QPushButton(toolbar);
    volumeIcon->setCheckable(true);
    volumeIcon->setIcon(QIcon(":/icons/volume-2.svg"));

    m_musicVolumeSlider = new QSlider(Qt::Horizontal, toolbar);
    m_musicVolumeSlider->setRange(0, 100);
    m_musicVolumeSlider->setValue(70);
    m_musicVolumeSlider->setMaximumWidth(150);
    m_musicVolumeSlider->setToolTip("Music volume");

    m_musicVolumeLabel = new QLabel(toolbar);
    m_musicVolumeLabel->setMinimumWidth(40);
    m_musicVolumeLabel->setText(QString("70%"));

    timeEditButton = new QPushButton(toolbar);
    timeEditButton->setToolTip("Seek");
    timeEditButton->setMaximumWidth(25);
    timeEditButton->setIcon(QIcon(":/icons/edit.svg"));
    timeEditButton->setCheckable(true);
    timeEditButton->setChecked(false);
    timeEdit = new QLineEdit(toolbar);
    timeEdit->setMaximumWidth(75);
    timeEdit->setDisabled(true);
    timeEdit->setPlaceholderText("hh:mm:ss");
    timeEdit->setToolTip("Format: hh:mm:ss or mm:ss");
    QRegularExpression timeRegex("^([0-9]{1,2}:)?[0-5]?[0-9]:[0-5][0-9]$");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(timeRegex, timeEdit);
    timeEdit->setValidator(validator);
    toolbar->addWidget(timeEditButton);
    toolbar->addWidget(timeEdit);

    openCueButton = new QPushButton("Cue", toolbar);
    openCueButton->setMaximumWidth(50);
    openCueButton->setToolTip("Open Cue file importer");
    openCueButton->setCheckable(true);
    openCueButton->setChecked(false);
    toolbar->addWidget(openCueButton);

    toolbar->addSeparator();
    openVideoButton = new QPushButton("Video", toolbar);
    openVideoButton->setCheckable(true);
    openVideoButton->setChecked(false);
    toolbar->addWidget(openVideoButton);

    return toolbar;
}

QToolBar *MainWindow::createBinauralToolbar() {
    QToolBar *toolbar = new QToolBar("Binaural Generator", this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    toneTypeCombo = new QComboBox(toolbar);
    toneTypeCombo->addItem("BINAURAL", BINAURAL);
    toneTypeCombo->addItem("ISOCHRONIC", ISOCHRONIC);
    toneTypeCombo->addItem("GENERATOR", GENERATOR);
    toneTypeCombo->setCurrentIndex(0); // Select BINAURAL

    toolbar->addWidget(toneTypeCombo);
    toolbar->addSeparator();

    m_binauralPowerButton = new QPushButton("●", toolbar);
    m_binauralPowerButton->setCheckable(true);
    m_binauralPowerButton->setToolTip("Enable/disable binaural tones");
    m_binauralPowerButton->setMaximumWidth(30);
    toolbar->addWidget(m_binauralPowerButton);

    toolbar->addSeparator();

    QLabel *leftLabel = new QLabel("L:", toolbar);
    toolbar->addWidget(leftLabel);
    m_leftFreqInput = new QDoubleSpinBox(toolbar);
    m_leftFreqInput->setRange(20.0, 20000.0);
    m_leftFreqInput->setValue(360.0);
    m_leftFreqInput->setDecimals(2);
    m_leftFreqInput->setSuffix(" Hz");
    m_leftFreqInput->setMaximumWidth(100);
    m_leftFreqInput->setToolTip("Left channel frequency (20-20000 Hz)");
    m_leftFreqInput->setEnabled(false);
    toolbar->addWidget(m_leftFreqInput);

    QLabel *rightLabel = new QLabel("R:", toolbar);
    toolbar->addWidget(rightLabel);

    m_rightFreqInput = new QDoubleSpinBox(toolbar);
    m_rightFreqInput->setRange(20.0, 20000.0);
    m_rightFreqInput->setValue(367.83);
    m_rightFreqInput->setDecimals(2);
    m_rightFreqInput->setSuffix(" Hz");
    m_rightFreqInput->setMaximumWidth(100);
    m_rightFreqInput->setToolTip("Right channel frequency (20-20000 Hz)");
    m_rightFreqInput->setEnabled(false);
    toolbar->addWidget(m_rightFreqInput);

    beatLabel = new QLabel("BIN:", toolbar);
    toolbar->addWidget(beatLabel);

    m_beatFreqLabel = new QLabel("7.83 Hz", toolbar);

    m_beatFreqLabel->setMinimumWidth(95);
    m_beatFreqLabel->setAlignment(Qt::AlignLeft);
    m_beatFreqLabel->setStyleSheet(
                "background-color: #f0f0f0; padding: 2px; border: 1px solid #ccc;");
    m_beatFreqLabel->setToolTip("Binaural beat frequency (Right - Left)");
    toolbar->addWidget(m_beatFreqLabel);

    isoPulseLabel = new QLabel("ISO:", toolbar);
    toolbar->addWidget(isoPulseLabel);

    m_pulseFreqLabel = new QDoubleSpinBox(toolbar);
    m_pulseFreqLabel->setRange(0.0, 100.0);
    m_pulseFreqLabel->setValue(7.83);
    m_pulseFreqLabel->setDecimals(2);
    m_pulseFreqLabel->setSuffix(" Hz");
    m_pulseFreqLabel->setDisabled(true);
    toolbar->addWidget(m_pulseFreqLabel);
    toolbar->addSeparator();

    m_waveformCombo = new QComboBox(toolbar);
    m_waveformCombo->addItem("Sine");
    m_waveformCombo->addItem("Square");
    m_waveformCombo->addItem("Triangle");
    m_waveformCombo->addItem("Sawtooth");
    m_waveformCombo->setMaximumWidth(90);
    m_waveformCombo->setToolTip("Waveform type");
    m_waveformCombo->setEnabled(false);
    toolbar->addWidget(m_waveformCombo);


    volLabel = new QLabel("Vol:", toolbar);
    toolbar->addWidget(volLabel);

    m_binauralVolumeInput = new QDoubleSpinBox(toolbar);

    m_binauralVolumeInput->setRange(0.0, 100.0);
    m_binauralVolumeInput->setValue(15.0);
    m_binauralVolumeInput->setDecimals(1);
    m_binauralVolumeInput->setSuffix("%");
    m_binauralVolumeInput->setMaximumWidth(70);
    m_binauralVolumeInput->setToolTip("Binaural volume (0-100%)");
    m_binauralVolumeInput->setEnabled(false);
    toolbar->addWidget(m_binauralVolumeInput);

    toolbar->addSeparator();

    return toolbar;
}

QToolBar *MainWindow::createBinauralToolbarExt() {
    QToolBar *toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    tbarOpenPresetButton = new QPushButton(toolbar);
    tbarOpenPresetButton->setToolTip("Load Preset");
    tbarOpenPresetButton->setIcon(QIcon(":/icons/folder.svg"));

    connect(tbarOpenPresetButton, &QPushButton::clicked, this,
            &MainWindow::onLoadPresetClicked);
    toolbar->addWidget(tbarOpenPresetButton);

    tbarSavePresetButton = new QPushButton(toolbar);
    tbarSavePresetButton->setToolTip("Save Preset");
    tbarSavePresetButton->setIcon(QIcon(":/icons/save.svg"));

    connect(tbarSavePresetButton, &QPushButton::clicked, this,
            &MainWindow::onSavePresetClicked);
    toolbar->addWidget(tbarSavePresetButton);

    tbarResetBinauralSettingsButton = new QPushButton(toolbar);
    tbarResetBinauralSettingsButton->setIcon(QIcon(":/icons/refresh-cw.svg"));
    tbarResetBinauralSettingsButton->setToolTip(
                "Reset Brainwave settings to default");
    toolbar->addWidget(tbarResetBinauralSettingsButton);
    connect(tbarResetBinauralSettingsButton, &QPushButton::clicked, this, [this] {
        if (m_binauralEngine && m_binauralEngine->isPlaying()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(
                        this, tr("Binaural Engine Active"),
                        tr("Binaural engine is playing.\nStop and proceed?"),
                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

            if (reply == QMessageBox::Cancel) {
                return; // User canceled, don't proceed
            }

            m_binauralEngine->stop();
        }
        toneTypeCombo->setCurrentIndex(0); // Binaural
        m_leftFreqInput->setValue(360.0);
        m_rightFreqInput->setValue(367.83);
        m_waveformCombo->setCurrentIndex(0); // Sine
        m_pulseFreqLabel->setValue(7.83);
        m_binauralVolumeInput->setValue(15.0);
        updateBinauralBeatDisplay();
        statusBar()->showMessage("Brainwave settings reset to defaults", 3000);
    });
    toolbar->addSeparator();

    m_binauralPlayButton = new QPushButton(toolbar);
    m_binauralPlayButton->setMinimumWidth(80);
    m_binauralPlayButton->setIcon(QIcon(":/icons/play.svg"));
    m_binauralPlayButton->setToolTip("Start binaural tones");
    m_binauralPlayButton->setEnabled(false);
    toolbar->addWidget(m_binauralPlayButton);

    m_binauralPauseButton = new QPushButton(toolbar);
    m_binauralPauseButton->setMinimumWidth(50);
    m_binauralPauseButton->setIcon(QIcon(":/icons/pause.svg"));
    m_binauralPauseButton->setToolTip("Pause binaural tones");
    m_binauralPauseButton->setEnabled(false);
    toolbar->addWidget(m_binauralPauseButton);

    m_binauralStopButton = new QPushButton(toolbar);
    m_binauralStopButton->setIcon(QIcon(":/icons/square.svg"));

    m_binauralStopButton->setMinimumWidth(50);

    m_binauralStopButton->setToolTip("Stop binaural tones");
    m_binauralStopButton->setEnabled(false);
    toolbar->addWidget(m_binauralStopButton);

    durationLabel = new QLabel("Timer:", toolbar);
    durationLabel->setToolTip("Auto-stop after selected duration");
    toolbar->addWidget(durationLabel);

    m_brainwaveDuration = new QSpinBox(toolbar);
    bool unlimited = settings.value("binaural/unlimitedDuration", false).toBool();
    if (unlimited) {
    m_brainwaveDuration->setRange(1, 360);
    } else {
        m_brainwaveDuration->setRange(1, 45);
    }
    m_brainwaveDuration->setValue(45);
    m_brainwaveDuration->setSuffix(" min");
    m_brainwaveDuration->setMaximumWidth(75);
    m_brainwaveDuration->setToolTip(
                "Auto-stop brainwave audio after X minutes (1-45)");
    m_brainwaveDuration->setEnabled(false); // Enabled when binaural power is on
    toolbar->addWidget(m_brainwaveDuration);

    m_countdownLabel = new QLabel("--:--", toolbar);
    m_countdownLabel->setMinimumWidth(50);
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setStyleSheet(
                "background-color: #f0f0f0; padding: 3px; border: 1px solid #ccc; "
                "border-radius: 3px; color: #7B68EE;");
    m_countdownLabel->setToolTip("Time remaining until auto-stop");
    m_countdownLabel->setVisible(true); // Only show when timer is active
    toolbar->addWidget(m_countdownLabel);

    toolbar->addSeparator();
    m_openSessionManagerButton = new QPushButton(" Sessions", toolbar);
    m_openSessionManagerButton->setCheckable(true);
    m_openSessionManagerButton->setChecked(false);
    m_openSessionManagerButton->setEnabled(false);
    m_openSessionManagerButton->setIcon(QIcon(":/icons/layers.svg"));
    m_openSessionManagerButton->setToolTip("Open Session Manager");

    toolbar->addWidget(m_openSessionManagerButton);

    toolbar->addSeparator();
    m_visStimButton = new QPushButton("Visual", toolbar);
    m_visStimButton->setCheckable(true);
    m_visStimButton->setToolTip("Visual Stimulation");



    connect(m_visStimButton, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) {
            showFlickerTab();
            if (m_visStimDialog) {
                m_visStimDialog->show();
                m_visStimDialog->raise();
            }
        } else {
            if (m_flickerWidget) {
                m_flickerWidget->stopFlicker();
            }
            if (m_flickerOriginalTabIndex != -1) {
                m_playlistTabs->tabBar()->setTabVisible(m_flickerOriginalTabIndex, false);
            }
            if (m_visStimDialog) {
                m_visStimDialog->hide();
            }

        }
    });

    toolbar->addWidget(m_visStimButton);

    return toolbar;
}

QToolBar *MainWindow::createNatureToolbar() {
    QToolBar *toolbar = new QToolBar("Nature Sounds", this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));
    ambientTitleLabel = new QLabel("🌳 AMBIENCE", toolbar);
    ambientTitleLabel->setStyleSheet("QLabel { font-weight: bold; color: #2E8B57; }");
    toolbar->addWidget(ambientTitleLabel);
    toolbar->addSeparator();

    m_naturePowerButton = new QPushButton("○", toolbar); // Start OFF (○)
    m_naturePowerButton->setCheckable(true);
    m_naturePowerButton->setChecked(false); // Start unchecked
    m_naturePowerButton->setToolTip("Enable/disable ALL nature sounds");
    m_naturePowerButton->setMaximumWidth(30);
    m_naturePowerButton->setStyleSheet("QPushButton { color: #888888; }");
    toolbar->addWidget(m_naturePowerButton);
    toolbar->addSeparator();

    openAmbientPresetButton = new QPushButton(toolbar);
    openAmbientPresetButton->setToolTip("Load Preset");
    openAmbientPresetButton->setIcon(QIcon(":/icons/folder.svg"));

    connect(openAmbientPresetButton, &QPushButton::clicked, this,
            [this] { loadAmbientPreset(""); });
    toolbar->addWidget(openAmbientPresetButton);

    saveAmbientPresetButton = new QPushButton(toolbar);
    saveAmbientPresetButton->setToolTip("Save Preset");
    saveAmbientPresetButton->setIcon(QIcon(":/icons/save.svg"));

    connect(saveAmbientPresetButton, &QPushButton::clicked, this,
            [this] { saveAmbientPreset(""); });
    toolbar->addWidget(saveAmbientPresetButton);

    resetPlayersButton = new QPushButton(toolbar);
    resetPlayersButton->setToolTip("Reset all players to defaults");
    resetPlayersButton->setIcon(QIcon(":/icons/refresh-cw.svg"));
    resetPlayersButton->setEnabled(false);
    connect(resetPlayersButton, &QPushButton::clicked, this, [this] {
        QMessageBox::StandardButton reply = QMessageBox::question(
                    this, "Reset All Players",
                    "Reset all ambient players to default settings?\n\n"
                    "This will clear all audio files and reset volumes.",
                    QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            resetAllPlayersToDefaults();
        }
    });
    toolbar->addWidget(resetPlayersButton);

    toolbar->addSeparator();

    QLabel *masterLabel = new QLabel(toolbar);

    m_masterPlayButton = new QPushButton("▶", toolbar);
    m_masterPlayButton->setToolTip("Play all ON nature sounds");
    m_masterPlayButton->setMaximumWidth(30);
    m_masterPlayButton->setStyleSheet(
                "QPushButton { font-weight: bold; color: #2E8B57; }");
    m_masterPlayButton->setEnabled(false);
    toolbar->addWidget(m_masterPlayButton);

    m_masterPauseButton = new QPushButton("||", toolbar);
    m_masterPauseButton->setToolTip("Pause all ON nature sounds");
    m_masterPauseButton->setMaximumWidth(30);
    m_masterPauseButton->setStyleSheet(
                "QPushButton { font-weight: bold; color: #FF8C00; }");
    m_masterPauseButton->setEnabled(false);

    toolbar->addWidget(m_masterPauseButton);

    m_masterStopButton = new QPushButton("■", toolbar);
    m_masterStopButton->setToolTip("Stop all ON nature sounds");
    m_masterStopButton->setMaximumWidth(30);
    m_masterStopButton->setStyleSheet(
                "QPushButton { font-weight: bold; color: #DC143C; }");
    m_masterStopButton->setEnabled(false);

    toolbar->addWidget(m_masterStopButton);

    toolbar->addSeparator();

    QLabel *playersLabel = new QLabel(toolbar);

    if (!m_ambientPlayers.isEmpty()) {
        QStringList playerOrder = {"player1", "player2", "player3", "player4",
                                   "player5"};
        for (const QString &key : playerOrder) {
            if (m_ambientPlayers.contains(key)) {
                toolbar->addWidget(m_ambientPlayers[key]->button());
            }
        }
    } else {
        for (int i = 1; i <= 5; i++) {
            QPushButton *placeholder =
                    new QPushButton(QString("P%1").arg(i), toolbar);
            placeholder->setEnabled(false);
            placeholder->setStyleSheet("QPushButton { color: gray; }");
            toolbar->addWidget(placeholder);
        }
    }

    toolbar->addSeparator();

    volumeLabel = new QLabel("VOL:", toolbar);
    volumeLabel->setVisible(false);
    toolbar->addWidget(volumeLabel);

    m_masterVolumeSlider = new QSlider(Qt::Horizontal, toolbar);
    m_masterVolumeSlider->setVisible(false);

    m_masterVolumeSlider->setRange(0, 100);
    m_masterVolumeSlider->setValue(100);
    m_masterVolumeSlider->setMaximumWidth(100);
    m_masterVolumeSlider->setToolTip(
                "Master volume for all ON nature sounds (0-100%)");
    toolbar->addWidget(m_masterVolumeSlider);

    m_masterVolumeLabel = new QLabel("100%", toolbar);
    m_masterVolumeLabel->setMinimumWidth(40);
    m_masterVolumeLabel->setAlignment(Qt::AlignRight);
    m_masterVolumeLabel->setVisible(false);
    toolbar->addWidget(m_masterVolumeLabel);

    return toolbar;
}


void MainWindow::setupLayout() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    QHBoxLayout *playlistHeaderLayout = new QHBoxLayout();

    QLabel *playlistTitle = new QLabel("□ PLAYLISTS");
    playlistTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
    playlistHeaderLayout->addWidget(playlistTitle);

    addPlaylistBtn = new QPushButton("New");
    renamePlaylistBtn = new QPushButton("Rename");
    renamePlaylistBtn->setToolTip("Rename Current Playlist");
    addPlaylistBtn->setMaximumWidth(80);
    addPlaylistBtn->setToolTip("Add New Playlist");
    renamePlaylistBtn->setMaximumWidth(80);
    searchLabel = new QLabel("Search: ", this);
    searchEdit = new QLineEdit(this);
    searchEdit->setMaximumWidth(120);
    searchEdit->setEnabled(false);

    searchButton = new QPushButton(this);
    searchButton->setCheckable(true);
    searchButton->setChecked(false);
    connect(searchButton, &QPushButton::clicked,
            [this](bool checked) {
        if (!checked) {
            searchEdit->clear();
        }
        searchEdit->setEnabled(checked);
    });
    searchButton->setToolTip("Search Playlist");
    searchEdit->setToolTip("Search Playlist");

    searchButton->setIcon(QIcon(":/icons/edit.svg"));

    connect(searchEdit, &QLineEdit::textChanged, [this]() {
        QListWidget *playlist = currentPlaylistWidget();
        if (!playlist || !searchEdit->isEnabled())
            return;

        QString searchText = searchEdit->text().trimmed();

        if (searchText.isEmpty()) {
            for (int i = 0; i < playlist->count(); ++i) {
                playlist->item(i)->setHidden(false);
            }
            return;
        }

        for (int i = 0; i < playlist->count(); ++i) {
            QListWidgetItem *item = playlist->item(i);
            QString itemText = item->text();
            bool matches = itemText.contains(searchText, Qt::CaseInsensitive);
            item->setHidden(!matches);
        }
    });

    playlistHeaderLayout->addStretch();
    playlistHeaderLayout->addWidget(addPlaylistBtn);
    playlistHeaderLayout->addWidget(renamePlaylistBtn);
    playlistHeaderLayout->addWidget(searchLabel);
    playlistHeaderLayout->addWidget(searchEdit);
    playlistHeaderLayout->addWidget(searchButton);
    mainLayout->addLayout(playlistHeaderLayout);

    m_playlistTabs = new QTabWidget();
    m_playlistTabs->setDocumentMode(true);
    m_playlistTabs->setTabsClosable(true);
    m_playlistTabs->setMovable(true);

    mainLayout->addWidget(m_playlistTabs, 1); // Stretch factor 1

    addNewPlaylist("Default");

    playlistButtonLayout = new QHBoxLayout();
    playlistButtonLayout->setSpacing(5);

    m_addFilesButton = new QPushButton("Load Music", this);
    m_addFilesButton->setToolTip("Load Music Files");
    m_removeTrackButton = new QPushButton("Remove", this);

    m_removeTrackButton->setToolTip("Remove Selected Track");

    m_clearPlaylistButton = new QPushButton("Clear", this);
    m_clearPlaylistButton->setToolTip("Clear Currently Opened Playlist");

    m_trackInfoButton = new QPushButton(this);
    m_trackInfoButton->setCheckable(true);
    m_trackInfoButton->setChecked(false);
    m_trackInfoButton->setIcon(QIcon(":/icons/info.svg"));
    m_trackInfoButton->setToolTip("Display metadata on the current track");
    playlistButtonLayout->addWidget(m_addFilesButton);
    playlistButtonLayout->addWidget(m_removeTrackButton);
    playlistButtonLayout->addWidget(m_clearPlaylistButton);

    playlistButtonLayout->addSpacing(8);
    playlistButtonLayout->addWidget(volumeIcon);
    playlistButtonLayout->addWidget(m_musicVolumeSlider);
    playlistButtonLayout->addWidget(m_musicVolumeLabel);
    playlistButtonLayout->addWidget(m_seekSlider);
    playlistButtonLayout->addWidget(m_currentTimeLabel);
    playlistButtonLayout->addWidget(m_totalTimeLabel);
    playlistButtonLayout->addWidget(m_trackInfoButton);
    mainLayout->addLayout(playlistButtonLayout);
}

void MainWindow::setupConnections() {

    connect(m_loadMusicButton, &QPushButton::clicked, this,
            &MainWindow::onLoadMusicClicked);
    connect(m_playMusicButton, &QPushButton::clicked, this,
            &MainWindow::onPlayMusicClicked);
    connect(m_pauseMusicButton, &QPushButton::clicked, this,
            &MainWindow::onPauseMusicClicked);
    connect(m_stopMusicButton, &QPushButton::clicked, this,
            &MainWindow::onStopMusicClicked);
    connect(m_nextButton, &QPushButton::clicked, this,
            &MainWindow::playNextTrack);
    connect(m_previousButton, &QPushButton::clicked, this,
            &MainWindow::playPreviousTrack);
    connect(m_repeatButton, &QPushButton::clicked, this,
            &MainWindow::onRepeatClicked);

    connect(m_shuffleButton, &QPushButton::clicked, [this](bool checked) {
        if (m_isVideoEnabled) {
            m_shuffleButton->setChecked(false);
            return;
        }
        if (checked) {
            m_shuffleButton->setToolTip("Shuffle: ON");
        } else {
            m_shuffleButton->setToolTip("Shuffle: OFF");
        }
        isShuffle = checked;
    });

    connect(m_musicVolumeSlider, &QSlider::valueChanged, this,
            &MainWindow::onMusicVolumeChanged);
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this,
            &MainWindow::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this,
            &MainWindow::onPlaybackStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this,
            &MainWindow::onMediaPlayerError);

    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this,
            &MainWindow::onDurationChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this,
            &MainWindow::onPositionChanged);
    connect(m_seekSlider, &QSlider::sliderReleased, this,
            &MainWindow::onSeekSliderReleased);

    connect(openCueButton, &QPushButton::clicked, this, [this](bool checked) {
        if (checked) {
            m_cueDialog->show();
        } else {
            m_cueDialog->hide();
        }
    });
    connect(m_cueDialog, &CueSheetDialog::hideRequested, this,
            [this] { openCueButton->setChecked(false); });

    connect(m_cueDialog, &CueSheetDialog::trackSelected, this,
            &MainWindow::onCueTrackSelected);

    connect(m_cueDialog, &CueSheetDialog::trackPositionChanged, this,
            &MainWindow::onCuePositionChanged);


    connect(m_mediaPlayer, &QMediaPlayer::metaDataChanged, this,
            &MainWindow::handleMetaDataUpdated);

    connect(timeEditButton, &QPushButton::clicked, this, [this](bool checked) {
        timeEdit->setEnabled(checked);
        if (checked)
            timeEdit->clear();
    });
    connect(timeEdit, &QLineEdit::returnPressed, this, &MainWindow::onSeekTrack);

    connect(openVideoButton, &QPushButton::clicked, this, [this](bool checked) {
        if (checked) {
            if(m_visStimButton->isChecked() && m_visStimDialog && m_flickerContainer) m_visStimButton->setChecked(false);
            const QString VIDEO_TAB_NAME = "Video Player";
            const QString VPLAYLIST_TAB_NAME = "Video Playlist";

            int videoTabIndex = -1;
            int vplaylistTabIndex = -1;

            for (int i = 0; i < m_playlistTabs->count(); ++i) {
                QString tabName = m_playlistTabs->tabText(i);
                if (tabName == VIDEO_TAB_NAME) {
                    videoTabIndex = i;
                } else if (tabName == VPLAYLIST_TAB_NAME) {
                    vplaylistTabIndex = i;
                }
            }

            if (videoTabIndex == -1) {
                if (!videoWidget) {
                    videoWidget = new QVideoWidget();
                }
                videoTabIndex = m_playlistTabs->addTab(videoWidget, VIDEO_TAB_NAME);
            }

            if (vplaylistTabIndex == -1) {
                QWidget *vplaylistWidget = new QWidget();
                vplaylistTabIndex =
                        m_playlistTabs->addTab(vplaylistWidget, VPLAYLIST_TAB_NAME);
            }

            m_playlistTabs->tabBar()->setTabVisible(videoTabIndex, true);
            m_playlistTabs->tabBar()->setTabVisible(vplaylistTabIndex, true);

            m_playlistTabs->setCurrentIndex(videoTabIndex);

            m_binauralToolbar->setVisible(false);
            m_binauralToolbarExt->setVisible(false);
            m_natureToolbar->setVisible(false);

            statusBar()->showMessage("Mouse Left -> show/hide playbar, Mouse Right -> context menu");

            m_isVideoEnabled = true;
        } else {
            for (int i = 0; i < m_playlistTabs->count(); ++i) {
                QString tabName = m_playlistTabs->tabText(i);
                if (tabName == "Video Player" || tabName == "Video Playlist") {
                    m_playlistTabs->tabBar()->setTabVisible(i, false);
                }
            }

            for (int i = 0; i < m_playlistTabs->count(); ++i) {
                QString tabName = m_playlistTabs->tabText(i);
                if (tabName != "Video Player" && tabName != "Video Playlist") {
                    m_playlistTabs->setCurrentIndex(i);
                    break;
                }
            }

            bool isBinauralToolbarHidden =
                    settings.value("UI/BinauralToolbarHidden", false).toBool();
            bool isNatureToolbarHidden =
                    settings.value("UI/NatureToolbarHidden", false).toBool();

            m_binauralToolbar->setVisible(!isBinauralToolbarHidden);
            m_binauralToolbarExt->setVisible(!isBinauralToolbarHidden);
            m_natureToolbar->setVisible(!isNatureToolbarHidden);

            m_isVideoEnabled = false;

        }
    });

    connect(m_binauralPowerButton, &QPushButton::toggled, this,
            &MainWindow::onBinauralPowerToggled);
    connect(m_leftFreqInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onLeftFrequencyChanged);
    connect(m_rightFreqInput,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &MainWindow::onRightFrequencyChanged);
    connect(m_waveformCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onWaveformChanged);
    connect(m_binauralVolumeInput,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &MainWindow::onBinauralVolumeChanged);
    connect(m_binauralPlayButton, &QPushButton::clicked, this,
            &MainWindow::onBinauralPlayClicked);
    connect(m_binauralPauseButton, &QPushButton::clicked, this,
            &MainWindow::onBinauralPauseClicked);

    connect(m_binauralStopButton, &QPushButton::clicked, this,
            &MainWindow::onBinauralStopClicked);

    connect(toneTypeCombo, &QComboBox::currentIndexChanged, this,
            &MainWindow::onToneTypeComboIndexChanged);

    connect(m_pulseFreqLabel, &QDoubleSpinBox::valueChanged, [this](double hz) {
        m_binauralEngine->setPulseFrequency(hz);
        m_binauralStatusLabel->setText(formatBinauralString());
        ConstantGlobals::currentIsonFreq = hz;

        if(m_flickerWidget && ConstantGlobals::currentToneType == ToneType::ISOCHRONIC){
           if (m_visStimDialog) m_visStimDialog->syncFrequency(hz);
            m_flickerWidget->setFrequency(hz);
        }
    });
    connect(m_openSessionManagerButton, &QPushButton::clicked, this,
            [this](bool checked) {
        if (checked) {
            if(isDarkTheme) toggleTheme(false);
            m_sessionManagerDialog->show();
        } else {
            toggleTheme(isDarkTheme);
            m_sessionManagerDialog->hide();
        }
    });
    connect(m_sessionManagerDialog, &SessionDialog::dialogHidden, this,
            [this] {
        toggleTheme(isDarkTheme);
        m_openSessionManagerButton->setChecked(false);
    });

    connect(m_sessionManagerDialog, &SessionDialog::stageChanged, this,
            &MainWindow::onSessionStageChanged);

    connect(m_sessionManagerDialog, &SessionDialog::sessionStarted, this,
            &MainWindow::onSessionStarted);

    connect(m_sessionManagerDialog, &SessionDialog::sessionEnded, this,
            &MainWindow::onSessionEnded);

    connect(m_masterPlayButton, &QPushButton::clicked, this,
            &MainWindow::onMasterPlayClicked);
    connect(m_masterPauseButton, &QPushButton::clicked, this,
            &MainWindow::onMasterPauseClicked);
    connect(m_masterStopButton, &QPushButton::clicked, this,
            &MainWindow::onMasterStopClicked);
    connect(m_masterVolumeSlider, &QSlider::valueChanged, this,
            &MainWindow::onMasterVolumeChanged);
    connect(m_naturePowerButton, &QPushButton::toggled, this,
            &MainWindow::onNaturePowerToggled);

    connect(m_addFilesButton, &QPushButton::clicked, this,
            &MainWindow::onAddFilesClicked);
    connect(m_removeTrackButton, &QPushButton::clicked, this,
            &MainWindow::onRemoveTrackClicked);
    connect(m_clearPlaylistButton, &QPushButton::clicked, this,
            &MainWindow::onClearPlaylistClicked);

    connect(m_trackInfoButton, &QPushButton::clicked, [this](bool checked) {
        if (checked) {
            trackInfoDialog->show();
        } else {
            trackInfoDialog->hide();
        }
    });

    connect(addPlaylistBtn, &QPushButton::clicked, this,
            &MainWindow::onAddNewPlaylistClicked);
    connect(renamePlaylistBtn, &QPushButton::clicked, this,
            &MainWindow::onRenamePlaylistClicked);
    connect(m_playlistTabs, &QTabWidget::tabCloseRequested, this,
            &MainWindow::onClosePlaylistTab);
    connect(m_playlistTabs, &QTabWidget::currentChanged, this,
            &MainWindow::onPlaylistTabChanged);

    connect(m_binauralEngine, &DynamicEngine::playbackStarted, this,
            &MainWindow::onBinauralPlaybackStarted);
    connect(m_binauralEngine, &DynamicEngine::playbackStopped, this,
            &MainWindow::onBinauralPlaybackStopped);
    connect(m_binauralEngine, &DynamicEngine::errorOccurred, this,
            &MainWindow::onBinauralError);

    connect(savePresetAction, &QAction::triggered, this,
            &MainWindow::onSavePresetClicked);
    connect(loadPresetAction, &QAction::triggered, this,
            &MainWindow::onLoadPresetClicked);
    connect(managePresetsAction, &QAction::triggered, this,
            &MainWindow::onManagePresetsClicked);

    connect(openPlaylistAction, &QAction::triggered, this,
            &MainWindow::onOpenPlaylistClicked);
    connect(saveCurrentPlaylistAction, &QAction::triggered, this,
            &MainWindow::onSaveCurrentPlaylistClicked);
    connect(saveCurrentPlaylistAsAction, &QAction::triggered, this,
            &MainWindow::onSaveCurrentPlaylistAsClicked);
    connect(saveAllPlaylistsAction, &QAction::triggered, this,
            &MainWindow::onSaveAllPlaylistsClicked);
    connect(quitAction, &QAction::triggered, [this] { qApp->quit(); });

    connect(unlimitedDurationAction, &QAction::triggered, [this](bool checked) {
        if (checked) {
            QMessageBox::warning(this, "Prolonged Exposure Warning",
                                 "Prolonged exposure to binaural audio may cause "
                                 "fatigue or discomfort.\nUse responsibly.");
            m_brainwaveDuration->setRange(1, 360);
            settings.setValue("binaural/unlimitedDuration", true);
        } else {
            m_brainwaveDuration->setRange(1, 45);
            settings.setValue("binaural/unlimitedDuration", false);
        }
        m_sessionManagerDialog->setUnlimitedDuration(checked);
    });

    connect(&m_fadeTimer, &QTimer::timeout, this, [this]() {
        if (m_sessionManagerDialog) {
            m_sessionManagerDialog->pauseButton()->setDisabled(true);
            m_sessionManagerDialog->stopButton()->setDisabled(true);
        }

        m_fadeSteps++;

        double progress = m_fadeSteps / 100.0;
        double vol =
                m_fadeStartVolume + (m_fadeTargetVolume - m_fadeStartVolume) * progress;
        m_binauralVolumeInput->setValue(vol);

        if (m_fadeSteps >= 100) {
            m_fadeTimer.stop();
            m_binauralVolumeInput->setValue(m_fadeTargetVolume);
            if (m_sessionManagerDialog) {
                m_sessionManagerDialog->pauseButton()->setEnabled(true);
                m_sessionManagerDialog->stopButton()->setEnabled(true);
            }
        }
    });

    connect(m_sessionManagerDialog, &SessionDialog::fadeRequested, this,
            [this](double targetVolume) {
        m_fadeStartVolume = m_binauralEngine->getVolume() * 100;

        if (m_fadeStartVolume == targetVolume)
            return;

        m_fadeTargetVolume = targetVolume;
        m_fadeSteps = 0;
        m_fadeTimer.start();
    });

    connect(m_sessionManagerDialog, &SessionDialog::pauseRequested, this, [this] {
        m_binauralPauseButton->setEnabled(true);
        m_binauralPauseButton->click();
        m_binauralPauseButton->setDisabled(true);
    });
    connect(m_sessionManagerDialog, &SessionDialog::resumeRequested, this,
            [this] {
        m_binauralPlayButton->setEnabled(true);
        m_binauralPlayButton->click();
        m_binauralPlayButton->setDisabled(true);
    });
    connect(m_sessionManagerDialog, &SessionDialog::syncTimersRequested, this,
            [this](int currentIndex, int remainingTime) {
        /*
            int minutes = remainingTime / 60;
            int seconds = remainingTime % 60;
            m_countdownLabel->setText(QString("%1:%2")
                .arg(minutes, 2, 10, QLatin1Char('0'))
                .arg(seconds, 2, 10, QLatin1Char('0')));
            */
        currentStageIndex = currentIndex;
        totalRemainigTime = remainingTime;
    });
}

void MainWindow::initializeAudioEngines() {
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);

    m_mediaPlayer->setAudioOutput(m_audioOutput);
    m_mediaPlayer->setVideoOutput(videoWidget);

    m_audioOutput->setVolume(m_musicVolumeSlider->value() / 100.0f);
}


void MainWindow::styleToolbar(QToolBar *toolbar, const QString &color) {
    QString style = QString("QToolBar {"
                            "  background-color: %1;"
                            "  border: 1px solid %2;"
                            "  border-radius: 4px;"
                            "  padding: 4px;"
                            "  spacing: 8px;"
                            "}"
                            "QToolBar::separator {"
                            "  background-color: %2;"
                            "  width: 1px;"
                            "  margin: 4px 2px;"
                            "}")
            .arg(color)
            .arg(QColor(color).darker(120).name());

    toolbar->setStyleSheet(style);
}

void MainWindow::updateBinauralBeatDisplay() {
    double leftFreq = m_leftFreqInput->value();
    double rightFreq = m_rightFreqInput->value();
    double beatFreq = qAbs(rightFreq - leftFreq);

    m_beatFreqLabel->setText(QString("%1 Hz").arg(beatFreq, 0, 'f', 2));
}

void MainWindow::updateBinauralPowerState(bool enabled) {
    toneTypeCombo->setEnabled(enabled);
    m_leftFreqInput->setEnabled(enabled);
    m_rightFreqInput->setEnabled(enabled);
    m_waveformCombo->setEnabled(enabled);
    m_binauralVolumeInput->setEnabled(enabled);
    m_binauralPlayButton->setEnabled(enabled);
    m_binauralPauseButton->setEnabled(enabled);
    m_binauralStopButton->setEnabled(enabled);
    tbarOpenPresetButton->setEnabled(enabled);
    tbarSavePresetButton->setEnabled(enabled);
    tbarOpenPresetButton->setEnabled(enabled);
    tbarSavePresetButton->setEnabled(enabled);
    m_openSessionManagerButton->setEnabled(enabled);
    tbarResetBinauralSettingsButton->setEnabled(enabled);
    m_binauralPowerButton->setText(enabled ? "●" : "○");
    m_binauralPowerButton->setStyleSheet(
                enabled ? "color: #00FF00; font-weight: bold;" : "color: #888888;");
    m_brainwaveDuration->setEnabled(enabled);

    if (!enabled && m_autoStopTimer) {
        stopAutoStopTimer();
    }
}

void MainWindow::updateNaturePowerState(bool enabled) {

    if (m_naturePowerButton) {
        m_naturePowerButton->setText(enabled ? "●" : "○");
        m_naturePowerButton->setStyleSheet(
                    enabled ? "QPushButton { color: #32CD32; font-weight: bold; }"
                            : "QPushButton { color: #888888; }");
    }

    for (AmbientPlayer *player : std::as_const(m_ambientPlayers)) {
        if (player) {
            player->setEnabled(enabled);
        }
    }

}


void MainWindow::onPauseMusicClicked() {
    if (m_mediaPlayer) {
        if (m_isStream && !m_currentStreamUrl.isEmpty()) {
            m_mediaPlayer->pause(); // Pause the stream

            QUrl url(m_currentStreamUrl);
            QString displayName = url.fileName();
            if (displayName.isEmpty()) {
                displayName = url.host();
                if (displayName.isEmpty())
                    displayName = "Stream";
            }
            statusBar()->showMessage("Paused: " + displayName);

            return; // Exit the method - don't check playlist
        }
    }

    if (m_isVideoEnabled)
        m_currentPlaylistName = "Video Playlist";

    if (!m_currentPlaylistName.isEmpty() && m_currentTrackIndex >= 0 &&
            m_currentTrackIndex < m_playlistFiles[m_currentPlaylistName].size()) {

        QString filePath =
                m_playlistFiles[m_currentPlaylistName].at(m_currentTrackIndex);

        m_mediaPlayer->pause();
        m_pauseMusicButton->setToolTip("Paused");
        m_playMusicButton->setToolTip("Resume Track");
        updatePlayerStatus(filePath);

    } else {
        m_mediaPlayer->pause();
        statusBar()->showMessage("Paused (no track loaded)");
    }

}

void MainWindow::onStopMusicClicked() {
    if (m_isStream) {
    m_isStream = false;
    }


    if (m_isVideoEnabled)
        m_currentPlaylistName = m_lastVideoPlaylistName;

    if (!m_currentPlaylistName.isEmpty() && m_currentTrackIndex >= 0 &&
            m_currentTrackIndex < m_playlistFiles[m_currentPlaylistName].size()) {

        QString filePath =
                m_playlistFiles[m_currentPlaylistName].at(m_currentTrackIndex);

        m_mediaPlayer->stop();
        m_playMusicButton->setToolTip("Play Track");
        m_stopMusicButton->setToolTip("Stopped");
        m_pauseMusicButton->setToolTip("Pause Playback");
        updatePlayerStatus(filePath);

    } else {
        m_mediaPlayer->stop();
        statusBar()->showMessage("Stopped (no track loaded)");
    }
}

void MainWindow::onMusicVolumeChanged(int value) {
    static bool updating = false; // Prevent re-entry

    if (!updating) {
        updating = true;

        if (sender() == m_vvolumeSlider && m_musicVolumeSlider->value() != value) {
            m_musicVolumeSlider->setValue(value);
        }
        else if (sender() == m_musicVolumeSlider &&
                 m_vvolumeSlider->value() != value) {
            m_vvolumeSlider->setValue(value);
        }

        updating = false;
    }

    float volume = value / 100.0f;

    if (m_audioOutput) {
        m_audioOutput->setVolume(volume);
    }

    m_musicVolumeLabel->setText(QString("%1%").arg(value));
    m_volShowLabel->setText(QString("%1%").arg(value));

}

void MainWindow::onBinauralPowerToggled(
        bool checked) { // Show warning and check if user wants to proceed
    if (checked) {
    bool proceed = showBinauralWarning();

    if (!proceed) {
        m_binauralPowerButton->blockSignals(true);
        m_binauralPowerButton->setChecked(false);
        m_binauralPowerButton->blockSignals(false);
        updateBinauralPowerState(false);
        return;
    }
    }

    updateBinauralPowerState(checked);
    if (!checked) {
        m_binauralEngine->stop();
        toneTypeCombo->setCurrentIndex(0);
        m_waveformCombo->setCurrentIndex(0);

    } else {
        toneTypeCombo->setCurrentIndex(0);
        m_waveformCombo->setCurrentIndex(0);

    }
    m_binauralStatusLabel->setText(checked ? "Binaural engine enabled"
                                           : "Binaural engine disabled");
}

void MainWindow::onLeftFrequencyChanged(double value) {
    /*
  if (m_binauralEngine->isPlaying()) {
      QMessageBox::information(this,
          "Stop Playback First",
          "Please stop playback before changing frequency.");
      return; // Exit method - no frequency change
  }
  */

    m_binauralEngine->setLeftFrequency(value);
    updateBinauralBeatDisplay();
    m_binauralStatusLabel->setText(formatBinauralString());

    if(ConstantGlobals::currentToneType == ToneType::BINAURAL || ConstantGlobals::currentToneType == ToneType::GENERATOR){
        double visualFrequency = std::abs(value - m_rightFreqInput->value());
        ConstantGlobals::currentBinFreq = visualFrequency;
        if (m_visStimDialog) m_visStimDialog->syncFrequency(visualFrequency);
        if(m_flickerWidget) m_flickerWidget->setFrequency(visualFrequency);
    }
}

void MainWindow::onRightFrequencyChanged(double value) {
    /*
  if (m_binauralEngine->isPlaying()) {
      QMessageBox::information(this,
          "Stop Playback First",
          "Please stop playback before changing frequency.");
      return; // Exit method - no frequency change
  }
  */

    m_binauralEngine->setRightFrequency(value);
    updateBinauralBeatDisplay();
    m_binauralStatusLabel->setText(formatBinauralString());

    if(ConstantGlobals::currentToneType == ToneType::BINAURAL || ConstantGlobals::currentToneType == ToneType::GENERATOR){
        double visualFrequency = std::abs(value - m_leftFreqInput->value());
        ConstantGlobals::currentBinFreq = visualFrequency;
        if (m_visStimDialog) m_visStimDialog->syncFrequency(visualFrequency);

        if(m_flickerWidget) m_flickerWidget->setFrequency(visualFrequency);
    }
}

void MainWindow::onWaveformChanged(int index) {
    DynamicEngine::Waveform waveform =
            static_cast<DynamicEngine::Waveform>(index);
    m_binauralEngine->setWaveform(waveform);
    m_binauralStatusLabel->setText(formatBinauralString());

    if (index >= 0 && index < 4)
          m_visStimDialog->syncWaveType(index);
}

void MainWindow::onBinauralVolumeChanged(double value) {
    m_binauralEngine->setVolume(value / 100.0); // Convert percentage to 0.0-1.0
}

void MainWindow::onBinauralPlayClicked() {

    if (ConstantGlobals::currentToneType == 1) {
        double leftInputValue = m_leftFreqInput->value();
        m_rightFreqInput->setValue(leftInputValue);
    }
    if (m_binauralEngine->start()) {
        if (ConstantGlobals::currentToneType == 0 ||
                ConstantGlobals::currentToneType == 2) {
            m_leftFreqInput->setEnabled(true);
            m_rightFreqInput->setEnabled(true);
        } else {
            m_rightFreqInput->setDisabled(true);
        }
        m_binauralStatusLabel->setText(formatBinauralString());
        startAutoStopTimer();
        m_engineIsPaused = false;
        m_brainwaveDuration->setDisabled(true);
        m_binauralPauseButton->setToolTip("Pause binaural tones");
        m_binauralPlayButton->setToolTip("Play binaural tones");
        unlimitedDurationAction->setDisabled(true);
    }
}

void MainWindow::onBinauralPauseClicked() {
    if (!m_binauralEngine->isPlaying())
        return;

    if (!m_engineIsPaused) {
        m_binauralEngine->stop(); // stop audio
        stopAutoStopTimer();      // stop timer, but do NOT reset remaining seconds
        m_engineIsPaused = true;
        m_binauralPauseButton->setToolTip("Paused");
        m_binauralPlayButton->setToolTip("Resume");

        m_binauralStatusLabel->setText("Binaural tones paused");
    }
}

void MainWindow::onBinauralStopClicked() {
    m_binauralEngine->stop();

    m_binauralPauseButton->setToolTip("Pause binaural tones");
    m_binauralPlayButton->setToolTip("Play binaural tones");

    stopAutoStopTimer();
    m_remainingSeconds = 0;

    m_engineIsPaused = false;

    m_leftFreqInput->setEnabled(true);
    m_rightFreqInput->setEnabled(true);

    m_brainwaveDuration->setEnabled(true);
    m_countdownLabel->setText(m_brainwaveDuration->text());
    if (ConstantGlobals::currentToneType == 1) {
        m_rightFreqInput->setDisabled(true);
    }
    m_binauralStatusLabel->setText("Binaural tones stopped");
    unlimitedDurationAction->setEnabled(true);
}


void MainWindow::onNaturePowerToggled(bool checked) {

    updateNaturePowerState(checked);

    statusBar()->showMessage(checked ? "Nature sounds enabled"
                                     : "Nature sounds disabled");

    openAmbientPresetButton->setEnabled(checked);
    saveAmbientPresetButton->setEnabled(checked);
    resetPlayersButton->setEnabled(checked);
    m_masterPlayButton->setEnabled(checked);
    m_masterPauseButton->setEnabled(checked);
    m_masterStopButton->setEnabled(checked);

    for (auto it = m_ambientPlayers.begin(); it != m_ambientPlayers.end(); ++it) {
        AmbientPlayer *player = it.value();
        if (player && player->button()) {
            player->button()->setEnabled(checked);
        }
    }
}

void MainWindow::onAddFilesClicked() {
    onLoadMusicClicked();
} // Reuse same functionality

void MainWindow::onBinauralPlaybackStarted() {
    m_binauralStatusLabel->setText(formatBinauralString());
}

void MainWindow::onBinauralPlaybackStopped() {
    m_binauralPlayButton->setToolTip("Start binaural tones");
    m_binauralStatusLabel->setText("Binaural tones stopped");
}

void MainWindow::onBinauralError(const QString &error) {
    m_binauralStatusLabel->setText("Error: " + error); // Show for 5 seconds
    QMessageBox::warning(this, "Binaural Engine Error", error);
}

void MainWindow::showFirstLaunchWarning() {

    if (settings.value("firstLaunchWarned", false).toBool()) {
        return; // Already shown and acknowledged
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Welcome to Binaural Media Player");
    msgBox.setIcon(QMessageBox::Information);

    QString message =
            "<div style='margin: 10px;'>"
            "<h3>Welcome! 🎵</h3>"
            "<p>This app creates audio tones for relaxation and focus.</p>"

            "<p><b>For a safe experience:</b></p>"
            "<ul>"
            "<li>Use moderate volume levels</li>"
            "<li>Take breaks during extended use</li>"
            "<li>Find a comfortable environment</li>"
            "</ul>"

            "<p><b>Important considerations:</b></p>"
            "<ul>"
            "<li>If you have epilepsy or are prone to seizures, consult a doctor</li>"
            "<li>Discontinue use if you feel dizzy or uncomfortable</li>"
            "<li>Do not use while driving or operating machinery</li>"
            "</ul>"

            "<p><i>This tool is for personal enjoyment and exploration.</i></p>"
            "</div>";

    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(message);

    QCheckBox *dontShowAgain =
            new QCheckBox("Don't show this message again", &msgBox);
    msgBox.setCheckBox(dontShowAgain);

    msgBox.addButton(QMessageBox::Ok);

    msgBox.exec();

    if (dontShowAgain->isChecked()) {
        settings.setValue("firstLaunchWarned", true);
    }
}

bool MainWindow::showBinauralWarning() {

    if (settings.value("binauralWarned", false).toBool()) {
        return true; // Already shown and acknowledged, proceed
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Binaural & Isochronic Audio");
    msgBox.setIcon(QMessageBox::Information);

    QString message =
            "<div style='margin: 10px;'>"
            "<h3>Brainwave Audio Activation 🧠</h3>"
            "<p>You're about to enable audio tones that may influence brain "
            "activity.</p>"

            "<p><b>Important usage notes:</b></p>"
            "<ul>"
            "<li><b>Binaural mode:</b> Requires headphones for the full effect</li>"
            "<li><b>Isochronic mode:</b> Works with speakers or headphones</li>"
            "<li>Audio will auto-pause after 45 minutes (recommended break)</li>"
            "<li>Stop immediately if you feel any discomfort</li>"
            "</ul>"

            "<p><b>Safety reminder:</b></p>"
            "<ul>"
            "<li>Not recommended for those with epilepsy or seizure disorders</li>"
            "<li>Consult a doctor if you have heart conditions or other medical "
            "concerns</li>"
            "<li>Use at your own discretion</li>"
            "</ul>"
            "</div>";

    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(message);

    QCheckBox *dontShowAgain =
            new QCheckBox("Don't show this warning again", &msgBox);
    msgBox.setCheckBox(dontShowAgain);

    QPushButton *okButton =
            msgBox.addButton("Enable Audio", QMessageBox::AcceptRole);
    QPushButton *cancelButton =
            msgBox.addButton("Cancel", QMessageBox::RejectRole);
    msgBox.setDefaultButton(okButton);

    msgBox.exec();

    bool proceed = (msgBox.clickedButton() == okButton);

    if (proceed && dontShowAgain->isChecked()) {
        settings.setValue("binauralWarned", true);
    }

    return proceed;
}

void MainWindow::startAutoStopTimer() {
    stopAutoStopTimer();

    if (!m_engineIsPaused) {
        m_remainingSeconds = m_brainwaveDuration->value() * 60;
    }


    m_autoStopTimer = new QTimer(this);
    connect(m_autoStopTimer, &QTimer::timeout, this,
            &MainWindow::onAutoStopTimerTimeout);

    m_autoStopTimer->start(1000); // Update every second

    m_countdownLabel->setVisible(true);
    updateCountdownDisplay();

}

void MainWindow::stopAutoStopTimer() {
    if (m_autoStopTimer) {
        m_autoStopTimer->stop();
        delete m_autoStopTimer;
        m_autoStopTimer = nullptr;
    }

    m_countdownLabel->setVisible(false);
}

void MainWindow::updateCountdownDisplay() {
    int minutes = m_remainingSeconds / 60;
    int seconds = m_remainingSeconds % 60;

    QString timeText = QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));

    if (isDarkTheme) {
        if (minutes == 0) {
            m_countdownLabel->setStyleSheet(
                        "background-color: #5a1a1a; color: #ff8888; padding: 3px; "
                        "border: 1px solid #8a3a3a; border-radius: 3px; font-weight: bold;");
        } else if (minutes <= 5) {
            m_countdownLabel->setStyleSheet(
                        "background-color: #5a4a1a; color: #ffcc88; padding: 3px; "
                        "border: 1px solid #8a6a3a; border-radius: 3px;");
        } else {
            m_countdownLabel->setStyleSheet(
                        "background-color: #16213e; color: #7B68EE; padding: 3px; "
                        "border: 1px solid #0f3460; border-radius: 3px;");
        }
    } else {
        if (minutes == 0) {
            m_countdownLabel->setStyleSheet(
                        "background-color: #F8D7DA; color: #721C24; padding: 3px; "
                        "border: 1px solid #F5C6CB; border-radius: 3px; font-weight: bold;");
        } else if (minutes <= 5) {
            m_countdownLabel->setStyleSheet(
                        "background-color: #FFF3CD; color: #856404; padding: 3px; "
                        "border: 1px solid #FFE082; border-radius: 3px;");
        } else {
            m_countdownLabel->setStyleSheet(
                        "background-color: #f0f0f0; color: #7B68EE; padding: 3px; "
                        "border: 1px solid #ccc; border-radius: 3px;");
        }
    }

    m_countdownLabel->setText(timeText);
}

void MainWindow::onAutoStopTimerTimeout() {
    if (m_remainingSeconds > 0) {
        m_remainingSeconds--;
        updateCountdownDisplay();

        if (m_remainingSeconds == 60) {
        }
    } else {
        stopAutoStopTimer();

        if (m_binauralEngine && m_binauralEngine->isPlaying()) {
            m_binauralEngine->stop();
            m_binauralStatusLabel->setText("Brainwave session completed");
        }

        QMessageBox::information(
                    this, "Session Complete",
                    "Your brainwave session has finished.\n\n"
                    "Taking a short break is recommended before starting a new session.");
    }
}

void MainWindow::onBrainwaveDurationChanged(int minutes) {
}
void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {

    switch (status) {
    case QMediaPlayer::LoadingMedia:

        break;
    case QMediaPlayer::LoadedMedia:

        break;
    case QMediaPlayer::BufferedMedia:
        break;
    case QMediaPlayer::EndOfMedia:

        if (!m_isRepeat) {
            if (isShuffle) {
                playRandomTrack();
            } else {
                playNextTrack();
            }
        } else {
            if (m_currentTrackIndex >= 0) {
                m_mediaPlayer->setPosition(0);
                m_mediaPlayer->play();
            }
        }
        break;
    case QMediaPlayer::InvalidMedia:
        statusBar()->showMessage("Error: Invalid media file", 3000);
        playNextTrack();
        break;
    default:
        break;
    }
}

void MainWindow::onPlaybackStateChanged(QMediaPlayer::PlaybackState state) {
    switch (state) {
    case QMediaPlayer::PlayingState:
        m_playMusicButton->setEnabled(true);
        m_pauseMusicButton->setEnabled(true);
        m_stopMusicButton->setEnabled(true);
        m_playingTrackIndex = m_currentTrackIndex;
        ConstantGlobals::playbackState = QMediaPlayer::PlayingState;
        break;

    case QMediaPlayer::PausedState:
        m_playMusicButton->setEnabled(true);
        m_pauseMusicButton->setEnabled(false);
        m_stopMusicButton->setEnabled(true);
        m_pausedPosition = m_mediaPlayer->position();
        ConstantGlobals::playbackState = QMediaPlayer::PausedState;

        break;

    case QMediaPlayer::StoppedState:
        m_playMusicButton->setEnabled(true);
        m_pauseMusicButton->setEnabled(false);
        m_stopMusicButton->setEnabled(false);
        ConstantGlobals::playbackState = QMediaPlayer::StoppedState;

        break;
    }
}

void MainWindow::onMediaPlayerError(QMediaPlayer::Error error,
                                    const QString &errorString) {
    Q_UNUSED(error);
    statusBar()->showMessage("Media error: " + errorString, 5000);
}


void MainWindow::playNextTrack() {
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (m_isVideoEnabled) {
        playlist = m_lastVideoPlaylist;

        playlistName = m_lastVideoPlaylistName;
        m_currentPlaylistName = m_lastVideoPlaylistName;

    }

    if (!playlist || playlistName.isEmpty() ||
            m_playlistFiles[playlistName].isEmpty())
        return;

    int nextIndex = m_currentTrackIndex + 1;

    if (m_isVideoEnabled)
        nextIndex = m_lastVideoTrackIndex + 1;

    if (nextIndex >= m_playlistFiles[playlistName].size()) {
        nextIndex = 0;
    }

    m_currentPlaylistName = playlistName;
    if (m_isVideoEnabled)
        m_currentPlaylistName = m_lastVideoPlaylistName;
    m_currentTrackIndex = nextIndex;
    m_lastVideoTrackIndex = nextIndex;

    playlist->setCurrentRow(nextIndex);
    if (m_isVideoEnabled)
        playlist->setCurrentRow(m_lastVideoTrackIndex);

    QString filePath = m_playlistFiles[playlistName].at(nextIndex);
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));

    QFileInfo fileInfo(filePath);
    QString trackName = fileInfo.fileName();

    QTimer::singleShot(100, this, [this, trackName]() {
        m_mediaPlayer->play();
        updatePlayerStatus(trackName);
    });
}

void MainWindow::onPlayMusicClicked() {

    if (m_mediaPlayer) {

        if (m_isStream && !m_currentStreamUrl.isEmpty() &&
                m_mediaPlayer->playbackState() == QMediaPlayer::PausedState) {
            m_mediaPlayer->play(); // Resume the paused stream

            QUrl url(m_currentStreamUrl);
            QString name = url.fileName();
            if (name.isEmpty())
                name = url.host();
            if (name.isEmpty())
                name = "Stream";

            statusBar()->showMessage("▶ Streaming: " + name);
            return; // Exit - we handled the stream resume
        }
    }

    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (m_isVideoEnabled) {
        if (playlistName != "Video Playlist") {
            for (int i = 0; i < m_playlistTabs->count(); ++i) {
                if (m_playlistTabs->tabText(i) == "Video Playlist") {
                    playlist = qobject_cast<QListWidget *>(m_playlistTabs->widget(i));
                    m_lastVideoPlaylist = playlist;
                    playlistName = "Video Playlist";
                    m_currentPlaylistName = "Video Playlist";
                    break;
                }
            }
        }
    }

       if (m_isVideoEnabled) m_lastVideoPlaylist = playlist;

    if (!playlist || playlistName.isEmpty() ||
            !m_playlistFiles.contains(playlistName) ||
            m_playlistFiles[playlistName].isEmpty()) {
        statusBar()->showMessage("No music loaded in current playlist", 3000);
        return;
    }

    int selectedRow = playlist->currentRow();

    if (selectedRow >= 0 && selectedRow < m_playlistFiles[playlistName].size()) {
        m_currentTrackIndex = selectedRow;
        m_lastVideoTrackIndex = selectedRow;
        m_currentPlaylistName = playlistName;
        m_lastVideoPlaylistName = playlistName;
    }
    else if (m_currentTrackIndex == -1 || m_currentPlaylistName != playlistName) {
        m_currentTrackIndex = 0;
        m_currentPlaylistName = playlistName;
        playlist->setCurrentRow(0);
    }

    QString filePath = m_playlistFiles[playlistName].at(m_currentTrackIndex);

    QMediaPlayer::PlaybackState state = m_mediaPlayer->playbackState();

    if (state == QMediaPlayer::PausedState) {
        QString currentSource = m_mediaPlayer->source().toLocalFile();
        bool sameTrack = (currentSource == filePath);

        if (sameTrack) {
            if (m_pausedPosition > 0) {
                m_mediaPlayer->setPosition(m_pausedPosition);
            }
            m_mediaPlayer->play();
            m_playMusicButton->setToolTip("Playing...");
            m_pauseMusicButton->setToolTip("Pause Playback");

            updatePlayerStatus(filePath);
            return;
        }
    } else if (state == QMediaPlayer::PlayingState) {
        QString currentSource = m_mediaPlayer->source().toLocalFile();
        bool sameTrack = (currentSource == filePath);

        if (sameTrack) {
            return;
        }
    }

    m_mediaPlayer->stop();
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    m_mediaPlayer->play();
    m_playMusicButton->setToolTip("Playing...");
    m_stopMusicButton->setToolTip("Stop Playback");
    m_pauseMusicButton->setToolTip("Pause Playback");
    statusBar()->showMessage("Playing: " + QFileInfo(filePath).fileName());
}

void MainWindow::onPlaylistItemClicked(QListWidgetItem *item) {
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (!playlist || playlistName.isEmpty()) {
        return;
    }

    int index = playlist->row(item);
    m_currentTrackIndex = index;
    m_currentPlaylistName =
            playlistName; // Track which playlist this selection belongs to

}

void MainWindow::onDurationChanged(qint64 durationMs) {

    if (durationMs <= 0) {
        m_totalTimeLabel->setText("00:00");
        m_seekSlider->setEnabled(false);
        return;
    }

    int totalSeconds = durationMs / 1000;
    int minutes = (totalSeconds / 60) % 60;
    int seconds = totalSeconds % 60;
    int hours = totalSeconds / 3600;

    QString durationStr;
    if (hours > 0) {
        durationStr = QString("%1:%2:%3")
                .arg(hours, 2, 10, QChar('0'))
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0'));
    } else {
        durationStr = QString("%1:%2")
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0'));
    }

    m_totalTimeLabel->setText(durationStr);
    m_seekSlider->setRange(0, totalSeconds);
    m_seekSlider->setEnabled(true);

    if (m_currentTrackIndex >= 0) {
    }
}

void MainWindow::onPositionChanged(qint64 positionMs) {
    if (m_seekSlider->isSliderDown()) {
        return;
    }

    int currentSeconds = positionMs / 1000;
    int minutes = currentSeconds / 60;
    int seconds = currentSeconds % 60;

    m_currentTimeLabel->setText(QString("%1:%2")
                                .arg(minutes, 2, 10, QChar('0'))
                                .arg(seconds, 2, 10, QChar('0')));

    m_seekSlider->setValue(currentSeconds);
}

void MainWindow::onSeekSliderMoved(int value) {
    m_mediaPlayer->setPosition(value * 1000);

    int minutes = value / 60;
    int seconds = value % 60;
    m_currentTimeLabel->setText(QString("%1:%2")
                                .arg(minutes, 2, 10, QChar('0'))
                                .arg(seconds, 2, 10, QChar('0')));
}

void MainWindow::onSeekSliderReleased() {
    int value = m_seekSlider->value();
    m_mediaPlayer->setPosition(value * 1000);
}

void MainWindow::playPreviousTrack() {
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (m_isVideoEnabled) {
        playlist = m_lastVideoPlaylist;

        playlistName = m_lastVideoPlaylistName;
        m_currentPlaylistName = m_lastVideoPlaylistName;

    }

    if (!playlist || playlistName.isEmpty() ||
            m_playlistFiles[playlistName].isEmpty())
        return;

    int prevIndex = m_currentTrackIndex - 1;
    if (m_isVideoEnabled)
        prevIndex = m_lastVideoTrackIndex - 1;

    if (prevIndex < 0) {
        prevIndex = m_playlistFiles[playlistName].size() - 1;
    }

    m_currentPlaylistName = playlistName;

    if (m_isVideoEnabled)
        m_currentPlaylistName = m_lastVideoPlaylistName;

    m_currentTrackIndex = prevIndex;
    m_lastVideoTrackIndex = prevIndex;

    playlist->setCurrentRow(prevIndex);
    if (m_isVideoEnabled)
        playlist->setCurrentRow(m_lastVideoTrackIndex);

    QString filePath = m_playlistFiles[playlistName].at(prevIndex);
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));

    QFileInfo fileInfo(filePath);
    QString trackName = fileInfo.fileName();

    QTimer::singleShot(100, this, [this, trackName]() {
        m_mediaPlayer->play();
        updatePlayerStatus(trackName);
    });
}

void MainWindow::onRepeatClicked() {
    if (m_isVideoEnabled) {
        m_repeatButton->setChecked(false);
        return;
    }

    if (m_repeatButton->isChecked()) {
        m_isRepeat = true;
        m_repeatButton->setToolTip("Repeat: ON");
    } else {
        m_isRepeat = false;
        m_repeatButton->setToolTip("Repeat: OFF");
    }
}

void MainWindow::onToneTypeComboIndexChanged(int index) {

    int toneValue = toneTypeCombo->itemData(index).toInt();

    switch (toneValue) {

    case BINAURAL:
        ConstantGlobals::currentToneType = 0; // Set to 0
        m_rightFreqInput->setEnabled(true);
        m_leftFreqInput->setValue(360.00);
        m_rightFreqInput->setValue(367.83);
        m_pulseFreqLabel->setDisabled(true);
        m_binauralStatusLabel->setText(formatBinauralString());
        if (m_flickerWidget) m_flickerWidget->setFrequency(ConstantGlobals::currentBinFreq);
        break;
    case ISOCHRONIC:
        ConstantGlobals::currentToneType = 1; // Set to 0
        m_leftFreqInput->setValue(360.00);
        m_pulseFreqLabel->setValue(7.83);
        m_rightFreqInput->setDisabled(true);
        m_pulseFreqLabel->setEnabled(true);
        m_binauralStatusLabel->setText(formatBinauralString());
        if (m_flickerWidget) m_flickerWidget->setFrequency(ConstantGlobals::currentIsonFreq);

        break;

    case GENERATOR:
        ConstantGlobals::currentToneType = 2; // Set to 0
        m_leftFreqInput->setValue(360.00);
        m_rightFreqInput->setValue(360.00);
        m_rightFreqInput->setEnabled(true);
        m_pulseFreqLabel->setDisabled(true);
        m_binauralStatusLabel->setText(formatBinauralString());
        if (m_flickerWidget) m_flickerWidget->setFrequency(ConstantGlobals::currentBinFreq);

        break;

    default:
        break;
    }
}

void MainWindow::onMuteButtonClicked() {
    bool checked = volumeIcon->isChecked();

    mutePlayingAmbientPlayers(checked);
    m_audioOutput->setMuted(checked);

    QAudioSink *binauralOutput =
            m_binauralEngine ? m_binauralEngine->audioOutput() : nullptr;
    if (checked) {

        if (binauralOutput) {
            binEngineVolume = binauralOutput->volume();
            binauralOutput->setVolume(0);
            m_binauralStopButton->setDisabled(true);
        }
        volumeIcon->setIcon(QIcon(":/icons/volume-x.svg"));
        m_stopMusicButton->setDisabled(true);
        m_masterStopButton->setDisabled(true);

    } else {
        volumeIcon->setIcon(QIcon(":/icons/volume-2.svg"));

        m_stopMusicButton->setEnabled(true);
        m_masterStopButton->setEnabled(true);

        if (binauralOutput) {
            binauralOutput->setVolume(binEngineVolume);
            m_binauralStopButton->setEnabled(true);
        }
    }
}


void MainWindow::playRandomTrack() {
    QString playlistName = m_currentPlaylistName;
    if (playlistName.isEmpty() || !m_playlistFiles.contains(playlistName)) {
        statusBar()->showMessage("No active playlist", 2000);
        return;
    }

    const QStringList &files = m_playlistFiles[playlistName];
    if (files.isEmpty()) {
        statusBar()->showMessage("No tracks in current playlist", 2000);
        return;
    }

    int randomIndex;
    do {
        randomIndex = QRandomGenerator::global()->bounded(files.size());
    } while (randomIndex == m_currentTrackIndex && files.size() > 1);

    m_currentTrackIndex = randomIndex;

    QListWidget *playlist = currentPlaylistWidget();
    if (playlist && m_currentPlaylistName == playlistName) {
        playlist->setCurrentRow(randomIndex);
    }

    QString filePath = files.at(randomIndex);
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));

    QTimer::singleShot(100, this, [this]() {
        m_mediaPlayer->play();
        statusBar()->showMessage("Playing random track...");
    });
}


QListWidget *MainWindow::currentPlaylistWidget() const {
    if (!m_playlistTabs || m_playlistTabs->count() == 0)
        return nullptr;
    QWidget *widget = m_playlistTabs->currentWidget();
    if (!widget)
        return nullptr;
    return qobject_cast<QListWidget *>(widget);
}

QString MainWindow::currentPlaylistName() const {
    if (m_playlistTabs->count() == 0)
        return QString();
    return m_playlistTabs->tabText(m_playlistTabs->currentIndex());
}

void MainWindow::addNewPlaylist(const QString &name) {

    if (name == "Video Playlist")
        return;

    QListWidget *newPlaylist = new QListWidget();
    newPlaylist->setAlternatingRowColors(true);
    newPlaylist->setSelectionMode(QAbstractItemView::SingleSelection);
    QString playlistName =
            name.isEmpty() ? QString("Playlist %1").arg(m_playlistTabs->count() - 1)
                           : name; // was +1 changed to -1 to disregard video tabs

    m_playlistFiles[playlistName] = QStringList();
    m_playlistTabs->addTab(newPlaylist, playlistName);
    m_playlistTabs->setCurrentWidget(newPlaylist);

    connect(newPlaylist, &QListWidget::itemClicked, this,
            &MainWindow::onPlaylistItemClicked);
    connect(newPlaylist, &QListWidget::itemDoubleClicked, this,
            &MainWindow::onPlaylistItemDoubleClicked);

    updateCurrentPlaylistReference();

    if (m_currentPlaylistName.isEmpty()) {
        m_currentPlaylistName = playlistName;
    }
}

void MainWindow::updateCurrentPlaylistReference() {
    m_currentPlaylistWidget = currentPlaylistWidget();
}

void MainWindow::onAddNewPlaylistClicked() {
    bool ok;
    QString name = QInputDialog::getText(
                this, "New Playlist", "Enter playlist name:", QLineEdit::Normal,
                QString("Playlist %1")
                .arg(m_playlistTabs->count() -
                     1), // was +1 changed to -1 to disregard video tabs
                &ok);
    if (ok && !name.isEmpty()) {
        addNewPlaylist(name);
        updatePlaylistButtonsState();
        statusBar()->showMessage("Created new playlist: " + name);
    }
}

void MainWindow::onRenamePlaylistClicked() {

    QListWidget *widget = currentPlaylistWidget();
    if (!widget)
        return;

    QString oldName = currentPlaylistName();

    if (oldName == "Video Playlist" || oldName == "Video Player")
        return;

    bool ok;
    QString newName =
            QInputDialog::getText(this, "Rename Playlist",
                                  "Enter new name:", QLineEdit::Normal, oldName, &ok);
    if (ok && !newName.isEmpty() && newName != oldName) {
        int currentIndex = m_playlistTabs->currentIndex();
        m_playlistTabs->setTabText(currentIndex, newName);

        if (m_playlistFiles.contains(oldName)) {
            m_playlistFiles[newName] = m_playlistFiles.take(oldName);
        }

        if (m_currentPlaylistName == oldName) {
            m_currentPlaylistName = newName;
        }

        statusBar()->showMessage("Renamed playlist to: " + newName);
    }
}

void MainWindow::onClosePlaylistTab(int index) {
    if (m_playlistTabs->count() <= 1) {
        QMessageBox::warning(this, "Cannot Close",
                             "You must have at least one playlist.");
        return;
    }

    QString tabName = m_playlistTabs->tabText(index);

    if (tabName == "Video Player" || tabName == "Video Playlist") {
        QMessageBox::information(
                    this, "Cannot Close",
        "Video tabs cannot be closed. Use the video toggle button instead.");
        return; // Exit without closing
    }

    QString playlistName = m_playlistTabs->tabText(index);

    QListWidget *playlist =
            qobject_cast<QListWidget *>(m_playlistTabs->widget(index));
    if (playlist && playlist->count() > 0) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
                    this, "Close Playlist",
                    QString("Playlist '%1' contains %2 tracks. Close anyway?")
                    .arg(playlistName)
                    .arg(playlist->count()),
                    QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No)
            return;
    }

    m_playlistFiles.remove(playlistName);

    m_playlistTabs->removeTab(index);

    updateCurrentPlaylistReference();
    statusBar()->showMessage("Closed playlist: " + playlistName);
}

void MainWindow::onPlaylistTabChanged(int index) {
    updateCurrentPlaylistReference();
    m_currentTrackIndex = -1;

    updatePlaylistButtonsState();

    QString playlistName = currentPlaylistName();
    if (!playlistName.isEmpty()) {
        int trackCount = m_playlistFiles.value(playlistName).size();
        statusBar()->showMessage(QString("Switched to '%1' (%2 tracks)")
                                 .arg(playlistName)
                                 .arg(trackCount),
                                 2000);
    }

}


void MainWindow::onLoadMusicClicked() {

    QString filter = "All Supported Media (*.mp3 *.wav *.flac *.ogg *.m4a *.mp4 "
                     "*.m4v *.avi *.mkv);;"
                     "Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a);;"
                     "Video Files (*.mp4 *.m4v *.avi *.mkv);;"
                     "All Files (*.*)";
    QStringList files = QFileDialog::getOpenFileNames(
                this, "Select Music Files", ConstantGlobals::musicFilePath,
                filter);
    if (!files.isEmpty()) {

        ConstantGlobals::lastMusicDirPath = QFileInfo(files.first()).absolutePath();

        QListWidget *playlist = currentPlaylistWidget();
        QString playlistName = currentPlaylistName();

        if (m_isVideoEnabled) {
            for (int i = 0; i < m_playlistTabs->count(); ++i) {
                if (m_playlistTabs->tabText(i) == "Video Playlist") {
                    playlist = qobject_cast<QListWidget *>(m_playlistTabs->widget(i));
                    playlistName = "Video Playlist";
                    m_playlistTabs->setCurrentIndex(i);
                    break;
                }
            }
        }

        foreach (const QString &file, files) {
            QString fileName = QFileInfo(file).fileName();
            playlist->addItem(fileName);
            m_playlistFiles[playlistName].append(file);
        }
        if (playlist->count() > 0) {
            playlist->setCurrentRow(0); // This highlights and selects the first item
            playlist->scrollToTop();
        }
        statusBar()->showMessage(QString("Added %1 file(s) to '%2'")
                                 .arg(files.size())
                                 .arg(playlistName));
    }
}


void MainWindow::onRemoveTrackClicked() {
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (!playlist || playlistName.isEmpty())
        return;

    int playingIndex = m_playingTrackIndex;

    int selectedRow = playlist->currentRow();
    if (selectedRow >= 0 && selectedRow < m_playlistFiles[playlistName].size()) {

        if (playlistName == m_currentPlaylistName && selectedRow == playingIndex) {
            QMessageBox::warning(
                        this, "Cannot Remove Track",
                        "Cannot remove the currently playing track. Stop playback first.");
            return;
        }

        QListWidgetItem *item = playlist->takeItem(selectedRow);
        delete item;

        m_playlistFiles[playlistName].removeAt(selectedRow);

        if (playlistName == m_currentPlaylistName) {
            if (selectedRow < playingIndex) {
                m_playingTrackIndex = playingIndex - 1;
            }

            m_currentTrackIndex = m_playingTrackIndex;

            if (m_playingTrackIndex >= 0) {
                playlist->setCurrentRow(m_playingTrackIndex);
            }
        }

        statusBar()->showMessage("Track removed from playlist");
    } else {
        statusBar()->showMessage("No track selected");
    }
}

void MainWindow::onClearPlaylistClicked() {
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (!playlist || playlistName.isEmpty())
        return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
                this, "Clear Playlist",
                QString("Clear all tracks from '%1'?").arg(playlistName),
                QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_mediaPlayer &&
                m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
            m_mediaPlayer->stop();
        }
        playlist->clear();
        m_playlistFiles[playlistName].clear();

        if (playlistName == m_currentPlaylistName) {
            m_currentTrackIndex = -1;
        }

        statusBar()->showMessage("Playlist cleared");
    }
}


void MainWindow::onPlaylistItemDoubleClicked(QListWidgetItem *item) {

    if (m_isVideoEnabled)
        return;

    QListWidget *playlist = qobject_cast<QListWidget *>(sender());
    if (!playlist) {
        playlist = currentPlaylistWidget();
    }

    QString playlistName = currentPlaylistName();

    if (!playlist || playlistName.isEmpty())
        return;

    int index = playlist->row(item);
    if (index >= 0 && index < m_playlistFiles[playlistName].size()) {
        m_currentPlaylistName = playlistName;
        m_currentTrackIndex = index;
        QString filePath = m_playlistFiles[playlistName][index];
        m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
        m_mediaPlayer->play();
        m_pauseMusicButton->setToolTip("Pause Playback");
        m_stopMusicButton->setToolTip("Stop Playback");
        m_playMusicButton->setToolTip("Playing...");
        statusBar()->showMessage("Playing: " + item->text());
    }
}

void MainWindow::updatePlaylistButtonsState() {
    return;
    QListWidget *playlist = currentPlaylistWidget();
    bool hasSelection = playlist && !playlist->selectedItems().isEmpty();
    bool hasItems = playlist && playlist->count() > 0;

    m_removeTrackButton->setEnabled(hasSelection);
    m_clearPlaylistButton->setEnabled(hasItems);
}


QJsonObject MainWindow::BrainwavePreset::toJson() const {
    QJsonObject json;
    json["name"] = name;
    json["toneType"] = toneType;
    json["leftFrequency"] = leftFrequency;
    json["rightFrequency"] = rightFrequency;
    json["waveform"] = waveform;
    json["pulseFrequency"] = pulseFrequency;
    json["volume"] = volume;
    json["version"] = "1.0";
    json["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    return json;
}

MainWindow::BrainwavePreset
MainWindow::BrainwavePreset::fromJson(const QJsonObject &json) {
    BrainwavePreset preset;
    preset.name = json["name"].toString();
    preset.toneType = json["toneType"].toInt();
    preset.leftFrequency = json["leftFrequency"].toDouble();
    preset.rightFrequency = json["rightFrequency"].toDouble();
    preset.waveform = json["waveform"].toInt();
    preset.pulseFrequency = json["pulseFrequency"].toDouble();
    preset.volume = json["volume"].toDouble();
    return preset;
}

bool MainWindow::BrainwavePreset::isValid() const {
    return !name.isEmpty() && toneType >= 0 && toneType <= 2 &&
            leftFrequency >= 20.0 && leftFrequency <= 20000.0 &&
            rightFrequency >= 20.0 && rightFrequency <= 20000.0 && waveform >= 0 &&
            waveform <= 3 && pulseFrequency >= 0.0 && pulseFrequency <= 100.0 &&
            volume >= 0.0 && volume <= 100.0;
}

QJsonObject MainWindow::PlaylistTrack::toJson() const {
    QJsonObject json;
    json["filePath"] = filePath;
    json["title"] = title;
    json["duration"] = duration;
    return json;
}

MainWindow::PlaylistTrack
MainWindow::PlaylistTrack::fromJson(const QJsonObject &json) {
    PlaylistTrack track;
    track.filePath = json["filePath"].toString();
    track.title = json["title"].toString();
    track.duration = json["duration"].toVariant().toLongLong();
    return track;
}


QString MainWindow::generateDefaultPresetName() const {
    QString toneType;
    switch (ConstantGlobals::currentToneType) {
    case 0:
        toneType = "BIN";
        break;
    case 1:
        toneType = "ISO";
        break;
    case 2:
        toneType = "GEN";
        break;
    default:
        toneType = "PRESET";
        break;
    }

    if (ConstantGlobals::currentToneType == 0) {
        double beatFreq =
                qAbs(m_rightFreqInput->value() - m_leftFreqInput->value());
        return QString("%1-%2Hz").arg(toneType).arg(beatFreq, 0, 'f', 2);
    } else if (ConstantGlobals::currentToneType == 1) {
        return QString("%1-%2Hz").arg(toneType).arg(m_pulseFreqLabel->value(), 0,
                                                    'f', 2);
    } else {
        return QString("%1-%2Hz").arg(toneType).arg(m_leftFreqInput->value(), 0,
                                                    'f', 2);
    }
}

bool MainWindow::ensureDirectoryExists(const QString &path) {
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}


void MainWindow::onSavePresetClicked() {
    QString defaultName = generateDefaultPresetName();

    bool ok;
    QString presetName = QInputDialog::getText(
                this, "Save Brainwave Preset", "Enter preset name:", QLineEdit::Normal,
                defaultName, &ok);

    if (!ok || presetName.isEmpty()) {
        return;
    }

    BrainwavePreset preset;
    preset.name = presetName;
    preset.toneType = ConstantGlobals::currentToneType;
    preset.leftFrequency = m_leftFreqInput->value();
    preset.rightFrequency = m_rightFreqInput->value();
    preset.waveform = m_waveformCombo->currentIndex();
    preset.pulseFrequency = m_pulseFreqLabel->value();
    preset.volume = m_binauralVolumeInput->value();

    if (!preset.isValid()) {
        QMessageBox::warning(
                    this, "Invalid Preset",
                    "Current settings are invalid. Please check frequency ranges.");
        return;
    }

    if (!ensureDirectoryExists(ConstantGlobals::presetFilePath)) {
        QMessageBox::warning(this, "Save Error",
                             "Cannot create presets directory.");
        return;
    }

    QString filename =
            ConstantGlobals::presetFilePath + "/" + presetName + ".json";
    if (savePresetToFile(filename, preset)) {
        statusBar()->showMessage("Preset saved: " + presetName, 3000);
    } else {
        QMessageBox::warning(this, "Save Error", "Failed to save preset to file.");
    }
}

void MainWindow::onLoadPresetClicked() {
    QDir presetDir(ConstantGlobals::presetFilePath + "/");
    QStringList presetFiles = presetDir.entryList({"*.json"}, QDir::Files);

    if (presetFiles.isEmpty()) {
        QMessageBox::information(this, "No Presets",
                                 "No saved presets found in:\n" +
                                 ConstantGlobals::presetFilePath);
        return;
    }

    QStringList presetNames;
    foreach (const QString &file, presetFiles) {
        presetNames << QFileInfo(file).completeBaseName();
    }

    bool ok;
    QString selectedPreset = QInputDialog::getItem(
                this, "Load Preset", "Select preset to load:", presetNames, 0, false,
                &ok);

    if (!ok || selectedPreset.isEmpty()) {
        return;
    }

    QString filename =
            ConstantGlobals::presetFilePath + "/" + selectedPreset + ".json";
    BrainwavePreset preset = loadPresetFromFile(filename);

    if (!preset.isValid()) {
        QMessageBox::warning(this, "Load Error",
                             "Failed to load preset or preset is invalid.");
        return;
    }

    bool wasPlaying = m_binauralEngine->isPlaying();
    if (wasPlaying) {
        m_binauralEngine->stop();
    }

    ConstantGlobals::currentToneType = preset.toneType;
    toneTypeCombo->setCurrentIndex(preset.toneType);
    m_leftFreqInput->setValue(preset.leftFrequency);
    m_rightFreqInput->setValue(preset.rightFrequency);
    m_waveformCombo->setCurrentIndex(preset.waveform);
    m_pulseFreqLabel->setValue(preset.pulseFrequency);
    m_binauralVolumeInput->setValue(preset.volume);

    updateBinauralBeatDisplay();

    if (wasPlaying) {
        onBinauralPlayClicked();
    }

    statusBar()->showMessage("Preset loaded: " + preset.name, 3000);
}

void MainWindow::onManagePresetsClicked() {
    QUrl presetUrl = QUrl::fromLocalFile(ConstantGlobals::presetFilePath);
    QDesktopServices::openUrl(presetUrl);
}

bool MainWindow::savePresetToFile(const QString &filename,
                                  const BrainwavePreset &preset) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open preset file for writing:" << filename;
        return false;
    }

    QJsonObject json = preset.toJson();
    QJsonDocument doc(json);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

MainWindow::BrainwavePreset
MainWindow::loadPresetFromFile(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open preset file:" << filename;
        return BrainwavePreset();
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error in preset file:" << error.errorString();
        return BrainwavePreset();
    }

    if (!doc.isObject()) {
        qWarning() << "Preset file is not a valid JSON object";
        return BrainwavePreset();
    }

    BrainwavePreset preset = BrainwavePreset::fromJson(doc.object());
    return preset;
}

QList<MainWindow::BrainwavePreset> MainWindow::loadAllPresets() {
    QList<BrainwavePreset> presets;
    QDir presetDir(ConstantGlobals::presetFilePath);

    QStringList presetFiles = presetDir.entryList({"*.json"}, QDir::Files);
    foreach (const QString &file, presetFiles) {
        QString filepath = ConstantGlobals::presetFilePath + file;
        BrainwavePreset preset = loadPresetFromFile(filepath);
        if (preset.isValid()) {
            presets.append(preset);
        }
    }

    return presets;
}


void MainWindow::onOpenPlaylistClicked() {

    if (m_isVideoEnabled) {
    }

    QString filename = QFileDialog::getOpenFileName(
                this, "Open Playlist", ConstantGlobals::playlistFilePath,
                "Playlist Files (*.json);;All Files (*)");

    if (filename.isEmpty()) {
        return;
    }

    if (loadPlaylistFromFile(filename)) {
        statusBar()->showMessage("Playlist loaded", 3000);
    } else {
        QMessageBox::warning(this, "Load Error",
                             "Failed to load playlist from file.");
    }
}

void MainWindow::onSaveCurrentPlaylistClicked() {

    if (m_isVideoEnabled) {
    }

    QString playlistName = currentPlaylistName();
    if (playlistName == "Video Playlist")
    if (playlistName.isEmpty()) {
        QMessageBox::warning(this, "Save Error", "No active playlist.");
        return;
    }

    if (!ensureDirectoryExists(ConstantGlobals::playlistFilePath)) {
        QMessageBox::warning(this, "Save Error",
                             "Cannot create playlists directory.");
        return;
    }

    QString filename =
            ConstantGlobals::playlistFilePath + "/" + playlistName + ".json";
    if (savePlaylistToFile(filename, playlistName)) {
        statusBar()->showMessage("Playlist saved: " + playlistName, 3000);
    } else {
        QMessageBox::warning(this, "Save Error",
                             "Failed to save playlist to file.");
    }
}

void MainWindow::onSaveCurrentPlaylistAsClicked() {

    QString playlistName = currentPlaylistName();

    if (playlistName == "Video Playlist") {
        onSaveCurrentPlaylistClicked();
        return;
    }
    if (playlistName == "Video Player")
        return;

    if (playlistName.isEmpty()) {
        QMessageBox::warning(this, "Save Error", "No active playlist.");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
                this, "Save Playlist As",
                ConstantGlobals::playlistFilePath + "/" + playlistName + ".json",
                "Playlist Files (*.json);;All Files (*)");

    if (filename.isEmpty()) {
        return;
    }

    if (savePlaylistToFile(filename, playlistName)) {
        statusBar()->showMessage(
                    "Playlist saved as: " + QFileInfo(filename).fileName(), 3000);
    } else {
        QMessageBox::warning(this, "Save Error",
                             "Failed to save playlist to file.");
    }
}

void MainWindow::onSaveAllPlaylistsClicked() {

    int successCount = 0;
    int failCount = 0;

    if (!ensureDirectoryExists(ConstantGlobals::playlistFilePath)) {
        QMessageBox::warning(this, "Save Error",
                             "Cannot create playlists directory.");
        return;
    }

    for (int i = 0; i < m_playlistTabs->count(); ++i) {
        QString playlistName = m_playlistTabs->tabText(i);

        if (playlistName == "Video Player") {
            successCount++;

            continue;
        }

        QString filename =
                ConstantGlobals::playlistFilePath + "/" + playlistName + ".json";

        m_playlistTabs->setCurrentIndex(i);
        updateCurrentPlaylistReference();

        if (savePlaylistToFile(filename, playlistName)) {
            successCount++;
        } else {
            failCount++;
        }
    }

    if (failCount == 0) {
        statusBar()->showMessage(
                    QString("All %1 playlists saved successfully").arg(successCount), 3000);
    } else {
        QMessageBox::warning(
                    this, "Save Warning",
                    QString("Saved %1 playlists, failed to save %2 playlists")
            .arg(successCount)
                    .arg(failCount));
    }
}

bool MainWindow::savePlaylistToFile(const QString &filename,
                                    const QString &playlistName) {

    QListWidget *playlist = currentPlaylistWidget();
    if (!playlist || playlist->count() == 0) {
        statusBar()->showMessage("Playlist is empty", 2000);
        return false;
    }

    QStringList filePaths = m_playlistFiles.value(playlistName, QStringList());

    QJsonObject playlistJson;
    playlistJson["name"] = playlistName;
    playlistJson["version"] = "1.0";
    playlistJson["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    playlistJson["trackCount"] = filePaths.size();

    QJsonArray tracksArray;
    for (int i = 0; i < playlist->count() && i < filePaths.size(); ++i) {
        QListWidgetItem *item = playlist->item(i);

        PlaylistTrack track;
        track.filePath = filePaths.at(i);
        track.title = item->text();

        track.duration = 0;

        tracksArray.append(track.toJson());
    }

    playlistJson["tracks"] = tracksArray;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open playlist file for writing:" << filename;
        return false;
    }

    QJsonDocument doc(playlistJson);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    return true;
}

bool MainWindow::loadPlaylistFromFile(const QString &filename) {

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open playlist file:" << filename;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error in playlist file:" << error.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "Playlist file is not a valid JSON object";
        return false;
    }

    QJsonObject playlistJson = doc.object();
    QString playlistName = playlistJson["name"].toString();

    if (playlistName.isEmpty()) {
        playlistName = QFileInfo(filename).completeBaseName();
    }

    for (int i = 0; i < m_playlistTabs->count(); ++i) {

        if (m_playlistTabs->tabText(i) == playlistName &&
                m_playlistTabs->tabText(i) == "Video Playlist") {
            m_playlistTabs->setCurrentIndex(i);
            updateCurrentPlaylistReference();
            m_playlistFiles[playlistName].clear();
            QListWidget *existingPlaylist = currentPlaylistWidget();

            if (existingPlaylist) {
                existingPlaylist->clear();
            }
            break;
        }

        if (m_playlistTabs->tabText(i) == playlistName) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                        this, "Playlist Exists",
                        QString("Playlist '%1' already exists. Replace it?")
                        .arg(playlistName),
                        QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::No) {
                bool ok;
                playlistName =
                        QInputDialog::getText(this, "Rename Playlist",
                                              "Enter new playlist name:", QLineEdit::Normal,
                                              playlistName + "_copy", &ok);

                if (!ok || playlistName.isEmpty()) {
                    return false;
                }
            } else {
                m_playlistTabs->setCurrentIndex(i);
                updateCurrentPlaylistReference();
                m_playlistFiles[playlistName].clear();
                QListWidget *existingPlaylist = currentPlaylistWidget();

                if (existingPlaylist) {
                    existingPlaylist->clear();
                }
            }
            break;
        }
    }

    if (m_currentPlaylistWidget == nullptr ||
            m_playlistTabs->currentWidget() != m_currentPlaylistWidget) {
        addNewPlaylist(playlistName);
    } else {
        int currentIndex = m_playlistTabs->currentIndex();
        m_playlistTabs->setTabText(currentIndex, playlistName);
    }

    QListWidget *playlist = currentPlaylistWidget();
    playlist->clear();
    m_playlistFiles[playlistName].clear();

    QJsonArray tracksArray = playlistJson["tracks"].toArray();
    foreach (const QJsonValue &trackValue, tracksArray) {
        PlaylistTrack track = PlaylistTrack::fromJson(trackValue.toObject());

        playlist->addItem(track.title);

        m_playlistFiles[playlistName].append(track.filePath);
    }

    updatePlaylistButtonsState();

    return true;
}

void MainWindow::updatePlaylistFromCurrentTab(const QString &filename) {
    QString playlistName = currentPlaylistName();
    if (!playlistName.isEmpty()) {
        savePlaylistToFile(filename, playlistName);
    }
}

void MainWindow::addActions() {
    savePresetAction = new QAction("Save &Preset...", this);
    savePresetAction->setShortcut(QKeySequence::Save);
    savePresetAction->setStatusTip("Save current brainwave settings as preset");
    savePresetAction->setIcon(QIcon(":/icons/save.svg"));

    loadPresetAction = new QAction("&Load Preset...", this);
    loadPresetAction->setShortcut(QKeySequence::Open);
    loadPresetAction->setStatusTip("Load a saved brainwave preset");
    loadPresetAction->setIcon(QIcon(":/icons/folder.svg"));

    managePresetsAction = new QAction("&Manage Presets...", this);
    managePresetsAction->setStatusTip("Open presets folder in file explorer");
    managePresetsAction->setIcon(QIcon(":/icons/settings.svg"));

    openPlaylistAction = new QAction("&Open Playlist...", this);
    openPlaylistAction->setShortcut(Qt::CTRL | Qt::Key_O);
    openPlaylistAction->setStatusTip("Open a saved playlist file");
    openPlaylistAction->setIcon(QIcon(":/icons/music.svg"));

    saveCurrentPlaylistAction = new QAction("&Save Playlist", this);
    saveCurrentPlaylistAction->setShortcut(Qt::CTRL | Qt::Key_S);
    saveCurrentPlaylistAction->setStatusTip("Save current playlist");
    saveCurrentPlaylistAction->setIcon(QIcon(":/icons/copy.svg"));

    saveCurrentPlaylistAsAction = new QAction("Save Playlist &As...", this);
    saveCurrentPlaylistAsAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    saveCurrentPlaylistAsAction->setStatusTip(
                "Save current playlist with a new name");
    saveCurrentPlaylistAsAction->setIcon(QIcon(":/icons/save.svg"));

    saveAllPlaylistsAction = new QAction("Save A&ll Playlists", this);
    saveAllPlaylistsAction->setShortcut(Qt::CTRL | Qt::ALT | Qt::Key_S);
    saveAllPlaylistsAction->setStatusTip("Save all open playlists");
    saveAllPlaylistsAction->setIcon(QIcon(":/icons/copy.svg"));

    quitAction = new QAction("E&xit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setStatusTip("Exit the application");
    quitAction->setIcon(QIcon(":/icons/power.svg"));
}

void MainWindow::setupMenus() {
    QMenu *fileMenu = menuBar()->addMenu("&File");

    fileMenu->addAction(openPlaylistAction);
    fileMenu->addAction(saveCurrentPlaylistAction);
    fileMenu->addAction(saveCurrentPlaylistAsAction);
    fileMenu->addAction(saveAllPlaylistsAction);
    fileMenu->addSeparator();

    QAction *streamAction = fileMenu->addAction("&Stream from URL", this,
                                                &MainWindow::onStreamFromUrl);
    streamAction->setShortcut(QKeySequence("Ctrl+U"));
    streamAction->setIcon(QIcon(":/icons/rss.svg"));
    fileMenu->addAction(streamAction);
    fileMenu->addSeparator();


    fileMenu->addAction(quitAction);

    QMenu *viewMenu = menuBar()->addMenu("&View");

    QAction *hideBinauralToolbarAction =
            new QAction("Hide Binaural Toolbar", viewMenu);
    hideBinauralToolbarAction->setCheckable(true);
    bool isToolbarHidden =
            settings.value("UI/BinauralToolbarHidden", false).toBool();
    hideBinauralToolbarAction->setChecked(isToolbarHidden);
    m_binauralToolbar->setVisible(!isToolbarHidden);
    m_binauralToolbarExt->setVisible(!isToolbarHidden);
    connect(hideBinauralToolbarAction, &QAction::triggered, [this](bool checked) {
        m_binauralToolbar->setVisible(!checked);
        m_binauralToolbarExt->setVisible(!checked);
        toggleTheme(isDarkTheme);
        settings.setValue("UI/BinauralToolbarHidden", checked);
    });
    viewMenu->addAction(hideBinauralToolbarAction);
    viewMenu->addSeparator();

    QAction *hideNatureToolbarAction =
            new QAction("Hide Nature Toolbar", viewMenu);
    hideNatureToolbarAction->setCheckable(true);
    bool isNatureToolbarHidden =
            settings.value("UI/NatureToolbarHidden", false).toBool();
    hideNatureToolbarAction->setChecked(isNatureToolbarHidden);
    m_natureToolbar->setVisible(!isNatureToolbarHidden);
    connect(hideNatureToolbarAction, &QAction::triggered, [this](bool checked) {
        m_natureToolbar->setVisible(!checked);
        toggleTheme(isDarkTheme);

        settings.setValue("UI/NatureToolbarHidden", checked);
    });
    viewMenu->addAction(hideNatureToolbarAction);

    viewMenu->addSeparator();
    enableDarkThemeAction = new QAction("Set Dark Theme", viewMenu);

    enableDarkThemeAction->setCheckable(true);
    enableDarkThemeAction->setChecked(false);
    connect(enableDarkThemeAction, &QAction::triggered, this, [this](bool enableDark){

        isDarkTheme = enableDark;
        settings.setValue("isdarktheme", enableDark);
        toggleTheme(enableDark);
    });
    viewMenu->addAction(enableDarkThemeAction);

    QMenu *settingsMenu = menuBar()->addMenu("&Settings");
    QAction *factoryResetAction = new QAction("Factory Reset", settingsMenu);
    factoryResetAction->setIcon(QIcon(":/icons/refresh-cw.svg"));
    connect(factoryResetAction, &QAction::triggered, [this] {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
                    this, "Factory Reset Confirmation",
                    "Are you sure you want to reset all settings to defaults?\n\n"
                    "This will delete all your preferences and customizations.\n"
                    "Your music files, saved playlists, and presets will NOT be affected.",
                    QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

        if (reply == QMessageBox::Ok) {
            settings.clear();
            QMessageBox::information(
                        this, "Factory Reset Complete",
                        "All settings have been reset to defaults.\n"
                        "Please restart the application for changes to take full effect.");

        }
    });
    settingsMenu->addAction(factoryResetAction);
    settingsMenu->addSeparator();

    unlimitedDurationAction = new QAction("Unlimited Duration", settingsMenu);
    unlimitedDurationAction->setCheckable(true);

    bool unlimited = settings.value("binaural/unlimitedDuration", false).toBool();
    unlimitedDurationAction->setChecked(unlimited);

    settingsMenu->addAction(unlimitedDurationAction);

    QMenu *presetsMenu = menuBar()->addMenu("&Presets");

    presetsMenu->addAction(savePresetAction);
    presetsMenu->addAction(loadPresetAction);
    presetsMenu->addSeparator();

    QAction *resetPresetsAction = new QAction("&Binaural Defaults", this);
    resetPresetsAction->setStatusTip("Reset brainwave settings to defaults");
    resetPresetsAction->setIcon(QIcon(":/icons/target.svg"));
    connect(resetPresetsAction, &QAction::triggered, this, [this]() {
        if (m_binauralEngine && m_binauralEngine->isPlaying()) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(
                        this, tr("Binaural Engine Active"),
                        tr("Binaural engine is playing.\nStop and proceed?"),
                        QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

            if (reply == QMessageBox::Cancel) {
                return; // User canceled, don't proceed
            }

            m_binauralEngine->stop();
        }
        toneTypeCombo->setCurrentIndex(0); // Binaural
        m_leftFreqInput->setValue(360.0);
        m_rightFreqInput->setValue(367.83);
        m_waveformCombo->setCurrentIndex(0); // Sine
        m_pulseFreqLabel->setValue(7.83);
        m_binauralVolumeInput->setValue(15.0);
        updateBinauralBeatDisplay();
        statusBar()->showMessage("Brainwave settings reset to defaults", 3000);
    });
    presetsMenu->addAction(resetPresetsAction);

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("About BinauralPlayer");
    connect(aboutAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::About, this);
        dialog.exec();
    });

    QAction *featuresAction = helpMenu->addAction("Features");
    connect(featuresAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Features, this);
        dialog.exec();
    });

    QAction *instructionsAction = helpMenu->addAction("Instructions");
    connect(instructionsAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Instructions, this);
        dialog.exec();
    });

    QAction *bestPracticesAction = helpMenu->addAction("Best Practices");
    connect(bestPracticesAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::BestPractices, this);
        dialog.exec();
    });

    QAction *whatsNewAction = helpMenu->addAction("What's New");
    connect(whatsNewAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::WhatsNew, this);
        dialog.exec();
    });

    QAction *supportusAction = helpMenu->addAction("Support Us");
    connect(supportusAction, &QAction::triggered, [this]() {
        DonationDialog dialog(this);
        dialog.exec();
    });
}

void MainWindow::updatePlayerStatus(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    QString trackName = fileInfo.fileName();

    QMediaPlayer::PlaybackState state = m_mediaPlayer->playbackState();

    QString status;
    switch (state) {
    case QMediaPlayer::PlayingState:
        status = "Playing: ";
        break;
    case QMediaPlayer::PausedState:
        status = "Paused: ";
        break;
    case QMediaPlayer::StoppedState:
        status = "Stopped";
        statusBar()->showMessage(status);
        return;
    }

    statusBar()->showMessage(status + trackName);
}

QString MainWindow::formatBinauralString() {
    QString result = "";

    int toneType = toneTypeCombo->currentData().toInt();

    switch (toneType) {
    case BINAURAL:
        result = "BIN:"; // Only BIN for binaural
    {
        QString beatText = m_beatFreqLabel->text();
        QString beatValue = beatText.split(" ")[0];
        result += beatValue;
    }
        break;

    case ISOCHRONIC:
        result = "ISO:"; // Only ISO for isochronic
        result += QString("%1").arg(m_pulseFreqLabel->value(), 0, 'f', 1);
        break;

    case GENERATOR:
        result = "GEN:"; // GEN for generator
        result += QString("L:%1/R:%2")
                .arg(m_leftFreqInput->value(), 0, 'f', 2)
                .arg(m_rightFreqInput->value(), 0, 'f', 2);
        break;

    default:
        result = "OFF";
        return result;
    }

    QString waveform = m_waveformCombo->currentText();

    if (waveform == "Sine")
        result += ":Sine";
    else if (waveform == "Square")
        result += ":Sqr";
    else if (waveform == "Triangle")
        result += ":Tri";
    else if (waveform == "Sawtooth")
        result += ":Saw";

    return result;
}


void MainWindow::onStreamFromUrl() {
    bool ok;
    QString url = QInputDialog::getText(
                this, "Stream from URL", "Enter audio stream URL:", QLineEdit::Normal,
                m_currentStreamUrl.isEmpty() ? "https://"
                                             : m_currentStreamUrl, // Default suggestion
                &ok);

    if (ok && !url.isEmpty()) {
        playRemoteStream(url);
    }
}

void MainWindow::playRemoteStream(const QString &urlString) {
    if (!m_mediaPlayer) {
        m_mediaPlayer = new QMediaPlayer(this);
        m_audioOutput = new QAudioOutput(this);
        m_mediaPlayer->setAudioOutput(m_audioOutput);
    }

    QUrl audioUrl(urlString);

    if (!audioUrl.isValid()) {
        statusBar()->showMessage("❌ Invalid URL format", 3000);
        return;
    }

    QMediaPlayer::PlaybackState state = m_mediaPlayer->playbackState();
    if (state == QMediaPlayer::PlayingState ||
            state == QMediaPlayer::PausedState) {
        m_mediaPlayer->stop();
    }

    m_isStream = true;
    m_currentStreamUrl = urlString; // Store for pause/resume

    m_mediaPlayer->setSource(audioUrl);

    m_mediaPlayer->play();

    QString displayName = audioUrl.fileName();
    if (displayName.isEmpty()) {
        displayName = audioUrl.host();
        if (displayName.isEmpty())
            displayName = "Network Stream";
    }

    statusBar()->showMessage("Streaming: " + displayName);
}

void MainWindow::onFileOpened(const QString &filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        return;
    }

    QString suffix = fileInfo.suffix().toLower();
    QStringList supportedExtensions = {"mp3", "wav", "flac", "ogg", "m4a",
                                       "mp4", "m4v", "avi",  "mkv"};

    if (!supportedExtensions.contains(suffix)) {
        statusBar()->showMessage("Unsupported file format: ." + suffix, 3000);
        return;
    }

    QString playlistName = currentPlaylistName();
    QString fileName = fileInfo.fileName();

    currentPlaylistWidget()->addItem(fileName);

    m_playlistFiles[playlistName].append(filePath);


    m_playMusicButton->click();
    statusBar()->showMessage("Opened: " + fileName, 3000);
}

QString MainWindow::getTrackMetadata() {
    metaData = m_mediaPlayer->metaData();

    QString displayMetaData = "Metadata:\n";

    QList<QMediaMetaData::Key> allKeys = {
        QMediaMetaData::Title, QMediaMetaData::Author, QMediaMetaData::Genre,
        QMediaMetaData::Date, QMediaMetaData::Copyright, QMediaMetaData::Comment,

        QMediaMetaData::AlbumTitle, QMediaMetaData::AlbumArtist,
        QMediaMetaData::ContributingArtist, QMediaMetaData::TrackNumber,
        QMediaMetaData::Composer, QMediaMetaData::CoverArtImage,

        QMediaMetaData::Duration, QMediaMetaData::AudioBitRate,
        QMediaMetaData::AudioCodec, QMediaMetaData::FileFormat};

    for (const auto &key : allKeys) {
        QVariant value = metaData.value(key);

        if (!value.isValid() || value.isNull()) {
            continue;
        }

        if (value.typeId() == QMetaType::QString && value.toString().isEmpty()) {
            continue;
        }
        if (value.typeId() == QMetaType::QStringList &&
                value.toStringList().isEmpty()) {
            continue;
        }
        if (value.typeId() == QMetaType::Int && value.toInt() == 0) {
            continue; // Except for TrackNumber which could be 0
        }

        if (key == QMediaMetaData::TrackNumber && value.toInt() == 0) {
            continue;
        }

        QString keyName = QMediaMetaData::metaDataKeyToString(key);
        if (keyName.isEmpty()) {
            keyName = QString("Key_%1").arg(static_cast<int>(key));
        }

        QString formattedKeyName = keyName.toUpper();

        while (formattedKeyName.length() < 16) {
            formattedKeyName += " ";
        }

        QString valueStr;
        if (value.typeId() == QMetaType::QStringList) {
            valueStr = value.toStringList().join(", ");
        } else if (value.typeId() == QMetaType::QDateTime) {
            QDateTime dt = value.toDateTime();
            if (dt.isValid()) {
                valueStr = dt.toString("yyyy-MM-dd");
            } else {
                continue; // Skip invalid dates
            }
        } else if (value.typeId() == QMetaType::QImage) {
            valueStr = "[Image]"; // Just indicate there's image data
        } else if (value.typeId() == QMetaType::Int ||
                   value.typeId() == QMetaType::LongLong) {
            valueStr = value.toString();
        } else if (value.typeId() == QMetaType::Bool) {
            valueStr = value.toBool() ? "true" : "false";
        } else {
            valueStr = value.toString();
        }

        displayMetaData +=
                QString("    %1: %2\n").arg(formattedKeyName).arg(valueStr);
    }

    qint64 durationMs = metaData.value(QMediaMetaData::Duration).toLongLong();
    if (durationMs > 0) {
        int totalSeconds = durationMs / 1000;
        int hours = totalSeconds / 3600;
        int minutes = (totalSeconds % 3600) / 60;
        int seconds = totalSeconds % 60;
        int centiseconds = (durationMs % 1000) / 10;

        displayMetaData += QString("\n  Duration: %1:%2:%3.%4")
                .arg(hours, 2, 10, QChar('0'))
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0'))
                .arg(centiseconds, 2, 10, QChar('0'));
    }

    return displayMetaData.trimmed();
}

void MainWindow::handleMetaDataUpdated() {
    currentTrackMetadata = getTrackMetadata();
    if (trackInfoDialog) {

        metadataBrowser->setText(currentTrackMetadata);

        if (trackInfoDialog->isVisible()) {
            trackInfoDialog->update(); // Force UI refresh
        }
    }
}

void MainWindow::createInfoDialog() {

    trackInfoDialog = new QDialog(this);
    trackInfoDialog->setWindowTitle("Track Information");
    trackInfoDialog->setWindowModality(Qt::NonModal);
    trackInfoDialog->resize(500, 400);

    metadataBrowser = new QTextBrowser(trackInfoDialog);
    metadataBrowser->setReadOnly(true);
    metadataBrowser->setFont(QFont("Monospace", 10));

    QVBoxLayout *layout = new QVBoxLayout(trackInfoDialog);
    layout->addWidget(metadataBrowser);
    trackInfoDialog->setLayout(layout);
    connect(trackInfoDialog, &QDialog::finished, this, [this](int result) {
        Q_UNUSED(result); // We don't care if it was accepted or rejected
        if (m_trackInfoButton) {
            m_trackInfoButton->setChecked(false);
        }
    });
}


void MainWindow::setupAmbientPlayers() {
    for (int i = 1; i <= 5; i++) {
        QString key = QString("player%1").arg(i);

        AmbientPlayer *player = new AmbientPlayer(this);
        player->setName(QString("Player %1").arg(i));

        m_ambientPlayers[key] = player;
        AmbientPlayerDialog *dialog = new AmbientPlayerDialog(player, this);
        dialog->setWindowTitle(QString("Ambient Player %1").arg(i));
        dialog->hide(); // Start hidden
        m_playerDialogs[key] = dialog;

        connect(player->button(), &QPushButton::clicked, this, [this, key]() {
            if (m_playerDialogs.contains(key)) {
                m_playerDialogs[key]->show();
                m_playerDialogs[key]->raise();
                m_playerDialogs[key]->activateWindow();
            }
        });
        player->button()->setProperty("playerKey", key);
    }
}

void MainWindow::onAmbientButtonClicked() {
    QPushButton *clickedButton = qobject_cast<QPushButton *>(sender());
    if (!clickedButton)
        return;

    QString playerKey = clickedButton->property("playerKey").toString();

    if (m_playerDialogs.contains(playerKey)) {
        AmbientPlayerDialog *dialog = m_playerDialogs[playerKey];

        dialog->show();
        dialog->raise();
        dialog->activateWindow();
    }
}

/*
void MainWindow::onMasterPlayClicked()
{
    for (AmbientPlayer* player : std::as_const(m_ambientPlayers)) {
        if (player->isEnabled()) {
            player->play();
        }

    }
}
*/

void MainWindow::onMasterPlayClicked() {
    for (const QString &key : m_ambientPlayers.keys()) {
        AmbientPlayer *player = m_ambientPlayers[key];
        if (!player->isEnabled())
            continue;

        QMediaPlayer *mediaPlayer = player->mediaPlayer();
        if (mediaPlayer &&
                mediaPlayer->playbackState() != QMediaPlayer::PlayingState) {
            mediaPlayer->play(); // ensure actual playback
        }

        if (m_playerDialogs.contains(key)) {
            AmbientPlayerDialog *dlg = m_playerDialogs[key];
            dlg->state = mediaPlayer->playbackState(); // sync dialog state
        }
    }
}

/*
void MainWindow::onMasterPauseClicked()
{
    for (AmbientPlayer* player : std::as_const(m_ambientPlayers)) {
        if (player->isEnabled()) {
            player->pause();
        }
    }
}
*/

void MainWindow::onMasterPauseClicked() {
    for (const QString &key : m_ambientPlayers.keys()) {
        AmbientPlayer *player = m_ambientPlayers[key];
        if (!player->isEnabled())
            continue;

        QMediaPlayer *mediaPlayer = player->mediaPlayer();
        if (mediaPlayer &&
                mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
            mediaPlayer->pause(); // pause actual playback
        }

        if (m_playerDialogs.contains(key)) {
            AmbientPlayerDialog *dlg = m_playerDialogs[key];
            dlg->state = mediaPlayer->playbackState(); // should be PausedState now
        }
    }
}

/*
void MainWindow::onMasterStopClicked()
{
    for (AmbientPlayer* player : std::as_const(m_ambientPlayers)) {
        if (player->isEnabled()) {
            player->stop();
        }
    }
}
*/

void MainWindow::onMasterStopClicked() {
    for (const QString &key : m_ambientPlayers.keys()) {
        AmbientPlayer *player = m_ambientPlayers[key];
        if (!player->isEnabled())
            continue;

        QMediaPlayer *mediaPlayer = player->mediaPlayer();
        if (mediaPlayer &&
                mediaPlayer->playbackState() != QMediaPlayer::StoppedState) {
            mediaPlayer->stop(); // stop actual playback
        }

        if (m_playerDialogs.contains(key)) {
            AmbientPlayerDialog *dlg = m_playerDialogs[key];
            dlg->state = mediaPlayer->playbackState(); // should be StoppedState now
        }
    }
}

void MainWindow::onMasterVolumeChanged(int value) {
    return;
    float masterRatio = value / 100.0f;
    for (AmbientPlayer *player : std::as_const(m_ambientPlayers)) {
        if (player->isEnabled()) {
            int actualVolume = player->volume() * masterRatio;
            player->setVolume(actualVolume); // Overwrites base volume!
        }
    }
    m_masterVolumeLabel->setText(QString("%1%").arg(value));
}

void MainWindow::saveAmbientPreset(const QString &presetName) {

    QString presetDir = ConstantGlobals::ambientPresetFilePath;

    QString defaultFileName = presetName.isEmpty()
            ? "ambient_preset.json"
            : QString("ambient_%1.json").arg(presetName);
    QString defaultFilePath = QDir(presetDir).filePath(defaultFileName);

    QString fileName =
            QFileDialog::getSaveFileName(this, "Save Ambient Preset", defaultFilePath,
                                         "JSON Files (*.json);;All Files (*)");

    if (fileName.isEmpty()) {
        return;
    }

    QString finalPresetName = presetName;
    if (finalPresetName.isEmpty()) {
        finalPresetName = QFileInfo(fileName).baseName();
        if (finalPresetName.startsWith("ambient_")) {
            finalPresetName = finalPresetName.mid(8);
        }
    }

    QJsonObject presetObject;
    presetObject["presetName"] = finalPresetName;
    presetObject["saveTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonArray playersArray;

    for (auto it = m_ambientPlayers.begin(); it != m_ambientPlayers.end(); ++it) {
        AmbientPlayer *player = it.value();

        QJsonObject playerObj;
        playerObj["key"] = it.key();
        playerObj["name"] = player->name();
        playerObj["filePath"] = player->filePath();
        playerObj["volume"] = player->volume();
        playerObj["enabled"] = player->isEnabled();
        playerObj["autoRepeat"] = player->autoRepeat();
        playerObj["playState"] = static_cast<int>(player->playbackState());

        playersArray.append(playerObj);
    }

    presetObject["players"] = playersArray;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(presetObject).toJson());
        file.close();
    } else {
        qWarning() << "Failed to save preset:" << file.errorString();
    }
}

void MainWindow::loadAmbientPreset(const QString &presetName) {
    QString fileName;
    QString presetPath = ConstantGlobals::ambientPresetFilePath;

    if (!presetName.isEmpty()) {
        fileName =
                QDir(presetPath).filePath(QString("ambient_%1.json").arg(presetName));
    } else {
        fileName = QFileDialog::getOpenFileName(
                    this, "Load Ambient Preset", ConstantGlobals::ambientPresetFilePath,
                    "JSON Files (*.json);;All Files (*)");

        if (fileName.isEmpty()) {
            return;
        }
    }

    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Preset not found:" << fileName;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (doc.isNull()) {
        qWarning() << "Invalid JSON in preset:" << fileName;
        return;
    }

    QJsonObject presetObject = doc.object();
    QJsonArray playersArray = presetObject["players"].toArray();

    for (AmbientPlayer *player : m_ambientPlayers) {
        player->stop();
    }

    for (const QJsonValue &playerValue : playersArray) {
        QJsonObject playerObj = playerValue.toObject();
        QString key = playerObj["key"].toString();

        if (m_ambientPlayers.contains(key)) {
            AmbientPlayer *player = m_ambientPlayers[key];

            player->setName(playerObj["name"].toString());
            player->setFilePath(playerObj["filePath"].toString());
            player->setVolume(playerObj["volume"].toInt());
            player->setEnabled(playerObj["enabled"].toBool());
            player->setAutoRepeat(playerObj["autoRepeat"].toBool());

            if (m_playerDialogs.contains(key)) {
                AmbientPlayerDialog *dialog = m_playerDialogs[key];
                dialog->loadPlayerData(); // This refreshes the dialog UI
            }


        }
    }
}

void MainWindow::resetAllPlayersToDefaults() {
    for (auto it = m_ambientPlayers.begin(); it != m_ambientPlayers.end(); ++it) {
        AmbientPlayer *player = it.value();

        player->setFilePath("");   // Clear audio file
        player->setName(it.key()); // Reset name to default (player1, player2, etc.)
        player->setVolume(50);     // Default volume
        player->setEnabled(false); // Enabled: OFF by default
        player->setAutoRepeat(true); // Auto-repeat: ON by default
        player->stop();              // Stop playback

        if (m_playerDialogs.contains(it.key())) {
            m_playerDialogs[it.key()]->loadPlayerData();
        }
    }

}

void MainWindow::saveAmbientPlayersSettings() {
    return;
    settings.beginGroup("AmbientPlayers");

    settings.remove(""); // Remove all in this group

    for (auto it = m_ambientPlayers.begin(); it != m_ambientPlayers.end(); ++it) {
        QString key = it.key(); // "player1", "player2", etc.
        AmbientPlayer *player = it.value();

        settings.beginGroup(key);
        settings.setValue("name", player->name());
        settings.setValue("filePath", player->filePath());
        settings.setValue("volume", player->volume());
        settings.setValue("enabled", player->isEnabled());
        settings.setValue("autoRepeat", player->autoRepeat());
        settings.endGroup();
    }

    settings.endGroup();
    settings.sync(); // Force write to disk
}

void MainWindow::loadAmbientPlayersSettings() {
    return;
    settings.beginGroup("AmbientPlayers");

    QStringList playerKeys =
            settings.childGroups(); // ["player1", "player2", ...]

    for (const QString &key : playerKeys) {
        if (m_ambientPlayers.contains(key)) {
            AmbientPlayer *player = m_ambientPlayers[key];

            settings.beginGroup(key);

            player->setName(settings.value("name", player->name()).toString());

            QString filePath = settings.value("filePath", "").toString();
            player->setFilePath(filePath);

            player->setVolume(settings.value("volume", 50).toInt());
            player->setEnabled(settings.value("enabled", true)
                               .toBool()); // Changed from false to true
            player->setAutoRepeat(settings.value("autoRepeat", true).toBool());

            settings.endGroup();

            if (m_playerDialogs.contains(key)) {
                AmbientPlayerDialog *dialog = m_playerDialogs[key];
                dialog->loadPlayerData();
            }
        }
    }

    settings.endGroup();
}

QStringList MainWindow::getAvailablePresets() const {
    QStringList presets;
    QString presetPath = ConstantGlobals::ambientPresetFilePath;
    QDir presetDir(presetPath);

    if (presetDir.exists()) {
        QStringList filters = {"ambient_*.json"};
        QStringList files = presetDir.entryList(filters, QDir::Files);

        for (const QString &file : files) {
            QString name =
                    file.mid(8, file.length() - 13);
            presets << name;
        }
    }

    return presets;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);
}

void MainWindow::copyUserFiles() {

    bool filesCopied = settings.value("userFilesCopied", false).toBool();
    if (filesCopied) {
        return; // Exit early if already done
    }

    auto copyFileWithPerms = [](const QString &resourcePath,
            const QString &destPath) {
        if (!QFile::exists(destPath)) {
            if (QFile::copy(resourcePath, destPath)) {
                QFile(destPath).setPermissions(QFile::ReadOwner | QFile::WriteOwner |
                                               QFile::ReadGroup | QFile::ReadOther);
                return true;
            }
        }
        return false;
    };

    copyFileWithPerms(":/files/AmbientNatureSounds.txt",
                      ConstantGlobals::ambientFilePath +
                      "/AmbientNatureSounds.txt");

    copyFileWithPerms(":/files/FrequencyList.txt",
                      ConstantGlobals::presetFilePath + "/FrequencyList.txt");

    copyFileWithPerms(":/files/README.txt",
                      ConstantGlobals::ambientFilePath + "/README.txt");

    copyFileWithPerms(":/files/README.txt",
                      ConstantGlobals::presetFilePath + "/README.txt");

    copyFileWithPerms(":/files/MUSIC.txt",
                      ConstantGlobals::musicFilePath + "/MUSIC.txt");

    settings.setValue("userFilesCopied", true);
}

void MainWindow::mutePlayingAmbientPlayers(bool needMute) {
    for (auto it = m_ambientPlayers.begin(); it != m_ambientPlayers.end(); ++it) {
        AmbientPlayer *ambientPlayer = it.value();

        if (ambientPlayer && ambientPlayer->mediaPlayer()) {
            QMediaPlayer *mediaPlayer = ambientPlayer->mediaPlayer();

            if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
                mediaPlayer->audioOutput()->setMuted(needMute);
            }
        }
    }
}


void MainWindow::onSessionStageChanged(int toneType, double leftFreq,
                                       double rightFreq, int waveform,
                                       double pulseFreq, double volume) {
    targetVolume = volume;
    toneTypeCombo->setCurrentIndex(toneType);
    m_waveformCombo->setCurrentIndex(waveform);

    if (toneType == 1) { // ISOCHRONIC
        m_leftFreqInput->setValue(leftFreq);   // Carrier frequency
        m_rightFreqInput->setValue(leftFreq);  // Pulse frequency (for display)
        m_pulseFreqLabel->setValue(pulseFreq); // Pulse in pulse field

        m_beatFreqLabel->setText("__.__");

    } else { // BINAURAL or GENERATOR
        m_leftFreqInput->setValue(leftFreq);
        m_rightFreqInput->setValue(rightFreq);

        double beatFreq = qAbs(rightFreq - leftFreq);
        m_beatFreqLabel->setText(QString("%1 Hz").arg(beatFreq, 0, 'f', 2));

        m_pulseFreqLabel->setValue(7.83);
    }
}

void MainWindow::onSessionStarted(int totalSeconds) {
    m_binauralVolumeInput->setValue(0.0);


    int totalMinutes = totalSeconds / 60;
    m_brainwaveDuration->setValue(totalMinutes);

    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    m_countdownLabel->setText(QString("%1:%2")
                              .arg(minutes, 2, 10, QLatin1Char('0'))
                              .arg(seconds, 2, 10, QLatin1Char('0')));



    m_binauralPlayButton->click();

    m_binauralPlayButton->setEnabled(false); // Session controls playback
    m_binauralStopButton->setEnabled(false); // Allow emergency stop
    m_binauralPauseButton->setEnabled(false);

    statusBar()->showMessage("Session started", 3000);
}

void MainWindow::onSessionEnded() {



    m_binauralPlayButton->setEnabled(true); // Session controls playback
    m_binauralStopButton->setEnabled(true); // Allow emergency stop
    m_binauralPauseButton->setEnabled(true);
    m_binauralStopButton->click();

    if (m_sessionManagerDialog->isSessionActive()) {
        m_sessionManagerDialog->stopSession();
    }
    m_sessionManagerDialog->stopSession();
    m_brainwaveDuration->setValue(45);
    m_countdownLabel->setText("--:--");
    statusBar()->showMessage("Session ended", 3000);
    QTimer::singleShot(5000, this,
                       [this]() { m_binauralVolumeInput->setValue(15.0); });
}

void MainWindow::onSeekTrack() {
    if (!timeEdit || !m_mediaPlayer)
        return;


    QString input = timeEdit->text().trimmed();

    qint64 totalMs = parseTimeStringToMs(input);
    qint64 duration = m_mediaPlayer->duration();

    if (totalMs < 0 || totalMs > duration) {
        timeEdit->clear();
        return;
    }

    m_mediaPlayer->setPosition(totalMs);

    if (m_seekSlider) {
        m_seekSlider->setValue(totalMs);
    }

}

qint64 MainWindow::parseTimeStringToMs(const QString &timeStr) {
    if (timeStr.isEmpty()) {
        return 0; // Empty string = seek to beginning
    }

    QString cleanStr = timeStr.trimmed();
    QStringList parts = cleanStr.split(':');

    if (parts.size() == 2) {
        bool minutesOk, secondsOk;
        int minutes = parts[0].toInt(&minutesOk);
        int seconds = parts[1].toInt(&secondsOk);

        if (minutesOk && secondsOk && minutes >= 0 && minutes < 60 &&
                seconds >= 0 && seconds < 60) {
            return (minutes * 60000) + (seconds * 1000);
        }
    }

    else if (parts.size() == 3) {
        bool hoursOk, minutesOk, secondsOk;
        int hours = parts[0].toInt(&hoursOk);
        int minutes = parts[1].toInt(&minutesOk);
        int seconds = parts[2].toInt(&secondsOk);

        if (hoursOk && minutesOk && secondsOk && hours >= 0 && minutes >= 0 &&
                minutes < 60 && seconds >= 0 && seconds < 60) {
            return (hours * 3600000) + (minutes * 60000) + (seconds * 1000);
        }
    }

    return -1;
}

/*
void MainWindow::onCueTrackSelected(const QString &audioFile, qint64 startMs)
{
    if (!m_mediaPlayer) return;

    if (m_mediaPlayer->source().toLocalFile() != audioFile) {
        m_mediaPlayer->setSource(QUrl::fromLocalFile(audioFile));
        m_mediaPlayer->play();

        QObject::connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this,
            [this, startMs](qint64 pos) {
                QObject::disconnect(m_mediaPlayer,
&QMediaPlayer::positionChanged, this, nullptr);
                m_mediaPlayer->setPosition(startMs);
            });
    } else {
        m_mediaPlayer->setPosition(startMs);
        if (m_mediaPlayer->playbackState() != QMediaPlayer::PlayingState) {
            m_mediaPlayer->play();
        }
    }
}
*/

void MainWindow::onCuePositionChanged(qint64 positionMs) {
    if (!m_seekSlider)
        return;

    m_seekSlider->blockSignals(true);
    m_seekSlider->setValue(positionMs);
    m_seekSlider->blockSignals(false);

    if (m_currentTimeLabel) {
    }
}

void MainWindow::onCueTrackSelected(const QString &audioFile,
                                    qint64 startSeconds) // Now seconds!
{
    if (!m_mediaPlayer)
        return;

    qint64 startMs = startSeconds * 1000;

    if (m_mediaPlayer->source().toLocalFile() != audioFile) {
        m_mediaPlayer->setPosition(startMs);
    } else {
        m_mediaPlayer->setPosition(startMs);

    }
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();

        bool allSupported = true;
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();
            QString suffix = QFileInfo(filePath).suffix().toLower();

            if (!(suffix == "mp3" || suffix == "wav" || suffix == "flac" ||
                  suffix == "ogg" || suffix == "m4a" || suffix == "mp4" ||
                  suffix == "m4v" || suffix == "avi" || suffix == "mkv")) {
                allSupported = false;
                break;
            }
        }

        if (allSupported && !urls.isEmpty()) {
            event->acceptProposedAction();
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QStringList filePaths;

        for (const QUrl &url : urls) {
            filePaths.append(url.toLocalFile());
        }

        processDroppedFiles(filePaths);
    }
    event->acceptProposedAction();
}

void MainWindow::processDroppedFiles(const QStringList &filePaths) {
    if (filePaths.isEmpty())
        return;

    ConstantGlobals::lastMusicDirPath =
            QFileInfo(filePaths.first()).absolutePath();

    QString playlistName = currentPlaylistName();
    QListWidget *playlist = currentPlaylistWidget();

    if (m_isVideoEnabled) {
        for (int i = 0; i < m_playlistTabs->count(); ++i) {
            if (m_playlistTabs->tabText(i) == "Video Playlist") {
                playlist = qobject_cast<QListWidget *>(m_playlistTabs->widget(i));
                playlistName = "Video Playlist";
                m_playlistTabs->setCurrentIndex(i);
                break;
            }
        }
    }

    int addedCount = 0;

    foreach (const QString &filePath, filePaths) {
        QString fileName = QFileInfo(filePath).fileName();

        playlist->addItem(fileName);

        m_playlistFiles[playlistName].append(filePath);

        addedCount++;
    }

    if (playlist->count() > 0) {
        playlist->setCurrentRow(0); // This highlights and selects the first item
        playlist->scrollToTop();    // Optional: Ensure first item is visible
    }

    statusBar()->showMessage(QString("Added %1 file(s) to '%2' via drag & drop")
                             .arg(addedCount)
                             .arg(playlistName));

    /*
  if (m_mediaPlayer && m_mediaPlayer->playbackState() !=
  QMediaPlayer::PlayingState) { if (!filePaths.isEmpty()) {
          m_mediaPlayer->setSource(QUrl::fromLocalFile(filePaths.first()));
          m_mediaPlayer->play();
      }
  }
  */
}

void MainWindow::onVideoContextMenu(const QPoint &pos) {

    QMenu menu(this);


    QAction *playAction = menu.addAction("Play");

    QAction *pauseAction = menu.addAction("Pause");

    QAction *stopAction = menu.addAction("Stop");
    menu.addSeparator();

    QAction *playNextAction = menu.addAction("Next Track");
    QAction *playPreviousAction = menu.addAction("Previous Track");

    menu.addSeparator();

    QAction *increaseVolumeAction = menu.addAction("Volume Up");
    increaseVolumeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Up));
    increaseVolumeAction->setShortcutContext(Qt::ApplicationShortcut);

    QAction *decreaseVolumeAction = menu.addAction("Volume Down");
    decreaseVolumeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Down));
    decreaseVolumeAction->setShortcutContext(Qt::ApplicationShortcut);

    QAction *muteAction = menu.addAction("Mute/Unmute");

    menu.addSeparator();
    QAction* fullscreenAction = menu.addAction(
                videoWidget && videoWidget->isFullScreen() ? "Exit Fullscreen"
                                                           : "Fullscreen");

    menu.addSeparator();

    QMenu *aspectMenu = menu.addMenu("Aspect Ratio");
    QAction* actionKeepAspect = aspectMenu->addAction("Default");
    menu.addSeparator();

    QAction* actionZoom = aspectMenu->addAction("Zoom");
    QAction* actionStretch = aspectMenu->addAction("Stretch");


    connect(actionKeepAspect, &QAction::triggered, this, [this]() {
        if (videoWidget) {
            videoWidget->setAspectRatioMode(Qt::KeepAspectRatio);

            statusBar()->showMessage("Aspect: Default", 2000);
        }
    });

    connect(actionZoom, &QAction::triggered, this, [this]() {
        if (videoWidget) {
            videoWidget->setAspectRatioMode(Qt::KeepAspectRatioByExpanding);

            statusBar()->showMessage("Aspect: Zoom - edges may be cropped", 2000);
        }
    });

    connect(actionStretch, &QAction::triggered, this, [this]() {
        if (videoWidget) {
            videoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);

            statusBar()->showMessage("Aspect: Stretched", 2000);
        }
    });



    menu.addSeparator();

    connect(playAction, &QAction::triggered, this, [this]() {
        if (m_mediaPlayer) {
            if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {

            } else {
                m_playMusicButton->click();
            }
        }
    });

    connect(pauseAction, &QAction::triggered, this, [this]() {
        if (m_mediaPlayer) {
            if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
                m_pauseMusicButton->click();
            } else {
            }
        }
    });

    connect(stopAction, &QAction::triggered, this, [this]() {
        if (m_mediaPlayer) {
            if (m_mediaPlayer->playbackState() == QMediaPlayer::PlayingState ||
                    m_mediaPlayer->playbackState() == QMediaPlayer::PausedState) {
                m_stopMusicButton->click();
            } else {
            }
        }
    });

    connect(playNextAction, &QAction::triggered, this, [this]() {
        if (m_mediaPlayer) {
            playNextTrack();
        }
    });

    connect(playPreviousAction, &QAction::triggered, this, [this]() {
        if (m_mediaPlayer) {
            playPreviousTrack();
        }
    });

    connect(muteAction, &QAction::triggered, this,
            [this]() { volumeIcon->click(); });

    connect(increaseVolumeAction, &QAction::triggered, this, [this]() {
        int currentVolume = m_masterVolumeSlider->value();
        m_masterVolumeSlider->setValue(currentVolume + 1); // Pre-increment
    });

    connect(decreaseVolumeAction, &QAction::triggered, this, [this]() {
        int currentVolume = m_masterVolumeSlider->value();
        m_masterVolumeSlider->setValue(currentVolume - 1); // Pre-decrement
    });

    connect(fullscreenAction, &QAction::triggered, this,
            &MainWindow::toggleFullScreen);

    menu.exec(videoWidget->mapToGlobal(pos));
}


void MainWindow::setupVideoPlayer() {
    videoWidget->setObjectName("videoWidget");

    videoWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    m_videoPlayerContainer = new QWidget();
    m_videoPlayerContainer->setObjectName("videoPlayerContainer");


    QVBoxLayout *containerLayout = new QVBoxLayout(m_videoPlayerContainer);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    containerLayout->addWidget(videoWidget);

    createVideoToolbar();
    containerLayout->addWidget(m_videoToolbar);

    videoPlaylist = new QListWidget();
    videoPlaylist->setObjectName("videoPlaylist");

    int videoPlayerTabIndex = m_playlistTabs->addTab(m_videoPlayerContainer, "Video Player");
    int videoPlayListTabIndex = m_playlistTabs->addTab(videoPlaylist, "Video Playlist");

    m_playlistTabs->tabBar()->setTabButton(videoPlayerTabIndex, QTabBar::RightSide, nullptr);
    m_playlistTabs->tabBar()->setTabButton(videoPlayListTabIndex, QTabBar::RightSide, nullptr);

    m_playlistTabs->tabBar()->setTabVisible(videoPlayerTabIndex, false);
    m_playlistTabs->tabBar()->setTabVisible(videoPlayListTabIndex, false);

    m_videoPlayerContainer->setMouseTracking(true);
    videoWidget->setMouseTracking(true);

    videoWidget->installEventFilter(this);

    m_videoToolbar->show();
}




void MainWindow::createVideoToolbar() {
    m_videoToolbar = new QWidget();
    m_videoToolbar->setObjectName("videoToolbar");
    m_videoToolbar->setFixedHeight(50);

    QHBoxLayout *toolbarLayout = new QHBoxLayout(m_videoToolbar);
    toolbarLayout->setContentsMargins(10, 5, 10, 5);
    toolbarLayout->setSpacing(10);

    m_loadVideoButton = new QPushButton(m_videoToolbar);
    m_loadVideoButton->setObjectName("loadButton");
    m_loadVideoButton->setIcon(QIcon(":/icons-white/folder.svg"));
    m_loadVideoButton->setFixedSize(48, 32);
    m_loadVideoButton->setToolTip("Load Videos");
    m_playButton = new QPushButton(m_videoToolbar);
    m_playButton->setObjectName("playButton");
    m_playButton->setIcon(QIcon(":/icons-white/play.svg"));
    m_playButton->setFixedSize(32, 32);

    m_pauseButton = new QPushButton(m_videoToolbar);
    m_pauseButton->setObjectName("pauseButton");
    m_pauseButton->setIcon(QIcon(":/icons-white/pause.svg"));
    m_pauseButton->setFixedSize(32, 32);

    m_stopButton = new QPushButton(m_videoToolbar);
    m_stopButton->setObjectName("stopButton");
    m_stopButton->setIcon(QIcon(":/icons-white/square.svg"));
    m_stopButton->setFixedSize(32, 32);

  m_vpreviousButton = new QPushButton(m_videoToolbar);
  m_vpreviousButton->setObjectName("previousButton");
  m_vpreviousButton->setIcon(QIcon(":/icons-white/skip-back.svg"));
  m_vpreviousButton->setFixedSize(32, 32);

  m_vnextButton = new QPushButton(m_videoToolbar);
  m_vnextButton->setObjectName("nextButton");
  m_vnextButton->setIcon(QIcon(":/icons-white/skip-forward.svg"));
  m_vnextButton->setFixedSize(32, 32);


    QLabel *m_volLabel = new QLabel("Vol:", m_videoToolbar);
    m_volShowLabel = new QLabel("70%", m_videoToolbar);

    m_vvolumeSlider = new QSlider(Qt::Horizontal);
    m_vvolumeSlider->setObjectName("vvolumeSlider");
    m_vvolumeSlider->setMinimumWidth(150);
    m_vvolumeSlider->setRange(0, 100);
    m_vvolumeSlider->setValue(70);

    m_progressSlider = new QSlider(Qt::Horizontal);
    m_progressSlider->setObjectName("progressSlider");
    m_progressSlider->setRange(0, 100);
    m_progressSlider->setValue(0);

    m_timeLabel = new QLabel("00:00 / 00:00");
    m_timeLabel->setObjectName("timeLabel");
    m_timeLabel->setStyleSheet("color: white; font-size: 12px;");

    m_fullscreenButton = new QPushButton(m_videoToolbar);
    m_fullscreenButton->setObjectName("fullscreenButton");
    m_fullscreenButton->setIcon(QIcon(":/icons-white/maximize-2.svg"));
    m_fullscreenButton->setFixedSize(32, 32);


    toolbarLayout->addWidget(m_loadVideoButton);
    toolbarLayout->addWidget(m_playButton);
    toolbarLayout->addWidget(m_pauseButton);
    toolbarLayout->addWidget(m_stopButton);
    toolbarLayout->addWidget(m_vpreviousButton);
    toolbarLayout->addWidget(m_vnextButton);
    toolbarLayout->addWidget(m_volLabel);

    toolbarLayout->addWidget(m_vvolumeSlider);
    toolbarLayout->addWidget(m_volShowLabel);

    toolbarLayout->addWidget(m_progressSlider, 1); // Give slider stretch factor
    toolbarLayout->addWidget(m_timeLabel);
    toolbarLayout->addWidget(m_fullscreenButton);

    connect(m_playButton, &QPushButton::clicked, this,
            &MainWindow::onPlayClicked);
    connect(m_pauseButton, &QPushButton::clicked, this,
            &MainWindow::onPauseClicked);
    connect(m_stopButton, &QPushButton::clicked, this,
            &MainWindow::onStopClicked);
    connect(m_fullscreenButton, &QPushButton::clicked, this,
            &MainWindow::toggleFullScreen);
    connect(m_progressSlider, &QSlider::sliderReleased, this,
            &MainWindow::onVideoSliderReleased);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this,
            &MainWindow::onVideoPositionChanged);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this,
            &MainWindow::onVideoDurationChanged);
    connect(videoWidget, &QVideoWidget::customContextMenuRequested, this,
            &MainWindow::onVideoContextMenu);
    connect(m_vpreviousButton, &QPushButton::clicked, this,
        &MainWindow::playPreviousTrack);
    connect(m_vnextButton, &QPushButton::clicked, this,
        &MainWindow::playNextTrack);
    connect(m_vvolumeSlider, &QSlider::valueChanged, this,
            &MainWindow::onMusicVolumeChanged);
    connect(m_loadVideoButton, &QPushButton::clicked, this,
            &MainWindow::onLoadMusicClicked);

    QString toolbarStyle =
            "#videoToolbar {"
            "    background-color: rgba(0, 0, 0, 220);" // was 180
            "    border-top: 1px solid rgba(255, 255, 255, 50);"
            "    /* Remove opacity from main toolbar to keep text fully visible */"
            "}"
            "/* Make the entire toolbar thinner by reducing padding */"
            "#videoToolbar {"
            "    padding: 2px 8px; /* Reduced vertical padding */"
            "    spacing: 4px; /* Reduced spacing between items */"
            "}"
            "/* Ensure all QLabel text is white */"
            "#videoToolbar QLabel {"
            "    color: white;"
            "    font-size: 12px;"
            "}"
            "#videoToolbar QPushButton {"
            "    color: white;"
            "    background-color: rgba(255, 255, 255, 30);"
            "    border: none;"
            "    border-radius: 3px; /* Slightly smaller radius for thinner look */"
            "    padding: 4px 8px; /* Reduced button padding */"
            "    font-size: 13px; /* Slightly smaller font */"
            "    min-height: 24px; /* Reduced minimum height */"
            "}"
            "#videoToolbar QPushButton:hover {"
            "    background-color: rgba(255, 255, 255, 50);"
            "}"
            "#videoToolbar QPushButton:pressed {"
            "    background-color: rgba(255, 255, 255, 70);"
            "}"
            "#videoToolbar QSlider {"
            "    height: 18px; /* Reduced from 20px */"
            "}"
            "#videoToolbar QSlider::groove:horizontal {"
            "    height: 3px; /* Thinner groove */"
            "    background: rgba(255, 255, 255, 50);"
            "    border-radius: 1.5px;"
            "}"
            "#videoToolbar QSlider::handle:horizontal {"
            "    background: white;"
            "    width: 10px; /* Smaller handle */"
            "    height: 10px;"
            "    margin: -3.5px 0; /* Adjusted for new height */"
            "    border-radius: 5px;"
            "}";

    m_videoToolbar->setStyleSheet(toolbarStyle);
}

void MainWindow::showVideoToolbar() {
    if (!m_videoToolbar->isVisible()) {
        m_videoToolbar->show();
    }
}



bool MainWindow::eventFilter(QObject *watched, QEvent *event) {

    if (watched == m_flickerContainer) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Escape) {
                toggleFlickerFullscreen();
                return true;
            }
        }
    }

    if (watched == m_videoPlayerContainer || watched == videoWidget ||
            watched == m_videoToolbar) {

        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                if (m_videoToolbar->isVisible()) {
                    m_videoToolbar->hide();
                } else {
                    m_videoToolbar->show();
                }
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::onPlayClicked() { m_playMusicButton->click(); }

void MainWindow::onPauseClicked() { m_pauseMusicButton->click(); }

void MainWindow::onStopClicked() {
    m_stopMusicButton->click();
    m_progressSlider->setValue(0);
    m_timeLabel->setText("00:00 / 00:00");
}

void MainWindow::onFullscreenClicked() {
}

void MainWindow::onVideoPositionChanged(qint64 position) {
    if (m_progressSlider->isSliderDown()) {
        return;
    }

    updateVideoTimeDisplay(position, m_mediaPlayer->duration());

    m_progressSlider->setValue(position / 1000);
}

void MainWindow::onVideoDurationChanged(qint64 duration) {
    m_progressSlider->setMaximum(static_cast<int>(duration / 1000));

    if (duration > 0) {
        updateVideoTimeDisplay(m_mediaPlayer->position(), duration);
    } else {
        m_timeLabel->setText("00:00 / 00:00");
    }
}


void MainWindow::toggleFullScreen() {
    if (videoWidget) {
        if (m_videoPlayerContainer->isFullScreen()) {
            m_videoPlayerContainer->setWindowFlags(Qt::Widget);
            m_videoPlayerContainer->showNormal();

            if (m_videoOriginalTabIndex >= 0) {
                m_videoPlayerContainer->setParent(nullptr);
                m_playlistTabs->insertTab(m_videoOriginalTabIndex, m_videoPlayerContainer, "Video Player");
                m_playlistTabs->setCurrentIndex(m_videoOriginalTabIndex);
                m_playlistTabs->tabBar()->setTabButton(m_videoOriginalTabIndex, QTabBar::RightSide, nullptr);
                m_playlistTabs->tabBar()->setTabVisible(m_videoOriginalTabIndex, true);
            }
            m_videoOriginalTabIndex = -1;
        } else {
            m_videoOriginalTabIndex = m_playlistTabs->currentIndex();
            m_playlistTabs->removeTab(m_videoOriginalTabIndex);
            m_videoPlayerContainer->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
            m_videoPlayerContainer->showFullScreen();
            m_videoToolbar->show();
        }
    }
}

void MainWindow::onVideoSliderReleased() {
    int value = m_progressSlider->value();
    m_mediaPlayer->setPosition(value * 1000);
}

void MainWindow::updateVideoTimeDisplay(qint64 position, qint64 duration) {
    int posSeconds = position / 1000;
    int posMinutes = posSeconds / 60;
    int posSecs = posSeconds % 60;

    int durSeconds = duration / 1000;
    int durMinutes = durSeconds / 60;
    int durSecs = durSeconds % 60;

    QString timeText = QString("%1:%2 / %3:%4")
            .arg(posMinutes, 2, 10, QChar('0'))
            .arg(posSecs, 2, 10, QChar('0'))
            .arg(durMinutes, 2, 10, QChar('0'))
            .arg(durSecs, 2, 10, QChar('0'));

    m_timeLabel->setText(timeText);
}


void MainWindow::setupFlickerTab() {
    m_flickerContainer = new QWidget();
    m_flickerContainer->installEventFilter(this);

    QVBoxLayout *flickerLayout = new QVBoxLayout(m_flickerContainer);
    flickerLayout->setContentsMargins(0, 0, 0, 0);
    flickerLayout->setSpacing(0);

    if (!m_flickerWidget) {
        m_flickerWidget = new FlickerWidget(m_flickerContainer);
        m_flickerWidget->setAttribute(Qt::WA_TranslucentBackground);
        m_flickerWidget->setStyleSheet("background: transparent;");

        connect(m_flickerWidget, &FlickerWidget::toggleFullscreenRequested,
                this, &MainWindow::toggleFlickerFullscreen);
        connect(m_flickerWidget, &FlickerWidget::toggleDialogRequested,
                this, [this]() {
            if (m_visStimDialog) {
                if (m_visStimDialog->isVisible()) {
                    m_visStimDialog->hide();
                } else {
                    if (m_flickerContainer && ! m_flickerContainer->isFullScreen()) {
                    m_visStimDialog->show();
                    m_visStimDialog->raise();
                    }else{
                        }
                }
            }
        });
    } else {
        m_flickerWidget->setParent(m_flickerContainer);
    }

    flickerLayout->addWidget(m_flickerWidget);

    if (!m_visStimDialog) {
        m_visStimDialog = new VisStimDialog(m_flickerWidget, m_flickerContainer);
    }

    m_flickerOriginalTabIndex = m_playlistTabs->addTab(m_flickerContainer, "Visual Stimulation");

    m_playlistTabs->tabBar()->setTabButton(m_flickerOriginalTabIndex, QTabBar::RightSide, nullptr);

    m_playlistTabs->tabBar()->setTabVisible(m_flickerOriginalTabIndex, false);
}


void MainWindow::showFlickerTab() {
    if (m_flickerOriginalTabIndex == -1) {
        setupFlickerTab();
    }

    m_playlistTabs->tabBar()->setTabVisible(m_flickerOriginalTabIndex, true);
    m_playlistTabs->setCurrentIndex(m_flickerOriginalTabIndex);

    if (m_flickerWidget) {
    }
}


void MainWindow::toggleFlickerFullscreen()
{
    if (!m_flickerContainer) return;

    if (m_flickerContainer->isFullScreen()) {
        if (m_visStimDialog) {
            m_visStimDialog->setParent(m_flickerContainer);
        }

        m_flickerContainer->setWindowFlags(Qt::Widget);
        m_flickerContainer->showNormal();

        m_flickerContainer->setParent(m_playlistTabs);
        m_playlistTabs->insertTab(m_flickerOriginalTabIndex, m_flickerContainer, "Visual Stimulation");
        m_playlistTabs->tabBar()->setTabButton(m_flickerOriginalTabIndex, QTabBar::RightSide, nullptr);
        m_playlistTabs->tabBar()->setTabVisible(m_flickerOriginalTabIndex, true);
        m_playlistTabs->setCurrentIndex(m_flickerOriginalTabIndex);

    } else {
        if (m_visStimDialog) {
            m_visStimDialog->setParent(m_flickerContainer);

        }

        m_flickerOriginalTabIndex = m_playlistTabs->currentIndex();
        m_playlistTabs->removeTab(m_flickerOriginalTabIndex);

        m_flickerContainer->setParent(nullptr);
        m_flickerContainer->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        m_flickerContainer->showFullScreen();
    }
}


void MainWindow::toggleTheme(bool enableDark)
{
    if (enableDark) {
        qApp->setStyleSheet("");

        QFile styleFile(":/files/dark-theme.css");
        if (styleFile.open(QFile::ReadOnly)) {
            QString styleSheet = QLatin1String(styleFile.readAll());
            qApp->setStyleSheet(styleSheet);
            styleToolbar(m_mediaToolbar, "#0f0f15");
            styleToolbar(m_binauralToolbar, "#0f0f15");
            styleToolbar(m_binauralToolbarExt, "#0f0f15");
            styleToolbar(m_natureToolbar, "#0f0f15");



            m_loadMusicButton->setIcon(QIcon(":/icons-white/music.svg"));
            tbarOpenPlaylistButton->setIcon(QIcon(":/icons-white/folder.svg"));
            tbarSavePlaylistButton->setIcon(QIcon(":/icons-white/save.svg"));
            tbarSaveAllPlaylistsButton->setIcon(QIcon(":/icons-white/copy.svg"));
            m_previousButton->setIcon(QIcon(":/icons-white/skip-back.svg"));
            m_playMusicButton->setIcon(QIcon(":/icons-white/play.svg"));
            m_pauseMusicButton->setIcon(QIcon(":/icons-white/pause.svg"));
            m_stopMusicButton->setIcon(QIcon(":/icons-white/square.svg"));
            m_nextButton->setIcon(QIcon(":/icons-white/skip-forward.svg"));
            m_shuffleButton->setIcon(QIcon(":/icons-white/shuffle.svg"));
            m_repeatButton->setIcon(QIcon(":/icons-white/repeat.svg"));
            volumeIcon->setIcon(QIcon(":/icons-white/volume-2.svg"));
            timeEditButton->setIcon(QIcon(":/icons-white/edit.svg"));

            tbarOpenPresetButton->setIcon(QIcon(":/icons-white/folder.svg"));
            tbarSavePresetButton->setIcon(QIcon(":/icons-white/save.svg"));
            tbarResetBinauralSettingsButton->setIcon(QIcon(":/icons-white/refresh-cw.svg"));
            m_binauralPlayButton->setIcon(QIcon(":/icons-white/play.svg"));
            m_binauralPauseButton->setIcon(QIcon(":/icons-white/pause.svg"));
            m_binauralStopButton->setIcon(QIcon(":/icons-white/square.svg"));
            m_openSessionManagerButton->setIcon(QIcon(":/icons-white/layers.svg"));
            m_visStimButton->setIcon(QIcon()); // Text button "Visual" - no icon needed
            searchButton->setIcon(QIcon(":/icons-white/edit.svg"));
            ambientTitleLabel->setStyleSheet("QLabel { font-weight: bold; color: #ffffff; }");
            m_masterPauseButton->setStyleSheet(
                            "QPushButton { font-weight: bold; color: #FF8C00; }");
            openAmbientPresetButton->setIcon(QIcon(":/icons-white/folder.svg"));
            saveAmbientPresetButton->setIcon(QIcon(":/icons-white/save.svg"));
            resetPlayersButton->setIcon(QIcon(":/icons-white/refresh-cw.svg"));
            m_beatFreqLabel->setStyleSheet(
                "background-color: #16213e; padding: 2px; border: 1px solid #0f3460; color: #ffffff;");
            statusBar()->showMessage("Dark theme applied", 2000);
        } else {
            qWarning() << "Could not load dark theme file";
            statusBar()->showMessage("Failed to load dark theme", 2000);
        }

        m_countdownLabel->setStyleSheet(
            "background-color: #16213e; padding: 3px; border: 1px solid #0f3460; "
            "border-radius: 3px; color: #7B68EE;");

        if (metadataBrowser) {
            metadataBrowser->setStyleSheet("color: #ffffff; background-color: #0f0f15;");
        }



    } else {
        qApp->setStyleSheet("");



        m_loadMusicButton->setIcon(QIcon(":/icons/music.svg"));
        tbarOpenPlaylistButton->setIcon(QIcon(":/icons/folder.svg"));
        tbarSavePlaylistButton->setIcon(QIcon(":/icons/save.svg"));
        tbarSaveAllPlaylistsButton->setIcon(QIcon(":/icons/copy.svg"));
        m_previousButton->setIcon(QIcon(":/icons/skip-back.svg"));
        m_playMusicButton->setIcon(QIcon(":/icons/play.svg"));
        m_pauseMusicButton->setIcon(QIcon(":/icons/pause.svg"));
        m_stopMusicButton->setIcon(QIcon(":/icons/square.svg"));
        m_nextButton->setIcon(QIcon(":/icons/skip-forward.svg"));
        m_shuffleButton->setIcon(QIcon(":/icons/shuffle.svg"));
        m_repeatButton->setIcon(QIcon(":/icons/repeat.svg"));
        volumeIcon->setIcon(QIcon(":/icons/volume-2.svg"));
        timeEditButton->setIcon(QIcon(":/icons/edit.svg"));
        searchButton->setIcon(QIcon(":/icons/edit.svg"));
        ambientTitleLabel->setStyleSheet("QLabel { font-weight: bold; color: #2E8B57; }");


        tbarOpenPresetButton->setIcon(QIcon(":/icons/folder.svg"));
        tbarSavePresetButton->setIcon(QIcon(":/icons/save.svg"));
        tbarResetBinauralSettingsButton->setIcon(QIcon(":/icons/refresh-cw.svg"));
        m_binauralPlayButton->setIcon(QIcon(":/icons/play.svg"));
        m_binauralPauseButton->setIcon(QIcon(":/icons/pause.svg"));
        m_binauralStopButton->setIcon(QIcon(":/icons/square.svg"));
        m_openSessionManagerButton->setIcon(QIcon(":/icons/layers.svg"));


        m_masterPauseButton->setStyleSheet(
                        "QPushButton { font-weight: bold; color: #FF8C00; }");
        openAmbientPresetButton->setIcon(QIcon(":/icons/folder.svg"));
        saveAmbientPresetButton->setIcon(QIcon(":/icons/save.svg"));
        resetPlayersButton->setIcon(QIcon(":/icons/refresh-cw.svg"));
        m_beatFreqLabel->setStyleSheet(
                        "background-color: #f0f0f0; padding: 2px; border: 1px solid #ccc;");
        styleToolbar(m_mediaToolbar, "#4A90E2");       // Blue
        styleToolbar(m_binauralToolbar, "#7B68EE");    // Purple
        styleToolbar(m_binauralToolbarExt, "#7B68EE"); // Purple
        styleToolbar(m_natureToolbar, "#32CD32");      // Green

        m_countdownLabel->setStyleSheet(
                    "background-color: #f0f0f0; padding: 3px; border: 1px solid #ccc; ");

        if (metadataBrowser) {
            metadataBrowser->setStyleSheet("");  // Reset to default
        }



        statusBar()->showMessage("Light theme restored", 2000);
    }
}
