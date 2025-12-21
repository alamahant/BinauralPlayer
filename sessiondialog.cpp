#include "sessiondialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QCloseEvent>
#include <QFile>
#include <QTextStream>
#include <QtMath>
#include"constants.h"

SessionDialog::SessionDialog(QWidget *parent)
    : QDialog(parent)
    , m_textEdit(nullptr)
    , m_statusLabel(nullptr)
    , m_stageInfoLabel(nullptr)
    , m_totalTimeLabel(nullptr)
    , m_parseButton(nullptr)
    , m_loadButton(nullptr)
    , m_saveButton(nullptr)
    , m_clearButton(nullptr)
    , m_playButton(nullptr)
    , m_pauseButton(nullptr)
    , m_stopButton(nullptr)
    , m_currentStageIndex(-1)
    , m_stageTimeRemainingSec(0)
    , m_totalTimeRemainingSec(0)
    , m_sessionActive(false)
    , m_paused(false)
    , m_unlimitedDuration(false)
    , m_stageTimer(nullptr)
{
    setWindowTitle("Multi-Stage Session");
    setMinimumSize(700, 600);

    // Setup UI first
    setupUI();

    // Create timers
    m_stageTimer = new QTimer(this);
    m_stageTimer->setInterval(1000);



    // Connect all signals and slots
    setupConnections();

    // Initial UI state
    updateUIFromState();

    // Non-modal dialog
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

SessionDialog::~SessionDialog()
{
    if (m_sessionActive) {
        endSession();
    }
}

void SessionDialog::setupUI()
{
    QLabel *headerLabel = new QLabel(this);
    headerLabel->setText(
        "<table border='0' cellspacing='8'>"
        "<tr>"
        "<th align='left'>TYPE</th>"
        "<th align='left'>LEFT</th>"
        "<th align='left'>RIGHT</th>"
        "<th align='left'>WAVE</th>"
        "<th align='left'>DUR</th>"
        "<th align='left'>VOL</th>"
        "<th align='left' style='color: #666;'>DESCRIPTION</th>"
        "</tr>"
        "<tr>"
        "<td><b>BINAURAL</b></td>"
        "<td>360</td>"
        "<td>367</td>"
        "<td>SINE</td>"
        "<td>10</td>"
        "<td>15</td>"
        "<td style='color: #666;'>Alpha (7Hz beat, 15% vol)</td>"
        "</tr>"
        "<tr>"
        "<td><b>BINAURAL</b></td>"
        "<td>528</td>"
        "<td>535</td>"
        "<td>TRIANGLE</td>"
        "<td>10</td>"
        "<td><b>30</b></td>"
        "<td style='color: #666;'>Alpha (7Hz beat, 30% vol)</td>"
        "</tr>"
        "<tr>"
        "<td><b>ISOCHRONIC</b></td>"
        "<td>333</td>"
        "<td><b>10</b></td>"
        "<td>SQUARE</td>"
        "<td>5</td>"
        "<td>15</td>"
        "<td style='color: #666;'>Focus (333Hz + 10Hz pulse)</td>"
        "</tr>"
        "<tr>"
        "<td><b>ISOCHRONIC</b></td>"
        "<td>432</td>"
        "<td><b>10</b></td>"
        "<td>SAWTOOTH</td>"
        "<td>5</td>"
        "<td><b>25</b></td>"
        "<td style='color: #666;'>Focus (432Hz + 10Hz pulse, 25% vol)</td>"
        "</tr>"
        "<tr>"
        "<td><b>GENERATOR</b></td>"
        "<td>432</td>"
        "<td>432</td>"
        "<td>SINE</td>"
        "<td>3</td>"
        "<td>15</td>"
        "<td style='color: #666;'>Relaxation (432Hz mono)</td>"
        "</tr>"
        "</table>"
    );
    headerLabel->setStyleSheet(
        "QLabel {"
        "  background-color: #f8f9fa;"
        "  border: 1px solid #dee2e6;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  margin-bottom: 5px;"
        "}"
        "QLabel th {"
        "  color: #495057;"
        "  font-weight: bold;"
        "}"
        "QLabel td b {"
        "  color: #dc3545;"  // Red for important values
        "}"
    );

    // Create format reminder label
    QLabel *formatLabel = new QLabel(this);
    formatLabel->setText("Format: <b>TYPE:LEFT:RIGHT:WAVE:DUR(min):[OPTIONAL] VOL(%)</b> <span style='color:#666'>(default:15%)</span>");
    formatLabel->setStyleSheet(
        "QLabel {"
        "  background-color: #e3f2fd;"
        "  border: 1px solid #bbdefb;"
        "  border-radius: 4px;"
        "  padding: 6px;"
        "  margin-bottom: 5px;"
        "}"
    );

    // Create text editor with placeholder
    m_textEdit = new QTextEdit(this);
    m_textEdit->setFontPointSize(14);

    m_textEdit->setPlaceholderText(
        "Enter session stages (one per line):\n\n"
        "5 fields (volume=15% default):\n"
        "  binaural:387:306:sine:10\n"
        "  isochronic:200:10:square:5\n"
        "  generator:432:432:sine:3\n\n"
        "6 fields (specify volume):\n"
        "  binaural:250:358:sine:10:30\n"
        "  isochronic:200:10:square:5:25\n"
        "  generator:432:432:sine:3:50\n\n"
        "Note:\n"
        "- ISOCHRONIC: RIGHT field = PULSE frequency (Hz)\n"
        "- Volume optional (0-100%, default 15%)"
    );
    // Create control buttons
    m_parseButton = new QPushButton("&Parse Stages", this);
    m_loadButton = new QPushButton("&Load Session...", this);
    m_saveButton = new QPushButton("&Save Session...", this);
    m_clearButton = new QPushButton("C&lear All", this);
    m_playButton = new QPushButton("▶ &Play", this);
    m_pauseButton = new QPushButton("⏸ &Pause", this);
    m_stopButton = new QPushButton("■ &Stop", this);

    // Create status labels
    m_statusLabel = new QLabel("No session loaded", this);
    m_stageInfoLabel = new QLabel("Stage: --/-- | Time: --:--/--:--", this);
    m_totalTimeLabel = new QLabel("Total: --:--", this);

    // Setup layouts
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(headerLabel);
    mainLayout->addWidget(formatLabel);
    // Text editor area
    mainLayout->addWidget(m_textEdit);

    // File operation buttons (first row)
    QHBoxLayout *fileButtonLayout = new QHBoxLayout();
    fileButtonLayout->addWidget(m_parseButton);
    fileButtonLayout->addWidget(m_loadButton);
    fileButtonLayout->addWidget(m_saveButton);
    fileButtonLayout->addWidget(m_clearButton);
    fileButtonLayout->addStretch();
    mainLayout->addLayout(fileButtonLayout);

    // Playback buttons (second row)
    QHBoxLayout *playbackLayout = new QHBoxLayout();
    playbackLayout->addStretch();
    playbackLayout->addWidget(m_playButton);
    playbackLayout->addWidget(m_pauseButton);
    playbackLayout->addWidget(m_stopButton);
    playbackLayout->addStretch();
    mainLayout->addLayout(playbackLayout);

    // Status bar (third row)
    QHBoxLayout *statusLayout = new QHBoxLayout();
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_stageInfoLabel);
    statusLayout->addWidget(m_totalTimeLabel);
    mainLayout->addLayout(statusLayout);
}

void SessionDialog::setupConnections()
{
    connect(m_parseButton, &QPushButton::clicked, this, &SessionDialog::onParseClicked);
    connect(m_loadButton, &QPushButton::clicked, this, &SessionDialog::onLoadClicked);
    connect(m_saveButton, &QPushButton::clicked, this, &SessionDialog::onSaveClicked);
    connect(m_clearButton, &QPushButton::clicked, this, &SessionDialog::onClearClicked);
    connect(m_playButton, &QPushButton::clicked, this, &SessionDialog::onPlayClicked);
    connect(m_pauseButton, &QPushButton::clicked, this, &SessionDialog::onPauseClicked);
    connect(m_stopButton, &QPushButton::clicked, this, &SessionDialog::onStopClicked);
    connect(m_stageTimer, &QTimer::timeout, this, &SessionDialog::onStageTimerTimeout);
}

void SessionDialog::onParseClicked()
{
    if (parseStagesFromText()) {
        m_statusLabel->setText(QString("✓ Parsed %1 stage(s)").arg(m_stages.size()));
        calculateTotalTime();
        m_playButton->setEnabled(true);
        highlightCurrentStage();
    }
}

bool SessionDialog::parseStagesFromText()
{
    m_stages.clear();
    QString text = m_textEdit->toPlainText();
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);

    bool hasErrors = false;
    QStringList errorMessages;
    int validLineCount = 0;

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        bool ok;
        QString error;
        Stage stage = parseLine(line, ok, error);

        if (!ok) {
            // Skip line but don't fail entire parse
            continue;
        }

        QString validationError;
        if (!validateStage(stage, i + 1, validationError)) {
            errorMessages.append(QString("Line %1: %2").arg(i + 1).arg(validationError));
            hasErrors = true;
            continue;
        }


        m_stages.append(stage);
        validLineCount++;
    }

    if (hasErrors && !errorMessages.isEmpty()) {
        QMessageBox::warning(this, "Parse Errors",
                           "Some lines had errors:\n" + errorMessages.join("\n"));
    }

    if (m_stages.isEmpty()) {
        m_statusLabel->setText("✗ No valid stages found");
        m_playButton->setEnabled(false);
        return false;
    }

    return true;
}

Stage SessionDialog::parseLine(const QString &line, bool &ok, QString &error)
{
    Stage stage;
    ok = false;
    error.clear();

    QStringList parts = line.split(':');

    // Support both 5-field (without volume) and 6-field (with volume) formats
    if (parts.size() != 5 && parts.size() != 6) {
        error = QString("Need 5 or 6 parts separated by ':' (got %1)").arg(parts.size());
        return stage;
    }

    // Parse tone type FIRST
    QString typeStr = parts[0].trimmed().toUpper();
    if (typeStr == "BINAURAL") {
        stage.toneType = 0;
    } else if (typeStr == "ISOCHRONIC") {
        stage.toneType = 1;
    } else if (typeStr == "GENERATOR") {
        stage.toneType = 2;
    } else {
        error = "Invalid type. Use: BINAURAL, ISOCHRONIC, or GENERATOR";
        return stage;
    }

    // Parse left frequency (always carrier frequency)
    bool leftFreqOk;
    stage.leftFreq = parts[1].trimmed().toDouble(&leftFreqOk);
    if (!leftFreqOk) {
        error = "Invalid left/carrier frequency";
        return stage;
    }

    // Parse "right" frequency - MEANING DEPENDS ON TONE TYPE
    bool rightFreqOk;
    double parsedRight = parts[2].trimmed().toDouble(&rightFreqOk);
    if (!rightFreqOk) {
        error = "Invalid frequency number";
        return stage;
    }

    // CRITICAL: Assign rightFreq and pulseFreq based on tone type
    if (stage.toneType == 1) { // ISOCHRONIC
        stage.rightFreq = stage.leftFreq; // Right channel = left (carrier)
        stage.pulseFreq = parsedRight;    // Pulse = parsed "right" field
    } else {
        // For BINAURAL/GENERATOR: normal interpretation
        stage.rightFreq = parsedRight;    // Right channel frequency
        stage.pulseFreq = 7.83;           // Default pulse frequency
    }

    // Parse waveform
    QString waveStr = parts[3].trimmed().toUpper();
    if (waveStr == "SINE") {
        stage.waveform = 0;
    } else if (waveStr == "SQUARE") {
        stage.waveform = 1;
    } else if (waveStr == "TRIANGLE") {
        stage.waveform = 2;
    } else if (waveStr == "SAWTOOTH") {
        stage.waveform = 3;
    } else {
        error = "Invalid waveform. Use: SINE, SQUARE, TRIANGLE, SAWTOOTH";
        return stage;
    }

    // Parse duration (now field 4)
    bool timeOk1;
    stage.durationMinutes = parts[4].trimmed().toInt(&timeOk1);
    if (!timeOk1) {
        error = "Invalid duration number";
        return stage;
    }

    // Parse volume (field 5, optional)
    if (parts.size() == 6) {
        bool volumeOk;
        stage.volumePercent = parts[5].trimmed().toDouble(&volumeOk);
        if (!volumeOk) {
            error = "Invalid volume number";
            return stage;
        }
        // Validate volume range
        if (stage.volumePercent < 0.0 || stage.volumePercent > 100.0) {
            error = "Volume must be 0-100%";
            return stage;
        }
    } else {
        // 5-field format: default volume
        stage.volumePercent = 15.0;
    }

    ok = true;
    return stage;
}

bool SessionDialog::validateStage(const Stage &stage, int lineNum, QString &error)
{
    // Carrier frequency validation (always applies)
    if (stage.leftFreq < 20.0 || stage.leftFreq > 20000.0) {
        error = "Carrier/left frequency must be 20-20000 Hz";
        return false;
    }

    // Tone-specific validations
    if (stage.toneType == 0) { // BINAURAL
        // Right channel frequency
        if (stage.rightFreq < 20.0 || stage.rightFreq > 20000.0) {
            error = "Right frequency must be 20-20000 Hz";
            return false;
        }

        // Ensure right > left for positive beat
        /*
        if (stage.rightFreq <= stage.leftFreq) {
            error = "BINAURAL requires right frequency > left frequency";
            return false;
        }
        */

    } else if (stage.toneType == 1) { // ISOCHRONIC
        // For ISOCHRONIC: rightFreq should equal leftFreq (carrier)
        if (qAbs(stage.rightFreq - stage.leftFreq) > 0.1) {
            error = "ISOCHRONIC carrier mismatch (right should equal left)";
            return false;
        }
        // Pulse frequency validation
        if (stage.pulseFreq < 0.5 || stage.pulseFreq > 100.0) {
            error = "ISOCHRONIC pulse must be 0.5-100 Hz";
            return false;
        }

    } else if (stage.toneType == 2) { // GENERATOR
        // For GENERATOR: frequencies should match

        /*
        if (qAbs(stage.rightFreq - stage.leftFreq) > 0.1) {
            error = "GENERATOR requires left = right frequency";
            return false;
        }
        */

    }

    // Duration validation
    if (stage.durationMinutes < 1) {
        error = "Duration must be at least 1 minute";
        return false;
    }

    // Check unlimited setting
    int maxMinutes = m_unlimitedDuration ? 360 : 45;
    if (stage.durationMinutes > maxMinutes) {
        error = QString("Duration exceeds maximum (%1 min)").arg(maxMinutes);
        return false;
    }

    // Volume validation (always applies)
    if (stage.volumePercent < 0.0 || stage.volumePercent > 100.0) {
        error = "Volume must be 0-100%";
        return false;
    }

    return true;
}



void SessionDialog::onLoadClicked()
{
    if (m_sessionActive) {
        QMessageBox::warning(this, "Session Active",
                           "Cannot load new session while one is active.");
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Load Session",                          // caption
        ConstantGlobals::presetFilePath,         // initial directory
        "Session Files (*.txt *.bsession);;"
        "All Files (*)"
    );

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Load Error",
                           "Could not open file for reading.");
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    m_textEdit->setPlainText(content);

    // Auto-parse after load
    //if (parseStagesFromText()) {
        m_parseButton->click();
        m_statusLabel->setText(QString("✓ Loaded and parsed %1 stage(s)").arg(m_stages.size()));
    //}
}

void SessionDialog::onSaveClicked()
{
    // Validate before saving
    if (!parseStagesFromText()) {
        QMessageBox::warning(this, "Validation Error",
                           "Cannot save invalid session. Please fix errors first.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Save Session",                          // caption
        ConstantGlobals::presetFilePath + "/Session",         // initial directory / path
        "Session Files (*.bsession);;"
        "Text Files (*.txt);;"
        "All Files (*)"
    );

    if (fileName.isEmpty()) return;

    // Ensure file extension
    if (!fileName.endsWith(".bsession") && !fileName.endsWith(".txt")) {
        fileName += ".bsession";
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Save Error",
                           "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);
    out << m_textEdit->toPlainText();
    file.close();

    m_statusLabel->setText(QString("✓ Session saved to: %1").arg(QFileInfo(fileName).fileName()));
}

void SessionDialog::onClearClicked()
{
    if (m_sessionActive) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
            "Session Active",
            "A session is currently active. Stop and clear?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);

        if (reply == QMessageBox::No) return;

        endSession();
    }

    m_textEdit->clear();
    m_stages.clear();
    m_currentStageIndex = -1;

    m_statusLabel->setText("Cleared");
    m_stageInfoLabel->setText("Stage: --/-- | Time: --:--/--:--");
    m_totalTimeLabel->setText("Total: --:--");
    m_playButton->setEnabled(false);

    highlightCurrentStage(); // Clears any highlights
}

void SessionDialog::onPlayClicked()
{
    if (!m_sessionActive) {
        startSession();
    } else if (m_paused) {
        resumeSession();
    }
}

void SessionDialog::startSession()
{
    if (m_stages.isEmpty()) {
        QMessageBox::warning(this, "No Session",
                           "No valid stages to play. Parse first.");
        return;
    }

    // Emit signal to MainWindow that session is starting
    emit sessionStarted(m_totalTimeRemainingSec);

    m_sessionActive = true;
    m_paused = false;
    m_currentStageIndex = 0;

    // Start first stage
    startStage(0);

    // Update UI state
    updateUIFromState();
}

void SessionDialog::startStage(int index)
{
    if (index < 0 || index >= m_stages.size()) return;

    m_currentStageIndex = index;
    const Stage &stage = m_stages[index];

    //emit syncTimersRequested(m_currentStageIndex, m_totalTimeRemainingSec);

    //critical: do not delete or comment out next line
    m_stageTimeRemainingSec = stage.durationSeconds();


    // Update parameters in MainWindow via signal
    emit stageChanged(stage.toneType, stage.leftFreq, stage.rightFreq,
                      stage.waveform, stage.pulseFreq, stage.volumePercent);


    //dirty fix to remedy one sec diff between the timers
    if(m_currentStageIndex > 0) {
        m_stageTimeRemainingSec--;
        m_totalTimeRemainingSec--;
    }

    emit fadeRequested(stage.volumePercent);

    // Set stage timer
    //m_stageTimeRemainingSec = stage.durationSeconds();

    if (!m_paused) {
        m_stageTimer->start();
    }

    // Highlight current line
    highlightCurrentStage();
}

void SessionDialog::onPauseClicked()
{
    if (!m_sessionActive) return;

    if (m_paused) {
        resumeSession();
    } else {
        pauseSession();
    }
}

void SessionDialog::pauseSession()
{
    m_paused = true;
    emit pauseRequested();
    m_stageTimer->stop();
    updateUIFromState();
}

void SessionDialog::resumeSession()
{
    m_paused = false;
    emit resumeRequested();
    m_stageTimer->start();

    updateUIFromState();
}

void SessionDialog::onStopClicked()
{
    endSession();
    m_parseButton->click();


}

void SessionDialog::stopSession()
{
    // Public method called from MainWindow
    endSession();
}

void SessionDialog::endSession()
{
    if (!m_sessionActive) return;

    m_sessionActive = false;
    m_paused = false;

    // Stop timers
    m_stageTimer->stop();

    // Update UI
    updateUIFromState();

    // Clear highlight
    highlightCurrentStage();

    m_parseButton->click();

    // Emit signal to MainWindow
    emit sessionEnded();
}

void SessionDialog::onStageTimerTimeout()
{
    if(m_stageTimeRemainingSec == 5){
//        emit syncTimersRequested(m_currentStageIndex, m_totalTimeRemainingSec);
        emit fadeRequested(0.0);
    }

    if (m_stageTimeRemainingSec <= 0) {
        // Stage completed
        if (m_currentStageIndex < m_stages.size() - 1) {
            // Start transition to next stage
            startTransition(m_currentStageIndex, m_currentStageIndex + 1);
        } else {
            // Last stage completed
            emit sessionEnded();
           // endSession();

        }
        return;
    }

    m_stageTimeRemainingSec--;
    m_totalTimeRemainingSec--;

    // Update UI
    updateUIFromState();

}

QPushButton *SessionDialog::stopButton() const
{
    return m_stopButton;
}

QPushButton *SessionDialog::pauseButton() const
{
    return m_pauseButton;
}

void SessionDialog::setPauseButton(QPushButton *newPauseButton)
{
    m_pauseButton = newPauseButton;
}




void SessionDialog::startTransition(int fromIndex, int toIndex)
{

    m_currentStageIndex = toIndex;
    startStage(toIndex);
}

void SessionDialog::updateUIFromState()
{
    // Update button states
    bool hasStages = !m_stages.isEmpty();

    m_parseButton->setEnabled(!m_sessionActive);
    m_loadButton->setEnabled(!m_sessionActive);
    m_saveButton->setEnabled(!m_sessionActive);
    m_clearButton->setEnabled(true); // Always enabled

    m_playButton->setEnabled(hasStages && !m_sessionActive);
    m_pauseButton->setEnabled(m_sessionActive);
    m_stopButton->setEnabled(m_sessionActive);

    // Update button text based on state
    if (m_sessionActive) {
        m_playButton->setText(m_paused ? "▶ &Resume" : "▶ &Playing");
        m_pauseButton->setText(m_paused ? "⏸ &Paused" : "⏸ &Pause");
    } else {
        m_playButton->setText("▶ &Play");
        m_pauseButton->setText("⏸ &Pause");
    }

    // Update status labels
    if (m_sessionActive && m_currentStageIndex >= 0 && m_currentStageIndex < m_stages.size()) {
        const Stage &stage = m_stages[m_currentStageIndex];

        QString stageTime;
        formatTimeString(m_stageTimeRemainingSec, stageTime);

        QString totalTime;
        formatTimeString(m_totalTimeRemainingSec, totalTime);

        QString typeStr;
        switch (stage.toneType) {
            case 0: typeStr = "BIN"; break;
            case 1: typeStr = "ISO"; break;
            case 2: typeStr = "GEN"; break;
        }

        m_stageInfoLabel->setText(
                    QString("Stage: %1/%2 %3 | Time: %4/%5")
                        .arg(m_currentStageIndex + 1)
                        .arg(m_stages.size())
                        .arg(typeStr)
                        .arg(stageTime)
                        .arg(stage.durationMinutes));
        m_totalTimeLabel->setText(QString("Total: %1").arg(totalTime));

        if (m_paused) {
            m_statusLabel->setText("⏸ Paused");
        } else {
            m_statusLabel->setText("▶ Playing");
        }
    } else {
        if (m_stages.isEmpty()) {
            m_statusLabel->setText("No session loaded");
        } else {
            m_statusLabel->setText(QString("%1 stage(s) ready").arg(m_stages.size()));
        }
    }

    m_textEdit->setReadOnly(m_sessionActive);
}

void SessionDialog::highlightCurrentStage()
{
    // Clear all formatting first
    QTextCursor cursor(m_textEdit->document());
    cursor.select(QTextCursor::Document);
    QTextCharFormat defaultFormat;
    cursor.mergeCharFormat(defaultFormat);

    if (m_currentStageIndex < 0 || !m_sessionActive) {
        return;
    }

    // Find and highlight the current stage line
    QString text = m_textEdit->toPlainText();
    QStringList lines = text.split('\n');

    int currentLine = -1;
    int parsedLines = 0;

    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        if (parsedLines == m_currentStageIndex) {
            currentLine = i;
            break;
        }
        parsedLines++;
    }

    if (currentLine >= 0) {
        QTextCursor highlightCursor(m_textEdit->document());
        highlightCursor.movePosition(QTextCursor::Start);
        for (int i = 0; i < currentLine; ++i) {
            highlightCursor.movePosition(QTextCursor::Down);
        }
        highlightCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

        QTextCharFormat highlightFormat;
        highlightFormat.setBackground(QColor(255, 255, 200)); // Light yellow
        highlightFormat.setFontWeight(QFont::Bold);
        highlightCursor.mergeCharFormat(highlightFormat);

        // Ensure visible
        m_textEdit->setTextCursor(highlightCursor);
        m_textEdit->ensureCursorVisible();
    }
}

void SessionDialog::calculateTotalTime()
{
    m_totalTimeRemainingSec = 0;

    for (int i = 0; i < m_stages.size(); ++i) {
        const Stage &stage = m_stages[i];
        m_totalTimeRemainingSec += stage.durationSeconds();


    }
}

void SessionDialog::formatTimeString(int seconds, QString &timeStr)
{
    int minutes = seconds / 60;
    int secs = seconds % 60;
    timeStr = QString("%1:%2").arg(minutes, 2, 10, QLatin1Char('0'))
                              .arg(secs, 2, 10, QLatin1Char('0'));
}

void SessionDialog::closeEvent(QCloseEvent *event)
{

        emit dialogHidden();
}
