#include "binauralengine.h"


#include <QDebug>
#include <QtMath>
#include<QTimer>
#include<QTime>
#include"constants.h"

// =================== CONSTRUCTOR/DESTRUCTOR ===================
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

// =================== INITIALIZATION METHODS ===================
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
// =================== CORE PLAYBACK CONTROL ===================

bool BinauralEngine::start()
{

    if (m_isPlaying) {
        return true;
    }

    if (!initializeAudioOutput()) {
        return false;
    }

    // === ONLY GENERATE BUFFER IF NEEDED ===
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

    // === ALWAYS SEEK TO START ===
    m_audioBuffer->seek(0);

    m_audioOutput->start(m_audioBuffer);
    m_isPlaying = true;

    // Connect to the idle state for looping
    //connect(m_audioOutput, &QAudioSink::stateChanged,
            //this, &BinauralEngine::handleAudioStateChanged);

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



// =================== FREQUENCY CONTROL ===================
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
            // Pulse frequency validation (e.g., 0.5-30Hz)
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
        // Original binaural validation
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

// =================== WAVEFORM & AUDIO CONTROL ===================
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

// =================== ENGINE CONFIGURATION ===================
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

// =================== AUDIO STATE INFORMATION ===================
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

// =================== PRIVATE AUDIO GENERATION METHODS ===================


void BinauralEngine::generateAudioBuffer(int durationMs)
{
    if (ConstantGlobals::currentToneType == 1) {
           generateIsochronicBuffer(durationMs);
           return;
       }

    // STOP playback if active
        bool wasPlaying = m_isPlaying;
        if (wasPlaying && m_audioOutput) {
            m_audioOutput->stop();
            //m_isPlaying = false;
        }

    if (durationMs <= 0) return;

    // Generate buffer that's 4x longer for seamless looping
    //int loopCount = 4;
    //int totalDurationMs = durationMs * loopCount;
    int totalDurationMs = durationMs;      // <-- USE DIRECT DURATION
    qint64 sampleCount = (static_cast<qint64>(m_sampleRate) * totalDurationMs) / 1000;

    QByteArray audioData;
    audioData.resize(sampleCount * 2 * sizeof(int16_t));
    int16_t *data = reinterpret_cast<int16_t*>(audioData.data());

    // Generate continuous audio without phase reset
    double leftPhaseIncrement = (2.0 * M_PI * m_leftFrequency) / m_sampleRate;
    double rightPhaseIncrement = (2.0 * M_PI * m_rightFrequency) / m_sampleRate;

    for (qint64 i = 0; i < sampleCount; ++i) {
        // Calculate samples using current phase
        double leftSample = calculateSample(m_phaseLeft, m_currentWaveform);
        double rightSample = calculateSample(m_phaseRight, m_currentWaveform);

        leftSample *= m_amplitude;
        rightSample *= m_amplitude;

        // Convert to 16-bit
        data[2 * i] = static_cast<int16_t>(leftSample * 32767);
        data[2 * i + 1] = static_cast<int16_t>(rightSample * 32767);

        // Update phase continuously
        m_phaseLeft += leftPhaseIncrement;
        m_phaseRight += rightPhaseIncrement;

        // Keep phase in range
        if (m_phaseLeft > 2.0 * M_PI) m_phaseLeft -= 2.0 * M_PI;
        if (m_phaseRight > 2.0 * M_PI) m_phaseRight -= 2.0 * M_PI;
    }

    // Apply crossfade between loops (eliminates click)
   // applyCrossfade(audioData, durationMs);
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

    // 50ms fade at start and end
    int fadeSamples = (m_sampleRate * 50) / 1000;

    // Fade IN first 50ms
    for (int i = 0; i < fadeSamples && i < totalSamples; i++) {
        float fade = static_cast<float>(i) / fadeSamples;
        data[i] = static_cast<int16_t>(data[i] * fade);
    }

    // Fade OUT last 50ms
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

    // Crossfade duration: 20ms
    int crossfadeSamples = (m_sampleRate * 20) / 1000;
    int loopSamples = (m_sampleRate * loopDurationMs) / 1000 * 2; // ×2 for stereo

    // Apply crossfade at each loop boundary
    for (int loop = 1; loop < 4; loop++) {
        int boundaryStart = loop * loopSamples;

        for (int i = 0; i < crossfadeSamples; i++) {
            // Fade out previous loop
            int idxOut = boundaryStart - crossfadeSamples + i;
            float fadeOut = 1.0f - (static_cast<float>(i) / crossfadeSamples);

            // Fade in next loop
            int idxIn = boundaryStart + i;
            float fadeIn = static_cast<float>(i) / crossfadeSamples;

            // Mix both (crossfade)
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
        // Calculate left channel sample
        double leftSample = calculateSample(m_phaseLeft, m_currentWaveform);
        leftSample *= m_amplitude;

        // Calculate right channel sample
        double rightSample = calculateSample(m_phaseRight, m_currentWaveform);
        rightSample *= m_amplitude;

        // Convert to 16-bit and interleave (L, R, L, R...)
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
    // Simple square wave: +1 when sin(phase) >= 0, -1 otherwise
    //return (std::sin(phase) >= 0.0) ? 1.0 : -1.0;

    // For ISOCHRONIC: On/Off (not +1/-1)
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

// =================== PARAMETER VALIDATION ===================
bool BinauralEngine::validateFrequency(double hz)
{
    return (hz >= MIN_FREQUENCY && hz <= MAX_FREQUENCY);
}

bool BinauralEngine::validateAmplitude(double amplitude)
{
    return (amplitude >= MIN_AMPLITUDE && amplitude <= MAX_AMPLITUDE);
}

// =================== INTERNAL UPDATE METHODS ===================
void BinauralEngine::updateAudioParameters()
{
    if (!m_isPlaying || !m_parametersChanged) {
        return;
    }

    // Regenerate buffer with new parameters
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



// =================== AUDIO STATE HANDLER ===================
void BinauralEngine::handleAudioStateChanged(QAudio::State state)
{
    switch (state) {
        case QAudio::ActiveState:
            // Audio is playing normally
            break;

        case QAudio::SuspendedState:
            break;

        case QAudio::StoppedState:
            if (m_isPlaying) {
                //m_isPlaying = false;
                emit playbackStopped();
            }
            break;

    case QAudio::IdleState:
        // Buffer has been fully played

            if (m_isPlaying && m_audioBuffer) {  // <-- KEEP only this check


                QTimer::singleShot(0, this, [this]() {
                           // Now Qt has finished state transition
                           m_audioBuffer->seek(0);
                           m_audioOutput->start(m_audioBuffer);
                       });

            }
            break;

        default:
            break;
    }

    // Check for errors
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

//ISOCHRONIC METHODS

void BinauralEngine::generateIsochronicBuffer(int durationMs) {
    bool wasPlaying = m_isPlaying;
    if (wasPlaying && m_audioOutput) {
        m_audioOutput->stop();
    }

    if (durationMs <= 0) return;

    // Generate buffer with same duration logic
    int totalDurationMs = durationMs;
    qint64 sampleCount = (static_cast<qint64>(m_sampleRate) * totalDurationMs) / 1000;

    QByteArray audioData;
    audioData.resize(sampleCount * 2 * sizeof(int16_t));  // Stereo buffer
    int16_t *data = reinterpret_cast<int16_t*>(audioData.data());

    // ISOCHRONIC LOGIC:
    // m_leftFrequency = Carrier frequency (e.g., 200Hz)
    // m_rightFrequency = Pulse rate (e.g., 10Hz for 10 pulses/second)
    double carrierFreq = m_leftFrequency;
    double pulseFreq = m_pulseFrequency;

    // Phase increments (same as binaural but different meaning)
    double carrierPhaseIncrement = (2.0 * M_PI * carrierFreq) / m_sampleRate;
    double pulsePhaseIncrement = (2.0 * M_PI * pulseFreq) / m_sampleRate;

    // Reuse existing phase variables
    double carrierPhase = m_phaseLeft;
    double pulsePhase = m_phaseRight;

    for (qint64 i = 0; i < sampleCount; ++i) {
        // 1. Generate carrier wave (sine/square based on m_currentWaveform)
        double carrierSample = calculateSample(carrierPhase, m_currentWaveform);

        // 2. Generate pulse wave (square wave for on/off)
        // sin(pulsePhase) >= 0 ? 1.0 : 0.0 = Square wave, 50% duty cycle
        double pulseValue = (std::sin(pulsePhase) >= 0.0) ? 1.0 : 0.0;

        // 3. Modulate: carrier × pulse
        double modulatedSample = carrierSample * pulseValue * m_amplitude;

        // 4. Same signal to both ears (stereo identical)
        data[2 * i] = static_cast<int16_t>(modulatedSample * 32767);     // Left
        data[2 * i + 1] = static_cast<int16_t>(modulatedSample * 32767); // Right

        // 5. Update phases (same as binaural)
        carrierPhase += carrierPhaseIncrement;
        pulsePhase += pulsePhaseIncrement;

        // 6. Keep phase in range (same as binaural)
        if (carrierPhase > 2.0 * M_PI) carrierPhase -= 2.0 * M_PI;
        if (pulsePhase > 2.0 * M_PI) pulsePhase -= 2.0 * M_PI;
    }

    // Save phases for continuation (same as binaural)
    m_phaseLeft = carrierPhase;
    m_phaseRight = pulsePhase;

    // Apply same fade as binaural
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
    // Triangle wave: linear rise/fall between -1 and +1
    // Period is 2π, so scale phase to 0-1 range
    double normalized = phase / (2.0 * M_PI);

    // Create triangle: rise for half period, fall for half period
    double triangle;
    if (normalized < 0.5) {
        // Rising edge: 0 → 1
        triangle = 4.0 * normalized - 1.0;
    } else {
        // Falling edge: 1 → -1
        triangle = 3.0 - 4.0 * normalized;
    }

    return triangle;
}

double BinauralEngine::calculateSawtoothSample(double phase) {
    // Sawtooth wave: linear ramp from -1 to +1, then instant reset
    // Formula: 2 * (phase/(2π) - floor(phase/(2π) + 0.5))

    double normalized = phase / (2.0 * M_PI);  // Convert to 0-1 range
    double sawtooth = 2.0 * (normalized - std::floor(normalized + 0.5));

    return sawtooth;
}
