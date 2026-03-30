#include "vistimdialog.h"
#include "flickerwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QSlider>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QButtonGroup>
#include <QColorDialog>
#include <QMessageBox>
#include <QFrame>
#include <QtMath>
#include "constants.h"

// ── helpers ──────────────────────────────────────────────────────────────────

static QString swatchStyle(const QColor &c)
{
    return QString(
        "background:%1;"
        "border:1px solid rgba(255,255,255,0.25);"
        "border-radius:4px;"
    ).arg(c.name(QColor::HexArgb));
}

static QFrame *hline()
{
    auto *f = new QFrame();
    f->setFrameShape(QFrame::HLine);
    f->setFrameShadow(QFrame::Sunken);
    return f;
}

// ── VisStimDialog ─────────────────────────────────────────────────────────────

VisStimDialog::VisStimDialog(FlickerWidget *flicker, QWidget *parent)
    : QDialog(parent)
    , m_flicker(flicker)
{
    setWindowTitle("Visual Stimulation");
    setModal(false);
    setMinimumWidth(480);
    buildUi();
    updateSyncBadges();

    // Connect to flicker signals to keep UI in sync
    if (m_flicker) {
        connect(m_flicker, &FlickerWidget::flickerStopped, this, [this]() {
            // Flicker stopped externally (e.g., by main toolbar button)
            m_startStopBtn->setText("▶  Start");
            m_running = false;
        });

        connect(m_flicker, &FlickerWidget::flickerStarted, this, [this]() {
            // Flicker started externally
            m_startStopBtn->setText("■  Stop");
            m_running = true;
        });
    }
}

// ── public sync API ──────────────────────────────────────────────────────────

void VisStimDialog::syncFrequency(double hz)
{
    m_syncedFreq = hz;
    updateSyncBadges();
    if (!m_freqOverrideCb->isChecked()) {
        //m_flicker->setFrequency(hz);
        updateBandLabel(hz);
    }
}

void VisStimDialog::syncWaveType(int type)
{
    m_syncedWave = type;
    updateSyncBadges();
    if (!m_envOverrideCb->isChecked()) {
        m_flicker->setEnvelope(static_cast<FlickerWidget::Envelope>(type));
    }
}

// ── private slots ────────────────────────────────────────────────────────────

void VisStimDialog::onFreqOverrideToggled(bool checked)
{
    m_freqSpin->setEnabled(checked);
    m_freqSyncBadge->setEnabled(!checked);
    if (!checked) {
        // snap spinbox back to synced value and revert flicker
        m_freqSpin->setValue(m_syncedFreq);
        m_flicker->setFrequency(m_syncedFreq);
        updateBandLabel(m_syncedFreq);
    }
}

void VisStimDialog::onEnvOverrideToggled(bool checked)
{
    // enable/disable the three envelope buttons
    for (auto *btn : m_envGroup->buttons())
        btn->setEnabled(checked);
    m_envSyncBadge->setEnabled(!checked);
    if (!checked)
        m_flicker->setEnvelope(static_cast<FlickerWidget::Envelope>(m_syncedWave));
}

void VisStimDialog::onFreqSpinChanged(double hz)
{
    updateBandLabel(hz);
    m_flicker->setFrequency(hz);
}

void VisStimDialog::onIntensityChanged(int value)
{
    m_intensityLabel->setText(QString("%1%").arg(value));
    m_flicker->setIntensity(value / 100.0f);
}

void VisStimDialog::onPickOnColor()
{
    QColor c = QColorDialog::getColor(m_onColor, this, "Flicker — on color");
    if (c.isValid()) {
        m_onColor = c;
        setSwatchColor(m_onColorBtn, c);
        m_flicker->setOnColor(c);
    }
}

void VisStimDialog::onPickOffColor()
{
    QColor c = QColorDialog::getColor(m_offColor, this, "Flicker — off color");
    if (c.isValid()) {
        m_offColor = c;
        setSwatchColor(m_offColorBtn, c);
        m_flicker->setOffColor(c);
    }
}

void VisStimDialog::onPickTextColor()
{
    QColor c = QColorDialog::getColor(m_textColor, this, "Text color");
    if (c.isValid()) {
        m_textColor = c;
        setSwatchColor(m_textColorBtn, c);
        m_flicker->setSubliminalColor(c);
    }
}

void VisStimDialog::onPickTextBgColor()
{
    // allow alpha so user can pick transparent
    QColor c = QColorDialog::getColor(
        m_textBgColor, this, "Text background",
        QColorDialog::ShowAlphaChannel);
    if (c.isValid()) {
        m_textBgColor = c;
        setSwatchColor(m_textBgBtn, c);
        m_flicker->setSubliminalBgColor(c);
    }
}

void VisStimDialog::onStartStop()
{
    if (!m_running) {
        // photosensitivity warning on first start
        static bool warned = false;
        if (!warned) {
            QMessageBox::warning(this, "Photosensitivity warning",
                "This feature produces flickering light effects.\n"
                "Do not use if you are sensitive to flashing lights\n"
                "or have a history of photosensitive epilepsy.");
            warned = true;
        }
        applyToFlicker();
        if(ConstantGlobals::currentToneType == 0 || ConstantGlobals::currentToneType == 2){
        m_flicker->setFrequency(ConstantGlobals::currentBinFreq);

        m_freqSyncBadge->setText(
            QString("⟳  %1 Hz · %2")
                .arg(ConstantGlobals::currentBinFreq, 0, 'f', 2)
                .arg(bandName(ConstantGlobals::currentBinFreq)));

        }else{

        m_flicker->setFrequency(ConstantGlobals::currentIsonFreq);
        m_freqSyncBadge->setText(
            QString("⟳  %1 Hz · %2")
                .arg(ConstantGlobals::currentBinFreq, 0, 'f', 2)
                .arg(bandName(ConstantGlobals::currentIsonFreq)));
        }

        m_flicker->startFlicker();
        m_startStopBtn->setText("■  Stop");
        m_running = true;
    } else {
        m_flicker->stopFlicker();
        m_startStopBtn->setText("▶  Start");
        m_running = false;
    }
}

// ── private helpers ──────────────────────────────────────────────────────────

void VisStimDialog::applyToFlicker()
{
    // frequency
    double hz = m_freqOverrideCb->isChecked()
              ? m_freqSpin->value()
              : m_syncedFreq;
    m_flicker->setFrequency(hz);

    // envelope
    if (m_envOverrideCb->isChecked()) {
        int id = m_envGroup->checkedId();
        m_flicker->setEnvelope(static_cast<FlickerWidget::Envelope>(id));
    } else {
        m_flicker->setEnvelope(static_cast<FlickerWidget::Envelope>(m_syncedWave));
    }

    // intensity
    m_flicker->setIntensity(m_intensitySlider->value() / 100.0f);

    // colors
    m_flicker->setOnColor(m_onColor);
    m_flicker->setOffColor(m_offColor);

    // subliminal
    m_flicker->setSubliminalText(m_textEdit->toPlainText().trimmed());
    m_flicker->setSubliminalColor(m_textColor);
    m_flicker->setSubliminalBgColor(m_textBgColor);
    m_flicker->setSubliminalFontSize(m_fontSizeSpin->value());
    m_flicker->setSubliminalMode(m_displayGroup->checkedId());
}

void VisStimDialog::updateSyncBadges()
{
    QString envNames[] = { "sine", "square", "triangle", "sawtooth" };
    QString env = (m_syncedWave >= 0 && m_syncedWave < 4)
                ? envNames[m_syncedWave] : "?";

    m_freqSyncBadge->setText(
        QString("⟳  %1 Hz · %2")
            .arg(m_syncedFreq, 0, 'f', 1)
            .arg(bandName(m_syncedFreq)));

    m_envSyncBadge->setText(QString("⟳  %1").arg(env));
}

void VisStimDialog::updateBandLabel(double hz)
{
    m_bandLabel->setText(bandName(hz));
}

QString VisStimDialog::bandName(double hz) const
{
    if (hz < 4.0)  return "delta";
    if (hz < 8.0)  return "theta";
    if (hz < 12.0) return "alpha";
    if (hz < 30.0) return "beta";
    return "gamma";
}

void VisStimDialog::setSwatchColor(QPushButton *btn, const QColor &color)
{
    btn->setStyleSheet(swatchStyle(color));
}

QPushButton *VisStimDialog::startStopBtn() const
{
    return m_startStopBtn;
}

// ── buildUi ──────────────────────────────────────────────────────────────────

void VisStimDialog::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setSpacing(12);
    root->setContentsMargins(16, 16, 16, 16);

    // ════════════════════════════════════════════════════════════════
    // LEFT + RIGHT side by side
    // ════════════════════════════════════════════════════════════════
    auto *columns = new QHBoxLayout();
    columns->setSpacing(16);
    root->addLayout(columns);

    // ── LEFT: flicker engine ─────────────────────────────────────────
    auto *flickerGroup = new QGroupBox("Flicker engine");
    auto *flickerLayout = new QVBoxLayout(flickerGroup);
    flickerLayout->setSpacing(10);
    columns->addWidget(flickerGroup, 1);

    // — frequency row —
    {
        auto *row = new QVBoxLayout();

        m_freqSyncBadge = new QLabel();
        m_freqSyncBadge->setStyleSheet(
            "color: #2e7d32; background: #e8f5e9;"
            "border-radius:4px; padding:2px 8px; font-size:11px;");

        m_freqOverrideCb = new QCheckBox("Override frequency");
        connect(m_freqOverrideCb, &QCheckBox::toggled,
                this, &VisStimDialog::onFreqOverrideToggled);

        auto *spinRow = new QHBoxLayout();
        m_freqSpin = new QDoubleSpinBox();
        m_freqSpin->setRange(0.5, 100.0);
        m_freqSpin->setSingleStep(0.5);
        m_freqSpin->setDecimals(2);
        m_freqSpin->setSuffix(" Hz");
        m_freqSpin->setValue(m_syncedFreq);
        m_freqSpin->setEnabled(false);
        connect(m_freqSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, &VisStimDialog::onFreqSpinChanged);

        m_bandLabel = new QLabel(bandName(m_syncedFreq));
        m_bandLabel->setStyleSheet(
            "color:#555; background:#f0f0f0;"
            "border-radius:3px; padding:2px 6px; font-size:11px;");
        m_bandLabel->setEnabled(false);

        spinRow->addWidget(m_freqSpin);
        spinRow->addWidget(m_bandLabel);
        spinRow->addStretch();

        row->addWidget(m_freqSyncBadge);
        row->addWidget(m_freqOverrideCb);
        row->addLayout(spinRow);
        flickerLayout->addLayout(row);
    }

    flickerLayout->addWidget(hline());

    // — envelope row —
    {
        auto *row = new QVBoxLayout();

        m_envSyncBadge = new QLabel();
        m_envSyncBadge->setStyleSheet(
            "color: #2e7d32; background: #e8f5e9;"
            "border-radius:4px; padding:2px 8px; font-size:11px;");

        m_envOverrideCb = new QCheckBox("Override envelope");
        connect(m_envOverrideCb, &QCheckBox::toggled,
                this, &VisStimDialog::onEnvOverrideToggled);

        auto *btnRow = new QHBoxLayout();
        m_envGroup = new QButtonGroup(this);
        m_envGroup->setExclusive(true);

        QStringList envNames = { "Sine", "Square", "Triangle", "Sawtooth" };
        for (int i = 0; i < 4; ++i) {
            auto *btn = new QPushButton(envNames[i]);
            btn->setCheckable(true);
            btn->setEnabled(false);
            if (i == 0) btn->setChecked(true);  // default: sine matches audio
            m_envGroup->addButton(btn, i);
            btnRow->addWidget(btn);
        }

        connect(m_envGroup, QOverload<int>::of(&QButtonGroup::idClicked),
                this, [this](int id) {
            m_flicker->setEnvelope(static_cast<FlickerWidget::Envelope>(id));
        });

        row->addWidget(m_envSyncBadge);
        row->addWidget(m_envOverrideCb);
        row->addLayout(btnRow);
        flickerLayout->addLayout(row);
    }

    flickerLayout->addWidget(hline());

    // — intensity —
    {
        auto *row = new QHBoxLayout();
        row->addWidget(new QLabel("Intensity"));
        m_intensitySlider = new QSlider(Qt::Horizontal);
        m_intensitySlider->setRange(5, 100);
        m_intensitySlider->setValue(40);
        connect(m_intensitySlider, &QSlider::valueChanged,
                this, &VisStimDialog::onIntensityChanged);
        m_intensityLabel = new QLabel("40%");
        m_intensityLabel->setFixedWidth(36);
        row->addWidget(m_intensitySlider, 1);
        row->addWidget(m_intensityLabel);
        flickerLayout->addLayout(row);
    }

    // — colors —
    {
        auto *row = new QHBoxLayout();
        row->addWidget(new QLabel("Colors"));

        m_onColorBtn = new QPushButton();
        m_onColorBtn->setFixedSize(28, 28);
        m_onColorBtn->setToolTip("On color");
        setSwatchColor(m_onColorBtn, m_onColor);
        connect(m_onColorBtn, &QPushButton::clicked,
                this, &VisStimDialog::onPickOnColor);

        m_offColorBtn = new QPushButton();
        m_offColorBtn->setFixedSize(28, 28);
        m_offColorBtn->setToolTip("Off color");
        setSwatchColor(m_offColorBtn, m_offColor);
        connect(m_offColorBtn, &QPushButton::clicked,
                this, &VisStimDialog::onPickOffColor);

        row->addWidget(m_onColorBtn);
        row->addWidget(new QLabel("on"));
        row->addSpacing(8);
        row->addWidget(m_offColorBtn);
        row->addWidget(new QLabel("off"));
        row->addStretch();
        flickerLayout->addLayout(row);
    }

    flickerLayout->addStretch();

    // ── RIGHT: subliminal text ────────────────────────────────────────
    auto *subGroup = new QGroupBox("Subliminal text");
    auto *subLayout = new QVBoxLayout(subGroup);
    subLayout->setSpacing(10);
    columns->addWidget(subGroup, 1);

    // — message —
    m_textEdit = new QTextEdit();
    m_textEdit->setPlaceholderText("Enter message…");
    m_textEdit->setFixedHeight(64);
    connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
        m_flicker->setSubliminalText(m_textEdit->toPlainText().trimmed());
    });
    subLayout->addWidget(m_textEdit);

    // — display mode —
    {
        auto *row = new QHBoxLayout();
        row->addWidget(new QLabel("Display"));
        m_displayGroup = new QButtonGroup(this);
        m_displayGroup->setExclusive(true);
        QStringList modes = { "Off", "Flash", "Always" };
        for (int i = 0; i < 3; ++i) {
            auto *btn = new QPushButton(modes[i]);
            btn->setCheckable(true);
            if (i == 0) btn->setChecked(true);
            m_displayGroup->addButton(btn, i);
            row->addWidget(btn);
        }
        connect(m_displayGroup, QOverload<int>::of(&QButtonGroup::idClicked),
                this, [this](int id) {
            m_flicker->setSubliminalMode(id);
        });
        subLayout->addLayout(row);
    }

    // — font size —
    {
        auto *row = new QHBoxLayout();
        row->addWidget(new QLabel("Font size"));
        m_fontSizeSpin = new QSpinBox();
        m_fontSizeSpin->setRange(8, 120);
        m_fontSizeSpin->setValue(24);
        m_fontSizeSpin->setSuffix(" px");
        connect(m_fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged),
                this, [this](int v) {
            m_flicker->setSubliminalFontSize(v);
        });
        row->addWidget(m_fontSizeSpin);

        row->addSpacing(16);

        row->addWidget(new QLabel("Subliminal"));
        m_subliminalFactorSpin = new QDoubleSpinBox();
        m_subliminalFactorSpin->setRange(0.5, 10.0);
        m_subliminalFactorSpin->setSingleStep(0.5);
        m_subliminalFactorSpin->setDecimals(1);
        m_subliminalFactorSpin->setValue(3.0);
        m_subliminalFactorSpin->setToolTip("Higher = more subliminal");
        connect(m_subliminalFactorSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, [this](double v) {
            m_flicker->setSubliminalFactor(static_cast<float>(v));
        });
        row->addWidget(m_subliminalFactorSpin);
        row->addStretch();
        subLayout->addLayout(row);
    }

    // — text colors —
    {
        auto *row = new QHBoxLayout();
        row->addWidget(new QLabel("Text color"));

        m_textColorBtn = new QPushButton();
        m_textColorBtn->setFixedSize(28, 28);
        m_textColorBtn->setToolTip("Text foreground color");
        setSwatchColor(m_textColorBtn, m_textColor);
        connect(m_textColorBtn, &QPushButton::clicked,
                this, &VisStimDialog::onPickTextColor);

        m_textBgBtn = new QPushButton();
        m_textBgBtn->setFixedSize(28, 28);
        m_textBgBtn->setToolTip("Text background (alpha supported)");
        setSwatchColor(m_textBgBtn, m_textBgColor);
        connect(m_textBgBtn, &QPushButton::clicked,
                this, &VisStimDialog::onPickTextBgColor);

        row->addWidget(m_textColorBtn);
        row->addWidget(new QLabel("fg"));
        row->addSpacing(8);
        row->addWidget(m_textBgBtn);
        row->addWidget(new QLabel("bg"));
        row->addStretch();
        subLayout->addLayout(row);
    }

    subLayout->addStretch();

    // ════════════════════════════════════════════════════════════════
    // footer
    // ════════════════════════════════════════════════════════════════
    root->addWidget(hline());

    auto *footer = new QHBoxLayout();
    footer->addStretch();

    auto *resetBtn = new QPushButton("↺  Reset");
    connect(resetBtn, &QPushButton::clicked, this, &VisStimDialog::onReset);
    footer->addWidget(resetBtn);

    auto *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::hide);
    footer->addWidget(closeBtn);

    m_startStopBtn = new QPushButton("▶  Start");
    m_startStopBtn->setDefault(true);
    connect(m_startStopBtn, &QPushButton::clicked,
            this, &VisStimDialog::onStartStop);
    footer->addWidget(m_startStopBtn);

    root->addLayout(footer);
}

void VisStimDialog::onReset()
{
    // stop flicker if running
    if (m_running) {
        m_flicker->stopFlicker();
        m_startStopBtn->setText("▶  Start");
        m_running = false;
    }

    // frequency
    m_freqOverrideCb->setChecked(false);
    m_freqSpin->setValue(m_syncedFreq);
    m_freqSpin->setEnabled(false);

    // envelope
    m_envOverrideCb->setChecked(false);
    for (auto *btn : m_envGroup->buttons())
        btn->setEnabled(false);
    m_envGroup->button(0)->setChecked(true); // default sine

    // intensity
    m_intensitySlider->setValue(40);
    m_intensityLabel->setText("40%");
    m_flicker->setIntensity(0.4f);

    // colors
    m_onColor  = Qt::white;
    m_offColor = Qt::black;
    setSwatchColor(m_onColorBtn,  m_onColor);
    setSwatchColor(m_offColorBtn, m_offColor);
    m_flicker->setOnColor(m_onColor);
    m_flicker->setOffColor(m_offColor);

    // subliminal
    m_textEdit->clear();
    m_displayGroup->button(0)->setChecked(true); // Off
    m_fontSizeSpin->setValue(24);
    m_subliminalFactorSpin->setValue(3.0);

    m_textColor   = Qt::white;
    m_textBgColor = QColor(0, 0, 0, 0);
    setSwatchColor(m_textColorBtn, m_textColor);
    setSwatchColor(m_textBgBtn,    m_textBgColor);

    m_flicker->setSubliminalText("");
    m_flicker->setSubliminalColor(m_textColor);
    m_flicker->setSubliminalBgColor(m_textBgColor);
    m_flicker->setSubliminalFontSize(24);
    m_flicker->setSubliminalMode(0);
    m_flicker->setSubliminalFactor(3.0f);
}
