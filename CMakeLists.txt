cmake_minimum_required(VERSION 3.16)
project(WallpaperPowerSwitch VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/PowerMonitor.cpp
    src/AppManager.cpp
    src/WallpaperManager.cpp
    src/AppValidator.cpp
    src/ConflictResolver.cpp
)

set(HEADERS
    src/MainWindow.h
    src/PowerMonitor.h
    src/AppManager.h
    src/WallpaperManager.h
    src/AppValidator.h
    src/ConflictResolver.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} resources/app.qrc)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# IOKit for power monitoring
target_link_libraries(${PROJECT_NAME} PRIVATE
    "-framework IOKit"
    "-framework AppKit"
)

# macOS bundle
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.flames.wallpaper-power-switch"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
    MACOSX_BUNDLE_ICON_FILE "icon.icns"
    RESOURCE "${CMAKE_SOURCE_DIR}/resources/icon.icns"
)
