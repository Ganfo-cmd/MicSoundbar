cmake_minimum_required(VERSION 3.10)
project(MicSoundbar)

set(Qt6_DIR "$ENV{QT_DIR}/lib/cmake/Qt6" CACHE PATH "Qt6 path")

# Включение авто-функции Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Нахождение Qt
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Указывание пути к библиотекам
link_directories(${CMAKE_SOURCE_DIR}/lib)

# Добавление GUI поддиректории
add_subdirectory(gui)

# Добавление исполняемого файла
add_executable(MicSoundbar src/main.cpp src/mp3_decoder.cpp src/sound_player.cpp src/media_files.cpp)

# После add_executable
target_include_directories(MicSoundbar PRIVATE
    ${CMAKE_SOURCE_DIR}/import/mpg123
    ${CMAKE_SOURCE_DIR}/import/portaudio
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/gui
)

# Линковка библиотек
target_link_libraries(MicSoundbar PRIVATE portaudio mpg123 winmm ole32 
    setupapi mmdevapi Shlwapi MicSoundbarGUI)

# Копирование необходимых Qt DLL файлов
if(WIN32)
    add_custom_command(TARGET MicSoundbar POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:Qt6::Core>"
            $<TARGET_FILE_DIR:MicSoundbar>
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:Qt6::Gui>"
            $<TARGET_FILE_DIR:MicSoundbar>
        COMMAND ${CMAKE_COMMAND} -E copy
            "$<TARGET_FILE:Qt6::Widgets>"
            $<TARGET_FILE_DIR:MicSoundbar>
    )
    
    # Копирование папки plugins для работы Qt
    add_custom_command(TARGET MicSoundbar POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$ENV{QT_DIR}/plugins"
            "$<TARGET_FILE_DIR:MicSoundbar>/plugins"
    )
endif()