#ifndef CONSTANTS_H
#define CONSTANTS_H
#include<QString>
#include<QStandardPaths>
#include<QDir>
#include<QMediaPlayer>

namespace ConstantGlobals {
extern const QString appDirPath;
extern const QString ambientFilePath;
extern const QString presetFilePath;
extern const QString playlistFilePath;
extern const QString musicFilePath;
extern const QString ambientPresetFilePath;
extern int currentToneType;
extern QMediaPlayer::PlaybackState playbackState;
extern QString lastMusicDirPath;
}


#endif // CONSTANTS_H
