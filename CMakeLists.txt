cmake_minimum_required(VERSION 3.16)

project(AutoSudoGUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23) # 23 is required to use SharedCppLib2
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools)

find_package(SharedCppLib2 REQUIRED)
find_package(AutoSudoSdk REQUIRED)

set(TS_FILES
    i18n/AutoSudoGUI_zh_CN.ts
    i18n/AutoSudoGUI_en_US.ts
)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h mainwindow.cpp mainwindow.ui
    servicecommthrd.h servicecommthrd.cpp
    ${TS_FILES}
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core)

    qt_add_executable(AutoSudoGUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        editwindow.h editwindow.cpp editwindow.ui
        rulemetadatamanager.h rulemetadatamanager.cpp
        comboboxdelegate.h comboboxdelegate.cpp
        payloaddelegate.h payloaddelegate.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET AutoSudoGUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    # qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

    qt6_add_translations(AutoSudoGUI
        TS_FILES ${TS_FILES}
    )
else()
    if(ANDROID)
        add_library(AutoSudoGUI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(AutoSudoGUI
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(AutoSudoGUI PRIVATE Qt6::Core Qt${QT_VERSION_MAJOR}::Widgets)

target_link_libraries(AutoSudoGUI PRIVATE AutoSudoSdk::AutoSudoSdk)

target_link_libraries(AutoSudoGUI PRIVATE
    SharedCppLib2::logt
)

# The rule editor talks to the control channel, which the service creates for
# administrators only. A release build therefore asks for elevation in its manifest, so
# Windows shows the UAC prompt when the tool starts, and the service can rely on the
# caller's token.
#
# A debug build is left alone: it is the one the service accepts unelevated rule callers
# from, and an unelevated process is also the one a debugger can attach to without
# running elevated itself.
target_link_options(AutoSudoGUI PRIVATE
    "$<$<CONFIG:Debug>:/MANIFESTUAC:NO>"
    "$<$<NOT:$<CONFIG:Debug>>:/MANIFESTUAC:level='requireAdministrator'>"
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.AutoSudoGUI)
endif()
set_target_properties(AutoSudoGUI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS AutoSudoGUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(AutoSudoGUI)
endif()
