#include "radionicsconsole.h"
#include <QDateTime>
#include <QGroupBox>
#include <QFormLayout>
#include<QRandomGenerator>
#include<QCloseEvent>
#include<QApplication>
#include"constants.h"
#include<QJsonDocument>
#include<QJsonObject>

// ============================================================
// SPIN KNOB CLASS
// ============================================================

SpinKnob::SpinKnob(const QString &label, QWidget *parent)
    : QDial(parent)
    , m_isSpinning(false)
    , m_isLocked(false)
    , m_lockedValue(0)
    , m_label(label)
    , m_gen(QRandomGenerator::securelySeeded())
{
    setRange(0, 9999);
    setValue(0);
    setWrapping(true);
    setNotchesVisible(true);
    setNotchTarget(10.0);
    setFixedSize(100, 100);
    //setToolTip("Click and hold to spin · Release to lock");
    //m_gen = QRandomGenerator::securelySeeded();

    m_valueLabel = new QLabel("0000", this);
    m_valueLabel->setAlignment(Qt::AlignCenter);
    m_valueLabel->setStyleSheet(
        "color: #00ccff;"
        "font-family: monospace;"
        "font-size: 16px;"
        "font-weight: bold;"
        "background: transparent;"
    );
    m_valueLabel->setGeometry(25, 38, 50, 25);

    m_timer.setInterval(30);
    connect(&m_timer, &QTimer::timeout, this, &SpinKnob::spin);
}

void SpinKnob::mousePressEvent(QMouseEvent *event)
{
    QDial::mousePressEvent(event);

    m_isLocked = false;
    m_lockedValue = 0;
    m_valueLabel->setText("0000");

    // Use m_gen instead of global()
    //int randomStart = m_gen.bounded(10000);
    //setValue(randomStart);

    m_isSpinning = true;
    m_timer.start();
}

void SpinKnob::mouseReleaseEvent(QMouseEvent *event)
{
    QDial::mouseReleaseEvent(event);
    if (m_isSpinning) {
        m_isSpinning = false;
        m_timer.stop();
        m_isLocked = true;
       // m_lockedValue = value();
        m_lockedValue = m_currentRandomValue;
        m_valueLabel->setText(QString("%1").arg(m_lockedValue, 4, 10, QChar('0')));

        emit locked(m_lockedValue);
    }
}

void SpinKnob::spin()
{
    m_currentRandomValue = m_gen.bounded(10000);

    m_valueLabel->setText(
        QString("%1").arg(m_currentRandomValue, 4, 10, QChar('0')));
   // int displayValue = m_currentRandomValue / 100;  // 0-9999 → 0-99
    //setValue(displayValue);

    //setValue(m_currentRandomValue); // purely visual
}

void SpinKnob::reset()
{
    m_isLocked = false;
    m_isSpinning = false;
    m_lockedValue = 0;
    setValue(0);
    m_valueLabel->setText("0000");
    m_timer.stop();
}

// ============================================================
// RADIONICS CONSOLE
// ============================================================

RadionicsConsole::RadionicsConsole(QWidget *parent)
    : QDialog(parent)
    , m_baseFrequency(250.0)
    , m_combinedSeed(0.0)
    , m_offset(0.0)
    , m_leftFrequency(250.0)
    , m_rightFrequency(250.0)
    , m_knob1Seed(0)
    , m_knob2Seed(0)
    , m_knob3Seed(0)
    , m_isLocked(false)
{
    setupUI();
    updateFrequencyDisplay();
}

RadionicsConsole::~RadionicsConsole()
{
    if (m_zoomPopup) delete m_zoomPopup;
}

// ============================================================
// SETUP UI
// ============================================================

void RadionicsConsole::setupUI()
{
    setWindowTitle("Radionics Console");
    setMinimumSize(900, 850);
    setStyleSheet("background: #1a1a1a;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(25, 25, 25, 25);
    // ============================================================
    // TOP ROW: TARGET (LEFT) + TREND (RIGHT)
    // ============================================================
    QHBoxLayout *topRow = new QHBoxLayout();
    topRow->setSpacing(20);

    // -- Target (Left) --
    QVBoxLayout *targetLayout = new QVBoxLayout();
    QLabel *targetLabel = new QLabel("TARGET", this);
    targetLabel->setStyleSheet("color: #cccccc; font-weight: bold; font-size: 16px;");
    targetEdit = new QTextEdit(this);
    targetEdit->setPlaceholderText("Who is this for?");
    //targetEdit->setStyleSheet("background: #2a2a2a; color: #ffffff; border: 1px solid #444; border-radius: 4px; padding: 8px; font-size: 14px;");
    targetEdit->setStyleSheet(
        "background: #2a2a2a; color: #ffffff; border: 1px solid #444; border-radius: 4px; "
        "padding: 8px; font-size: 14px;"
    );
    targetEdit->setFixedHeight(70);
    targetEdit->setTabChangesFocus(true);
    targetEdit->setAcceptRichText(false);

    QHBoxLayout *targetImageLayout = new QHBoxLayout();
    QPushButton *targetUploadBtn = new QPushButton("Upload", this);
    targetUploadBtn->setStyleSheet("background: #2a2a2a; color: #cccccc; border: 1px solid #444; border-radius: 4px; padding: 6px;");
    QLabel *targetImagePreview = new QLabel(this);
    targetImagePreview->setFixedSize(150, 150);
    targetImagePreview->setStyleSheet("border: 1px solid #444; border-radius: 4px; background: #111111; color: #666666; font-size: 12px;");
    targetImagePreview->setAlignment(Qt::AlignCenter);
    targetImagePreview->setText("No image");
    targetImageLayout->addWidget(targetUploadBtn);
    targetImageLayout->addWidget(targetImagePreview);
    targetImageLayout->addStretch();
    targetLayout->addWidget(targetLabel);
    targetLayout->addWidget(targetEdit);
    targetLayout->addLayout(targetImageLayout);

    // -- Trend (Right) --
    QVBoxLayout *trendLayout = new QVBoxLayout();
    QLabel *trendLabel = new QLabel("TREND", this);
    trendLabel->setStyleSheet("color: #cccccc; font-weight: bold; font-size: 16px;");
    trendEdit = new QTextEdit(this);
    trendEdit->setPlaceholderText("What do you want to achieve?");
    //trendEdit->setStyleSheet("background: #2a2a2a; color: #ffffff; border: 1px solid #444; border-radius: 4px; padding: 8px; font-size: 14px;");
    trendEdit->setStyleSheet(
        "background: #2a2a2a; color: #ffffff; border: 1px solid #444; border-radius: 4px; "
        "padding: 8px; font-size: 14px;"
    );
    trendEdit->setFixedHeight(70);
    trendEdit->setTabChangesFocus(true);
    trendEdit->setAcceptRichText(false);
    QHBoxLayout *trendImageLayout = new QHBoxLayout();
    QPushButton *trendUploadBtn = new QPushButton("Upload", this);
    trendUploadBtn->setStyleSheet("background: #2a2a2a; color: #cccccc; border: 1px solid #444; border-radius: 4px; padding: 6px;");
    QLabel *trendImagePreview = new QLabel(this);
    trendImagePreview->setFixedSize(150, 150);
    trendImagePreview->setStyleSheet("border: 1px solid #444; border-radius: 4px; background: #111111; color: #666666; font-size: 12px;");
    trendImagePreview->setAlignment(Qt::AlignCenter);
    trendImagePreview->setText("No image");
    trendImageLayout->addWidget(trendUploadBtn);
    trendImageLayout->addWidget(trendImagePreview);
    trendImageLayout->addStretch();
    trendLayout->addWidget(trendLabel);
    trendLayout->addWidget(trendEdit);
    trendLayout->addLayout(trendImageLayout);


    //
    bogusProgress = new QProgressBar(this);
    bogusProgress->setRange(0, 100);
    bogusProgress->setValue(50);  // Halfway
    bogusProgress->setFixedHeight(12);  // Thin horizontal bar
    bogusProgress->setFixedWidth(80);   // Width to fit between
    bogusProgress->setStyleSheet(
        "QProgressBar {"
        "    background: #2a2a2a;"
        "    border: 1px solid #444;"
        "    border-radius: 4px;"
        "}"
        "QProgressBar::chunk {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "        stop:0 #00ccff, stop:1 #0066aa);"
        "    border-radius: 3px;"
        "}"
    );
    bogusProgress->setTextVisible(false);

    m_bogusTimer = new QTimer(this);
    m_bogusTimer->setInterval(50);

    connect(m_bogusTimer, &QTimer::timeout, this, [this]() {
        int val = bogusProgress->value();
        if (val >= 100) {
            bogusProgress->setValue(0);
        } else {
            bogusProgress->setValue(val + 1);
        }
    });
    //


    topRow->addLayout(trendLayout);
    //topRow->addStretch();
    topRow->addWidget(bogusProgress);
    topRow->addLayout(targetLayout);

    // Store pointers
    m_targetLabel = targetLabel;
    //targetEdit = targetEdit;
    m_targetImagePreview = targetImagePreview;
    m_targetUploadBtn = targetUploadBtn;

    m_trendLabel = trendLabel;
    //trendEdit = trendEdit;
    m_trendImagePreview = trendImagePreview;
    m_trendUploadBtn = trendUploadBtn;

    m_targetImagePreview->setMouseTracking(true);
    m_trendImagePreview->setMouseTracking(true);


    m_targetImagePreview->installEventFilter(this);
    m_trendImagePreview->installEventFilter(this);




    // ============================================================
    // BASE FREQUENCY
    // ============================================================
    QHBoxLayout *freqLayout = new QHBoxLayout();
    QLabel *freqLabel = new QLabel("BASE FREQUENCY", this);
    freqLabel->setStyleSheet("color: #cccccc; font-weight: bold; font-size: 14px;");
    QSlider *freqSlider = new QSlider(Qt::Horizontal, this);
    freqSlider->setRange(20, 1000);
    freqSlider->setValue(250);
    freqSlider->setTickPosition(QSlider::TicksBelow);
    freqSlider->setTickInterval(50);
    freqSlider->setStyleSheet("QSlider::groove:horizontal { height: 8px; background: #333; border-radius: 4px; }"
                              "QSlider::handle:horizontal { width: 18px; background: #666; border-radius: 9px; }");
    QLabel *freqValue = new QLabel("250 Hz", this);
    freqValue->setStyleSheet("color: #00ccff; font-weight: bold; font-size: 16px;");
    freqLayout->addWidget(freqLabel);
    freqLayout->addWidget(freqSlider);
    freqLayout->addWidget(freqValue);

    m_baseFreqLabel = freqLabel;
    m_baseFreqSlider = freqSlider;
    m_baseFreqValue = freqValue;


    // duration slider

    QHBoxLayout *durationLayout = new QHBoxLayout();
    QLabel *durationLabel = new QLabel("DURATION", this);
    durationLabel->setStyleSheet("color: #cccccc; font-weight: bold; font-size: 14px;");
    QSlider *durationSlider = new QSlider(Qt::Horizontal, this);
    durationSlider->setRange(1, 360);
    durationSlider->setValue(45);
    durationSlider->setTickPosition(QSlider::TicksBelow);
    durationSlider->setTickInterval(30);
    durationSlider->setStyleSheet("QSlider::groove:horizontal { height: 8px; background: #333; border-radius: 4px; }"
                                  "QSlider::handle:horizontal { width: 18px; background: #666; border-radius: 9px; }");
    QLabel *durationValue = new QLabel("45 min", this);
    durationValue->setStyleSheet("color: #00ccff; font-weight: bold; font-size: 16px;");
    durationLayout->addWidget(durationLabel);
    durationLayout->addWidget(durationSlider);
    durationLayout->addWidget(durationValue);

    m_durationLabel = durationLabel;
    m_durationSlider = durationSlider;
    m_durationValue = durationValue;
    //

    // ============================================================
    // KNOBS SECTION
    // ============================================================
    QLabel *knobLabel = new QLabel("INTENTION SEEDS", this);
    knobLabel->setStyleSheet("color: #888888; font-weight: bold; font-size: 14px; margin-top: 10px;");
    knobLabel->setAlignment(Qt::AlignLeft);

    QHBoxLayout *knobLayout = new QHBoxLayout();
    knobLayout->setSpacing(40);

    m_knob1 = new SpinKnob("Dial 1", this);
    m_knob2 = new SpinKnob("Dial 2", this);
    m_knob3 = new SpinKnob("Dial 3", this);

    // Create labels for each knob
    QVBoxLayout *knob1Layout = new QVBoxLayout();
    knob1Layout->addWidget(m_knob1, 0, Qt::AlignCenter);
    QLabel *knob1Label = new QLabel("Dial 1", this);
    knob1Label->setStyleSheet("color: #888888; font-size: 12px;");
    knob1Label->setAlignment(Qt::AlignCenter);
    knob1Layout->addWidget(knob1Label);

    QVBoxLayout *knob2Layout = new QVBoxLayout();
    knob2Layout->addWidget(m_knob2, 0, Qt::AlignCenter);
    QLabel *knob2Label = new QLabel("Dial 2", this);
    knob2Label->setStyleSheet("color: #888888; font-size: 12px;");
    knob2Label->setAlignment(Qt::AlignCenter);
    knob2Layout->addWidget(knob2Label);

    QVBoxLayout *knob3Layout = new QVBoxLayout();
    knob3Layout->addWidget(m_knob3, 0, Qt::AlignCenter);
    QLabel *knob3Label = new QLabel("Dial 3", this);
    knob3Label->setStyleSheet("color: #888888; font-size: 12px;");
    knob3Label->setAlignment(Qt::AlignCenter);
    knob3Layout->addWidget(knob3Label);

    knobLayout->addStretch();
    knobLayout->addLayout(knob1Layout);
    knobLayout->addStretch();
    knobLayout->addLayout(knob2Layout);
    knobLayout->addStretch();
    knobLayout->addLayout(knob3Layout);
    knobLayout->addStretch();

    // Instruction label
    QLabel *knobInstruction = new QLabel("Click and hold a dial to spin · Release to lock", this);
    knobInstruction->setStyleSheet("color: #666666; font-size: 12px;");
    knobInstruction->setAlignment(Qt::AlignCenter);

    // ============================================================
    // STATUS DISPLAY
    // ============================================================
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->setSpacing(25);

    QLabel *seedLabel = new QLabel("COMBINED SEED:", this);
    seedLabel->setStyleSheet("color: #888888; font-weight: bold; font-size: 13px;");
    QLabel *seedValue = new QLabel("—", this);
    seedValue->setStyleSheet("color: #00ccff; font-family: monospace; font-size: 15px;");
    QLabel *offsetLabel = new QLabel("OFFSET:", this);
    offsetLabel->setStyleSheet("color: #888888; font-weight: bold; font-size: 13px;");
    QLabel *offsetValue = new QLabel("—", this);
    offsetValue->setStyleSheet("color: #ffcc00; font-family: monospace; font-size: 15px;");
    QLabel *leftLabel = new QLabel("LEFT:", this);
    leftLabel->setStyleSheet("color: #888888; font-weight: bold; font-size: 13px;");
    QLabel *leftValue = new QLabel("—", this);
    leftValue->setStyleSheet("color: #ff6666; font-family: monospace; font-size: 15px;");
    QLabel *rightLabel = new QLabel("RIGHT:", this);
    rightLabel->setStyleSheet("color: #888888; font-weight: bold; font-size: 13px;");
    QLabel *rightValue = new QLabel("—", this);
    rightValue->setStyleSheet("color: #66ff66; font-family: monospace; font-size: 15px;");

    statusLayout->addStretch();
    statusLayout->addWidget(seedLabel);
    statusLayout->addWidget(seedValue);
    statusLayout->addWidget(offsetLabel);
    statusLayout->addWidget(offsetValue);
    statusLayout->addWidget(leftLabel);
    statusLayout->addWidget(leftValue);
    statusLayout->addWidget(rightLabel);
    statusLayout->addWidget(rightValue);
    statusLayout->addStretch();

    m_seedLabel = seedValue;
    m_offsetLabel = offsetValue;
    m_leftFreqLabel = leftValue;
    m_rightFreqLabel = rightValue;

    // ============================================================
    // BOTTOM CONTROLS
    // ============================================================
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(15);

    QPushButton *resetBtn = new QPushButton("RESET", this);
    resetBtn->setStyleSheet("background: #4a2a2a; color: #ffffff; font-weight: bold; font-size: 14px; padding: 10px 20px; border: none; border-radius: 4px;");

    QPushButton *playBtn = new QPushButton("▶ PLAY", this);
    playBtn->setStyleSheet("background: #2a6a2a; color: #ffffff; font-weight: bold; font-size: 14px; padding: 10px 30px; border: none; border-radius: 4px;");

    QPushButton *stopBtn = new QPushButton("STOP", this);
    stopBtn->setStyleSheet("background: #6a2a2a; color: #ffffff; font-weight: bold; font-size: 14px; padding: 10px 30px; border: none; border-radius: 4px;");

    QPushButton *saveBtn = new QPushButton("SAVE", this);
    saveBtn->setStyleSheet("background: #2a2a4a; color: #ffffff; font-weight: bold; font-size: 14px; padding: 10px 30px; border: none; border-radius: 4px;");

    QPushButton *loadBtn = new QPushButton("LOAD", this);
    loadBtn->setStyleSheet("background: #2a2a4a; color: #ffffff; font-weight: bold; font-size: 14px; padding: 10px 30px; border: none; border-radius: 4px;");


    buttonLayout->addStretch();
    buttonLayout->addWidget(resetBtn);
    buttonLayout->addWidget(playBtn);
    buttonLayout->addWidget(stopBtn);
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(loadBtn);
    buttonLayout->addStretch();

    m_resetBtn = resetBtn;
    m_playBtn = playBtn;
    m_stopBtn = stopBtn;
    m_saveBtn = saveBtn;

    // ============================================================
    // ASSEMBLE
    // ============================================================
    mainLayout->addStretch();  // pushes everything to the bottom

    mainLayout->addLayout(topRow);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(freqLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(durationLayout);
    mainLayout->addSpacing(10);

    mainLayout->addWidget(knobLabel);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(knobLayout);
    mainLayout->addWidget(knobInstruction);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(statusLayout);
    mainLayout->addSpacing(15);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addStretch();  // pushes everything to the top
    // ============================================================
    // CONNECTIONS
    // ============================================================
    connect(m_targetUploadBtn, &QPushButton::clicked, this, &RadionicsConsole::onUploadTargetImage);
    connect(m_trendUploadBtn, &QPushButton::clicked, this, &RadionicsConsole::onUploadTrendImage);
    connect(m_baseFreqSlider, &QSlider::valueChanged, this, &RadionicsConsole::onBaseFrequencyChanged);
    connect(m_knob1, &SpinKnob::locked, this, &RadionicsConsole::onKnobLocked);
    connect(m_knob2, &SpinKnob::locked, this, &RadionicsConsole::onKnobLocked);
    connect(m_knob3, &SpinKnob::locked, this, &RadionicsConsole::onKnobLocked);
    connect(m_resetBtn, &QPushButton::clicked, this, &RadionicsConsole::onResetKnobs);
    connect(m_playBtn, &QPushButton::clicked, this, &RadionicsConsole::onPlay);
    connect(m_stopBtn, &QPushButton::clicked, this, &RadionicsConsole::onStop);
    connect(m_saveBtn, &QPushButton::clicked, this, &RadionicsConsole::onSave);
    connect(loadBtn, &QPushButton::clicked, this, &RadionicsConsole::loadSession);

    connect(m_durationSlider, &QSlider::valueChanged,
            this, &RadionicsConsole::onDurationChanged);
    m_stopBtn->setEnabled(false);

    // Create zoom popup (hidden by default)
    m_zoomPopup = new QLabel(nullptr, Qt::ToolTip | Qt::FramelessWindowHint);
    m_zoomPopup->setStyleSheet("border: 2px solid #444; border-radius: 8px; background: #1a1a1a;");
    m_zoomPopup->setAlignment(Qt::AlignCenter);
    m_zoomPopup->hide();
}

// ============================================================
// IMAGE UPLOADS
// ============================================================



void RadionicsConsole::onUploadTargetImage()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select Target Image",
        ConstantGlobals::appDirPath,
        "Images (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (!filePath.isEmpty()) {
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            m_originalTargetPixmap = pixmap;  // Store original
            m_lastTargetImagePath = filePath;  // Store path
            m_targetImagePreview->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_targetImagePreview->setText("");
        }
    }
}

void RadionicsConsole::onUploadTrendImage()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select Trend Image",
        ConstantGlobals::appDirPath,
        "Images (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (!filePath.isEmpty()) {
        QPixmap pixmap(filePath);
        if (!pixmap.isNull()) {
            m_originalTrendPixmap = pixmap;  // Store original
            m_lastTrendImagePath = filePath;  // Store path
            m_trendImagePreview->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            m_trendImagePreview->setText("");
        }
    }
}

// ============================================================
// BASE FREQUENCY
// ============================================================

void RadionicsConsole::onBaseFrequencyChanged(int value)
{
    m_baseFrequency = value;
    m_baseFreqValue->setText(QString("%1 Hz").arg(value));
    updateFrequencyDisplay();
    updateCombinedSeed();

    emit structuralLinkCaptured(
        m_combinedSeed,
        m_leftFrequency,
        m_rightFrequency,
        m_baseFrequency,
        m_offset,
        trendEdit->toPlainText().trimmed(),
        targetEdit->toPlainText().trimmed()
    );
}

// ============================================================
// KNOB HANDLING
// ============================================================

void RadionicsConsole::onKnobLocked(int value)
{
    // Determine which knob sent the signal
    SpinKnob *knob = qobject_cast<SpinKnob*>(sender());
    if (knob == m_knob1) {
        m_knob1Seed = value;
    } else if (knob == m_knob2) {
        m_knob2Seed = value;
    } else if (knob == m_knob3) {
        m_knob3Seed = value;
    }

    // Check if all three knobs are locked
    if (m_knob1->isLocked() && m_knob2->isLocked() && m_knob3->isLocked()) {
        m_isLocked = true;
        updateCombinedSeed();
        updateFrequencyDisplay();

        // Emit the structural link signal
        emit structuralLinkCaptured(
            m_combinedSeed,
            m_leftFrequency,
            m_rightFrequency,
            m_baseFrequency,
            m_offset,
            trendEdit->toPlainText().trimmed(),
            targetEdit->toPlainText().trimmed()
        );
    }
}
/*
void RadionicsConsole::onResetKnobs()
{
    m_stopBtn->click();
    m_knob1->reset();
    m_knob2->reset();
    m_knob3->reset();
    m_isLocked = false;
    m_knob1Seed = 0;
    m_knob2Seed = 0;
    m_knob3Seed = 0;
    m_combinedSeed = 0.0;
    m_offset = 0.0;
    m_leftFrequency = m_baseFrequency;
    m_rightFrequency = m_baseFrequency;
    updateFrequencyDisplay();
}
*/

void RadionicsConsole::onResetKnobs()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Reset Confirmation");
    msgBox.setText("This will clear all dials, images, and session data.\n\nAre you sure you want to reset?");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setStyleSheet(
        "QLabel { color: #ffffff; }"
        "QPushButton { color: #ffffff; background: #2a2a2a; border: 1px solid #444; border-radius: 4px; padding: 6px 12px; min-width: 60px; }"
        "QPushButton:hover { background: #3a3a3a; }"
    );

    if (msgBox.exec() != QMessageBox::Yes) {
        return;
    }

    m_stopBtn->click();
    m_knob1->reset();
    m_knob2->reset();
    m_knob3->reset();
    m_isLocked = false;
    m_knob1Seed = 0;
    m_knob2Seed = 0;
    m_knob3Seed = 0;
    m_combinedSeed = 0.0;
    m_offset = 0.0;
    m_leftFrequency = m_baseFrequency;
    m_rightFrequency = m_baseFrequency;

    // ✅ Clear images and paths
    m_originalTargetPixmap = QPixmap();
    m_originalTrendPixmap = QPixmap();
    m_lastTargetImagePath.clear();
    m_lastTrendImagePath.clear();

    m_targetImagePreview->clear();
    m_targetImagePreview->setText("No image");
    m_trendImagePreview->clear();
    m_trendImagePreview->setText("No image");

    updateFrequencyDisplay();
}

void RadionicsConsole::onExternalStopRequested()
{
    onStop();
}

// ============================================================
// COMBINE SEEDS
// ============================================================

void RadionicsConsole::updateCombinedSeed()
{
    // Combine the three seeds (0-9999 each) into one (0.000-1.000)
    m_combinedSeed = (m_knob1Seed + m_knob2Seed + m_knob3Seed) / 30000.0;
    m_combinedSeed = qBound(0.0, m_combinedSeed, 1.0);

    // Calculate offset and frequencies
    double maxOffset = 5.0;
    m_offset = m_combinedSeed * maxOffset;
    m_leftFrequency = m_baseFrequency + m_offset;
    m_rightFrequency = m_baseFrequency - m_offset;

}

// ============================================================
// UPDATE DISPLAY
// ============================================================

void RadionicsConsole::updateFrequencyDisplay()
{
    if (m_isLocked) {
        m_seedLabel->setText(QString::number(m_combinedSeed, 'f', 3));
        m_offsetLabel->setText(QString::number(m_offset, 'f', 2) + " Hz");
        m_leftFreqLabel->setText(QString::number(m_leftFrequency, 'f', 2) + " Hz");
        m_rightFreqLabel->setText(QString::number(m_rightFrequency, 'f', 2) + " Hz");
    } else {
        m_seedLabel->setText("—");
        m_offsetLabel->setText("—");
        m_leftFreqLabel->setText("—");
        m_rightFreqLabel->setText("—");
    }
}

// ============================================================
// PLAY / STOP / SAVE
// ============================================================

void RadionicsConsole::onPlay()
{

    if (!m_isLocked) {
        showMessageBox("No Structural Link", "Please set all three dials first.");
        return;
    }
    m_playBtn->setEnabled(false);
    m_stopBtn->setEnabled(true);
    // ✅ Start progress bar animation
    m_bogusTimer->start();
    emit playRequested(m_leftFrequency, m_rightFrequency);
}

void RadionicsConsole::onStop()
{
    m_playBtn->setEnabled(true);
    m_stopBtn->setEnabled(false);
    // ✅ Stop progress bar animation
    m_bogusTimer->stop();
    bogusProgress->setValue(50);  // Reset to halfway
    emit stopRequested();
}

void RadionicsConsole::onSave()
{
    if (!m_isLocked) {
        showMessageBox("No Structural Link", "Nothing to save. Please set all three dials first.");
        return;
    }

    // Get save file path
    QString filePath = QFileDialog::getSaveFileName(this,
        "Save Radionics Session",
        ConstantGlobals::radionicsFilePath + "/session_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".json",
        "JSON Files (*.json)");

    if (!filePath.isEmpty()) {
        saveSession(filePath);
        m_currentSessionPath = filePath;
        showMessageBox("Saved", "Session saved successfully to:\n" + filePath);
    }
}

void RadionicsConsole::saveSession(const QString &filePath)
{
    QJsonObject session;

    // Basic info
    session["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    session["version"] = "1.0";

    // Target and Trend
    session["target"] = targetEdit->toPlainText().trimmed();
    session["trend"] = trendEdit->toPlainText().trimmed();

    // Image paths (save relative paths if possible)
    session["targetImagePath"] = m_lastTargetImagePath;
    session["trendImagePath"] = m_lastTrendImagePath;

    // Frequencies and seeds
    session["baseFrequency"] = m_baseFrequency;
    session["combinedSeed"] = m_combinedSeed;
    session["offset"] = m_offset;
    session["leftFrequency"] = m_leftFrequency;
    session["rightFrequency"] = m_rightFrequency;

    // Knob values
    session["knob1Seed"] = m_knob1Seed;
    session["knob2Seed"] = m_knob2Seed;
    session["knob3Seed"] = m_knob3Seed;

    // Duration
    session["durationMinutes"] = m_durationMinutes;

    // Create JSON document and save
    QJsonDocument doc(session);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    } else {
        showMessageBox("Error", "Could not save session:\n" + file.errorString());
    }
}

void RadionicsConsole::loadSession()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        "Load Radionics Session",
        ConstantGlobals::radionicsFilePath,
        "JSON Files (*.json)");

    if (!filePath.isEmpty()) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            showMessageBox("Error", "Could not load session:\n" + file.errorString());
            return;
        }

        QByteArray data = file.readAll();
        file.close();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) {
            showMessageBox("Error", "Invalid session file.");
            return;
        }

        QJsonObject session = doc.object();

        // Restore everything
        targetEdit->setText(session["target"].toString());
        trendEdit->setText(session["trend"].toString());

        // ✅ Restore image paths from JSON
        m_lastTargetImagePath = session["targetImagePath"].toString();
        m_lastTrendImagePath = session["trendImagePath"].toString();

        if (!m_lastTargetImagePath.isEmpty() && QFile::exists(m_lastTargetImagePath)) {
            QPixmap pixmap(m_lastTargetImagePath);
            if (!pixmap.isNull()) {
                m_originalTargetPixmap = pixmap;
                m_targetImagePreview->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_targetImagePreview->setText("");
            }
        }

        if (!m_lastTrendImagePath.isEmpty() && QFile::exists(m_lastTrendImagePath)) {
            QPixmap pixmap(m_lastTrendImagePath);
            if (!pixmap.isNull()) {
                m_originalTrendPixmap = pixmap;
                m_trendImagePreview->setPixmap(pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                m_trendImagePreview->setText("");
            }
        }

        m_baseFrequency = session["baseFrequency"].toDouble(250.0);
        m_combinedSeed = session["combinedSeed"].toDouble(0.0);
        m_offset = session["offset"].toDouble(0.0);
        m_leftFrequency = session["leftFrequency"].toDouble(250.0);
        m_rightFrequency = session["rightFrequency"].toDouble(250.0);

        m_knob1Seed = session["knob1Seed"].toInt(0);
        m_knob2Seed = session["knob2Seed"].toInt(0);
        m_knob3Seed = session["knob3Seed"].toInt(0);

        m_durationMinutes = session["durationMinutes"].toInt(45);
        m_durationSlider->setValue(m_durationMinutes);

        m_baseFreqSlider->setValue(static_cast<int>(m_baseFrequency));
        m_baseFreqValue->setText(QString("%1 Hz").arg(static_cast<int>(m_baseFrequency)));

        m_isLocked = true;
        updateFrequencyDisplay();

        emit structuralLinkCaptured(
            m_combinedSeed,
            m_leftFrequency,
            m_rightFrequency,
            m_baseFrequency,
            m_offset,
            trendEdit->toPlainText().trimmed(),
            targetEdit->toPlainText().trimmed()
        );

        showMessageBox("Loaded", "Session loaded successfully.");
    }
}

void RadionicsConsole::closeEvent(QCloseEvent *event)
{
    // Hide the window instead of closing it
    hide();
    emit closeRequested();
    event->ignore();  // Don't actually close
}

void RadionicsConsole::onDurationChanged(int value)
{
    m_durationValue->setText(QString("%1 min").arg(value));
    // Store the duration for later use
    m_durationMinutes = value;
    emit durationChanged(value);
}

bool RadionicsConsole::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        QLabel *label = qobject_cast<QLabel*>(watched);

        if (label == m_targetImagePreview && !m_originalTargetPixmap.isNull()) {
            showZoomedImage(m_originalTargetPixmap, label);
        } else if (label == m_trendImagePreview && !m_originalTrendPixmap.isNull()) {
            showZoomedImage(m_originalTrendPixmap, label);
        }
    } else if (event->type() == QEvent::Leave) {
        hideZoomedImage();
    }

    return QDialog::eventFilter(watched, event);
}

/*
void RadionicsConsole::showZoomedImage(const QPixmap &pixmap, QLabel *sourceLabel)
{
    if (pixmap.isNull()) return;

    // 2x zoom
    QPixmap zoomed = pixmap.scaled(
        pixmap.width() * 2,
        pixmap.height() * 2,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    // Limit size to fit screen
    QSize maxSize = QApplication::primaryScreen()->availableSize() * 0.6;
    if (zoomed.width() > maxSize.width() || zoomed.height() > maxSize.height()) {
        zoomed = zoomed.scaled(maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    m_zoomPopup->setPixmap(zoomed);
    m_zoomPopup->adjustSize();

    // Position near cursor
    QPoint pos = QCursor::pos() + QPoint(15, 15);

    // Keep popup on screen
    QRect screen = QApplication::primaryScreen()->availableGeometry();
    if (pos.x() + m_zoomPopup->width() > screen.right()) {
        pos.setX(screen.right() - m_zoomPopup->width() - 5);
    }
    if (pos.y() + m_zoomPopup->height() > screen.bottom()) {
        pos.setY(screen.bottom() - m_zoomPopup->height() - 5);
    }

    m_zoomPopup->move(pos);
    m_zoomPopup->show();
    m_zoomPopup->raise();
}
*/

void RadionicsConsole::showZoomedImage(const QPixmap &pixmap, QLabel *sourceLabel)
{
    if (pixmap.isNull()) return;

    // 🎯 Show original size (no scaling at all)
    QPixmap original = pixmap;  // Just use the original pixmap

    // 🎯 Optional: Limit size to fit screen (so it doesn't overflow)
    QSize maxSize = QApplication::primaryScreen()->availableSize() * 0.7;
    if (original.width() > maxSize.width() || original.height() > maxSize.height()) {
        original = original.scaled(maxSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    m_zoomPopup->setPixmap(original);
    m_zoomPopup->adjustSize();
    m_zoomPopup->setStyleSheet("border: 2px solid #444; border-radius: 8px; background: #1a1a1a;");

    // Position near cursor
    QPoint pos = QCursor::pos() + QPoint(15, 15);

    // Keep popup on screen
    QRect screen = QApplication::primaryScreen()->availableGeometry();
    if (pos.x() + m_zoomPopup->width() > screen.right()) {
        pos.setX(screen.right() - m_zoomPopup->width() - 5);
    }
    if (pos.y() + m_zoomPopup->height() > screen.bottom()) {
        pos.setY(screen.bottom() - m_zoomPopup->height() - 5);
    }

    m_zoomPopup->move(pos);
    m_zoomPopup->show();
    m_zoomPopup->raise();
}

void RadionicsConsole::hideZoomedImage()
{
    if (m_zoomPopup) {
        m_zoomPopup->hide();
    }
}

void RadionicsConsole::showMessageBox(const QString &title, const QString &text)
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStyleSheet(
        "QLabel { color: #ffffff; }"
        "QPushButton { color: #ffffff; background: #2a2a2a; border: 1px solid #444; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background: #3a3a3a; }"
    );
    msgBox.exec();
}
