#include "constants.h"
#include<QApplication>
namespace ConstantGlobals {

const QString appDirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Binaural";
const QString ambientFilePath = appDirPath + "/ambient";
const QString presetFilePath = appDirPath + "/presets";
const QString playlistFilePath = appDirPath + "/playlists";
const QString musicFilePath = appDirPath + "/music";
int currentToneType = 0;
}
