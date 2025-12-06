#ifndef DYNAMICENGINE_H
#define DYNAMICENGINE_H

#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>
#include <QBuffer>
#include <QIODevice>
#include <QMediaDevices>
#include <atomic>
#include <cmath>

class DynamicEngine : public QObject
{
    Q_OBJECT

public:
    // EXACT SAME enums
    enum Waveform {
        SINE_WAVE = 0,
        SQUARE_WAVE = 1,
        TRIANGLE_WAVE = 2,
        SAWTOOTH_WAVE = 3
    };
    Q_ENUM(Waveform)

    // EXACT SAME constructor
    explicit DynamicEngine(QObject *parent = nullptr);
    ~DynamicEngine();

    // =================== CORE PLAYBACK CONTROL ===================
    bool start();
    void stop();
    bool isPlaying() const;

    // =================== FREQUENCY CONTROL ===================
    void setLeftFrequency(double hz);
    void setRightFrequency(double hz);
    void setBeatFrequency(double hz);
    void setCarrierFrequency(double hz);

    double getLeftFrequency() const;
    double getRightFrequency() const;
    double getBeatFrequency() const;

    // =================== WAVEFORM & AUDIO CONTROL ===================
    void setWaveform(Waveform type);
    Waveform getWaveform() const;

    void setAmplitude(double amplitude);
    void setVolume(double volume);

    double getAmplitude() const;
    double getVolume() const;

    // =================== ENGINE CONFIGURATION ===================
    void setSampleRate(int sampleRate);
    int getSampleRate() const;

    int getBufferDuration() const; // Returns 0 for dynamic

    // =================== AUDIO STATE INFORMATION ===================
    double getCurrentPhaseLeft() const;
    double getCurrentPhaseRight() const;

    bool isEngineActive() const;

    QAudioSink *audioOutput() const;
    void setPulseFrequency(double newPulseFrequency);
    QBuffer *audioBuffer() const; // Returns nullptr for dynamic
    void forceBufferRegeneration(); // No-op for dynamic

signals:
    // EXACT SAME signals
    void playbackStarted();
    void playbackStopped();
    void playbackResumed();
    void leftFrequencyChanged(double hz);
    void rightFrequencyChanged(double hz);
    void beatFrequencyChanged(double hz);
    void waveformChanged(Waveform type);
    void volumeChanged(double volume);
    void errorOccurred(const QString &errorMessage);
    void audioDeviceError(const QString &deviceError);
    void bufferUnderrun();
    void parametersUpdated();
    void audioLevelChanged(double peakLevel);

private slots:
    void handleAudioStateChanged(QAudio::State state);

private:
    // =================== PRIVATE METHODS ===================
    void initializeAudioFormat();
    bool initializeAudioOutput();
    void generateAudioBuffer(int durationMs = 300000); // Creates empty buffer
    double calculateSineSample(double phase);
    double calculateSquareSample(double phase);
    double calculateSample(double phase, Waveform waveform);
    void fillBufferWithSamples(QByteArray &buffer, int sampleCount);
    void applyVolumeToBuffer(QByteArray &buffer, double volume);
    bool validateFrequency(double hz);
    bool validateAmplitude(double amplitude);
    void updateAudioParameters(); // No-op for dynamic
    void resetPhase();
    void applyCrossfade(QByteArray &buffer, int loopDurationMs);
    void applyLoopFade(QByteArray &buffer, int durationMs);

    // Isochronic
    void generateIsochronicBuffer(int durationMs); // Creates empty buffer
    double getPulseFrequency() const;
    double calculateTriangleSample(double phase);
    double calculateSawtoothSample(double phase);

    // Dynamic-specific methods
    bool startDynamicPlayback();
    void stopDynamicPlayback();

    // =================== MEMBER VARIABLES ===================
    // EXACT SAME variables (some unused in dynamic)
    QAudioSink *m_audioOutput;
    QBuffer *m_audioBuffer;
    QAudioFormat m_audioFormat;

    std::atomic<double> m_leftFrequency;
    std::atomic<double> m_rightFrequency;
    std::atomic<double> m_amplitude;
    std::atomic<double> m_outputVolume;
    std::atomic<Waveform> m_currentWaveform;

    double m_phaseLeft;
    double m_phaseRight;

    std::atomic<bool> m_isPlaying;
    std::atomic<bool> m_parametersChanged;

    int m_sampleRate;
    qint64 m_bufferDurationMs;
    double m_pulseFrequency;

    // Constants (EXACT SAME)
    static constexpr double MIN_FREQUENCY = 20.0;
    static constexpr double MAX_FREQUENCY = 20000.0;
    static constexpr double MIN_AMPLITUDE = 0.0;
    static constexpr double MAX_AMPLITUDE = 1.0;
    static constexpr double DEFAULT_AMPLITUDE = 0.3;
    static constexpr double DEFAULT_VOLUME = 0.15;

    // Dynamic-specific variables
    QIODevice* m_dynamicDevice;
    class DynamicAudioDevice;
};

#endif // DYNAMICENGINE_H
