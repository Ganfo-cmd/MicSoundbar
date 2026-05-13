cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(GUI_Headers 
    include/mainwidget.h
    include/player_page.h
    include/text_to_speech_page.h
    include/sound_table_model.h
    include/play_button_delegate.h
    include/player_page_toolbar.h
    include/hotkey_delegate.h
)

set(GUI_Sources
    src/mainwidget.cpp
    src/player_page.cpp
    src/text_to_speech_page.cpp
    src/sound_table_model.cpp
    src/play_button_delegate.cpp
    src/player_page_toolbar.cpp
    src/hotkey_delegate.cpp
)

# Создаем GUI библиотеку
add_library(MicSoundbarGUI STATIC
    ${GUI_Headers}
    ${GUI_Sources}
    icons.qrc

)

target_include_directories(MicSoundbarGUI PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/import/mpg123    
    ${CMAKE_SOURCE_DIR}/import/portaudio
    ${Qt6Widgets_INCLUDE_DIRS}
)

target_link_libraries(MicSoundbarGUI 
    Qt6::Core 
    Qt6::Widgets
    portaudio
    mpg123
)