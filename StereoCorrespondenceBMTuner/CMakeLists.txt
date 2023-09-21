cmake_minimum_required(VERSION 3.16)
project(StereoCorrespondenceBMTuner VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Set up AUTOMOC and some sensible defaults for runtime execution
# When using Qt 6.3, you can replace the code block below with
# qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} OPTIONAL_COMPONENTS Widgets)
find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc highgui img_hash)

add_executable(StereoCorrespondenceBMTuner WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)

target_include_directories(StereoCorrespondenceBMTuner PUBLIC ${OpenCV_INCLUDE_DIRS})

target_link_libraries(StereoCorrespondenceBMTuner PRIVATE
    Qt::Core
    Qt::Gui
   ${OpenCV_LIBS}
)

if((QT_VERSION_MAJOR GREATER 4))
    target_link_libraries(StereoCorrespondenceBMTuner PRIVATE
        Qt::Widgets
    )
endif()

install(TARGETS StereoCorrespondenceBMTuner
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
