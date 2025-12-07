#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QHeaderView>
#include <QTableWidget>
#include<QCheckBox>
#include<QTimer>
#include<QAudioOutput>
#include<QStandardItemModel>
#include<QAudioSink>
#include"constants.h"
#include<QRandomGenerator>
#include<QInputDialog>
#include<QDesktopServices>
#include<QMenu>
#include<QMenuBar>
#include<QApplication>
#include"helpmenudialog.h"
#include"donationdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_binauralEngine(new BinauralEngine(this))
    //, m_dynamicEngine(new BinauralEngine(this))
    , m_mediaPlayer(nullptr)
    , m_audioOutput(nullptr)
    , m_currentTrackIndex(-1)
    , m_isShuffle(false)
    , m_isRepeat(false)
    , m_autoStopTimer(nullptr)
    , m_remainingSeconds(0)
    , m_seekSlider(new QSlider(Qt::Horizontal, this))
    , m_currentTimeLabel(new QLabel("00:00", this))
    , m_totalTimeLabel(new QLabel("00:00", this))
    , m_playlistTabs(nullptr)
    , m_currentPlaylistWidget(nullptr)
    , m_currentPlaylistName("")
    , m_isStream(false)
    , m_currentStreamUrl("")
{
    // Window properties
    setWindowTitle("Binaural Media Player");
    setMinimumSize(900, 700);
    setWindowIcon(QIcon(":/favicon/android-chrome-512x512.png"));

    // Initialize audio engines
    //initializeAudioEngines();

    // Create toolbars
    m_mediaToolbar = createMediaToolbar();
    m_binauralToolbar = createBinauralToolbar();
    m_binauralToolbarExt = createBinauralToolbarExt();
    m_natureToolbar = createNatureToolbar();
    m_natureToolbar->setVisible(false);
    // Add toolbars to window
    addToolBar(Qt::TopToolBarArea, m_mediaToolbar);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, m_binauralToolbar);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, m_binauralToolbarExt);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(Qt::TopToolBarArea, m_natureToolbar);

    // Create central widget and layout
    setupLayout();

    // Connect all signals and slots
    //setupConnections();

    // Apply styling
    styleToolbar(m_mediaToolbar, "#4A90E2");      // Blue
    styleToolbar(m_binauralToolbar, "#7B68EE");   // Purple
    styleToolbar(m_binauralToolbarExt, "#7B68EE");   // Purple
    styleToolbar(m_natureToolbar, "#32CD32");     // Green

    // Set initial states
    updateBinauralPowerState(false);
    updateNaturePowerState(false);

    initializeAudioEngines();
    addActions();

    setupMenus();
    createInfoDialog();
    setupConnections();
    model = qobject_cast<QStandardItemModel*>(m_waveformCombo->model());
    squareWaveItem = model->item(1);

    // Status bar
    connect(volumeIcon, &QPushButton::clicked, this, &MainWindow::onMuteButtonClicked);

    // Create permanent widgets for right side (binaural messages)
    m_binauralStatusLabel = new QLabel(this);
    m_binauralStatusLabel->setAlignment(Qt::AlignRight);
    m_binauralStatusLabel->setMinimumWidth(150); // Adjust as needed
    m_binauralStatusLabel->setMaximumWidth(250); // Adjust as needed

    //m_binauralStatusLabel->setStyleSheet("QLabel { padding: 0 5px; color: #555; }");
    m_binauralStatusLabel->setText("Binaural engine disabled");

    // Add to status bar
    statusBar()->addPermanentWidget(m_binauralStatusLabel);

    statusBar()->showMessage("Ready to play");
    showFirstLaunchWarning();
    createInfoDialog();

}

MainWindow::~MainWindow()
{
    // Optional - but good practice to stop explicitly
       if(m_binauralEngine && m_binauralEngine->isPlaying()) {
           m_binauralEngine->stop();
       }
      // if(m_dynamicEngine && m_dynamicEngine->isPlaying()) {
        //   m_dynamicEngine->stop();
       //}
       if(m_mediaPlayer && m_mediaPlayer->isPlaying()) {
               m_mediaPlayer->stop();
           }
}

// =================== TOOLBAR CREATION METHODS ===================

QToolBar *MainWindow::createMediaToolbar()
{
    QToolBar *toolbar = new QToolBar("Media Player", this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    // Label
    QLabel *titleLabel = new QLabel("🎵 MEDIA PLAYER", toolbar);
    toolbar->addWidget(titleLabel);
    toolbar->addSeparator();

    // File control
    m_loadMusicButton = new QPushButton(toolbar);
    m_loadMusicButton->setMinimumWidth(60);
    m_loadMusicButton->setIcon(QIcon(":icons/music.svg"));
    m_loadMusicButton->setToolTip("Load music files");
    toolbar->addWidget(m_loadMusicButton);

    QPushButton *tbarOpenPlaylistButton = new QPushButton(toolbar);
    tbarOpenPlaylistButton->setIcon(QIcon(":/icons/folder.svg"));
    tbarOpenPlaylistButton->setToolTip("Load Playlist");
    connect(tbarOpenPlaylistButton, &QPushButton::clicked, this, &MainWindow::onOpenPlaylistClicked);
    toolbar->addWidget(tbarOpenPlaylistButton);

    QPushButton *tbarSavePlaylistButton = new QPushButton(toolbar);
    tbarSavePlaylistButton->setToolTip("Save Playlist As");
    tbarSavePlaylistButton->setIcon(QIcon(":/icons/save.svg"));

    connect(tbarSavePlaylistButton, &QPushButton::clicked, this, &MainWindow::onSaveCurrentPlaylistAsClicked);
    toolbar->addWidget(tbarSavePlaylistButton);

    QPushButton *tbarSaveAllPlaylistsButton = new QPushButton(toolbar);
    tbarSaveAllPlaylistsButton->setToolTip("Save All Playlists");
    tbarSaveAllPlaylistsButton->setIcon(QIcon(":/icons/copy.svg"));

    connect(tbarSaveAllPlaylistsButton, &QPushButton::clicked, this, &MainWindow::onSaveAllPlaylistsClicked);
    toolbar->addWidget(tbarSaveAllPlaylistsButton);


    toolbar->addSeparator();
    //previous track
    m_previousButton = new QPushButton(toolbar);
    m_previousButton->setIcon(QIcon(":icons/skip-back.svg"));
    m_previousButton->setToolTip("Previous Track");

    toolbar->addWidget(m_previousButton);
    // Playback controls
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

    // Playback modes
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

    // Volume control
    volumeIcon = new QPushButton(toolbar);
    volumeIcon->setCheckable(true);
    volumeIcon->setIcon(QIcon(":/icons/volume-2.svg"));
    //toolbar->addWidget(volumeIcon);

    m_musicVolumeSlider = new QSlider(Qt::Horizontal, toolbar);
    m_musicVolumeSlider->setRange(0, 100);
    m_musicVolumeSlider->setValue(70);
    m_musicVolumeSlider->setMaximumWidth(150);
    m_musicVolumeSlider->setToolTip("Music volume");
    //toolbar->addWidget(m_musicVolumeSlider);

    m_musicVolumeLabel = new QLabel(toolbar);
    //m_musicVolumeLabel->setIcon(QIcon(":/icons/volume-2.svg"));
    m_musicVolumeLabel->setMinimumWidth(40);
    m_musicVolumeLabel->setText(QString("70%"));
    //duration slider
    //toolbar->addWidget(m_seekSlider);
    //toolbar->addWidget(m_currentTimeLabel);
    //toolbar->addWidget(m_totalTimeLabel);
    //toolbar->addSeparator();



    return toolbar;
}

QToolBar *MainWindow::createBinauralToolbar()
{
    QToolBar *toolbar = new QToolBar("Binaural Generator", this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    // Label
    //QLabel *titleLabel = new QLabel("🧠 BINAURAL", toolbar);
    toneTypeCombo = new QComboBox(toolbar);
    toneTypeCombo->addItem("BINAURAL", BINAURAL);
    toneTypeCombo->addItem("ISOCHRONIC", ISOCHRONIC);
    toneTypeCombo->addItem("GENERATOR", GENERATOR);
    // Set default
    toneTypeCombo->setCurrentIndex(0); // Select BINAURAL

    toolbar->addWidget(toneTypeCombo);
    toolbar->addSeparator();

    // Power toggle
    m_binauralPowerButton = new QPushButton("●", toolbar);
    m_binauralPowerButton->setCheckable(true);
    m_binauralPowerButton->setToolTip("Enable/disable binaural tones");
    m_binauralPowerButton->setMaximumWidth(30);
    toolbar->addWidget(m_binauralPowerButton);

    toolbar->addSeparator();

    // Left frequency
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

    // Right frequency
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

    // Beat frequency display
    QLabel *beatLabel = new QLabel("BIN:", toolbar);
    toolbar->addWidget(beatLabel);

    m_beatFreqLabel = new QLabel("7.83 Hz", toolbar);

    m_beatFreqLabel->setMinimumWidth(95);
    m_beatFreqLabel->setAlignment(Qt::AlignLeft);
    m_beatFreqLabel->setStyleSheet("background-color: #f0f0f0; padding: 2px; border: 1px solid #ccc;");
    m_beatFreqLabel->setToolTip("Binaural beat frequency (Right - Left)");
    toolbar->addWidget(m_beatFreqLabel);

    isoPulseLabel = new QLabel("ISO:",toolbar);
    toolbar->addWidget(isoPulseLabel);

    m_pulseFreqLabel = new QDoubleSpinBox(toolbar);
    m_pulseFreqLabel->setRange(0.0, 100.0);
    m_pulseFreqLabel->setValue(7.83);
    m_pulseFreqLabel->setDecimals(2);
    m_pulseFreqLabel->setSuffix(" Hz");
    m_pulseFreqLabel->setDisabled(true);
    toolbar->addWidget(m_pulseFreqLabel);
    toolbar->addSeparator();

    // Waveform selector
    m_waveformCombo = new QComboBox(toolbar);
    m_waveformCombo->addItem("Sine");
    m_waveformCombo->addItem("Square");
    m_waveformCombo->addItem("Triangle");
    m_waveformCombo->addItem("Sawtooth");
    m_waveformCombo->setMaximumWidth(80);
    m_waveformCombo->setToolTip("Waveform type");
    m_waveformCombo->setEnabled(false);
    toolbar->addWidget(m_waveformCombo);

    toolbar->addSeparator();

    // Volume control
    QLabel *volLabel = new QLabel("Vol:", toolbar);
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

    // Playback controls
    /*
    m_binauralPlayButton = new QPushButton(toolbar);
    m_binauralPlayButton->setIcon(QIcon(":/icons/play.svg"));
    m_binauralPlayButton->setMaximumWidth(30);
    m_binauralPlayButton->setToolTip("Start binaural tones");
    m_binauralPlayButton->setEnabled(false);
    toolbar->addWidget(m_binauralPlayButton);

    m_binauralStopButton = new QPushButton(toolbar);
    m_binauralStopButton->setIcon(QIcon(":/icons/square.svg"));

    m_binauralStopButton->setMaximumWidth(30);
    m_binauralStopButton->setToolTip("Stop binaural tones");
    m_binauralStopButton->setEnabled(false);
    toolbar->addWidget(m_binauralStopButton);
    //timer
    toolbar->addSeparator();

    // Duration label
    QLabel *durationLabel = new QLabel("Timer:", toolbar);
    durationLabel->setToolTip("Auto-stop after selected duration");
    toolbar->addWidget(durationLabel);

    // Duration selector
    m_brainwaveDuration = new QSpinBox(toolbar);
    m_brainwaveDuration->setRange(1, 45);
    m_brainwaveDuration->setValue(45);
    m_brainwaveDuration->setSuffix(" min");
    m_brainwaveDuration->setMaximumWidth(70);
    m_brainwaveDuration->setToolTip("Auto-stop brainwave audio after X minutes (1-45)");
    m_brainwaveDuration->setEnabled(false); // Enabled when binaural power is on
    toolbar->addWidget(m_brainwaveDuration);

    // Countdown display (optional, shows remaining time)
    m_countdownLabel = new QLabel("--:--", toolbar);
    m_countdownLabel->setMinimumWidth(50);
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setStyleSheet(
        "background-color: #f0f0f0; padding: 3px; border: 1px solid #ccc; "
        "border-radius: 3px; color: #7B68EE;"
    );
    m_countdownLabel->setToolTip("Time remaining until auto-stop");
    m_countdownLabel->setVisible(true); // Only show when timer is active
    toolbar->addWidget(m_countdownLabel);
    */

    return toolbar;
}


QToolBar *MainWindow::createBinauralToolbarExt()
{
    QToolBar *toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));


    tbarOpenPresetButton = new QPushButton(toolbar);
    tbarOpenPresetButton->setToolTip("Load Preset");
    tbarOpenPresetButton->setIcon(QIcon(":/icons/folder.svg"));

    connect(tbarOpenPresetButton, &QPushButton::clicked, this, &MainWindow::onLoadPresetClicked);
    toolbar->addWidget(tbarOpenPresetButton);

    tbarSavePresetButton = new QPushButton(toolbar);
    tbarSavePresetButton->setToolTip("Save Preset");
    tbarSavePresetButton->setIcon(QIcon(":/icons/save.svg"));

    connect(tbarSavePresetButton, &QPushButton::clicked, this, &MainWindow::onSavePresetClicked);
    toolbar->addWidget(tbarSavePresetButton);
    toolbar->addSeparator();

    // Playback controls
    m_binauralPlayButton = new QPushButton(toolbar);
    m_binauralPlayButton->setMinimumWidth(80);
    m_binauralPlayButton->setIcon(QIcon(":/icons/play.svg"));
    //m_binauralPlayButton->setMaximumWidth(30);
    m_binauralPlayButton->setToolTip("Start binaural tones");
    m_binauralPlayButton->setEnabled(false);
    toolbar->addWidget(m_binauralPlayButton);

    m_binauralStopButton = new QPushButton(toolbar);
    m_binauralStopButton->setIcon(QIcon(":/icons/square.svg"));

    //m_binauralStopButton->setMaximumWidth(30);
    m_binauralStopButton->setMinimumWidth(50);

    m_binauralStopButton->setToolTip("Stop binaural tones");
    m_binauralStopButton->setEnabled(false);
    toolbar->addWidget(m_binauralStopButton);
    //timer
    toolbar->addSeparator();

    // Duration label
    QLabel *durationLabel = new QLabel("Timer:", toolbar);
    durationLabel->setToolTip("Auto-stop after selected duration");
    toolbar->addWidget(durationLabel);

    // Duration selector
    m_brainwaveDuration = new QSpinBox(toolbar);
    m_brainwaveDuration->setRange(1, 45);
    m_brainwaveDuration->setValue(45);
    m_brainwaveDuration->setSuffix(" min");
    m_brainwaveDuration->setMaximumWidth(70);
    m_brainwaveDuration->setToolTip("Auto-stop brainwave audio after X minutes (1-45)");
    m_brainwaveDuration->setEnabled(false); // Enabled when binaural power is on
    toolbar->addWidget(m_brainwaveDuration);

    // Countdown display (optional, shows remaining time)
    m_countdownLabel = new QLabel("--:--", toolbar);
    m_countdownLabel->setMinimumWidth(50);
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setStyleSheet(
        "background-color: #f0f0f0; padding: 3px; border: 1px solid #ccc; "
        "border-radius: 3px; color: #7B68EE;"
    );
    m_countdownLabel->setToolTip("Time remaining until auto-stop");
    m_countdownLabel->setVisible(true); // Only show when timer is active
    toolbar->addWidget(m_countdownLabel);


    return toolbar;
}



QToolBar *MainWindow::createNatureToolbar()
{
    QToolBar *toolbar = new QToolBar("Nature Sounds", this);
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(24, 24));

    // Label
    QLabel *titleLabel = new QLabel("🌳 NATURE", toolbar);
    toolbar->addWidget(titleLabel);
    toolbar->addSeparator();

    // Power toggle
    m_naturePowerButton = new QPushButton("●", toolbar);
    m_naturePowerButton->setCheckable(true);
    m_naturePowerButton->setToolTip("Enable/disable nature sounds");
    m_naturePowerButton->setMaximumWidth(30);
    toolbar->addWidget(m_naturePowerButton);

    toolbar->addSeparator();

    // Nature sound buttons
    m_oceanButton = new QPushButton("Ocean", toolbar);
    m_oceanButton->setToolTip("Ocean waves sound");
    m_oceanButton->setEnabled(false);
    toolbar->addWidget(m_oceanButton);

    m_forestButton = new QPushButton("Forest", toolbar);
    m_forestButton->setToolTip("Forest birds sound");
    m_forestButton->setEnabled(false);
    toolbar->addWidget(m_forestButton);

    m_rainButton = new QPushButton("Rain", toolbar);
    m_rainButton->setToolTip("Rain storm sound");
    m_rainButton->setEnabled(false);
    toolbar->addWidget(m_rainButton);

    m_fireButton = new QPushButton("Fire", toolbar);
    m_fireButton->setToolTip("Crackling fire sound");
    m_fireButton->setEnabled(false);
    toolbar->addWidget(m_fireButton);

    m_streamButton = new QPushButton("Stream", toolbar);
    m_streamButton->setToolTip("Mountain stream sound");
    m_streamButton->setEnabled(false);
    toolbar->addWidget(m_streamButton);

    toolbar->addSeparator();

    // Volume control
    QLabel *volLabel = new QLabel("Vol:", toolbar);
    toolbar->addWidget(volLabel);

    m_natureVolumeInput = new QDoubleSpinBox(toolbar);
    m_natureVolumeInput->setRange(0.0, 100.0);
    m_natureVolumeInput->setValue(40.0);
    m_natureVolumeInput->setDecimals(1);
    m_natureVolumeInput->setSuffix("%");
    m_natureVolumeInput->setMaximumWidth(70);
    m_natureVolumeInput->setToolTip("Nature sounds volume (0-100%)");
    m_natureVolumeInput->setEnabled(false);
    toolbar->addWidget(m_natureVolumeInput);

    toolbar->addSeparator();

    // Stop button
    m_natureStopButton = new QPushButton("Stop", toolbar);
    m_natureStopButton->setToolTip("Stop all nature sounds");
    m_natureStopButton->setEnabled(false);
    toolbar->addWidget(m_natureStopButton);

    return toolbar;
}

// =================== LAYOUT & SETUP METHODS ===================

void MainWindow::setupLayout()
{
    // Central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(5);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Playlist tabs header
    QHBoxLayout *playlistHeaderLayout = new QHBoxLayout();

    //QLabel *playlistTitle = new QLabel("* PLAYLISTS");
    QLabel *playlistTitle = new QLabel("□ PLAYLISTS");
    playlistTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
    playlistHeaderLayout->addWidget(playlistTitle);

    // Playlist management buttons
    addPlaylistBtn = new QPushButton("New");
    //addPlaylistBtn->setIcon(QIcon(":/icons/plus.svg"));
    renamePlaylistBtn = new QPushButton("Rename");
    //renamePlaylistBtn->setIcon(QIcon(":/icons/hash.svg"));
    renamePlaylistBtn->setToolTip("Rename Current Playlist");
    addPlaylistBtn->setMaximumWidth(80);
    addPlaylistBtn->setToolTip("Add New Playlist");
    renamePlaylistBtn->setMaximumWidth(80);
    QLabel *searchLabel = new QLabel("Search: ", this);
    QLineEdit *searchEdit = new QLineEdit(this);
    searchEdit->setMaximumWidth(120);
    searchEdit->setEnabled(false);

    QPushButton *searchButton = new QPushButton(this);
    searchButton->setCheckable(true);
    searchButton->setChecked(false);
    connect(searchButton, &QPushButton::clicked, [this, searchEdit](bool checked){
        if(!checked){
            searchEdit->clear();
        }
        searchEdit->setEnabled(checked);
    });
    searchButton->setToolTip("Search Playlist");
    searchEdit->setToolTip("Search Playlist");

    searchButton->setIcon(QIcon(":/icons/edit.svg"));

    connect(searchEdit, &QLineEdit::textChanged, [this, searchEdit]() {
        QListWidget *playlist = currentPlaylistWidget();
        if (!playlist || !searchEdit->isEnabled()) return;

        QString searchText = searchEdit->text().trimmed();

        // If search is empty, show all items
        if (searchText.isEmpty()) {
            for (int i = 0; i < playlist->count(); ++i) {
                playlist->item(i)->setHidden(false);
            }
            return;
        }

        // Search through playlist items
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

    // Playlist tabs widget (REPLACES m_playlistWidget)
    m_playlistTabs = new QTabWidget();
    m_playlistTabs->setDocumentMode(true);
    m_playlistTabs->setTabsClosable(true);
    m_playlistTabs->setMovable(true);
    mainLayout->addWidget(m_playlistTabs, 1); // Stretch factor 1

    // Add initial playlist
    addNewPlaylist("Default");

    // Playlist buttons (add/remove tracks)
    QHBoxLayout *playlistButtonLayout = new QHBoxLayout();
    playlistButtonLayout->setSpacing(5);

    m_addFilesButton = new QPushButton("Load Music", this);
    m_addFilesButton->setMinimumWidth(80);
    //m_addFilesButton->setIcon(QIcon(":/icons/plus.svg"));
    m_addFilesButton->setToolTip("Load Music Files");
    m_removeTrackButton = new QPushButton(this);
    m_removeTrackButton->setMinimumWidth(60);
    m_removeTrackButton->setIcon(QIcon(":/icons/file-minus.svg"));

    m_removeTrackButton->setToolTip("Remove Selected Track");

    m_clearPlaylistButton = new QPushButton(this);
    m_clearPlaylistButton->setMinimumWidth(60);
    m_clearPlaylistButton->setIcon(QIcon(":/icons/folder-minus.svg"));
    m_clearPlaylistButton->setToolTip("Clear Playlist");

    m_trackInfoButton  = new QPushButton(this);
    m_trackInfoButton->setCheckable(true);
    m_trackInfoButton->setChecked(false);
    //m_trackInfoButton->setMinimumWidth(5);
    //m_trackInfoButton->setMaximumWidth(20);
    m_trackInfoButton->setIcon(QIcon(":/icons/info.svg"));
    m_trackInfoButton->setToolTip("Display metadata on the current track");
    playlistButtonLayout->addWidget(m_addFilesButton);
    playlistButtonLayout->addWidget(m_removeTrackButton);
    playlistButtonLayout->addWidget(m_clearPlaylistButton);

    // Media controls
    playlistButtonLayout->addSpacing(8);
    playlistButtonLayout->addWidget(volumeIcon);
    playlistButtonLayout->addWidget(m_musicVolumeSlider);
    playlistButtonLayout->addWidget(m_musicVolumeLabel);
    playlistButtonLayout->addWidget(m_seekSlider);
    playlistButtonLayout->addWidget(m_currentTimeLabel);
    playlistButtonLayout->addWidget(m_totalTimeLabel);
    playlistButtonLayout->addWidget(m_trackInfoButton);
    mainLayout->addLayout(playlistButtonLayout);

    // Connect new playlist management signals
    //connect(addPlaylistBtn, &QPushButton::clicked, this, &MainWindow::onAddNewPlaylistClicked);
    //connect(renamePlaylistBtn, &QPushButton::clicked, this, &MainWindow::onRenamePlaylistClicked);
    //connect(m_playlistTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::onClosePlaylistTab);
    //connect(m_playlistTabs, &QTabWidget::currentChanged, this, &MainWindow::onPlaylistTabChanged);
}


void MainWindow::setupConnections()
{

    // Media player connections
    connect(m_loadMusicButton, &QPushButton::clicked, this, &MainWindow::onLoadMusicClicked);
    connect(m_playMusicButton, &QPushButton::clicked, this, &MainWindow::onPlayMusicClicked);
    connect(m_pauseMusicButton, &QPushButton::clicked, this, &MainWindow::onPauseMusicClicked);
    connect(m_stopMusicButton, &QPushButton::clicked, this, &MainWindow::onStopMusicClicked);
    connect(m_nextButton, &QPushButton::clicked, this, &MainWindow::playNextTrack);
    connect(m_previousButton, &QPushButton::clicked, this, &MainWindow::playPreviousTrack);
    connect(m_repeatButton, &QPushButton::clicked, this, &MainWindow::onRepeatClicked);

    connect(m_shuffleButton, &QPushButton::clicked, [this](bool checked){
        if(checked){
            m_shuffleButton->setToolTip("Shuffle: ON");
        }else{
            m_shuffleButton->setToolTip("Shuffle: OFF");

        }
        isShuffle = checked;
    });

    connect(m_musicVolumeSlider, &QSlider::valueChanged, this, &MainWindow::onMusicVolumeChanged);
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged,
            this, &MainWindow::onMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, &MainWindow::onPlaybackStateChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred,
            this, &MainWindow::onMediaPlayerError);

    connect(m_mediaPlayer, &QMediaPlayer::durationChanged,
            this, &MainWindow::onDurationChanged);
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
               this, &MainWindow::onPositionChanged);
    // Connect seek slider
    connect(m_seekSlider, &QSlider::sliderMoved,
            this, &MainWindow::onSeekSliderMoved);
    //connect(volumeIcon, &QPushButton::clicked, this, &MainWindow::onMuteButtonClicked);

    connect(m_mediaPlayer, &QMediaPlayer::metaDataChanged,
            this, &MainWindow::handleMetaDataUpdated);

    // Binaural connections
    connect(m_binauralPowerButton, &QPushButton::toggled, this, &MainWindow::onBinauralPowerToggled);
    connect(m_leftFreqInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onLeftFrequencyChanged);
    connect(m_rightFreqInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onRightFrequencyChanged);
    connect(m_waveformCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onWaveformChanged);
    connect(m_binauralVolumeInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onBinauralVolumeChanged);
    connect(m_binauralPlayButton, &QPushButton::clicked, this, &MainWindow::onBinauralPlayClicked);
    connect(m_binauralStopButton, &QPushButton::clicked, this, &MainWindow::onBinauralStopClicked);

    connect(toneTypeCombo, &QComboBox::currentIndexChanged, this, &MainWindow::onToneTypeComboIndexChanged);

    connect(m_pulseFreqLabel, &QDoubleSpinBox::valueChanged, [this](double hz){
        m_binauralEngine->setPulseFrequency(hz);
        m_binauralStatusLabel->setText(formatBinauralString());
    });
    // Nature sounds connections
    connect(m_naturePowerButton, &QPushButton::toggled, this, &MainWindow::onNaturePowerToggled);
    connect(m_oceanButton, &QPushButton::clicked, this, &MainWindow::onOceanClicked);
    connect(m_forestButton, &QPushButton::clicked, this, &MainWindow::onForestClicked);
    connect(m_rainButton, &QPushButton::clicked, this, &MainWindow::onRainClicked);
    connect(m_fireButton, &QPushButton::clicked, this, &MainWindow::onFireClicked);
    connect(m_streamButton, &QPushButton::clicked, this, &MainWindow::onStreamClicked);
    connect(m_natureVolumeInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onNatureVolumeChanged);
    connect(m_natureStopButton, &QPushButton::clicked, this, &MainWindow::onNatureStopClicked);

    // Playlist connections
    connect(m_addFilesButton, &QPushButton::clicked, this, &MainWindow::onAddFilesClicked);
    connect(m_removeTrackButton, &QPushButton::clicked, this, &MainWindow::onRemoveTrackClicked);
    connect(m_clearPlaylistButton, &QPushButton::clicked, this, &MainWindow::onClearPlaylistClicked);

    connect(m_trackInfoButton, &QPushButton::clicked, [this](bool checked){
        if(checked){
            //trackInfoDialog->exec();
            trackInfoDialog->show();
        }else{
            trackInfoDialog->hide();
        }
    });

    // Connect new playlist management signals
    connect(addPlaylistBtn, &QPushButton::clicked, this, &MainWindow::onAddNewPlaylistClicked);
    connect(renamePlaylistBtn, &QPushButton::clicked, this, &MainWindow::onRenamePlaylistClicked);
    connect(m_playlistTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::onClosePlaylistTab);
    connect(m_playlistTabs, &QTabWidget::currentChanged, this, &MainWindow::onPlaylistTabChanged);

    // Binaural engine signal connections
    connect(m_binauralEngine, &BinauralEngine::playbackStarted,
            this, &MainWindow::onBinauralPlaybackStarted);
    connect(m_binauralEngine, &BinauralEngine::playbackStopped,
            this, &MainWindow::onBinauralPlaybackStopped);
    connect(m_binauralEngine, &BinauralEngine::errorOccurred,
            this, &MainWindow::onBinauralError);

    //save-load connections
    connect(savePresetAction, &QAction::triggered, this, &MainWindow::onSavePresetClicked);
    connect(loadPresetAction, &QAction::triggered, this, &MainWindow::onLoadPresetClicked);
    connect(managePresetsAction, &QAction::triggered, this, &MainWindow::onManagePresetsClicked);

    connect(openPlaylistAction, &QAction::triggered, this, &MainWindow::onOpenPlaylistClicked);
    connect(saveCurrentPlaylistAction, &QAction::triggered, this, &MainWindow::onSaveCurrentPlaylistClicked);
    connect(saveCurrentPlaylistAsAction, &QAction::triggered, this, &MainWindow::onSaveCurrentPlaylistAsClicked);
    connect(saveAllPlaylistsAction, &QAction::triggered, this, &MainWindow::onSaveAllPlaylistsClicked);
    connect(quitAction, &QAction::triggered, [this]{
        qApp->quit();
    });
}

void MainWindow::initializeAudioEngines()
{
    // Binaural engine is already created in constructor
    // Media player engine will be initialized when we implement it
    // Nature sounds engine will be initialized when we implement it
    // Initialize media player
    m_mediaPlayer = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);

    // Connect player to audio output (no playlist)
    m_mediaPlayer->setAudioOutput(m_audioOutput);

    m_audioOutput->setVolume(m_musicVolumeSlider->value() / 100.0f);
}

// =================== UI STYLING METHODS ===================

void MainWindow::styleToolbar(QToolBar *toolbar, const QString &color)
{
    QString style = QString(
        "QToolBar {"
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
        "}"
    ).arg(color).arg(QColor(color).darker(120).name());

    toolbar->setStyleSheet(style);
}

void MainWindow::updateBinauralBeatDisplay()
{
    double leftFreq = m_leftFreqInput->value();
    double rightFreq = m_rightFreqInput->value();
    double beatFreq = qAbs(rightFreq - leftFreq);

    m_beatFreqLabel->setText(QString("%1 Hz").arg(beatFreq, 0, 'f', 2));
    //m_beatFreqLabel->setValue(beatFreq);

}

void MainWindow::updateBinauralPowerState(bool enabled)
{
    toneTypeCombo->setEnabled(enabled);
    m_leftFreqInput->setEnabled(enabled);
    m_rightFreqInput->setEnabled(enabled);
    m_waveformCombo->setEnabled(enabled);
    m_binauralVolumeInput->setEnabled(enabled);
    m_binauralPlayButton->setEnabled(enabled);
    m_binauralStopButton->setEnabled(enabled);
    tbarOpenPresetButton->setEnabled(enabled);
    tbarSavePresetButton->setEnabled(enabled);
    // Update power button text
    m_binauralPowerButton->setText(enabled ? "●" : "○");
    m_binauralPowerButton->setStyleSheet(
        enabled ? "color: #00FF00; font-weight: bold;" : "color: #888888;"
    );
    // Enable/disable duration selector
       m_brainwaveDuration->setEnabled(enabled);

       // Reset timer when turning off
       if (!enabled && m_autoStopTimer) {
           stopAutoStopTimer();
       }
}

void MainWindow::updateNaturePowerState(bool enabled)
{
    m_oceanButton->setEnabled(enabled);
    m_forestButton->setEnabled(enabled);
    m_rainButton->setEnabled(enabled);
    m_fireButton->setEnabled(enabled);
    m_streamButton->setEnabled(enabled);
    m_natureVolumeInput->setEnabled(enabled);
    m_natureStopButton->setEnabled(enabled);

    // Update power button text
    m_naturePowerButton->setText(enabled ? "●" : "○");
    m_naturePowerButton->setStyleSheet(
        enabled ? "color: #00FF00; font-weight: bold;" : "color: #888888;"
    );
}

// =================== SLOT IMPLEMENTATIONS ===================

// Media player slots

void MainWindow::onPauseMusicClicked()
{
    //pause for streaming
    if (m_mediaPlayer) {
            // Handle stream pause
            if (m_isStream && !m_currentStreamUrl.isEmpty()) {
                m_mediaPlayer->pause();  // Pause the stream

                QUrl url(m_currentStreamUrl);
                QString displayName = url.fileName();
                if (displayName.isEmpty()) {
                    displayName = url.host();
                    if (displayName.isEmpty()) displayName = "Stream";
                }
                statusBar()->showMessage("Paused: " + displayName);

                return;  // Exit the method - don't check playlist
            }
    }

    // normal playlist track pausing
    if (!m_currentPlaylistName.isEmpty() &&
        m_currentTrackIndex >= 0 &&
        m_currentTrackIndex < m_playlistFiles[m_currentPlaylistName].size()) {

        // Get the file path of the current track
        QString filePath = m_playlistFiles[m_currentPlaylistName].at(m_currentTrackIndex);

        // Pause the player
        m_mediaPlayer->pause();
        m_pauseMusicButton->setToolTip("Paused");
        m_playMusicButton->setToolTip("Resume Track");
        // Update status with the paused track name
        updatePlayerStatus(filePath);

    } else {
        // No track is currently loaded
        m_mediaPlayer->pause();
        statusBar()->showMessage("Paused (no track loaded)");
    }

    //m_mediaPlayer->pause();  // JUST PAUSES
}

void MainWindow::onStopMusicClicked()
{
    if(m_isStream){
    m_isStream = false;
    //m_currentStreamUrl = "";
    }

    if (!m_currentPlaylistName.isEmpty() &&
        m_currentTrackIndex >= 0 &&
        m_currentTrackIndex < m_playlistFiles[m_currentPlaylistName].size()) {

        // Get the file path of the current track
        QString filePath = m_playlistFiles[m_currentPlaylistName].at(m_currentTrackIndex);

        // Pause the player
        m_mediaPlayer->stop();
        m_playMusicButton->setToolTip("Play Track");
        m_stopMusicButton->setToolTip("Stopped");
        m_pauseMusicButton->setToolTip("Pause Playback");
        // Update status with the paused track name
        updatePlayerStatus(filePath);

    } else {
        // No track is currently loaded
        m_mediaPlayer->stop();
        statusBar()->showMessage("Stopped (no track loaded)");
    }
    //m_mediaPlayer->stop();
}

void MainWindow::onMusicVolumeChanged(int value)
{
    // Convert 0-100 slider value to 0.0-1.0 volume range
    float volume = value / 100.0f;

    // Set the volume on audio output
    if (m_audioOutput) {
        m_audioOutput->setVolume(volume);
    }

    // Update the label
    m_musicVolumeLabel->setText(QString("%1%").arg(value));

    // Optional: Show brief feedback
    // statusBar()->showMessage(QString("Volume: %1%").arg(value), 1500);
}


// Binaural slots
void MainWindow::onBinauralPowerToggled(bool checked)
{       // Show warning and check if user wants to proceed
    if (checked) {
    bool proceed = showBinauralWarning();

    if (!proceed) {
        // User canceled - turn the power back off
        m_binauralPowerButton->blockSignals(true);
        m_binauralPowerButton->setChecked(false);
        m_binauralPowerButton->blockSignals(false);
        updateBinauralPowerState(false);
        return;
    }
    }

    updateBinauralPowerState(checked);
    if (!checked) {
        // If turning off, stop binaural engine
        m_binauralEngine->stop();
        toneTypeCombo->setCurrentIndex(0);
        m_waveformCombo->setCurrentIndex(0);

    }else{
        toneTypeCombo->setCurrentIndex(0);
        m_waveformCombo->setCurrentIndex(0);

        //squareWaveItem->setEnabled(false);
    }
    m_binauralStatusLabel->setText(checked ? "Binaural engine enabled" : "Binaural engine disabled");
}

void MainWindow::onLeftFrequencyChanged(double value)
{
    /*
    if (m_binauralEngine->isPlaying()) {
        // Show simple message and exit
        QMessageBox::information(this,
            "Stop Playback First",
            "Please stop playback before changing frequency.");
        return; // Exit method - no frequency change
    }
    */

    m_binauralEngine->setLeftFrequency(value);
    updateBinauralBeatDisplay();
    m_binauralStatusLabel->setText(formatBinauralString());
}

void MainWindow::onRightFrequencyChanged(double value)
{
    /*
    if (m_binauralEngine->isPlaying()) {
        // Show simple message and exit
        QMessageBox::information(this,
            "Stop Playback First",
            "Please stop playback before changing frequency.");
        return; // Exit method - no frequency change
    }
    */

    m_binauralEngine->setRightFrequency(value);
    updateBinauralBeatDisplay();
    m_binauralStatusLabel->setText(formatBinauralString());
}

void MainWindow::onWaveformChanged(int index)
{
    BinauralEngine::Waveform waveform = static_cast<BinauralEngine::Waveform>(index);
    m_binauralEngine->setWaveform(waveform);
    //m_binauralStatusLabel->setText(waveform == BinauralEngine::SINE_WAVE ?
      //                      "Waveform: Sine" : "Waveform: Square");
    m_binauralStatusLabel->setText(formatBinauralString());

}

void MainWindow::onBinauralVolumeChanged(double value)
{
    m_binauralEngine->setVolume(value / 100.0); // Convert percentage to 0.0-1.0
    //m_binauralStatusLabel->setText(QString("Binaural volume: %1%").arg(value));
}

void MainWindow::onBinauralPlayClicked()
{

    if(ConstantGlobals::currentToneType == 1) {
        double leftInputValue = m_leftFreqInput->value();
        m_rightFreqInput->setValue(leftInputValue);
    }
    if (m_binauralEngine->start()) {
        if(ConstantGlobals::currentToneType == 0 || ConstantGlobals::currentToneType == 2){
            m_leftFreqInput->setEnabled(true);
            m_rightFreqInput->setEnabled(true);
        }else{
            m_rightFreqInput->setDisabled(true);

        }
        m_binauralStatusLabel->setText(formatBinauralString());
        startAutoStopTimer();
        //m_leftFreqInput->setDisabled(true);
        //m_rightFreqInput->setDisabled(true);
        m_brainwaveDuration->setDisabled(true);
    }
}

void MainWindow::onBinauralStopClicked()
{
    m_binauralEngine->stop();
    stopAutoStopTimer();
    m_leftFreqInput->setEnabled(true);
    m_rightFreqInput->setEnabled(true);

    m_brainwaveDuration->setEnabled(true);
    if(ConstantGlobals::currentToneType == 1) {
       m_rightFreqInput->setDisabled(true);
    }
    m_binauralStatusLabel->setText("Binaural tones stopped");
}

// Nature sounds slots
void MainWindow::onNaturePowerToggled(bool checked)
{
    updateNaturePowerState(checked);
    statusBar()->showMessage(checked ? "Nature sounds enabled" : "Nature sounds disabled");
}

void MainWindow::onOceanClicked() { statusBar()->showMessage("Playing: Ocean waves"); }
void MainWindow::onForestClicked() { statusBar()->showMessage("Playing: Forest birds"); }
void MainWindow::onRainClicked() { statusBar()->showMessage("Playing: Rain storm"); }
void MainWindow::onFireClicked() { statusBar()->showMessage("Playing: Crackling fire"); }
void MainWindow::onStreamClicked() { statusBar()->showMessage("Playing: Mountain stream"); }

void MainWindow::onNatureVolumeChanged(double value)
{
    // TODO: Connect to nature sounds engine
    statusBar()->showMessage(QString("Nature volume: %1%").arg(value));
}

void MainWindow::onNatureStopClicked()
{
    // TODO: Stop all nature sounds
    statusBar()->showMessage("Nature sounds stopped");
}

// Playlist slots
void MainWindow::onAddFilesClicked() { onLoadMusicClicked(); } // Reuse same functionality

// Binaural engine signal handlers
void MainWindow::onBinauralPlaybackStarted()
{
    //m_binauralPlayButton->setText("⏸");
    //m_binauralPlayButton->setToolTip("Pause binaural tones");
    m_binauralStatusLabel->setText(formatBinauralString());
}

void MainWindow::onBinauralPlaybackStopped()
{
    //m_binauralPlayButton->setText("▶");
    m_binauralPlayButton->setToolTip("Start binaural tones");
    m_binauralStatusLabel->setText("Binaural tones stopped");
}

void MainWindow::onBinauralError(const QString &error)
{
    m_binauralStatusLabel->setText("Error: " + error); // Show for 5 seconds
    QMessageBox::warning(this, "Binaural Engine Error", error);
}

void MainWindow::showFirstLaunchWarning() {
    //QSettings settings;

    // Check if user has already acknowledged
    if (settings.value("firstLaunchWarned", false).toBool()) {
        return; // Already shown and acknowledged
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Welcome to Binaural Media Player");
    msgBox.setIcon(QMessageBox::Information);

    // HTML formatting for better readability
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

    // Add "Don't show again" checkbox
    QCheckBox *dontShowAgain = new QCheckBox("Don't show this message again", &msgBox);
    msgBox.setCheckBox(dontShowAgain);

    // Add OK button
    msgBox.addButton(QMessageBox::Ok);

    // Execute the message box
    msgBox.exec();

    // Save the setting if checkbox is checked
    if (dontShowAgain->isChecked()) {
        settings.setValue("firstLaunchWarned", true);
    }
}

bool MainWindow::showBinauralWarning() {
    //QSettings settings;

    // Check if user has already acknowledged
    if (settings.value("binauralWarned", false).toBool()) {
        return true; // Already shown and acknowledged, proceed
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Binaural & Isochronic Audio");
    msgBox.setIcon(QMessageBox::Information);

    // HTML formatting
    QString message =
        "<div style='margin: 10px;'>"
        "<h3>Brainwave Audio Activation 🧠</h3>"
        "<p>You're about to enable audio tones that may influence brain activity.</p>"

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
        "<li>Consult a doctor if you have heart conditions or other medical concerns</li>"
        "<li>Use at your own discretion</li>"
        "</ul>"
        "</div>";

    msgBox.setTextFormat(Qt::RichText);
    msgBox.setText(message);

    // Add "Don't show again" checkbox
    QCheckBox *dontShowAgain = new QCheckBox("Don't show this warning again", &msgBox);
    msgBox.setCheckBox(dontShowAgain);

    // Add two buttons: OK and Cancel
    QPushButton *okButton = msgBox.addButton("Enable Audio", QMessageBox::AcceptRole);
    QPushButton *cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);
    msgBox.setDefaultButton(okButton);

    // Execute the message box
    msgBox.exec();

    // Check which button was clicked
    bool proceed = (msgBox.clickedButton() == okButton);

    // Save the setting if checkbox is checked AND user clicked OK
    if (proceed && dontShowAgain->isChecked()) {
        settings.setValue("binauralWarned", true);
    }

    return proceed;
}

//timer
void MainWindow::startAutoStopTimer() {
    // Stop any existing timer
    stopAutoStopTimer();

    // Calculate seconds from selected minutes
    m_remainingSeconds = m_brainwaveDuration->value() * 60;

    // Create and start timer
    m_autoStopTimer = new QTimer(this);
    connect(m_autoStopTimer, &QTimer::timeout,
            this, &MainWindow::onAutoStopTimerTimeout);

    m_autoStopTimer->start(1000); // Update every second

    // Show countdown display
    m_countdownLabel->setVisible(true);
    updateCountdownDisplay();

    //m_binauralStatusLabel->setText(QString("Brainwave audio will auto-stop in %1 minutes").arg(m_brainwaveDuration->value()));


}

// Stop the auto-stop timer
void MainWindow::stopAutoStopTimer() {
    if (m_autoStopTimer) {
        m_autoStopTimer->stop();
        delete m_autoStopTimer;
        m_autoStopTimer = nullptr;
    }

    // Hide countdown display
    m_countdownLabel->setVisible(false);
    m_remainingSeconds = 0;
}

// Update countdown display
void MainWindow::updateCountdownDisplay() {
    int minutes = m_remainingSeconds / 60;
    int seconds = m_remainingSeconds % 60;

    QString timeText = QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));

    // Color coding for warnings
    if (minutes == 0) {
        m_countdownLabel->setStyleSheet(
            "background-color: #F8D7DA; color: #721C24; padding: 3px; "
            "border: 1px solid #F5C6CB; border-radius: 3px; font-weight: bold;"
        );
    } else if (minutes <= 5) {
        m_countdownLabel->setStyleSheet(
            "background-color: #FFF3CD; color: #856404; padding: 3px; "
            "border: 1px solid #FFE082; border-radius: 3px;"
        );
    } else {
        m_countdownLabel->setStyleSheet(
            "background-color: #f0f0f0; color: #7B68EE; padding: 3px; "
            "border: 1px solid #ccc; border-radius: 3px;"
        );
    }

    m_countdownLabel->setText(timeText);
}

// Timer timeout handler
void MainWindow::onAutoStopTimerTimeout() {
    if (m_remainingSeconds > 0) {
        m_remainingSeconds--;
        updateCountdownDisplay();

        // Last minute warning
        if (m_remainingSeconds == 60) {
            //m_binauralStatusLabel->setText("1 minute remaining in brainwave session");
        }
    } else {
        // Time's up - stop the audio
        stopAutoStopTimer();

        if (m_binauralEngine && m_binauralEngine->isPlaying()) {
            m_binauralEngine->stop();
            m_binauralStatusLabel->setText("Brainwave session completed");
        }

        // Optional: Show completion message
        QMessageBox::information(this, "Session Complete",
            "Your brainwave session has finished.\n\n"
            "Taking a short break is recommended before starting a new session.");
    }
}

// Duration changed handler
void MainWindow::onBrainwaveDurationChanged(int minutes) {
   // m_binauralStatusLabel->setText(QString("Brainwave audio will auto-stop after %1 minutes").arg(minutes));
}
//mediaplayer
void MainWindow::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::LoadingMedia:
            break;
    case QMediaPlayer::LoadedMedia:
           // Media is now loaded and ready to play
            break;
    case QMediaPlayer::BufferedMedia:
           break;
    case QMediaPlayer::EndOfMedia:
        // Track finished - play next track
        //QTimer::singleShot(100, this, &MainWindow::playNextTrack);
        if(!m_isRepeat){
            if(isShuffle){
                playRandomTrack();
            }else{
                playNextTrack();
            }
        }else{
            if (m_currentTrackIndex >= 0) {
                    m_mediaPlayer->setPosition(0);
                    m_mediaPlayer->play();
                    //statusBar()->showMessage("Replaying track");
                }
        }
        break;
    case QMediaPlayer::InvalidMedia:
        statusBar()->showMessage("Error: Invalid media file", 3000);
        // Optionally skip to next track
        playNextTrack();
        break;
    default:
        break;
    }

}

void MainWindow::onPlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    switch (state) {
       case QMediaPlayer::PlayingState:
           // Playing: can pause, can't play again
           m_playMusicButton->setEnabled(true);
           m_pauseMusicButton->setEnabled(true);
           m_stopMusicButton->setEnabled(true);
           break;

       case QMediaPlayer::PausedState:
           // Paused: can play or stop
           m_playMusicButton->setEnabled(true);
           m_pauseMusicButton->setEnabled(false);
           m_stopMusicButton->setEnabled(true);

           // Store pause position for resume
           m_pausedPosition = m_mediaPlayer->position();
           break;

       case QMediaPlayer::StoppedState:
           // Stopped: can play, can't pause
           m_playMusicButton->setEnabled(true);
           m_pauseMusicButton->setEnabled(false);
           m_stopMusicButton->setEnabled(false);


           break;
       }
}

void MainWindow::onMediaPlayerError(QMediaPlayer::Error error, const QString &errorString)
{
    Q_UNUSED(error);
    statusBar()->showMessage("Media error: " + errorString, 5000);
}

// Helper method for track navigation

void MainWindow::playNextTrack()
{
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (!playlist || playlistName.isEmpty() || m_playlistFiles[playlistName].isEmpty()) return;

    int nextIndex = m_currentTrackIndex + 1;
    if (nextIndex >= m_playlistFiles[playlistName].size()) {
        nextIndex = 0;
    }

    m_currentPlaylistName = playlistName;
    m_currentTrackIndex = nextIndex;
    playlist->setCurrentRow(nextIndex);

    QString filePath = m_playlistFiles[playlistName].at(nextIndex);
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));


    // Extract and display the track name
    QFileInfo fileInfo(filePath);
    QString trackName = fileInfo.fileName();

    QTimer::singleShot(100, this, [this, trackName]() {
        m_mediaPlayer->play();
        //statusBar()->showMessage("Playing: " + trackName);
        updatePlayerStatus(trackName);
    });
}


void MainWindow::onPlayMusicClicked()
{

    if (m_mediaPlayer) {

           // FIRST: Check if we have a paused stream to resume
           if (m_isStream && !m_currentStreamUrl.isEmpty() && m_mediaPlayer->playbackState() == QMediaPlayer::PausedState) {
               m_mediaPlayer->play();  // Resume the paused stream

               QUrl url(m_currentStreamUrl);
               QString name = url.fileName();
               if (name.isEmpty()) name = url.host();
               if (name.isEmpty()) name = "Stream";

               statusBar()->showMessage("▶ Streaming: " + name);
               return;  // Exit - we handled the stream resume
           }
    }

    // Get current playlist and its name
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    // Check if we have a valid playlist with files
    if (!playlist || playlistName.isEmpty() || !m_playlistFiles.contains(playlistName) || m_playlistFiles[playlistName].isEmpty()) {
        statusBar()->showMessage("No music loaded in current playlist", 3000);
        return;
    }

    // GET USER'S SELECTION
    int selectedRow = playlist->currentRow();

    // If user selected a track, use it
    if (selectedRow >= 0 && selectedRow < m_playlistFiles[playlistName].size()) {
        // Update tracking to user's selection BEFORE checking state
        m_currentTrackIndex = selectedRow;
        m_currentPlaylistName = playlistName;
    }
    // If nothing selected, use current or default
    else if (m_currentTrackIndex == -1 || m_currentPlaylistName != playlistName) {
        m_currentTrackIndex = 0;
        m_currentPlaylistName = playlistName;
        playlist->setCurrentRow(0);
    }

    // Get file path from current playlist (AFTER updating tracking)
    QString filePath = m_playlistFiles[playlistName].at(m_currentTrackIndex);

    // Get current playback state
    QMediaPlayer::PlaybackState state = m_mediaPlayer->playbackState();

    if (state == QMediaPlayer::PausedState) {
        // Check if paused track is the SAME as selected track
        QString currentSource = m_mediaPlayer->source().toLocalFile();
        bool sameTrack = (currentSource == filePath);

        if (sameTrack) {
            // Resume same track
            if (m_pausedPosition > 0) {
                m_mediaPlayer->setPosition(m_pausedPosition);
            }
            m_mediaPlayer->play();
            m_playMusicButton->setToolTip("Playing...");
            m_pauseMusicButton->setToolTip("Pause Playback");

            updatePlayerStatus(filePath);
            return;
        }
    }
    else if (state == QMediaPlayer::PlayingState) {
        // Check if currently playing track is the SAME as selected track
        QString currentSource = m_mediaPlayer->source().toLocalFile();
        bool sameTrack = (currentSource == filePath);

        if (sameTrack) {
            // Already playing this track
            return;
        }
    }

    // If we get here, play a different/new track
    m_mediaPlayer->stop();
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    m_mediaPlayer->play();
    m_playMusicButton->setToolTip("Playing...");
    m_stopMusicButton->setToolTip("Stop Playback");
    m_pauseMusicButton->setToolTip("Pause Playback");
    statusBar()->showMessage("Playing: " + QFileInfo(filePath).fileName());
}
// Playlist interaction

void MainWindow::onPlaylistItemClicked(QListWidgetItem *item)
{
    // JUST SELECT, DON'T LOAD
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (!playlist || playlistName.isEmpty()) {
        return;
    }

    int index = playlist->row(item);
    m_currentTrackIndex = index;
    m_currentPlaylistName = playlistName;  // Track which playlist this selection belongs to

    //statusBar()->showMessage(QString("Selected track %1 in '%2'")
      //                       .arg(index + 1)
        //                     .arg(playlistName));
}


void MainWindow::onDurationChanged(qint64 durationMs)
{

    if (durationMs <= 0) {
        m_totalTimeLabel->setText("00:00");
        m_seekSlider->setEnabled(false);
        return;
    }

    // Format duration as MM:SS or HH:MM:SS
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

    // Update UI
    m_totalTimeLabel->setText(durationStr);
    m_seekSlider->setRange(0, totalSeconds);
    m_seekSlider->setEnabled(true);

    // Store duration if needed for playlist
    if (m_currentTrackIndex >= 0) {
        //updatePlaylistDuration(m_currentTrackIndex, durationMs);
    }
}

void MainWindow::onPositionChanged(qint64 positionMs)
{
    // Don't update slider if user is dragging it
    if (m_seekSlider->isSliderDown()) {
        return;
    }

    // Format current position
    int currentSeconds = positionMs / 1000;
    int minutes = (currentSeconds / 60) % 60;
    int seconds = currentSeconds % 60;

    m_currentTimeLabel->setText(
        QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'))
    );

    // Update slider position
    m_seekSlider->setValue(currentSeconds);
}

void MainWindow::onSeekSliderMoved(int value)
{
    // Convert slider value (seconds) to milliseconds
    m_mediaPlayer->setPosition(value * 1000);

    // Update current time label while seeking
    int minutes = value / 60;
    int seconds = value % 60;
    m_currentTimeLabel->setText(
        QString("%1:%2")
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'))
    );
}

void MainWindow::playPreviousTrack()
{
    // Check current playlist
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

    // Calculate previous index
    int prevIndex = m_currentTrackIndex - 1;

    // Handle wrap-around
    if (prevIndex < 0) {
        prevIndex = files.size() - 1; // Go to last track
    }

    // Select and auto-play
    m_currentTrackIndex = prevIndex;

    // Update UI selection in current playlist
    QListWidget *playlist = currentPlaylistWidget();
    if (playlist && m_currentPlaylistName == playlistName) {
        playlist->setCurrentRow(prevIndex);
    }

    // Use the same logic as Play button
    QString filePath = files.at(prevIndex);
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));

    // Extract and display the track name
    QFileInfo fileInfo(filePath);
    QString trackName = fileInfo.fileName();

    // Delay for loading (same 100ms delay as nextTrack)
    QTimer::singleShot(100, this, [this, trackName]() {
        m_mediaPlayer->play();
        //statusBar()->showMessage("Playing: " + trackName);
        updatePlayerStatus(trackName);
    });
}


void MainWindow::onRepeatClicked()
{
    if(m_repeatButton->isChecked()){
        m_isRepeat = true;
        m_repeatButton->setToolTip("Repeat: ON");
    }else{
        m_isRepeat = false;
        m_repeatButton->setToolTip("Repeat: OFF");

    }
}

void MainWindow::onToneTypeComboIndexChanged(int index)
{
    if(m_binauralEngine) m_binauralEngine->stop();


    int toneValue = toneTypeCombo->itemData(index).toInt();

    switch (toneValue) {

    case BINAURAL:
       // squareWaveItem->setEnabled(false);  // ✅ THIS actually enables it
        ConstantGlobals::currentToneType = 0;  // Set to 0
        m_binauralEngine->forceBufferRegeneration();
        m_rightFreqInput->setEnabled(true);
        m_leftFreqInput->setValue(360.00);
        m_rightFreqInput->setValue(367.83);
        m_pulseFreqLabel->setDisabled(true);
        m_binauralStatusLabel->setText(formatBinauralString());
        break;
    case ISOCHRONIC:
        //squareWaveItem->setEnabled(true);  // ✅ THIS actually enables it
        ConstantGlobals::currentToneType = 1;  // Set to 0
        m_binauralEngine->forceBufferRegeneration();
        m_rightFreqInput->setDisabled(true);
        m_pulseFreqLabel->setEnabled(true);
        m_binauralStatusLabel->setText(formatBinauralString());

        break;

    case GENERATOR:
       // squareWaveItem->setEnabled(true);  // ✅ THIS actually enables it
        //item->setForeground(QBrush());  // Reset color
        ConstantGlobals::currentToneType = 2;  // Set to 0
        m_binauralEngine->forceBufferRegeneration();
        m_rightFreqInput->setEnabled(true);
        m_pulseFreqLabel->setDisabled(true);
        m_binauralStatusLabel->setText(formatBinauralString());

        break;

    default:
        break;
    }
}

void MainWindow::onMuteButtonClicked()
{
    bool checked = volumeIcon->isChecked();
    QAudioSink* binauralOutput = m_binauralEngine ?
                                       m_binauralEngine->audioOutput() : nullptr;
    if(checked) {

        if(binauralOutput){
            binEngineVolume = binauralOutput->volume();
            binauralOutput->setVolume(0);
            m_binauralStopButton->setDisabled(true);

        }
        volumeIcon->setIcon(QIcon(":/icons/volume-x.svg"));
        m_audioOutput->setMuted(checked);
        m_stopMusicButton->setDisabled(true);
    }else{
        volumeIcon->setIcon(QIcon(":/icons/volume-2.svg"));
        m_audioOutput->setMuted(checked);

        m_stopMusicButton->setEnabled(true);
        if(binauralOutput){
            binauralOutput->setVolume(binEngineVolume);
            m_binauralStopButton->setEnabled(true);

        }

        }

}

//shuffle


void MainWindow::playRandomTrack()
{
    // Check current playlist
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

    // Generate random index
    int randomIndex;
    do {
        randomIndex = QRandomGenerator::global()->bounded(files.size());
    } while (randomIndex == m_currentTrackIndex && files.size() > 1);

    // Select and auto-play
    m_currentTrackIndex = randomIndex;

    // Update UI selection in current playlist
    QListWidget *playlist = currentPlaylistWidget();
    if (playlist && m_currentPlaylistName == playlistName) {
        playlist->setCurrentRow(randomIndex);
    }

    // Use the same logic as Play button
    QString filePath = files.at(randomIndex);
    m_mediaPlayer->setSource(QUrl::fromLocalFile(filePath));

    // Delay for loading
    QTimer::singleShot(100, this, [this]() {
        m_mediaPlayer->play();
        statusBar()->showMessage("Playing random track...");
    });
}

//tabbedwidget

QListWidget* MainWindow::currentPlaylistWidget() const {
    //if (m_playlistTabs->count() == 0) return nullptr;
    //return qobject_cast<QListWidget*>(m_playlistTabs->currentWidget());
    if (!m_playlistTabs || m_playlistTabs->count() == 0) return nullptr;
        QWidget *widget = m_playlistTabs->currentWidget();
        if (!widget) return nullptr;
        return qobject_cast<QListWidget*>(widget);
}

QString MainWindow::currentPlaylistName() const {
    if (m_playlistTabs->count() == 0) return QString();
    return m_playlistTabs->tabText(m_playlistTabs->currentIndex());
}

void MainWindow::addNewPlaylist(const QString &name) {
    QListWidget *newPlaylist = new QListWidget();
    newPlaylist->setAlternatingRowColors(true);
    newPlaylist->setSelectionMode(QAbstractItemView::SingleSelection);
    // Initialize empty playlist in data structure
    QString playlistName = name.isEmpty() ?
    QString("Playlist %1").arg(m_playlistTabs->count() + 1) : name;

    m_playlistFiles[playlistName] = QStringList();
    m_playlistTabs->addTab(newPlaylist, playlistName);
    m_playlistTabs->setCurrentWidget(newPlaylist);

    // Connect signals for the new playlist
    connect(newPlaylist, &QListWidget::itemClicked,
            this, &MainWindow::onPlaylistItemClicked);
    connect(newPlaylist, &QListWidget::itemDoubleClicked,
            this, &MainWindow::onPlaylistItemDoubleClicked);

    updateCurrentPlaylistReference();

    if (m_currentPlaylistName.isEmpty()) {
            m_currentPlaylistName = playlistName;
        }
}

void MainWindow::updateCurrentPlaylistReference() {
    // Keep m_currentPlaylistWidget pointing to current tab for compatibility
    m_currentPlaylistWidget = currentPlaylistWidget();
}

void MainWindow::onAddNewPlaylistClicked() {
    bool ok;
    QString name = QInputDialog::getText(this, "New Playlist",
                                        "Enter playlist name:",
                                        QLineEdit::Normal,
                                        QString("Playlist %1").arg(m_playlistTabs->count() + 1),
                                        &ok);
    if (ok && !name.isEmpty()) {
        addNewPlaylist(name);
        statusBar()->showMessage("Created new playlist: " + name);
    }
}

void MainWindow::onRenamePlaylistClicked() {
    QListWidget *widget = currentPlaylistWidget();
    if (!widget) return;

    QString oldName = currentPlaylistName();
    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Playlist",
                                           "Enter new name:",
                                           QLineEdit::Normal,
                                           oldName, &ok);
    if (ok && !newName.isEmpty() && newName != oldName) {
        // Update tab text
        int currentIndex = m_playlistTabs->currentIndex();
        m_playlistTabs->setTabText(currentIndex, newName);

        // Update data structure
        if (m_playlistFiles.contains(oldName)) {
            m_playlistFiles[newName] = m_playlistFiles.take(oldName);
        }

        // Update current playlist if needed
        if (m_currentPlaylistName == oldName) {
            m_currentPlaylistName = newName;
        }

        statusBar()->showMessage("Renamed playlist to: " + newName);
    }
}

void MainWindow::onClosePlaylistTab(int index) {
    if (m_playlistTabs->count() <= 1) {
        QMessageBox::warning(this, "Cannot Close", "You must have at least one playlist.");
        return;
    }

    QString playlistName = m_playlistTabs->tabText(index);

    // Confirm if playlist has tracks
    QListWidget *playlist = qobject_cast<QListWidget*>(m_playlistTabs->widget(index));
    if (playlist && playlist->count() > 0) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Close Playlist",
                                     QString("Playlist '%1' contains %2 tracks. Close anyway?")
                                     .arg(playlistName).arg(playlist->count()),
                                     QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) return;
    }

    // Remove from data structures
    m_playlistFiles.remove(playlistName);

    // Remove the tab
    m_playlistTabs->removeTab(index);

    updateCurrentPlaylistReference();
    statusBar()->showMessage("Closed playlist: " + playlistName);
}

void MainWindow::onPlaylistTabChanged(int index) {
    updateCurrentPlaylistReference();
    //updatePlaylistButtonsState();
    // Clear current track selection when switching playlists
       m_currentTrackIndex = -1;

       // Update button states
       updatePlaylistButtonsState();

       // Show status message
       QString playlistName = currentPlaylistName();
       if (!playlistName.isEmpty()) {
           int trackCount = m_playlistFiles.value(playlistName).size();
           statusBar()->showMessage(
               QString("Switched to '%1' (%2 tracks)").arg(playlistName).arg(trackCount),
               2000
           );
       }
}

// In onLoadMusicClicked():
void MainWindow::onLoadMusicClicked()
{
    QString filter = "All Supported Media (*.mp3 *.wav *.flac *.ogg *.m4a *.mp4 *.m4v *.avi *.mkv);;"
                         "Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a);;"
                         "Video Files (*.mp4 *.m4v *.avi *.mkv);;"
                         "All Files (*.*)";
    QStringList files = QFileDialog::getOpenFileNames(this,
        "Select Music Files",
        ConstantGlobals::musicFilePath,
        //"Audio Files (*.mp3 *.wav *.flac *.ogg *.m4a)");
        filter);
    if (!files.isEmpty()) {
        QString playlistName = currentPlaylistName();
        foreach (const QString &file, files) {
            QString fileName = QFileInfo(file).fileName();
            currentPlaylistWidget()->addItem(fileName);
            m_playlistFiles[playlistName].append(file);
        }
        statusBar()->showMessage(QString("Added %1 file(s) to '%2'").arg(files.size()).arg(playlistName));
    }
}

void MainWindow::onRemoveTrackClicked()
{
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (!playlist || playlistName.isEmpty()) return;

    int selectedRow = playlist->currentRow();
    if (selectedRow >= 0 && selectedRow < m_playlistFiles[playlistName].size()) {
        // Remove from UI
        QListWidgetItem *item = playlist->takeItem(selectedRow);
        delete item;

        // Remove from data structure
        m_playlistFiles[playlistName].removeAt(selectedRow);

        // Fix current track index if needed
        if (playlistName == m_currentPlaylistName) {
            if (selectedRow < m_currentTrackIndex) {
                m_currentTrackIndex--;
            } else if (selectedRow == m_currentTrackIndex) {
                m_currentTrackIndex = -1;
            }
        }

        statusBar()->showMessage("Track removed from playlist");
    } else {
        statusBar()->showMessage("No track selected");
    }
}

void MainWindow::onClearPlaylistClicked()
{
    QListWidget *playlist = currentPlaylistWidget();
    QString playlistName = currentPlaylistName();

    if (!playlist || playlistName.isEmpty()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Clear Playlist",
                                 QString("Clear all tracks from '%1'?").arg(playlistName),
                                 QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        playlist->clear();
        m_playlistFiles[playlistName].clear();

        // If this was the current playlist, reset track index
        if (playlistName == m_currentPlaylistName) {
            m_currentTrackIndex = -1;
        }

        statusBar()->showMessage("Playlist cleared");
    }
}

// In onPlaylistItemDoubleClicked():

void MainWindow::onPlaylistItemDoubleClicked(QListWidgetItem *item)
{
    //QListWidget *playlist = currentPlaylistWidget();

    // Get the playlist that sent the signal
        QListWidget *playlist = qobject_cast<QListWidget*>(sender());
        if (!playlist) {
            playlist = currentPlaylistWidget();
        }

    QString playlistName = currentPlaylistName();

    if (!playlist || playlistName.isEmpty()) return;

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
        //updatePlayerStatus(filePath);
    }
}

void MainWindow::updatePlaylistButtonsState() {
    QListWidget *playlist = currentPlaylistWidget();
    //bool hasSelection = playlist && playlist->currentItem();
    bool hasSelection = playlist && !playlist->selectedItems().isEmpty();
    bool hasItems = playlist && playlist->count() > 0;

    m_removeTrackButton->setEnabled(hasSelection);
    m_clearPlaylistButton->setEnabled(hasItems);
}

////////////////////save-load

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

MainWindow::BrainwavePreset MainWindow::BrainwavePreset::fromJson(const QJsonObject &json) {
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
    return !name.isEmpty() &&
           toneType >= 0 && toneType <= 2 &&
           leftFrequency >= 20.0 && leftFrequency <= 20000.0 &&
           rightFrequency >= 20.0 && rightFrequency <= 20000.0 &&
           waveform >= 0 && waveform <= 3 &&
           pulseFrequency >= 0.0 && pulseFrequency <= 100.0 &&
           volume >= 0.0 && volume <= 100.0;
}

// PlaylistTrack methods
QJsonObject MainWindow::PlaylistTrack::toJson() const {
    QJsonObject json;
    json["filePath"] = filePath;
    json["title"] = title;
    json["duration"] = duration;
    return json;
}

MainWindow::PlaylistTrack MainWindow::PlaylistTrack::fromJson(const QJsonObject &json) {
    PlaylistTrack track;
    track.filePath = json["filePath"].toString();
    track.title = json["title"].toString();
    track.duration = json["duration"].toVariant().toLongLong();
    return track;
}

// =================== HELPER METHODS ===================

QString MainWindow::generateDefaultPresetName() const {
    QString toneType;
    switch(ConstantGlobals::currentToneType) {
        case 0: toneType = "BIN"; break;
        case 1: toneType = "ISO"; break;
        case 2: toneType = "GEN"; break;
        default: toneType = "PRESET"; break;
    }

    if (ConstantGlobals::currentToneType == 0) {
        // Binaural: BIN-7.83Hz
        double beatFreq = qAbs(m_rightFreqInput->value() - m_leftFreqInput->value());
        return QString("%1-%2Hz").arg(toneType).arg(beatFreq, 0, 'f', 2);
    } else if (ConstantGlobals::currentToneType == 1) {
        // Isochronic: ISO-10.00Hz
        return QString("%1-%2Hz").arg(toneType).arg(m_pulseFreqLabel->value(), 0, 'f', 2);
    } else {
        // Generator: GEN-200.00Hz
        return QString("%1-%2Hz").arg(toneType).arg(m_leftFreqInput->value(), 0, 'f', 2);
    }
}

bool MainWindow::ensureDirectoryExists(const QString &path) {
    QDir dir(path);
    if (!dir.exists()) {
        return dir.mkpath(".");
    }
    return true;
}

// =================== PRESET OPERATIONS ===================

void MainWindow::onSavePresetClicked() {
    // Generate default name based on current settings
    QString defaultName = generateDefaultPresetName();

    // Ask user for preset name
    bool ok;
    QString presetName = QInputDialog::getText(
        this,
        "Save Brainwave Preset",
        "Enter preset name:",
        QLineEdit::Normal,
        defaultName,
        &ok
    );

    if (!ok || presetName.isEmpty()) {
        return;
    }

    // Create preset from current UI state
    BrainwavePreset preset;
    preset.name = presetName;
    preset.toneType = ConstantGlobals::currentToneType;
    preset.leftFrequency = m_leftFreqInput->value();
    preset.rightFrequency = m_rightFreqInput->value();
    preset.waveform = m_waveformCombo->currentIndex();
    preset.pulseFrequency = m_pulseFreqLabel->value();
    preset.volume = m_binauralVolumeInput->value();

    // Validate
    if (!preset.isValid()) {
        QMessageBox::warning(this, "Invalid Preset",
            "Current settings are invalid. Please check frequency ranges.");
        return;
    }

    // Ensure preset directory exists
    if (!ensureDirectoryExists(ConstantGlobals::presetFilePath)) {
        QMessageBox::warning(this, "Save Error",
            "Cannot create presets directory.");
        return;
    }

    // Save to file
    QString filename = ConstantGlobals::presetFilePath + "/" + presetName + ".json";
    if (savePresetToFile(filename, preset)) {
        statusBar()->showMessage("Preset saved: " + presetName, 3000);
    } else {
        QMessageBox::warning(this, "Save Error",
            "Failed to save preset to file.");
    }
}

void MainWindow::onLoadPresetClicked() {
    // Get list of preset files
    QDir presetDir(ConstantGlobals::presetFilePath + "/");
    QStringList presetFiles = presetDir.entryList({"*.json"}, QDir::Files);

    if (presetFiles.isEmpty()) {
        QMessageBox::information(this, "No Presets",
            "No saved presets found in:\n" + ConstantGlobals::presetFilePath);
        return;
    }

    // Show selection dialog
    QStringList presetNames;
    foreach (const QString &file, presetFiles) {
        presetNames << QFileInfo(file).completeBaseName();
    }

    bool ok;
    QString selectedPreset = QInputDialog::getItem(
        this,
        "Load Preset",
        "Select preset to load:",
        presetNames,
        0,
        false,
        &ok
    );

    if (!ok || selectedPreset.isEmpty()) {
        return;
    }

    // Load preset
    QString filename = ConstantGlobals::presetFilePath + "/" + selectedPreset + ".json";
    BrainwavePreset preset = loadPresetFromFile(filename);

    if (!preset.isValid()) {
        QMessageBox::warning(this, "Load Error",
            "Failed to load preset or preset is invalid.");
        return;
    }

    // Apply preset to UI
    bool wasPlaying = m_binauralEngine->isPlaying();
    if (wasPlaying) {
        m_binauralEngine->stop();
    }

    // Update UI
    ConstantGlobals::currentToneType = preset.toneType;
    toneTypeCombo->setCurrentIndex(preset.toneType);
    m_leftFreqInput->setValue(preset.leftFrequency);
    m_rightFreqInput->setValue(preset.rightFrequency);
    m_waveformCombo->setCurrentIndex(preset.waveform);
    m_pulseFreqLabel->setValue(preset.pulseFrequency);
    m_binauralVolumeInput->setValue(preset.volume);

    // Update display
    updateBinauralBeatDisplay();

    if (wasPlaying) {
        // Restart with new settings
        onBinauralPlayClicked();
    }

    statusBar()->showMessage("Preset loaded: " + preset.name, 3000);
}

void MainWindow::onManagePresetsClicked() {
    // Open preset folder in system file explorer
    QUrl presetUrl = QUrl::fromLocalFile(ConstantGlobals::presetFilePath);
    QDesktopServices::openUrl(presetUrl);
}

bool MainWindow::savePresetToFile(const QString &filename, const BrainwavePreset &preset) {
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

MainWindow::BrainwavePreset MainWindow::loadPresetFromFile(const QString &filename) {
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

// =================== PLAYLIST OPERATIONS ===================

void MainWindow::onOpenPlaylistClicked() {
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Open Playlist",
        ConstantGlobals::playlistFilePath,
        "Playlist Files (*.json);;All Files (*)"
    );

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
    QString playlistName = currentPlaylistName();
    if (playlistName.isEmpty()) {
        QMessageBox::warning(this, "Save Error", "No active playlist.");
        return;
    }

    // Ensure playlist directory exists
    if (!ensureDirectoryExists(ConstantGlobals::playlistFilePath)) {
        QMessageBox::warning(this, "Save Error",
            "Cannot create playlists directory.");
        return;
    }

    QString filename = ConstantGlobals::playlistFilePath + "/"+ playlistName + ".json";
    if (savePlaylistToFile(filename, playlistName)) {
        statusBar()->showMessage("Playlist saved: " + playlistName, 3000);
    } else {
        QMessageBox::warning(this, "Save Error",
            "Failed to save playlist to file.");
    }
}

void MainWindow::onSaveCurrentPlaylistAsClicked() {
    QString playlistName = currentPlaylistName();
    if (playlistName.isEmpty()) {
        QMessageBox::warning(this, "Save Error", "No active playlist.");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Playlist As",
        ConstantGlobals::playlistFilePath + "/" + playlistName + ".json",
        "Playlist Files (*.json);;All Files (*)"
    );

    if (filename.isEmpty()) {
        return;
    }

    if (savePlaylistToFile(filename, playlistName)) {
        statusBar()->showMessage("Playlist saved as: " + QFileInfo(filename).fileName(), 3000);
    } else {
        QMessageBox::warning(this, "Save Error",
            "Failed to save playlist to file.");
    }
}

void MainWindow::onSaveAllPlaylistsClicked() {
    int successCount = 0;
    int failCount = 0;

    // Ensure playlist directory exists
    if (!ensureDirectoryExists(ConstantGlobals::playlistFilePath)) {
        QMessageBox::warning(this, "Save Error",
            "Cannot create playlists directory.");
        return;
    }

    // Save each playlist tab
    for (int i = 0; i < m_playlistTabs->count(); ++i) {
        QString playlistName = m_playlistTabs->tabText(i);
        QString filename = ConstantGlobals::playlistFilePath + "/" + playlistName + ".json";

        // Temporarily switch to this tab to get current data
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
            QString("All %1 playlists saved successfully").arg(successCount),
            3000
        );
    } else {
        QMessageBox::warning(this, "Save Warning",
            QString("Saved %1 playlists, failed to save %2 playlists")
            .arg(successCount).arg(failCount));
    }
}

bool MainWindow::savePlaylistToFile(const QString &filename, const QString &playlistName) {
    QListWidget *playlist = currentPlaylistWidget();
    if (!playlist || playlist->count() == 0) {
        statusBar()->showMessage("Playlist is empty", 2000);
        return false;
    }

    // Get file paths from data structure
    QStringList filePaths = m_playlistFiles.value(playlistName, QStringList());

    // Create JSON document
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

        // Try to get duration if we have a media player
        // (This could be enhanced to read actual file metadata)
        track.duration = 0;

        tracksArray.append(track.toJson());
    }

    playlistJson["tracks"] = tracksArray;

    // Write to file
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
        // Use filename as playlist name
        playlistName = QFileInfo(filename).completeBaseName();
    }

    // Check if playlist already exists
    for (int i = 0; i < m_playlistTabs->count(); ++i) {
        if (m_playlistTabs->tabText(i) == playlistName) {
            // Ask to replace or rename
            QMessageBox::StandardButton reply = QMessageBox::question(
                this,
                "Playlist Exists",
                QString("Playlist '%1' already exists. Replace it?").arg(playlistName),
                QMessageBox::Yes | QMessageBox::No
            );

            if (reply == QMessageBox::No) {
                // Ask for new name
                bool ok;
                playlistName = QInputDialog::getText(
                    this,
                    "Rename Playlist",
                    "Enter new playlist name:",
                    QLineEdit::Normal,
                    playlistName + "_copy",
                    &ok
                );

                if (!ok || playlistName.isEmpty()) {
                    return false;
                }
            } else {
                // Replace existing
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

    // Create new playlist tab if needed
    if (m_currentPlaylistWidget == nullptr ||
        m_playlistTabs->currentWidget() != m_currentPlaylistWidget) {
        addNewPlaylist(playlistName);
    } else {
        // Update existing tab name
        int currentIndex = m_playlistTabs->currentIndex();
        m_playlistTabs->setTabText(currentIndex, playlistName);
    }

    // Clear current playlist
    QListWidget *playlist = currentPlaylistWidget();
    playlist->clear();
    m_playlistFiles[playlistName].clear();

    // Load tracks
    QJsonArray tracksArray = playlistJson["tracks"].toArray();
    foreach (const QJsonValue &trackValue, tracksArray) {
        PlaylistTrack track = PlaylistTrack::fromJson(trackValue.toObject());

        // Add to UI
        playlist->addItem(track.title);

        // Add to data structure
        m_playlistFiles[playlistName].append(track.filePath);
    }

    // Update UI
    updatePlaylistButtonsState();

    return true;
}

void MainWindow::updatePlaylistFromCurrentTab(const QString &filename) {
    // This updates an existing playlist file when playlist changes
    QString playlistName = currentPlaylistName();
    if (!playlistName.isEmpty()) {
        savePlaylistToFile(filename, playlistName);
    }
}

//add actions
void MainWindow::addActions()
{
    // ========== PRESET ACTIONS ==========
    savePresetAction = new QAction("Save &Preset...", this);
    savePresetAction->setShortcut(QKeySequence::Save);
    savePresetAction->setStatusTip("Save current brainwave settings as preset");
    savePresetAction->setIcon(QIcon(":/icons/save.svg")); // Optional

    loadPresetAction = new QAction("&Load Preset...", this);
    loadPresetAction->setShortcut(QKeySequence::Open);
    loadPresetAction->setStatusTip("Load a saved brainwave preset");
    loadPresetAction->setIcon(QIcon(":/icons/folder.svg")); // Optional

    managePresetsAction = new QAction("&Manage Presets...", this);
    managePresetsAction->setStatusTip("Open presets folder in file explorer");
    managePresetsAction->setIcon(QIcon(":/icons/settings.svg")); // Optional

    // ========== PLAYLIST FILE ACTIONS ==========
    openPlaylistAction = new QAction("&Open Playlist...", this);
    openPlaylistAction->setShortcut(Qt::CTRL | Qt::Key_O);
    openPlaylistAction->setStatusTip("Open a saved playlist file");
    openPlaylistAction->setIcon(QIcon(":/icons/music.svg")); // Optional

    saveCurrentPlaylistAction = new QAction("&Save Playlist", this);
    saveCurrentPlaylistAction->setShortcut(Qt::CTRL | Qt::Key_S);
    saveCurrentPlaylistAction->setStatusTip("Save current playlist");
    saveCurrentPlaylistAction->setIcon(QIcon(":/icons/copy.svg")); // Optional

    saveCurrentPlaylistAsAction = new QAction("Save Playlist &As...", this);
    saveCurrentPlaylistAsAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_S);
    saveCurrentPlaylistAsAction->setStatusTip("Save current playlist with a new name");
    saveCurrentPlaylistAsAction->setIcon(QIcon(":/icons/save.svg")); // Optional

    saveAllPlaylistsAction = new QAction("Save A&ll Playlists", this);
    saveAllPlaylistsAction->setShortcut(Qt::CTRL | Qt::ALT | Qt::Key_S);
    saveAllPlaylistsAction->setStatusTip("Save all open playlists");
    saveAllPlaylistsAction->setIcon(QIcon(":/icons/copy.svg")); // Optional

    // ========== OPTIONAL: QUIT ACTION ==========
    quitAction = new QAction("E&xit", this);
    quitAction->setShortcut(QKeySequence::Quit);
    quitAction->setStatusTip("Exit the application");
    quitAction->setIcon(QIcon(":/icons/power.svg")); // Optional
}

//add menubar
void MainWindow::setupMenus()
{
    // ========== FILE MENU ==========
    QMenu *fileMenu = menuBar()->addMenu("&File");

    // Playlist operations
    fileMenu->addAction(openPlaylistAction);
    fileMenu->addAction(saveCurrentPlaylistAction);
    fileMenu->addAction(saveCurrentPlaylistAsAction);
    fileMenu->addAction(saveAllPlaylistsAction);
    fileMenu->addSeparator();

    QAction *streamAction = fileMenu->addAction("&Stream from URL",
                                                 this,
                                                 &MainWindow::onStreamFromUrl);
    streamAction->setShortcut(QKeySequence("Ctrl+U"));
    streamAction->setIcon(QIcon(":/icons/rss.svg"));
    fileMenu->addAction(streamAction);
    fileMenu->addSeparator();


    // Exit action

    fileMenu->addAction(quitAction);

    // =========== View Menu ============
    QMenu *viewMenu = menuBar()->addMenu("&View");
    QAction *hideBinauralToolbarAction = new QAction("Hide Binaural Toolbar",viewMenu);
    //hideBinauralToolbarAction->setIcon(QIcon(":/icons/eye.svg"));
    hideBinauralToolbarAction->setCheckable(true);
    bool isToolbarHidden = settings.value("UI/BinauralToolbarHidden", false).toBool();
    hideBinauralToolbarAction->setChecked(isToolbarHidden);
    m_binauralToolbar->setVisible(!isToolbarHidden);
    m_binauralToolbarExt->setVisible(!isToolbarHidden);
    //hideBinauralToolbarAction->setChecked(false);
    connect(hideBinauralToolbarAction, &QAction::triggered, [this](bool checked){
       m_binauralToolbar->setVisible(!checked);
       m_binauralToolbarExt->setVisible(!checked);
       settings.setValue("UI/BinauralToolbarHidden", checked);
    });
    viewMenu->addAction(hideBinauralToolbarAction);
    // ========== Settings Menu =========
    QMenu *settingsMenu= menuBar()->addMenu("&Settings");
    QAction *factoryResetAction = new QAction("Factory Reset", settingsMenu);
    factoryResetAction->setIcon(QIcon(":/icons/refresh-cw.svg"));
    connect(factoryResetAction, &QAction::triggered, [this]{
        // Confirmation dialog
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                     "Factory Reset Confirmation",
                                     "Are you sure you want to reset all settings to defaults?\n\n"
                                     "This will delete all your preferences and customizations.\n"
                                     "Your music files, saved playlists, and presets will NOT be affected.",
                                     QMessageBox::Ok | QMessageBox::Cancel,
                                     QMessageBox::Cancel);

        if (reply == QMessageBox::Ok) {
            settings.clear();
            QMessageBox::information(this,
                                    "Factory Reset Complete",
                                    "All settings have been reset to defaults.\n"
                                    "Please restart the application for changes to take full effect.");

            // Restart the application immediately
            // QTimer::singleShot(1000, this, &QApplication::quit);
        }
    });
    settingsMenu->addAction(factoryResetAction);

    // ========== PRESETS MENU ==========
    QMenu *presetsMenu = menuBar()->addMenu("&Presets");

    // Brainwave preset operations
    presetsMenu->addAction(savePresetAction);
    presetsMenu->addAction(loadPresetAction);
    presetsMenu->addSeparator();
    //presetsMenu->addAction(managePresetsAction);
    //presetsMenu->addSeparator();



    // Reset to defaults
    QAction *resetPresetsAction = new QAction("&Binaural Defaults", this);
    resetPresetsAction->setStatusTip("Reset brainwave settings to defaults");
    resetPresetsAction->setIcon(QIcon(":/icons/target.svg"));
    connect(resetPresetsAction, &QAction::triggered, this, [this]() {
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


    // ========== HELP MENU ==========
    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("About BinauralPlayer");
    connect(aboutAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::About, this);
        dialog.exec();
    });

    // Features
    QAction *featuresAction = helpMenu->addAction("Features");
    connect(featuresAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Features, this);
        dialog.exec();
    });

    // Instructions
    QAction *instructionsAction = helpMenu->addAction("Instructions");
    connect(instructionsAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::Instructions, this);
        dialog.exec();
    });

    // Best Practices
    QAction *bestPracticesAction = helpMenu->addAction("Best Practices");
    connect(bestPracticesAction, &QAction::triggered, [this]() {
        HelpMenuDialog dialog(HelpType::BestPractices, this);
        dialog.exec();
    });
    QAction* supportusAction = helpMenu->addAction("Support Us");
    connect(supportusAction, &QAction::triggered, [this]() {
            DonationDialog dialog(this);
                dialog.exec();
        });
}

//staus update
void MainWindow::updatePlayerStatus(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString trackName = fileInfo.fileName(); // Or use baseName() for cleaner name

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
            // Optionally don't show track name when stopped
            statusBar()->showMessage(status);
            return;
    }

    statusBar()->showMessage(status + trackName);
}

QString MainWindow::formatBinauralString()
{
    QString result = "";

    // Get current tone type from combo box
    int toneType = toneTypeCombo->currentData().toInt();

    switch (toneType) {
        case BINAURAL:
            result = "BIN:";  // Only BIN for binaural
            {
                // Add beat frequency from the label
                QString beatText = m_beatFreqLabel->text();
                // Extract just the number (remove " Hz")
                QString beatValue = beatText.split(" ")[0];
                result += beatValue;
            }
            break;

        case ISOCHRONIC:
            result = "ISO:";  // Only ISO for isochronic
            result += QString("%1").arg(m_pulseFreqLabel->value(), 0, 'f', 1);
            break;

        case GENERATOR:
            result = "GEN:";  // GEN for generator
            //result += QString("%1").arg(m_leftFreqInput->value(), 0, 'f', 1);
            result += QString("L:%1/R:%2").arg(
                    m_leftFreqInput->value(), 0, 'f', 2).arg(
                    m_rightFreqInput->value(), 0, 'f', 2);
            break;

        default:
            result = "OFF";
            return result;
    }

    // Add waveform type
    QString waveform = m_waveformCombo->currentText();

    // Abbreviate waveforms for compact display
    if (waveform == "Sine") result += ":Sine";
    else if (waveform == "Square") result += ":Sqr";
    else if (waveform == "Triangle") result += ":Tri";
    else if (waveform == "Sawtooth") result += ":Saw";

    return result;
}


//streaming

void MainWindow::onStreamFromUrl()
{
    bool ok;
    QString url = QInputDialog::getText(this,
                                        "Stream from URL",
                                        "Enter audio stream URL:",
                                        QLineEdit::Normal,
                                        m_currentStreamUrl.isEmpty() ? "https://" : m_currentStreamUrl,  // Default suggestion
                                        &ok);

    if (ok && !url.isEmpty()) {
        playRemoteStream(url);
    }
}

void MainWindow::playRemoteStream(const QString &urlString)
{
    if (!m_mediaPlayer) {
        // Initialize if needed (use your existing setup)
        m_mediaPlayer = new QMediaPlayer(this);
        m_audioOutput = new QAudioOutput(this);
        m_mediaPlayer->setAudioOutput(m_audioOutput);
       // setupMediaPlayerConnections(); // Your existing connections
    }

    QUrl audioUrl(urlString);

    if (!audioUrl.isValid()) {
        statusBar()->showMessage("❌ Invalid URL format", 3000);
        return;
    }

    // Stop current playback
    QMediaPlayer::PlaybackState state = m_mediaPlayer->playbackState();
    if (state == QMediaPlayer::PlayingState || state == QMediaPlayer::PausedState) {
        m_mediaPlayer->stop();
    }


    // Set stream flag and URL
    m_isStream = true;
    m_currentStreamUrl = urlString;  // Store for pause/resume

    // Clear previous source and set new one
    m_mediaPlayer->setSource(audioUrl);

    // Play immediately
    m_mediaPlayer->play();

    // Show status
    QString displayName = audioUrl.fileName();
    if (displayName.isEmpty()) {
        // For streams without filename, show host
        displayName = audioUrl.host();
        if (displayName.isEmpty()) displayName = "Network Stream";
    }

    //statusBar()->showMessage("Streaming: " + audioUrl.toString());
    statusBar()->showMessage("Streaming: " + displayName);

}

//open with
void MainWindow::onFileOpened(const QString &filePath)
{
    // 1. Check if file exists
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qDebug() << "File does not exist or is not a regular file:" << filePath;
        return;
    }

    // 2. Extract and check file extension (case-insensitive)
    QString suffix = fileInfo.suffix().toLower();
    QStringList supportedExtensions = {"mp3", "wav", "flac", "ogg", "m4a", "mp4", "m4v", "avi", "mkv"};

    if (!supportedExtensions.contains(suffix)) {
        qDebug() << "Unsupported file extension:" << suffix;
        statusBar()->showMessage("Unsupported file format: ." + suffix, 3000);
        return;
    }

    // 3. Use your existing playlist logic
    QString playlistName = currentPlaylistName();
    QString fileName = fileInfo.fileName();

    // Add to the UI widget
    currentPlaylistWidget()->addItem(fileName);

    // Add to  internal map (m_playlistFiles)
    m_playlistFiles[playlistName].append(filePath);

    // 4. Optional: Auto-start playback if playlist was empty
    // This part depends on your player's structure
    // Example:
    // if (m_player && m_player->state() != QMediaPlayer::PlayingState) {
    //     // You might need to load the playlist into your QMediaPlayer here
    //     m_player->play();
    // }

    // 5. Update status
    m_playMusicButton->click();
    statusBar()->showMessage("Opened: " + fileName, 3000);
    qDebug() << "Successfully added file to playlist:" << filePath;
}

//metadata for playing track

/*
QString MainWindow::getTrackMetadata() {

    metaData = m_mediaPlayer->metaData();
    // Access specific tags using keys from QMediaMetaData::Key
    QString title = metaData.stringValue(QMediaMetaData::Title);
    QString album = metaData.stringValue(QMediaMetaData::AlbumTitle);
    QStringList artists = metaData.value(QMediaMetaData::Author).toStringList(); // Note: Author for artists[citation:6]
    int trackNumber = metaData.value(QMediaMetaData::TrackNumber).toInt();
    qint64 duration = metaData.value(QMediaMetaData::Duration).toLongLong();

    // Format duration from milliseconds to MM:SS
       int totalSeconds = duration / 1000;
       int minutes = totalSeconds / 60;
       int seconds = totalSeconds % 60;
       QString durationFormatted = QString("%1:%2")
                                       .arg(minutes, 2, 10, QChar('0'))
                                       .arg(seconds, 2, 10, QChar('0'));

       // Build the formatted string for tooltip
       QString displayMetaData;
       if (!title.isEmpty()) {
           displayMetaData += QString("<b>Title:</b> %1<br/>").arg(title);
       }
       if (!artists.isEmpty()) {
           // Join multiple artists with comma
           displayMetaData += QString("<b>Artist:</b> %1<br/>").arg(artists.join(", "));
       }
       if (!album.isEmpty()) {
           displayMetaData += QString("<b>Album:</b> %1<br/>").arg(album);
       }
       if (trackNumber > 0) {
           displayMetaData += QString("<b>Track:</b> %1<br/>").arg(trackNumber);
       }
       if (duration > 0) {
           displayMetaData += QString("<b>Duration:</b> %1").arg(durationFormatted);
       }

    return displayMetaData;
}
*/
QString MainWindow::getTrackMetadata() {
    metaData = m_mediaPlayer->metaData();

    QString displayMetaData = "Metadata:\n";

    // List of ALL Qt metadata keys from the documentation
    QList<QMediaMetaData::Key> allKeys = {
        // Common attributes
        QMediaMetaData::Title,
        QMediaMetaData::Author,
        QMediaMetaData::Genre,
        QMediaMetaData::Date,
        QMediaMetaData::Copyright,
        QMediaMetaData::Comment,

        // Music-specific attributes
        QMediaMetaData::AlbumTitle,
        QMediaMetaData::AlbumArtist,
        QMediaMetaData::ContributingArtist,
        QMediaMetaData::TrackNumber,
        QMediaMetaData::Composer,
        QMediaMetaData::CoverArtImage,

        // Technical attributes
        QMediaMetaData::Duration,
        QMediaMetaData::AudioBitRate,
        QMediaMetaData::AudioCodec,
        QMediaMetaData::FileFormat
    };

    // Process each key - ONLY include if it has a valid, non-empty value
    for (const auto &key : allKeys) {
        QVariant value = metaData.value(key);

        // Skip if value is invalid, null, or empty (depending on type)
        if (!value.isValid() || value.isNull()) {
            continue;
        }

        // Check for empty values based on type
        if (value.typeId() == QMetaType::QString && value.toString().isEmpty()) {
            continue;
        }
        if (value.typeId() == QMetaType::QStringList && value.toStringList().isEmpty()) {
            continue;
        }
        if (value.typeId() == QMetaType::Int && value.toInt() == 0) {
            continue; // Except for TrackNumber which could be 0
        }

        // Special handling for TrackNumber - 0 might be valid
        if (key == QMediaMetaData::TrackNumber && value.toInt() == 0) {
            continue;
        }

        // Get the key name and format it like FFmpeg
        QString keyName = QMediaMetaData::metaDataKeyToString(key);
        if (keyName.isEmpty()) {
            keyName = QString("Key_%1").arg(static_cast<int>(key));
        }

        // Convert to uppercase like FFmpeg
        QString formattedKeyName = keyName.toUpper();

        // Pad with spaces for alignment (FFmpeg uses ~12-16 spaces)
        while (formattedKeyName.length() < 16) {
            formattedKeyName += " ";
        }

        // Format the value based on its type
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
            // Default: convert to string
            valueStr = value.toString();
        }

        // Add to output
        displayMetaData += QString("    %1: %2\n").arg(formattedKeyName).arg(valueStr);
    }

    // Add duration in FFmpeg format (always show if available)
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
    // Metadata is now guaranteed to be available (or updated)
    currentTrackMetadata = getTrackMetadata();
    if (trackInfoDialog) {

        metadataBrowser->setText(currentTrackMetadata);


        // Update the dialog if it's currently visible
        if (trackInfoDialog->isVisible()) {
            trackInfoDialog->update(); // Force UI refresh
        }
    }
    //updateTooltip(currentTrackMetadata);
}

void MainWindow::createInfoDialog() {

    trackInfoDialog = new QDialog(this);
    trackInfoDialog->setWindowTitle("Track Information");
    trackInfoDialog->setWindowModality(Qt::NonModal);
    trackInfoDialog->resize(500, 400);

    // Create QTextBrowser and add to dialog
    metadataBrowser = new QTextBrowser(trackInfoDialog);
    metadataBrowser->setReadOnly(true);
    metadataBrowser->setFont(QFont("Monospace", 10));

    // Set layout
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
