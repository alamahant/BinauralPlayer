#include "constants.h"
#include<QApplication>
namespace ConstantGlobals {

const QString appDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/BinauralPlayer";
const QString ambientFilePath = appDirPath + "/ambient-tracks";
const QString presetFilePath = appDirPath + "/brainwave-presets";
const QString playlistFilePath = appDirPath + "/playlists";
const QString musicFilePath = appDirPath + "/music";
const QString ambientPresetFilePath = appDirPath + "/ambient-presets";

int currentToneType = 0;
}
