#include "mainwindow.h"
#include"constants.h"
#include <QApplication>
#include<QDir>
#include<QTimer>
#include<QStyleFactory>


int main(int argc, char *argv[])
{

    QDir().mkpath(ConstantGlobals::appDirPath);
    QDir().mkpath(ConstantGlobals::ambientFilePath);
    QDir().mkpath(ConstantGlobals::presetFilePath);
    QDir().mkpath(ConstantGlobals::playlistFilePath);
    QDir().mkpath(ConstantGlobals::musicFilePath);
    QDir().mkpath(ConstantGlobals::ambientPresetFilePath);
    QDir().mkpath(ConstantGlobals::radionicsFilePath);

    QApplication::setApplicationName("BinauralPlayer");
    QApplication::setOrganizationName("Alamahant");
    QApplication::setApplicationVersion("1.6.3");

    QApplication a(argc, argv);

#ifndef FLATPAK_BUILD

    a.setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, Qt::white);
    lightPalette.setColor(QPalette::WindowText, Qt::black);
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, Qt::black);
    lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);

    a.setPalette(lightPalette);
#endif


    MainWindow w;
    w.show();
    if (argc == 2) {
            QString filePath = QString::fromLocal8Bit(argv[1]);
            QFileInfo fileInfo(filePath);

            if (fileInfo.isFile() && fileInfo.exists()) {
                QTimer::singleShot(0, [&w, filePath]() {
                    w.onFileOpened(filePath);
                });
            }

   }
    return a.exec();
}
