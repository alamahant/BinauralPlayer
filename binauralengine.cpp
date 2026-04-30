#include "binauralengine.h"


#include <QDebug>
#include <QtMath>
#include<QTimer>
#include<QTime>
#include"constants.h"

BinauralEngine::BinauralEngine(QObject *parent)
    : QObject(parent)
    , m_audioOutput(nullptr)
    , m_audioBuffer(nullptr)
    , m_leftFrequency(360.0)      // Default: 200Hz left
    , m_rightFrequency(367.83)    // Default: 207.83Hz right (7.83Hz beat)
    , m_amplitude(DEFAULT_AMPLITUDE)
    , m_outputVolume(DEFAULT_VOLUME)
    , m_currentWaveform(SINE_WAVE)
    , m_phaseLeft(0.0)
    , m_phaseRight(0.0)
    , m_isPlaying(false)
    , m_parametersChanged(false)
    , m_sampleRate(44100)         // CD quality
    , m_bufferDurationMs(300000) // 5 minute buffer = 300000
    , m_pulseFrequency(7.83)
{
    initializeAudioFormat();
}

BinauralEngine::~BinauralEngine()
{
    stop(); // Ensure audio is stopped
    delete m_audioBuffer;
    delete m_audioOutput;
}

void BinauralEngine::initializeAudioFormat()
{
    m_audioFormat.setSampleRate(m_sampleRate);
    m_audioFormat.setChannelCount(2); // Stereo for binaural
    m_audioFormat.setSampleFormat(QAudioFormat::Int16);
}

bool BinauralEngine::initializeAudioOutput()
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
    connect(m_audioOutput, &QAudioSink::stateChanged,
            this, &BinauralEngine::handleAudioStateChanged);

    m_audioOutput->setVolume(m_outputVolume);
    return true;
}

bool BinauralEngine::start()
{

    if (m_isPlaying) {
        return true;
    }

    if (!initializeAudioOutput()) {
        return false;
    }

    if (!m_audioBuffer || m_parametersChanged) {
        generateAudioBuffer(m_bufferDurationMs);

        if (!m_audioBuffer || m_audioBuffer->size() == 0) {
            if (m_audioBuffer) {
            }
            emit errorOccurred("Failed to generate audio buffer");
            return false;
        }

        m_audioBuffer->open(QIODevice::ReadOnly);
        m_parametersChanged = false; // Reset flag
    }

    m_audioBuffer->seek(0);

    m_audioOutput->start(m_audioBuffer);
    m_isPlaying = true;


    emit playbackStarted();
    return true;
}



void BinauralEngine::stop()
{


    if (m_audioOutput) {
        m_audioOutput->stop();
    }

    if (m_audioBuffer && m_audioBuffer->isOpen()) {
    }

    bool wasPlaying = m_isPlaying;
    m_isPlaying = false;
    resetPhase();

    if (wasPlaying) {
        emit playbackStopped();
    }
}



bool BinauralEngine::isPlaying() const
{
    return m_isPlaying;
}



void BinauralEngine::setLeftFrequency(double hz)
{
    if (!validateFrequency(hz)) {
        emit errorOccurred(QString("Invalid left frequency: %1 Hz").arg(hz));
        return;
    }

    m_leftFrequency = hz;
    m_parametersChanged = true;
    emit leftFrequencyChanged(hz);
    emit beatFrequencyChanged(getBeatFrequency());

    if (m_isPlaying) {
        updateAudioParameters();
    }
}
/*
void BinauralEngine::setRightFrequency(double hz)
{
        if (ConstantGlobals::currentToneType == 1) {
                    if (hz < 0.5 || hz > 30.0) {
                        emit errorOccurred(QString("Invalid pulse frequency: %1 Hz").arg(hz));
                        return;
        }else{
    if (!validateFrequency(hz)) {
        emit errorOccurred(QString("Invalid right frequency: %1 Hz").arg(hz));
        return;
    }

    m_rightFrequency = hz;
    m_parametersChanged = true;
    emit rightFrequencyChanged(hz);
    emit beatFrequencyChanged(getBeatFrequency());

    if (m_isPlaying) {
        updateAudioParameters();
    }
        }
}
}
*/

void BinauralEngine::setRightFrequency(double hz) {
    if (ConstantGlobals::currentToneType == 1) {

    } else {
        if (!validateFrequency(hz)) {
            emit errorOccurred(QString("Invalid right frequency: %1 Hz").arg(hz));
            return;
        }
    }

    m_rightFrequency = hz;
    m_parametersChanged = true;
    emit rightFrequencyChanged(hz);
    emit beatFrequencyChanged(getBeatFrequency());

    if (m_isPlaying) {
        updateAudioParameters();
    }
}

void BinauralEngine::setBeatFrequency(double hz)
{
    if (hz < 0.01 || hz > 100.0) {
        emit errorOccurred(QString("Invalid beat frequency: %1 Hz").arg(hz));
        return;
    }

    double newRightFreq = m_leftFrequency + hz;
    setRightFrequency(newRightFreq);
}

void BinauralEngine::setCarrierFrequency(double hz)
{
    setLeftFrequency(hz);
    setRightFrequency(hz);
}

double BinauralEngine::getLeftFrequency() const
{
    return m_leftFrequency;
}

double BinauralEngine::getRightFrequency() const
{
    return m_rightFrequency;
}

double BinauralEngine::getBeatFrequency() const
{
    return m_rightFrequency - m_leftFrequency;
}

void BinauralEngine::setWaveform(Waveform type)
{
    if (m_currentWaveform != type) {
        m_currentWaveform = type;
        m_parametersChanged = true;

        emit waveformChanged(type);

        if (m_isPlaying) {
            updateAudioParameters();
        }
    }
}

BinauralEngine::Waveform BinauralEngine::getWaveform() const
{
    return m_currentWaveform;
}

void BinauralEngine::setAmplitude(double amplitude)
{
    if (!validateAmplitude(amplitude)) {
        emit errorOccurred(QString("Invalid amplitude: %1").arg(amplitude));
        return;
    }

    m_amplitude = amplitude;
    m_parametersChanged = true;

    if (m_isPlaying) {
        updateAudioParameters();
    }
}

void BinauralEngine::setVolume(double volume)
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

double BinauralEngine::getAmplitude() const
{
    return m_amplitude;
}

double BinauralEngine::getVolume() const
{
    return m_outputVolume;
}

void BinauralEngine::setSampleRate(int sampleRate)
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

int BinauralEngine::getSampleRate() const
{
    return m_sampleRate;
}

int BinauralEngine::getBufferDuration() const
{
    return m_bufferDurationMs;
}

double BinauralEngine::getCurrentPhaseLeft() const
{
    return m_phaseLeft;
}

double BinauralEngine::getCurrentPhaseRight() const
{
    return m_phaseRight;
}

bool BinauralEngine::isEngineActive() const
{
    return m_isPlaying;
}



void BinauralEngine::generateAudioBuffer(int durationMs)
{
    if (ConstantGlobals::currentToneType == 1) {
           generateIsochronicBuffer(durationMs);
           return;
       }

        bool wasPlaying = m_isPlaying;
        if (wasPlaying && m_audioOutput) {
            m_audioOutput->stop();
        }

    if (durationMs <= 0) return;

    int totalDurationMs = durationMs;
    qint64 sampleCount = (static_cast<qint64>(m_sampleRate) * totalDurationMs) / 1000;

    QByteArray audioData;
    audioData.resize(sampleCount * 2 * sizeof(int16_t));
    int16_t *data = reinterpret_cast<int16_t*>(audioData.data());

    double leftPhaseIncrement = (2.0 * M_PI * m_leftFrequency) / m_sampleRate;
    double rightPhaseIncrement = (2.0 * M_PI * m_rightFrequency) / m_sampleRate;

    for (qint64 i = 0; i < sampleCount; ++i) {
        double leftSample = calculateSample(m_phaseLeft, m_currentWaveform);
        double rightSample = calculateSample(m_phaseRight, m_currentWaveform);

        leftSample *= m_amplitude;
        rightSample *= m_amplitude;

        data[2 * i] = static_cast<int16_t>(leftSample * 32767);
        data[2 * i + 1] = static_cast<int16_t>(rightSample * 32767);

        m_phaseLeft += leftPhaseIncrement;
        m_phaseRight += rightPhaseIncrement;

        if (m_phaseLeft > 2.0 * M_PI) m_phaseLeft -= 2.0 * M_PI;
        if (m_phaseRight > 2.0 * M_PI) m_phaseRight -= 2.0 * M_PI;
    }

    applyLoopFade(audioData, durationMs);

    if (m_audioBuffer) {
            if (m_audioBuffer->isOpen()) {
                m_audioBuffer->close();
            }
            delete m_audioBuffer;
            m_audioBuffer = nullptr;
        }

    m_audioBuffer = new QBuffer(this);
    m_audioBuffer->setData(audioData);
}

void BinauralEngine::applyLoopFade(QByteArray &buffer, int durationMs)
{
    int16_t *data = reinterpret_cast<int16_t*>(buffer.data());
    int totalSamples = buffer.size() / sizeof(int16_t);

    int fadeSamples = (m_sampleRate * 50) / 1000;

    for (int i = 0; i < fadeSamples && i < totalSamples; i++) {
        float fade = static_cast<float>(i) / fadeSamples;
        data[i] = static_cast<int16_t>(data[i] * fade);
    }

    for (int i = 0; i < fadeSamples && i < totalSamples; i++) {
        float fade = static_cast<float>(fadeSamples - i) / fadeSamples;
        int idx = totalSamples - fadeSamples + i;
        if (idx >= 0 && idx < totalSamples) {
            data[idx] = static_cast<int16_t>(data[idx] * fade);
        }
    }
}

void BinauralEngine::applyCrossfade(QByteArray &buffer, int loopDurationMs)
{
    int16_t *data = reinterpret_cast<int16_t*>(buffer.data());
    int totalSamples = buffer.size() / sizeof(int16_t);

    int crossfadeSamples = (m_sampleRate * 20) / 1000;
    int loopSamples = (m_sampleRate * loopDurationMs) / 1000 * 2; // ×2 for stereo

    for (int loop = 1; loop < 4; loop++) {
        int boundaryStart = loop * loopSamples;

        for (int i = 0; i < crossfadeSamples; i++) {
            int idxOut = boundaryStart - crossfadeSamples + i;
            float fadeOut = 1.0f - (static_cast<float>(i) / crossfadeSamples);

            int idxIn = boundaryStart + i;
            float fadeIn = static_cast<float>(i) / crossfadeSamples;

            if (idxOut >= 0 && idxOut < totalSamples) {
                data[idxOut] = static_cast<int16_t>(data[idxOut] * fadeOut);
            }
            if (idxIn < totalSamples) {
                data[idxIn] = static_cast<int16_t>(data[idxIn] * fadeIn);
            }
        }
    }
}



void BinauralEngine::fillBufferWithSamples(QByteArray &buffer, int sampleCount)
{
    int16_t *data = reinterpret_cast<int16_t*>(buffer.data());

    double leftPhaseIncrement = (2.0 * M_PI * m_leftFrequency) / m_sampleRate;
    double rightPhaseIncrement = (2.0 * M_PI * m_rightFrequency) / m_sampleRate;

    for (int i = 0; i < sampleCount; ++i) {
        double leftSample = calculateSample(m_phaseLeft, m_currentWaveform);
        leftSample *= m_amplitude;

        double rightSample = calculateSample(m_phaseRight, m_currentWaveform);
        rightSample *= m_amplitude;

        data[2 * i] = static_cast<int16_t>(leftSample * 32767);
        data[2 * i + 1] = static_cast<int16_t>(rightSample * 32767);

        m_phaseLeft += leftPhaseIncrement;
        m_phaseRight += rightPhaseIncrement;

        if (m_phaseLeft > 2.0 * M_PI) m_phaseLeft -= 2.0 * M_PI;
        if (m_phaseRight > 2.0 * M_PI) m_phaseRight -= 2.0 * M_PI;
    }
}

double BinauralEngine::calculateSample(double phase, Waveform waveform)
{
    switch (waveform) {
        case SINE_WAVE:
            return calculateSineSample(phase);
        case SQUARE_WAVE:
            return calculateSquareSample(phase);
        case TRIANGLE_WAVE:
            return calculateTriangleSample(phase);
        case SAWTOOTH_WAVE:    // Add this case
            return calculateSawtoothSample(phase);
        default:
            return calculateSineSample(phase);
    }
}

double BinauralEngine::calculateSineSample(double phase)
{
    return std::sin(phase);
}

double BinauralEngine::calculateSquareSample(double phase)
{

        if (ConstantGlobals::currentToneType == 1) {
            return (std::sin(phase) >= 0.0) ? 1.0 : 0.0;  // On/Off
        } else {
            return (std::sin(phase) >= 0.0) ? 1.0 : -1.0; // Original +1/-1
        }
}

void BinauralEngine::applyVolumeToBuffer(QByteArray &buffer, double volume)
{
    if (volume >= 1.0) return; // No adjustment needed

    int16_t *data = reinterpret_cast<int16_t*>(buffer.data());
    int sampleCount = buffer.size() / sizeof(int16_t);

    for (int i = 0; i < sampleCount; ++i) {
        data[i] = static_cast<int16_t>(data[i] * volume);
    }
}

bool BinauralEngine::validateFrequency(double hz)
{
    return (hz >= MIN_FREQUENCY && hz <= MAX_FREQUENCY);
}

bool BinauralEngine::validateAmplitude(double amplitude)
{
    return (amplitude >= MIN_AMPLITUDE && amplitude <= MAX_AMPLITUDE);
}

void BinauralEngine::updateAudioParameters()
{
    if (!m_isPlaying || !m_parametersChanged) {
        return;
    }

    bool wasPlaying = m_isPlaying;
    stop();

    if (wasPlaying) {
        start();
    }
}

void BinauralEngine::resetPhase()
{
    m_phaseLeft = 0.0;
    m_phaseRight = 0.0;
}

QBuffer *BinauralEngine::audioBuffer() const
{
    return m_audioBuffer;
}

QAudioSink *BinauralEngine::audioOutput() const
{
    return m_audioOutput;
}



void BinauralEngine::handleAudioStateChanged(QAudio::State state)
{
    switch (state) {
        case QAudio::ActiveState:
            break;

        case QAudio::SuspendedState:
            break;

        case QAudio::StoppedState:
            if (m_isPlaying) {
                emit playbackStopped();
            }
            break;

    case QAudio::IdleState:

            if (m_isPlaying && m_audioBuffer) {


                QTimer::singleShot(0, this, [this]() {
                           m_audioBuffer->seek(0);
                           m_audioOutput->start(m_audioBuffer);
                       });

            }
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


void BinauralEngine::generateIsochronicBuffer(int durationMs) {
    bool wasPlaying = m_isPlaying;
    if (wasPlaying && m_audioOutput) {
        m_audioOutput->stop();
    }

    if (durationMs <= 0) return;

    int totalDurationMs = durationMs;
    qint64 sampleCount = (static_cast<qint64>(m_sampleRate) * totalDurationMs) / 1000;

    QByteArray audioData;
    audioData.resize(sampleCount * 2 * sizeof(int16_t));  // Stereo buffer
    int16_t *data = reinterpret_cast<int16_t*>(audioData.data());

    double carrierFreq = m_leftFrequency;
    double pulseFreq = m_pulseFrequency;

    double carrierPhaseIncrement = (2.0 * M_PI * carrierFreq) / m_sampleRate;
    double pulsePhaseIncrement = (2.0 * M_PI * pulseFreq) / m_sampleRate;

    double carrierPhase = m_phaseLeft;
    double pulsePhase = m_phaseRight;

    for (qint64 i = 0; i < sampleCount; ++i) {
        double carrierSample = calculateSample(carrierPhase, m_currentWaveform);

        double pulseValue = (std::sin(pulsePhase) >= 0.0) ? 1.0 : 0.0;

        double modulatedSample = carrierSample * pulseValue * m_amplitude;

        data[2 * i] = static_cast<int16_t>(modulatedSample * 32767);     // Left
        data[2 * i + 1] = static_cast<int16_t>(modulatedSample * 32767); // Right

        carrierPhase += carrierPhaseIncrement;
        pulsePhase += pulsePhaseIncrement;

        if (carrierPhase > 2.0 * M_PI) carrierPhase -= 2.0 * M_PI;
        if (pulsePhase > 2.0 * M_PI) pulsePhase -= 2.0 * M_PI;
    }

    m_phaseLeft = carrierPhase;
    m_phaseRight = pulsePhase;

    applyLoopFade(audioData, durationMs);

    if (m_audioBuffer) {
        if (m_audioBuffer->isOpen()) {
            m_audioBuffer->close();
        }
        delete m_audioBuffer;
        m_audioBuffer = nullptr;
    }

    m_audioBuffer = new QBuffer(this);
    m_audioBuffer->setData(audioData);
}

double BinauralEngine::getPulseFrequency() const{
    return m_pulseFrequency;
}

void BinauralEngine::setPulseFrequency(double hz)
{
    if (hz < 0.5 || hz > 100.0) {
            emit errorOccurred(QString("Invalid pulse frequency: %1 Hz").arg(hz));
            return;
        }

        m_pulseFrequency = hz;
        m_parametersChanged = true;

        if (m_isPlaying) {
            updateAudioParameters();
        }
}


void BinauralEngine::forceBufferRegeneration() {
    m_parametersChanged = true;  // Force buffer rebuild
    if (m_audioBuffer) {
        delete m_audioBuffer;
        m_audioBuffer = nullptr;
    }
}

double BinauralEngine::calculateTriangleSample(double phase) {
    double normalized = phase / (2.0 * M_PI);

    double triangle;
    if (normalized < 0.5) {
        triangle = 4.0 * normalized - 1.0;
    } else {
        triangle = 3.0 - 4.0 * normalized;
    }

    return triangle;
}

double BinauralEngine::calculateSawtoothSample(double phase) {

    double normalized = phase / (2.0 * M_PI);  // Convert to 0-1 range
    double sawtooth = 2.0 * (normalized - std::floor(normalized + 0.5));

    return sawtooth;
}
