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
    enum Waveform {
        SINE_WAVE = 0,
        SQUARE_WAVE = 1,
        TRIANGLE_WAVE = 2,
        SAWTOOTH_WAVE = 3
    };
    Q_ENUM(Waveform)

    explicit DynamicEngine(QObject *parent = nullptr);
    ~DynamicEngine();

    bool start();
    void stop();
    bool isPlaying() const;

    void setLeftFrequency(double hz);
    void setRightFrequency(double hz);
    void setBeatFrequency(double hz);
    void setCarrierFrequency(double hz);

    double getLeftFrequency() const;
    double getRightFrequency() const;
    double getBeatFrequency() const;

    void setWaveform(Waveform type);
    Waveform getWaveform() const;

    void setAmplitude(double amplitude);
    void setVolume(double volume);

    double getAmplitude() const;
    double getVolume() const;

    void setSampleRate(int sampleRate);
    int getSampleRate() const;

    int getBufferDuration() const; // Returns 0 for dynamic

    double getCurrentPhaseLeft() const;
    double getCurrentPhaseRight() const;

    bool isEngineActive() const;

    QAudioSink *audioOutput() const;
    void setPulseFrequency(double newPulseFrequency);
    QBuffer *audioBuffer() const; // Returns nullptr for dynamic
    void forceBufferRegeneration(); // No-op for dynamic

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

    void generateIsochronicBuffer(int durationMs); // Creates empty buffer
    double getPulseFrequency() const;
    double calculateTriangleSample(double phase);
    double calculateSawtoothSample(double phase);

    bool startDynamicPlayback();
    void stopDynamicPlayback();

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

    static constexpr double MIN_FREQUENCY = 20.0;
    static constexpr double MAX_FREQUENCY = 20000.0;
    static constexpr double MIN_AMPLITUDE = 0.0;
    static constexpr double MAX_AMPLITUDE = 1.0;
    static constexpr double DEFAULT_AMPLITUDE = 0.3;
    static constexpr double DEFAULT_VOLUME = 0.15;

    QIODevice* m_dynamicDevice;
    class DynamicAudioDevice;

    // ============================================================
    // NOISE CONTROL
    // ============================================================
    public:
        void setNoiseType(int type);
        void setNoiseLevel(double level);
        void setNoiseEnabled(bool enabled);
        int getNoiseType() const;
        double getNoiseLevel() const;
        bool isNoiseEnabled() const;

    private:
        double generateWhiteNoise();
        double generatePinkNoise();
        double generateBrownNoise();
        double generateGreyNoise();

        void resetNoiseState();

        std::atomic<int> m_noiseType{0};        // 0=Off, 1=White, 2=Pink, 3=Brown 4=Grey
        std::atomic<double> m_noiseLevel{0.3};  // 0.0-1.0
        std::atomic<bool> m_noiseEnabled{false};

        // Pink noise filter state
        double m_pinkB0{0.0}, m_pinkB1{0.0}, m_pinkB2{0.0};
        double m_pinkB3{0.0}, m_pinkB4{0.0}, m_pinkB5{0.0}, m_pinkB6{0.0};
        // Brown noise state
        double m_brownState{0.0};
};

#endif // DYNAMICENGINE_H
