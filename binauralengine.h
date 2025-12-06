#ifndef BINAURALENGINE_H
#define BINAURALENGINE_H

#include <QObject>
#include <QAudioSink>
#include <QAudioFormat>
#include <QBuffer>
#include <QIODevice>
#include <QMediaDevices>
#include <atomic>
#include <cmath>

class BinauralEngine : public QObject
{
    Q_OBJECT

public:
    // Waveform types - simple and expandable
    enum Waveform {
        SINE_WAVE = 0,
        SQUARE_WAVE = 1,
        TRIANGLE_WAVE = 2,
        SAWTOOTH_WAVE = 3
        // FUTURE: TRIANGLE_WAVE, SAWTOOTH_WAVE
    };
    Q_ENUM(Waveform)

    explicit BinauralEngine(QObject *parent = nullptr);
    ~BinauralEngine();

    // =================== CORE PLAYBACK CONTROL ===================
    bool start();
    void stop();
    bool isPlaying() const;

    // =================== FREQUENCY CONTROL ===================
    void setLeftFrequency(double hz);
    void setRightFrequency(double hz);
    void setBeatFrequency(double hz); // Convenience method
    void setCarrierFrequency(double hz); // Set both channels same

    double getLeftFrequency() const;
    double getRightFrequency() const;
    double getBeatFrequency() const;

    // =================== WAVEFORM & AUDIO CONTROL ===================
    void setWaveform(Waveform type);
    Waveform getWaveform() const;

    void setAmplitude(double amplitude); // Raw signal level (0.0-1.0)
    void setVolume(double volume);       // Output volume (0.0-1.0)

    double getAmplitude() const;
    double getVolume() const;

    // =================== ENGINE CONFIGURATION ===================
    void setSampleRate(int sampleRate);
    int getSampleRate() const;

    int getBufferDuration() const; // Duration in milliseconds

    // =================== AUDIO STATE INFORMATION ===================
    double getCurrentPhaseLeft() const;
    double getCurrentPhaseRight() const;

    bool isEngineActive() const;


    QAudioSink *audioOutput() const;

    void setPulseFrequency(double newPulseFrequency);

    QBuffer *audioBuffer() const;

signals:
    // Playback state signals
    void playbackStarted();
    void playbackStopped();
    void playbackResumed();

    // Parameter change signals
    void leftFrequencyChanged(double hz);
    void rightFrequencyChanged(double hz);
    void beatFrequencyChanged(double hz);
    void waveformChanged(Waveform type);
    void volumeChanged(double volume);

    // Error signals
    void errorOccurred(const QString &errorMessage);
    void audioDeviceError(const QString &deviceError);
    void bufferUnderrun(); // If we can't generate audio fast enough

    // Information signals
    void parametersUpdated();
    void audioLevelChanged(double peakLevel); // For potential VU meter

private slots:
    void handleAudioStateChanged(QAudio::State state);

private:
    // =================== PRIVATE METHODS ===================
    void initializeAudioFormat();
    bool initializeAudioOutput();
    void generateAudioBuffer(int durationMs = 300000); // Default 1 hour

    // Waveform calculation methods
    double calculateSineSample(double phase);
    double calculateSquareSample(double phase);
    double calculateSample(double phase, Waveform waveform);

    // Buffer management
    void fillBufferWithSamples(QByteArray &buffer, int sampleCount);
    void applyVolumeToBuffer(QByteArray &buffer, double volume);

    // Parameter validation
    bool validateFrequency(double hz);
    bool validateAmplitude(double amplitude);

    // Internal update methods
    void updateAudioParameters();
    void resetPhase();

    // =================== MEMBER VARIABLES ===================
    // Audio playback components
    QAudioSink *m_audioOutput;
    QBuffer *m_audioBuffer;
    QAudioFormat m_audioFormat;

    // Current audio parameters (atomic for thread safety)
    std::atomic<double> m_leftFrequency;
    std::atomic<double> m_rightFrequency;
    std::atomic<double> m_amplitude;
    std::atomic<double> m_outputVolume;
    std::atomic<Waveform> m_currentWaveform;

    double m_phaseLeft;    // Regular double, not atomic
    double m_phaseRight;   // Regular double, not atomic

    // Phase tracking for continuous waveforms
    //std::atomic<double> m_phaseLeft;
    //std::atomic<double> m_phaseRight;

    // Engine state
    std::atomic<bool> m_isPlaying;
    std::atomic<bool> m_parametersChanged;

    // Audio configuration
    int m_sampleRate;
    qint64 m_bufferDurationMs;

    // Constants
    static constexpr double MIN_FREQUENCY = 20.0;
    static constexpr double MAX_FREQUENCY = 20000.0;
    static constexpr double MIN_AMPLITUDE = 0.0;
    static constexpr double MAX_AMPLITUDE = 1.0;
    static constexpr double DEFAULT_AMPLITUDE = 0.3;
    static constexpr double DEFAULT_VOLUME = 0.15; // Subtle background level

    void applyCrossfade(QByteArray &buffer, int loopDurationMs);
    void applyLoopFade(QByteArray &buffer, int durationMs);
    int m_loopCounter = 0;

    //isochronic
private:
    void generateIsochronicBuffer(int durationMs);
    double getPulseFrequency() const;
    double m_pulseFrequency;  // NEW: For ISO pulse rate only
    double calculateTriangleSample(double phase);
    double calculateSawtoothSample(double phase);

public:
    void forceBufferRegeneration();

};

#endif // BINAURALENGINE_H
