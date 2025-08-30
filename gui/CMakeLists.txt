cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Создаем GUI библиотеку
add_library(MicSoundbarGUI STATIC
    mainwidget.cpp
    mainwidget.h
    player_page.cpp
    player_page.h
    sound_file_widget.cpp
    sound_file_widget.h
    text_to_speech_page.cpp
    text_to_speech_page.h
)

target_link_libraries(MicSoundbarGUI Qt6::Core Qt6::Widgets)
target_include_directories(MicSoundbarGUI PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${Qt6Widgets_INCLUDE_DIRS})