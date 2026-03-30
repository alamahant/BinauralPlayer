cmake_minimum_required(VERSION 3.16)

project(BinauralPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(FLATPAK_BUILD "Build for Flathub/Flatpak" OFF)
if(FLATPAK_BUILD)
    add_compile_definitions(FLATPAK_BUILD)
endif()

find_package(Qt6 REQUIRED COMPONENTS
    Core Widgets Multimedia MultimediaWidgets OpenGL OpenGLWidgets)

qt_add_executable(BinauralPlayer
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
    binauralengine.cpp binauralengine.h
    constants.cpp constants.h
    dynamicengine.cpp dynamicengine.h
    helpmenudialog.cpp helpmenudialog.h
    donationdialog.cpp donationdialog.h
    ambientplayer.cpp ambientplayer.h
    ambientplayerdialog.cpp ambientplayerdialog.h
    sessiondialog.cpp sessiondialog.h
    cuesheetdialog.cpp cuesheetdialog.h
    flickerwidget.cpp flickerwidget.h
    vistimdialog.cpp vistimdialog.h
    resources.qrc
)

target_link_libraries(BinauralPlayer PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
)

set_target_properties(BinauralPlayer PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS BinauralPlayer
    BUNDLE  DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(FLATPAK_BUILD)
    # Install desktop file
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/io.github.alamahant.BinauralPlayer.desktop"
            DESTINATION "share/applications")

    # Install metainfo file
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/io.github.alamahant.BinauralPlayer.metainfo.xml"
            DESTINATION "share/metainfo")

    # Install icons from favicon directory
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/favicon/android-chrome-192x192.png"
            DESTINATION "share/icons/hicolor/192x192/apps"
            RENAME "io.github.alamahant.BinauralPlayer.png")

    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/favicon/android-chrome-512x512.png"
            DESTINATION "share/icons/hicolor/512x512/apps"
            RENAME "io.github.alamahant.BinauralPlayer.png")

    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/favicon/favicon-16x16.png"
            DESTINATION "share/icons/hicolor/16x16/apps"
            RENAME "io.github.alamahant.BinauralPlayer.png")

    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/favicon/favicon-32x32.png"
            DESTINATION "share/icons/hicolor/32x32/apps"
            RENAME "io.github.alamahant.BinauralPlayer.png")

endif()
qt_finalize_executable(BinauralPlayer)
