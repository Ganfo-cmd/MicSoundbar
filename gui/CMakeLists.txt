cmake_minimum_required(VERSION 3.10)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(GUI_Headers 
    include/mainwidget.h
    include/sound_table_model.h
    include/play_button_delegate.h
    include/hotkey_delegate.h
    include/sounds_table_widget.h
    include/list_widget_delegate.h
    include/toolbar.h
    include/player_control_widget.h
)

set(GUI_Sources
    src/mainwidget.cpp
    src/sound_table_model.cpp
    src/play_button_delegate.cpp
    src/hotkey_delegate.cpp
    src/sounds_table_widget.cpp
    src/list_widget_delegate.cpp
    src/toolbar.cpp
    src/player_control_widget.cpp
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