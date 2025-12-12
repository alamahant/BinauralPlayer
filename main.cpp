#include "mainwindow.h"
#include"constants.h"
#include <QApplication>
#include<QDir>
#include<QTimer>
#include<QMessageBox>

int main(int argc, char *argv[])
{
    QDir().mkpath(ConstantGlobals::appDirPath);
    QDir().mkpath(ConstantGlobals::ambientFilePath);
    QDir().mkpath(ConstantGlobals::presetFilePath);
    QDir().mkpath(ConstantGlobals::playlistFilePath);
    QDir().mkpath(ConstantGlobals::musicFilePath);
    QDir().mkpath(ConstantGlobals::ambientPresetFilePath);

    QApplication::setApplicationName("BinauralPlayer");
    QApplication::setOrganizationName("Alamahant");
    QApplication::setApplicationVersion("1.1.0");
    QApplication a(argc, argv);
    MainWindow w;

    w.show();
    //open with
    if (argc == 2) {
            QString filePath = QString::fromLocal8Bit(argv[1]);
            QFileInfo fileInfo(filePath);

            if (fileInfo.isFile() && fileInfo.exists()) {
                // Use singleShot to ensure MainWindow is fully ready
                QTimer::singleShot(0, [&w, filePath]() {
                    // Directly call the public slot
                    w.onFileOpened(filePath);
                });
            }

   }
    //
    return a.exec();
}
