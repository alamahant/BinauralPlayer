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
    enum Waveform {
        SINE_WAVE = 0,
        SQUARE_WAVE = 1,
        TRIANGLE_WAVE = 2,
        SAWTOOTH_WAVE = 3
    };
    Q_ENUM(Waveform)

    explicit BinauralEngine(QObject *parent = nullptr);
    ~BinauralEngine();

    bool start();
    void stop();
    bool isPlaying() const;

    void setLeftFrequency(double hz);
    void setRightFrequency(double hz);
    void setBeatFrequency(double hz); // Convenience method
    void setCarrierFrequency(double hz); // Set both channels same

    double getLeftFrequency() const;
    double getRightFrequency() const;
    double getBeatFrequency() const;

    void setWaveform(Waveform type);
    Waveform getWaveform() const;

    void setAmplitude(double amplitude); // Raw signal level (0.0-1.0)
    void setVolume(double volume);       // Output volume (0.0-1.0)

    double getAmplitude() const;
    double getVolume() const;

    void setSampleRate(int sampleRate);
    int getSampleRate() const;

    int getBufferDuration() const; // Duration in milliseconds

    double getCurrentPhaseLeft() const;
    double getCurrentPhaseRight() const;

    bool isEngineActive() const;


    QAudioSink *audioOutput() const;

    void setPulseFrequency(double newPulseFrequency);

    QBuffer *audioBuffer() const;

signals:
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
    void initializeAudioFormat();
    bool initializeAudioOutput();
    void generateAudioBuffer(int durationMs = 300000); // Default 1 hour

    double calculateSineSample(double phase);
    double calculateSquareSample(double phase);
    double calculateSample(double phase, Waveform waveform);

    void fillBufferWithSamples(QByteArray &buffer, int sampleCount);
    void applyVolumeToBuffer(QByteArray &buffer, double volume);

    bool validateFrequency(double hz);
    bool validateAmplitude(double amplitude);

    void updateAudioParameters();
    void resetPhase();

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

    static constexpr double MIN_FREQUENCY = 20.0;
    static constexpr double MAX_FREQUENCY = 20000.0;
    static constexpr double MIN_AMPLITUDE = 0.0;
    static constexpr double MAX_AMPLITUDE = 1.0;
    static constexpr double DEFAULT_AMPLITUDE = 0.3;
    static constexpr double DEFAULT_VOLUME = 0.15; // Subtle background level

    void applyCrossfade(QByteArray &buffer, int loopDurationMs);
    void applyLoopFade(QByteArray &buffer, int durationMs);
    int m_loopCounter = 0;

private:
    void generateIsochronicBuffer(int durationMs);
    double getPulseFrequency() const;
    double m_pulseFrequency;
    double calculateTriangleSample(double phase);
    double calculateSawtoothSample(double phase);

public:
    void forceBufferRegeneration();

};

#endif // BINAURALENGINE_H
