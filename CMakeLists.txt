cmake_minimum_required(VERSION 3.19)
project(LostMistRealm LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Multimedia Sql Network)

qt_standard_project_setup()

qt_add_executable(LostMistRealm
    WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/mainwindow.cpp
    include/mainwindow.h
    src/mainwindow.ui
    include/maze.h src/maze.cpp
    include/player.h src/player.cpp
    include/gamewidget.h src/gamewidget.cpp
    include/mazegenerator.h src/mazegenerator.cpp
    include/databasemanager.h src/databasemanager.cpp
    include/achievementsystem.h src/achievementsystem.cpp
    include/aimanager.h src/aimanager.cpp
    LostMistRealm.qrc
)

target_include_directories(LostMistRealm PRIVATE include)

target_link_libraries(LostMistRealm
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Multimedia
        Qt::Sql
        Qt::Network
)

install(TARGETS LostMistRealm
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET LostMistRealm
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})