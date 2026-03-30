#include "flickerwidget.h"

#include <QOpenGLFunctions>
#include <QPainter>
#include <QFont>
#include <QImage>
#include <QPixmap>
#include <QtMath>
#include <QDebug>
#include <QMenu>
#include <QContextMenuEvent>

// ── GLSL shaders ──────────────────────────────────────────────────────────────

static const char *VERT_SRC = R"(
#version 120
attribute vec2 aPos;
varying vec2 vUV;
void main() {
    // aPos is -1..1; convert to 0..1 UV
    vUV = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.0, 1.0);
}
)";

static const char *FRAG_SRC = R"(
#version 120
uniform float     uTime;
uniform float     uFreq;
uniform float     uIntensity;
uniform int       uEnvelope;       // 0=sine 1=square 2=triangle 3=sawtooth
uniform vec3      uOnColor;
uniform vec3      uOffColor;
uniform bool      uUseTex;
uniform sampler2D uTexSampler;
uniform bool uFlash;
varying vec2 vUV;

void main() {
    float phase = mod(uTime * uFreq, 1.0);

    float brightness;
    if (uEnvelope == 0) {
        brightness = 0.5 + 0.5 * sin(2.0 * 3.14159265 * phase - 1.5707963); // Sine
    } else if (uEnvelope == 1) {
        brightness = phase < 0.5 ? 1.0 : 0.0;                               // Square
    } else if (uEnvelope == 2) {
        brightness = 1.0 - abs(1.0 - 2.0 * phase);                          // Triangle
    } else {
        brightness = phase;                                                   // Sawtooth
}

    brightness *= uIntensity;
    vec3 col = mix(uOffColor, uOnColor, brightness);

    if (uUseTex) {
    vec4 texel = texture2D(uTexSampler, vUV);
    float alpha = uFlash ? texel.a * brightness : texel.a;
    col = mix(col, texel.rgb, alpha);
    }

    gl_FragColor = vec4(col, 1.0);
}
)";

// ── full-screen quad ──────────────────────────────────────────────────────────

static const float QUAD[] = {
    -1.f, -1.f,
     1.f, -1.f,
    -1.f,  1.f,
     1.f,  1.f,
};

// ── FlickerWidget ─────────────────────────────────────────────────────────────

FlickerWidget::FlickerWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    connect(&m_renderTimer, &QTimer::timeout, this, [this]() {
        if (m_running) update();
    });
    m_renderTimer.setInterval(16);
}

FlickerWidget::~FlickerWidget()
{
    makeCurrent();
    if (m_textTex) glDeleteTextures(1, &m_textTex);
    delete m_program;
    doneCurrent();
}

// ── public control ────────────────────────────────────────────────────────────

void FlickerWidget::startFlicker()
{
    m_running = true;
    m_elapsed.restart();
    m_renderTimer.start();
    show();
    raise();
    update();
    emit flickerStarted();
}

void FlickerWidget::stopFlicker()
{
    m_running = false;
    m_renderTimer.stop();
    emit flickerStopped();
}

void FlickerWidget::setFrequency(double hz)
{
    m_frequency = qBound(0.5, hz, 100.0);
}

void FlickerWidget::setEnvelope(Envelope env)
{
    m_envelope = env;
}

void FlickerWidget::setIntensity(float intensity)
{
    m_intensity = qBound(0.0f, intensity, 1.0f);
}

void FlickerWidget::setOnColor(const QColor &color)
{
    m_onColor = color;
}

void FlickerWidget::setOffColor(const QColor &color)
{
    m_offColor = color;
}

void FlickerWidget::setSubliminalText(const QString &text)
{
    m_subliminalText = text;
    rebuildTextTexture();
}

void FlickerWidget::setSubliminalColor(const QColor &color)
{
    m_subliminalColor = color;
    rebuildTextTexture();
}

void FlickerWidget::setSubliminalBgColor(const QColor &color)
{
    m_subliminalBg = color;
    rebuildTextTexture();
}

void FlickerWidget::setSubliminalFontSize(int px)
{
    m_subliminalFontPx = px;
    rebuildTextTexture();
}

void FlickerWidget::setSubliminalMode(int mode)
{
    m_subliminalMode = mode;
    rebuildTextTexture();
}

// ── OpenGL ────────────────────────────────────────────────────────────────────

void FlickerWidget::initializeGL()
{
    initializeOpenGLFunctions();
    buildShader();
    glClearColor(0.f, 0.f, 0.f, 1.f);
    rebuildTextTexture();
}

void FlickerWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    rebuildTextTexture();
}



void FlickerWidget::paintGL()
{
    if (!m_running) {
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    float t        = static_cast<float>(m_elapsed.elapsed()) / 1000.0f;
    float phase    = std::fmod(t * static_cast<float>(m_frequency), 1.0f);
    int   dynamicN = qMax(1, static_cast<int>(m_frequency * m_subliminalFactor));
    //bool  texFrame = phase < (1.0f / static_cast<float>(m_subliminalEveryN));
    bool  texFrame = phase < (1.0f / static_cast<float>(dynamicN));

    bool  showTex  = m_hasTex && (m_subliminalMode == 1 || m_subliminalMode == 2)
                     && (m_subliminalMode == 1 ? texFrame : true);
    m_program->bind();
    m_program->setUniformValue(m_uTime,      t);
    m_program->setUniformValue(m_uFreq,      static_cast<float>(m_frequency));
    m_program->setUniformValue(m_uIntensity, m_intensity);
    m_program->setUniformValue(m_uEnvelope,  static_cast<int>(m_envelope));
    m_program->setUniformValue(m_uOnColor,
        QVector3D(m_onColor.redF(),  m_onColor.greenF(),  m_onColor.blueF()));
    m_program->setUniformValue(m_uOffColor,
        QVector3D(m_offColor.redF(), m_offColor.greenF(), m_offColor.blueF()));

    if (showTex) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textTex);
        m_program->setUniformValue(m_uTexSampler, 0);
        m_program->setUniformValue(m_uUseTex,     true);
        m_program->setUniformValue(m_uFlash,      m_subliminalMode == 1);
    } else {
        m_program->setUniformValue(m_uUseTex, false);
    }

    int aPos = m_program->attributeLocation("aPos");
    m_program->enableAttributeArray(aPos);
    m_program->setAttributeArray(aPos, GL_FLOAT, QUAD, 2);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    m_program->disableAttributeArray(aPos);

    if (showTex)
        glBindTexture(GL_TEXTURE_2D, 0);

    m_program->release();
}


// ── private helpers ───────────────────────────────────────────────────────────

float FlickerWidget::computeBrightness(float phase) const
{
    switch (m_envelope) {
    case Envelope::Sine:   return phase < 0.5f ? 1.f : 0.f;
    case Envelope::Square:     return 0.5f + 0.5f * std::sin(2.f * M_PI * phase - M_PI_2);
    case Envelope::Triangle:     return 1.f - std::abs(1.f - 2.f * phase);
    case Envelope::Sawtooth: return phase;
    }
    return 0.f;
}

void FlickerWidget::setSubliminalFactor(float newSubliminalFactor)
{
    m_subliminalFactor = newSubliminalFactor;
}

void FlickerWidget::rebuildTextTexture()
{
    // need a valid GL context and a real size
    if (!isValid() || width() <= 0 || height() <= 0)
        return;

    // render text into a transparent pixmap matching the widget size
    QPixmap pm(width(), height());
    pm.fill(Qt::transparent);

    const bool shouldDraw = (m_subliminalMode != 0) && !m_subliminalText.isEmpty();

    if (shouldDraw) {
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);

        QFont font;
        font.setPixelSize(m_subliminalFontPx);
        p.setFont(font);

        QRect textRect = p.fontMetrics().boundingRect(
            QRect(0, 0, width(), height()),
            Qt::AlignCenter,
            m_subliminalText
        );
        textRect.adjust(-8, -4, 8, 4);

        if (m_subliminalBg.alpha() > 0)
            p.fillRect(textRect, m_subliminalBg);

        p.setPen(m_subliminalColor);
        p.drawText(textRect, Qt::AlignCenter, m_subliminalText);
        p.end();
    }

    // GL expects origin at bottom-left; QImage origin is top-left → mirror
    QImage img = pm.toImage()
                   .convertToFormat(QImage::Format_RGBA8888)
                   .mirrored(false, true);

    makeCurrent();

    if (!m_textTex)
        glGenTextures(1, &m_textTex);

    glBindTexture(GL_TEXTURE_2D, m_textTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 img.width(), img.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, img.constBits());
    glBindTexture(GL_TEXTURE_2D, 0);

    doneCurrent();

    m_hasTex = shouldDraw;
}

void FlickerWidget::buildShader()
{
    m_program = new QOpenGLShaderProgram(this);

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, VERT_SRC))
        qWarning() << "FlickerWidget: vert shader error:" << m_program->log();

    if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, FRAG_SRC))
        qWarning() << "FlickerWidget: frag shader error:" << m_program->log();

    if (!m_program->link())
        qWarning() << "FlickerWidget: link error:" << m_program->log();

    m_uTime       = m_program->uniformLocation("uTime");
    m_uFreq       = m_program->uniformLocation("uFreq");
    m_uIntensity  = m_program->uniformLocation("uIntensity");
    m_uEnvelope   = m_program->uniformLocation("uEnvelope");
    m_uOnColor    = m_program->uniformLocation("uOnColor");
    m_uOffColor   = m_program->uniformLocation("uOffColor");
    m_uUseTex     = m_program->uniformLocation("uUseTex");
    m_uTexSampler = m_program->uniformLocation("uTexSampler");
    m_uFlash = m_program->uniformLocation("uFlash");
}

// ── event handling ────────────────────────────────────────────────────────────

void FlickerWidget::showEvent(QShowEvent *event)
{
    QOpenGLWidget::showEvent(event);
    if (parentWidget()) {
        setGeometry(parentWidget()->rect());
        raise();
        parentWidget()->installEventFilter(this);
    }
}

void FlickerWidget::resizeEvent(QResizeEvent *event)
{
    QOpenGLWidget::resizeEvent(event);
    if (parentWidget()) {
        setGeometry(parentWidget()->rect());
        raise();
    }
}

bool FlickerWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() && event->type() == QEvent::Resize) {
        setGeometry(parentWidget()->rect());
        raise();
        return true;
    }
    return QOpenGLWidget::eventFilter(watched, event);
}

void FlickerWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    QAction *fullscreenAction  = menu.addAction("Toggle Fullscreen");
    QAction *dialogAction      = menu.addAction("Show/Hide Controls");
    QAction *startStopAction   = menu.addAction("Start/Stop Flickering");

    QAction *selected = menu.exec(event->globalPos());

    if (selected == fullscreenAction) {
        emit toggleFullscreenRequested();
    } else if (selected == dialogAction) {
        emit toggleDialogRequested();
    } else if (selected == startStopAction) {
        if (m_running) {
            stopFlicker();
        } else {
            startFlicker();
        }
    }
}
