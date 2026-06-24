#include "constants.h"
#include<QApplication>
namespace ConstantGlobals {

const QString appDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BinauralPlayer";
const QString ambientFilePath = appDirPath + "/ambient-tracks";
const QString presetFilePath = appDirPath + "/brainwave-presets";
const QString playlistFilePath = appDirPath + "/playlists";
const QString musicFilePath = appDirPath + "/music";
const QString ambientPresetFilePath = appDirPath + "/ambient-presets";
const QString radionicsFilePath = appDirPath + "/radionics";

int currentToneType = 0;
QMediaPlayer::PlaybackState playbackState = QMediaPlayer::StoppedState;
QString lastMusicDirPath;
double currentBinFreq = 7.83;
double currentIsonFreq = 7.83;
/*
const QStringList audioExtensions = {
    // Lossy formats
    ".mp3",     // MPEG Audio Layer 3
    ".aac",     // Advanced Audio Coding
    ".ogg",     // Ogg Vorbis
    ".m4a",     // MPEG-4 Audio (AAC/ALAC)
    ".wma",     // Windows Media Audio
    ".opus",    // Opus codec
    ".ac3",     // Dolby Digital AC-3
    ".eac3",    // Enhanced AC-3

    // Lossless formats
    ".wav",     // Waveform Audio
    ".flac",    // Free Lossless Audio Codec
    ".alac",    // Apple Lossless Audio Codec
    ".ape",     // Monkey's Audio
    ".dsf",     // DSD Audio File
    ".dff",     // DSDIFF Audio File
    ".m4b",     // Audiobook
    ".tak",     // Tom's lossless Audio Kompressor
    ".tta",     // True Audio
    ".wv",      // WavPack

    // Legacy formats
    ".aiff",    // Audio Interchange File Format
    ".au",      // Sun Audio
    ".mid",     // MIDI
    ".midi",    // MIDI
    ".s3m",     // ScreamTracker 3
    ".xm",      // FastTracker 2
    ".it",      // Impulse Tracker
    ".mod",     // Module
    ".ra",      // RealAudio
    ".rm",      // RealMedia
    ".ram",     // RealAudio Metadata
    ".voc",     // Creative Voice
    ".vox"      // Dialogic ADPCM
};

const QStringList videoExtensions = {".mp4", ".m4v", ".avi", ".mkv", ".mov", ".wmv", ".flv", ".webm"};

}
*/


const QStringList allMediaExtensions = {
    // ===== VIDEO FORMATS =====
    ".mp4",     // MPEG-4 Video
    ".m4v",     // MPEG-4 Video
    ".avi",     // Audio Video Interleave
    ".mkv",     // Matroska Video
    ".mov",     // QuickTime Movie
    ".wmv",     // Windows Media Video
    ".flv",     // Flash Video
    ".webm",    // WebM Video
    ".3gp",     // 3GPP Video
    ".mpeg",    // MPEG Video
    ".mpg",     // MPEG Video
    ".m2ts",    // Blu-ray BDAV
    ".mts",     // AVCHD Video
    ".vob",     // DVD Video Object
    ".asf",     // Advanced Systems Format
    ".divx",    // DivX Video
    ".f4v",     // Flash MP4 Video
    ".h264",    // H.264 Video
    ".hevc",    // H.265/HEVC Video
    ".ogv",     // Ogg Video
    ".ts",      // MPEG Transport Stream

    // ===== LOSSY AUDIO FORMATS =====
    ".mp3",     // MPEG Audio Layer 3
    ".aac",     // Advanced Audio Coding
    ".ogg",     // Ogg Vorbis
    ".m4a",     // MPEG-4 Audio (AAC/ALAC)
    ".wma",     // Windows Media Audio
    ".opus",    // Opus codec
    ".ac3",     // Dolby Digital AC-3
    ".eac3",    // Enhanced AC-3

    // ===== LOSSLESS AUDIO FORMATS =====
    ".wav",     // Waveform Audio
    ".flac",    // Free Lossless Audio Codec
    ".alac",    // Apple Lossless Audio Codec
    ".ape",     // Monkey's Audio
    ".dsf",     // DSD Audio File
    ".dff",     // DSDIFF Audio File
    ".m4b",     // Audiobook
    ".tak",     // Tom's lossless Audio Kompressor
    ".tta",     // True Audio
    ".wv",      // WavPack

    // ===== LEGACY/SPECIALTY AUDIO FORMATS =====
    ".aiff",    // Audio Interchange File Format
    ".au",      // Sun Audio
    ".mid",     // MIDI
    ".midi",    // MIDI
    ".s3m",     // ScreamTracker 3
    ".xm",      // FastTracker 2
    ".it",      // Impulse Tracker
    ".mod",     // Module
    ".ra",      // RealAudio
    ".rm",      // RealMedia
    ".ram",     // RealAudio Metadata
    ".voc",     // Creative Voice
    ".vox"      // Dialogic ADPCM
};

static QString cachedFilterString;
static bool filterStringInitialized = false;

QString getAllMediaFilterString() {
    if (!filterStringInitialized) {
        QStringList extensionsWithStar;
        for (const QString& ext : allMediaExtensions) {
            extensionsWithStar << ("*" + ext);
        }
        cachedFilterString = "All Supported Media (" + extensionsWithStar.join(" ") + ");;All Files (*)";
        filterStringInitialized = true;
    }
    return cachedFilterString;
}

}// namespace
