cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(GUI_Headers 
    include/mainwidget.h
    include/player_page.h
    include/sound_file_widget.h
    include/text_to_speech_page.h
)

set(GUI_Sources
    src/mainwidget.cpp
    src/player_page.cpp
    src/sound_file_widget.cpp
    src/text_to_speech_page.cpp
)

# Создаем GUI библиотеку
add_library(MicSoundbarGUI STATIC
    ${GUI_Headers}
    ${GUI_Sources}
)

target_link_libraries(MicSoundbarGUI Qt6::Core Qt6::Widgets)
target_include_directories(MicSoundbarGUI PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${Qt6Widgets_INCLUDE_DIRS})