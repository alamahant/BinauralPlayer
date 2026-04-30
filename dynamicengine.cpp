#include "dynamicengine.h"
#include <QDebug>
#include <QtMath>
#include <QTimer>
#include <QTime>
#include <QElapsedTimer>
#include "constants.h"

DynamicEngine::DynamicEngine(QObject *parent)
    : QObject(parent)
    , m_audioOutput(nullptr)
    , m_audioBuffer(nullptr)
    , m_leftFrequency(360.0)
    , m_rightFrequency(367.83)
    , m_amplitude(DEFAULT_AMPLITUDE)
    , m_outputVolume(DEFAULT_VOLUME)
    , m_currentWaveform(SINE_WAVE)
    , m_phaseLeft(0.0)
    , m_phaseRight(0.0)
    , m_isPlaying(false)
    , m_parametersChanged(false)
    , m_sampleRate(44100)
    , m_bufferDurationMs(300000)
    , m_pulseFrequency(7.83)
    , m_dynamicDevice(nullptr)
{
    initializeAudioFormat();
}

DynamicEngine::~DynamicEngine()
{
    stop();
    delete m_dynamicDevice;
    delete m_audioBuffer;
    delete m_audioOutput;
}

void DynamicEngine::initializeAudioFormat()
{
    m_audioFormat.setSampleRate(m_sampleRate);
    m_audioFormat.setChannelCount(2);
    m_audioFormat.setSampleFormat(QAudioFormat::Int16);
}

bool DynamicEngine::initializeAudioOutput()
{
    if (m_audioOutput) {
        delete m_audioOutput;
    }

    QAudioDevice audioDevice = QMediaDevices::defaultAudioOutput();
    if (audioDevice.isNull()) {
        emit errorOccurred("No audio output device available");
        return false;
    }

    if (!audioDevice.isFormatSupported(m_audioFormat)) {
        emit errorOccurred("Audio format not supported by device");
        return false;
    }

    m_audioOutput = new QAudioSink(audioDevice, m_audioFormat, this);
       m_audioOutput->setBufferSize(32768);
    connect(m_audioOutput, &QAudioSink::stateChanged,
            this, &DynamicEngine::handleAudioStateChanged);

    m_audioOutput->setVolume(m_outputVolume);
    return true;
}

bool DynamicEngine::start()
{

    if (m_isPlaying) {
        return true;
    }

    return startDynamicPlayback();
}

bool DynamicEngine::startDynamicPlayback()
{
    if (!initializeAudioOutput()) {
        return false;
    }

    class DynamicAudioDevice : public QIODevice {
    public:
        DynamicAudioDevice(DynamicEngine* engine) 
            : m_engine(engine), m_phaseLeft(0.0), m_phaseRight(0.0) {
            setOpenMode(QIODevice::ReadOnly);
        }

        bool isSequential() const override { return true; }

          qint64 bytesAvailable() const override
          {
              return 4096 + QIODevice::bytesAvailable();
          }
        
    protected:
        qint64 readData(char* data, qint64 maxlen) override {
            int16_t* samples = reinterpret_cast<int16_t*>(data);
            int sampleCount = maxlen / (2 * sizeof(int16_t)); // Stereo
            
            double leftFreq = m_engine->m_leftFrequency.load();
            double rightFreq = m_engine->m_rightFrequency.load();
            double amplitude = m_engine->m_amplitude.load();
            auto waveform = m_engine->m_currentWaveform.load();
            double sampleRate = m_engine->m_sampleRate;
            double pulseFreq = m_engine->m_pulseFrequency;
            
            bool isIsochronic = (ConstantGlobals::currentToneType == 1);
            
            for (int i = 0; i < sampleCount; ++i) {
                double leftSample = 0.0;
                double rightSample = 0.0;
                
                if (isIsochronic) {
                    double carrierPhaseInc = (2.0 * M_PI * leftFreq) / sampleRate;
                    double pulsePhaseInc = (2.0 * M_PI * pulseFreq) / sampleRate;
                    
                    double carrier = 0.0;
                    switch (waveform) {
                        case SINE_WAVE: carrier = sin(m_phaseLeft); break;
                        case SQUARE_WAVE: carrier = (sin(m_phaseLeft) >= 0.0) ? 1.0 : 0.0; break;
                        case TRIANGLE_WAVE: carrier = m_engine->calculateTriangleSample(m_phaseLeft); break;
                        case SAWTOOTH_WAVE: carrier = m_engine->calculateSawtoothSample(m_phaseLeft); break;
                    }
                    
                    double pulse = (sin(m_phaseRight) >= 0.0) ? 1.0 : 0.0;
                    
                    leftSample = carrier * pulse * amplitude;
                    rightSample = leftSample; // Stereo identical
                    
                    m_phaseLeft += carrierPhaseInc;
                    m_phaseRight += pulsePhaseInc;
                } else {
                    double leftPhaseInc = (2.0 * M_PI * leftFreq) / sampleRate;
                    double rightPhaseInc = (2.0 * M_PI * rightFreq) / sampleRate;
                    
                    leftSample = m_engine->calculateSample(m_phaseLeft, waveform);
                    rightSample = m_engine->calculateSample(m_phaseRight, waveform);
                    
                    leftSample *= amplitude;
                    rightSample *= amplitude;
                    
                    m_phaseLeft += leftPhaseInc;
                    m_phaseRight += rightPhaseInc;
                }
                
                samples[2 * i] = static_cast<int16_t>(leftSample * 32767);
                samples[2 * i + 1] = static_cast<int16_t>(rightSample * 32767);
                
                if (m_phaseLeft > 2.0 * M_PI) m_phaseLeft -= 2.0 * M_PI;
                if (m_phaseRight > 2.0 * M_PI) m_phaseRight -= 2.0 * M_PI;
            }
            
            return sampleCount * 2 * sizeof(int16_t);
        }
        
        qint64 writeData(const char* data, qint64 len) override {
            Q_UNUSED(data);
            Q_UNUSED(len);
            return 0;
        }
        
    private:
        DynamicEngine* m_engine;
        double m_phaseLeft;
        double m_phaseRight;
    };
    
    m_dynamicDevice = new DynamicAudioDevice(this);
    m_audioOutput->start(m_dynamicDevice);
    m_isPlaying = true;
    
    emit playbackStarted();
    return true;
}

void DynamicEngine::stop()
{
    stopDynamicPlayback();
}

void DynamicEngine::stopDynamicPlayback()
{
    if (m_audioOutput) {
        m_audioOutput->stop();
    }
    
    if (m_dynamicDevice) {
        m_dynamicDevice->close();
        delete m_dynamicDevice;
        m_dynamicDevice = nullptr;
    }
    
    bool wasPlaying = m_isPlaying;
    m_isPlaying = false;
    resetPhase();
    
    if (wasPlaying) {
        emit playbackStopped();
    }
}

bool DynamicEngine::isPlaying() const
{
    return m_isPlaying;
}

void DynamicEngine::setLeftFrequency(double hz)
{
    if (!validateFrequency(hz)) {
        emit errorOccurred(QString("Invalid left frequency: %1 Hz").arg(hz));
        return;
    }

    m_leftFrequency = hz;
    emit leftFrequencyChanged(hz);
    emit beatFrequencyChanged(getBeatFrequency());
}

void DynamicEngine::setRightFrequency(double hz)
{
    if (ConstantGlobals::currentToneType == 1) {
    } else {
        if (!validateFrequency(hz)) {
            emit errorOccurred(QString("Invalid right frequency: %1 Hz").arg(hz));
            return;
        }
    }

    m_rightFrequency = hz;
    emit rightFrequencyChanged(hz);
    emit beatFrequencyChanged(getBeatFrequency());
}

void DynamicEngine::setBeatFrequency(double hz)
{
    if (hz < 0.01 || hz > 100.0) {
        emit errorOccurred(QString("Invalid beat frequency: %1 Hz").arg(hz));
        return;
    }

    double newRightFreq = m_leftFrequency + hz;
    setRightFrequency(newRightFreq);
}

void DynamicEngine::setCarrierFrequency(double hz)
{
    setLeftFrequency(hz);
    setRightFrequency(hz);
}

double DynamicEngine::getLeftFrequency() const
{
    return m_leftFrequency;
}

double DynamicEngine::getRightFrequency() const
{
    return m_rightFrequency;
}

double DynamicEngine::getBeatFrequency() const
{
    return m_rightFrequency - m_leftFrequency;
}

void DynamicEngine::setWaveform(Waveform type)
{
    if (m_currentWaveform != type) {
        m_currentWaveform = type;
        emit waveformChanged(type);
    }
}

DynamicEngine::Waveform DynamicEngine::getWaveform() const
{
    return m_currentWaveform;
}

void DynamicEngine::setAmplitude(double amplitude)
{
    if (!validateAmplitude(amplitude)) {
        emit errorOccurred(QString("Invalid amplitude: %1").arg(amplitude));
        return;
    }

    m_amplitude = amplitude;
}

void DynamicEngine::setVolume(double volume)
{
    if (volume < 0.0 || volume > 1.0) {
        emit errorOccurred(QString("Invalid volume: %1").arg(volume));
        return;
    }

    m_outputVolume = volume;

    if (m_audioOutput) {
        m_audioOutput->setVolume(volume);
    }

    emit volumeChanged(volume);
}

double DynamicEngine::getAmplitude() const
{
    return m_amplitude;
}

double DynamicEngine::getVolume() const
{
    return m_outputVolume;
}

void DynamicEngine::setSampleRate(int sampleRate)
{
    if (sampleRate < 8000 || sampleRate > 192000) {
        emit errorOccurred(QString("Invalid sample rate: %1").arg(sampleRate));
        return;
    }

    if (m_isPlaying) {
        emit errorOccurred("Cannot change sample rate while playing");
        return;
    }

    m_sampleRate = sampleRate;
    initializeAudioFormat();
}

int DynamicEngine::getSampleRate() const
{
    return m_sampleRate;
}

int DynamicEngine::getBufferDuration() const
{
    return 0; // Dynamic engine doesn't use buffers
}

double DynamicEngine::getCurrentPhaseLeft() const
{
    return m_phaseLeft;
}

double DynamicEngine::getCurrentPhaseRight() const
{
    return m_phaseRight;
}

bool DynamicEngine::isEngineActive() const
{
    return m_isPlaying;
}

QAudioSink *DynamicEngine::audioOutput() const
{
    return m_audioOutput;
}

void DynamicEngine::setPulseFrequency(double hz)
{
    if (ConstantGlobals::currentToneType != 1) {
            return; // Don't set pulse for non-ISO tones
        }
    if (hz < 0.5 || hz > 100.0) {
        emit errorOccurred(QString("Invalid pulse frequency: %1 Hz").arg(hz));
        return;
    }

    m_pulseFrequency = hz;
}

QBuffer *DynamicEngine::audioBuffer() const
{
    return nullptr; // Dynamic engine doesn't use QBuffer
}

void DynamicEngine::forceBufferRegeneration()
{
}

void DynamicEngine::generateAudioBuffer(int durationMs)
{
    Q_UNUSED(durationMs);
    
    if (m_audioBuffer) {
        delete m_audioBuffer;
    }
    
    m_audioBuffer = new QBuffer(this);
    m_audioBuffer->setData(QByteArray());
}

void DynamicEngine::applyLoopFade(QByteArray &buffer, int durationMs)
{
    Q_UNUSED(buffer);
    Q_UNUSED(durationMs);
}

void DynamicEngine::applyCrossfade(QByteArray &buffer, int loopDurationMs)
{
    Q_UNUSED(buffer);
    Q_UNUSED(loopDurationMs);
}

void DynamicEngine::fillBufferWithSamples(QByteArray &buffer, int sampleCount)
{
    Q_UNUSED(buffer);
    Q_UNUSED(sampleCount);
}

double DynamicEngine::calculateSample(double phase, Waveform waveform)
{
    switch (waveform) {
        case SINE_WAVE:
            return calculateSineSample(phase);
        case SQUARE_WAVE:
            return calculateSquareSample(phase);
        case TRIANGLE_WAVE:
            return calculateTriangleSample(phase);
        case SAWTOOTH_WAVE:
            return calculateSawtoothSample(phase);
        default:
            return calculateSineSample(phase);
    }
}

double DynamicEngine::calculateSineSample(double phase)
{
    return std::sin(phase);
}

double DynamicEngine::calculateSquareSample(double phase)
{
    if (ConstantGlobals::currentToneType == 1) {
        return (std::sin(phase) >= 0.0) ? 1.0 : 0.0;
    } else {
        return (std::sin(phase) >= 0.0) ? 1.0 : -1.0;
    }
}

void DynamicEngine::applyVolumeToBuffer(QByteArray &buffer, double volume)
{
    Q_UNUSED(buffer);
    Q_UNUSED(volume);
}

bool DynamicEngine::validateFrequency(double hz)
{
    return (hz >= MIN_FREQUENCY && hz <= MAX_FREQUENCY);
}

bool DynamicEngine::validateAmplitude(double amplitude)
{
    return (amplitude >= MIN_AMPLITUDE && amplitude <= MAX_AMPLITUDE);
}

void DynamicEngine::updateAudioParameters()
{
}

void DynamicEngine::resetPhase()
{
    m_phaseLeft = 0.0;
    m_phaseRight = 0.0;
}

void DynamicEngine::handleAudioStateChanged(QAudio::State state)
{
    
    switch (state) {
        case QAudio::ActiveState:
            break;
            
        case QAudio::SuspendedState:
            break;
            
        case QAudio::StoppedState:
            if (m_isPlaying) {
                m_isPlaying = false;
                emit playbackStopped();
            }
            break;
            
        case QAudio::IdleState:

        /*
            if (m_isPlaying && m_dynamicDevice) {
            }
            */

            break;
            
        default:
            break;
    }
    
    if (m_audioOutput && m_audioOutput->error() != QAudio::NoError) {
        QString errorMsg;
        switch (m_audioOutput->error()) {
            case QAudio::OpenError:
                errorMsg = "Audio open error";
                break;
            case QAudio::IOError:
                errorMsg = "Audio I/O error";
                break;
            case QAudio::UnderrunError:
                errorMsg = "Audio buffer underrun";
                emit bufferUnderrun();
                break;
            case QAudio::FatalError:
                errorMsg = "Fatal audio error";
                break;
            default:
                errorMsg = "Unknown audio error";
        }
        emit audioDeviceError(errorMsg);
    }
}

void DynamicEngine::generateIsochronicBuffer(int durationMs)
{
    Q_UNUSED(durationMs);
    
    if (m_audioBuffer) {
        delete m_audioBuffer;
    }
    
    m_audioBuffer = new QBuffer(this);
    m_audioBuffer->setData(QByteArray());
}

double DynamicEngine::getPulseFrequency() const
{
    return m_pulseFrequency;
}

double DynamicEngine::calculateTriangleSample(double phase)
{
    double normalized = phase / (2.0 * M_PI);
    double triangle;
    if (normalized < 0.5) {
        triangle = 4.0 * normalized - 1.0;
    } else {
        triangle = 3.0 - 4.0 * normalized;
    }
    return triangle;
}

double DynamicEngine::calculateSawtoothSample(double phase)
{
    double normalized = phase / (2.0 * M_PI);
    double sawtooth = 2.0 * (normalized - std::floor(normalized + 0.5));
    return sawtooth;
}
