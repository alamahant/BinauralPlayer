#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QElapsedTimer>
#include <QTimer>
#include <QColor>
#include <QString>
#include <QEvent>

class FlickerWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    enum class Envelope { Sine, Square, Triangle, Sawtooth };
    explicit FlickerWidget(QWidget *parent = nullptr);
    ~FlickerWidget() override;
    void startFlicker();
    void stopFlicker();
    bool isRunning() const { return m_running; }
    void setSubliminalFactor(float newSubliminalFactor);

public slots:
    void setFrequency(double hz);
    void setEnvelope(Envelope env);
    void setIntensity(float intensity);
    void setOnColor(const QColor &color);
    void setOffColor(const QColor &color);
    void setSubliminalText(const QString &text);
    void setSubliminalColor(const QColor &color);
    void setSubliminalBgColor(const QColor &color);
    void setSubliminalFontSize(int px);
    void setSubliminalMode(int mode);
protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void showEvent(QShowEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
private:
    void buildShader();
    void rebuildTextTexture();
    float computeBrightness(float phase) const;

    QOpenGLShaderProgram *m_program = nullptr;
    QElapsedTimer         m_elapsed;
    QTimer                m_renderTimer;

    bool     m_running   = false;
    double   m_frequency = 10.0;
    Envelope m_envelope  = Envelope::Sine;
    float    m_intensity = 0.4f;
    QColor   m_onColor   = Qt::white;
    QColor   m_offColor  = Qt::black;

    // subliminal
    QString  m_subliminalText;
    QColor   m_subliminalColor  = Qt::white;
    QColor   m_subliminalBg     = QColor(0, 0, 0, 0);
    int      m_subliminalFontPx = 24;
    int      m_subliminalMode   = 0;

    // text texture
    GLuint   m_textTex    = 0;
    bool     m_hasTex     = false;

    // shader uniform locations
    int m_uTime        = -1;
    int m_uFreq        = -1;
    int m_uIntensity   = -1;
    int m_uEnvelope    = -1;
    int m_uOnColor     = -1;
    int m_uOffColor    = -1;
    int m_uUseTex      = -1;
    int m_uTexSampler  = -1;
    int m_uResolution  = -1;
    int m_uFlash      = -1;
signals:
    void flickerStarted();
    void flickerStopped();
    void toggleFullscreenRequested();
    void toggleDialogRequested();
private:
    float m_subliminalFactor = 3.0f;
};
