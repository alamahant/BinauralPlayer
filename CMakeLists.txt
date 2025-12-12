cmake_minimum_required(VERSION 3.16)

project(BinauralPlayer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#flatpak
option(FLATPAK_BUILD "Build for Flathub/Flatpak" OFF)
if(FLATPAK_BUILD)
    add_compile_definitions(FLATPAK_BUILD)
endif()

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Qt6Multimedia)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Multimedia)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

    qt_add_executable(BinauralPlayer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        binauralengine.h binauralengine.cpp
        resources.qrc
        constants.h constants.cpp
        dynamicengine.cpp dynamicengine.h
        helpmenudialog.h helpmenudialog.cpp donationdialog.h donationdialog.cpp
        ambientplayer.h ambientplayer.cpp
        ambientplayerdialog.h ambientplayerdialog.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET BinauralPlayer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(BinauralPlayer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(BinauralPlayer
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(BinauralPlayer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia)
target_link_libraries(BinauralPlayer PRIVATE Qt6::Core Qt6::Multimedia)
target_link_libraries(BinauralPlayer PRIVATE Qt6::Core)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.BinauralPlayer)
endif()
set_target_properties(BinauralPlayer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS BinauralPlayer
    BUNDLE DESTINATION .
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

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(BinauralPlayer)
endif()
