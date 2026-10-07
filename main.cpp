#include "mainwindow.h"
#include"constants.h"
#include <QApplication>
#include<QDir>
#include<QTimer>
#include<QStyleFactory>


int main(int argc, char *argv[])
{
    QApplication::setApplicationName("BinauralPlayer");
    QApplication::setOrganizationName("Alamahant");
    QApplication::setApplicationVersion("1.7.3");

#ifdef Q_OS_WIN
    QSettings::setDefaultFormat(QSettings::IniFormat);
#endif

    QDir().mkpath(PlayerGlobals::appDirPath);
    QDir().mkpath(PlayerGlobals::ambientFilePath);
    QDir().mkpath(PlayerGlobals::presetFilePath);
    QDir().mkpath(PlayerGlobals::playlistFilePath);
    QDir().mkpath(PlayerGlobals::musicFilePath);
    QDir().mkpath(PlayerGlobals::ambientPresetFilePath);
    QDir().mkpath(PlayerGlobals::radionicsFilePath);
    QDir().mkpath(PlayerGlobals::sessionsFilePath);

    QSettings settings;

    double factor = settings.value("ui/scaleFactor", 1.0).toDouble();
    qputenv("QT_SCALE_FACTOR", QByteArray::number(factor));
    PlayerGlobals::FONTSIZE = settings.value("ui/fontSize", PlayerGlobals::DEFAULTFONTSIZE).toReal();

    QApplication a(argc, argv);


#ifdef Q_OS_WIN
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
    a.setStyleSheet("QLineEdit { placeholder-text-color: #999999; }");
#endif

    if (PlayerGlobals::FONTSIZE > 0.0) {
           QFont appFont = a.font();
           appFont.setPointSizeF(PlayerGlobals::FONTSIZE);
           a.setFont(appFont);
    } else {
           PlayerGlobals::FONTSIZE = PlayerGlobals::DEFAULTFONTSIZE;
    }


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
