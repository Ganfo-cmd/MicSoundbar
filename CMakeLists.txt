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
add_executable(MicSoundbar src/main.cpp)

# Указывание пути к заголовкам для конкретной цели
target_include_directories(MicSoundbar PRIVATE
    ${CMAKE_SOURCE_DIR}/include/mpg123
    ${CMAKE_SOURCE_DIR}/include/portaudio)   

# Линковка библиотек
target_link_libraries(MicSoundbar PRIVATE portaudio mpg123 winmm ole32 
    setupapi mmdevapi Shlwapi MicSoundbarGUI Qt6::Core Qt6::Widgets)

# 
target_include_directories(MicSoundbar PRIVATE
    ${CMAKE_SOURCE_DIR}/include #
    ${CMAKE_SOURCE_DIR}/gui #
    ${Qt6Core_INCLUDE_DIRS} 
    ${Qt6Widgets_INCLUDE_DIRS}
)

# Копирование необходимых Qt DLL файлов
if(WIN32)
    add_custom_command(TARGET MicSoundbar POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
            "C:/Qt/6.9.0/mingw_64/bin/Qt6Core.dll"
            $<TARGET_FILE_DIR:MicSoundbar>
        COMMAND ${CMAKE_COMMAND} -E copy
            "C:/Qt/6.9.0/mingw_64/bin/Qt6Widgets.dll"  
            $<TARGET_FILE_DIR:MicSoundbar>
        COMMAND ${CMAKE_COMMAND} -E copy
            "C:/Qt/6.9.0/mingw_64/bin/Qt6Gui.dll"
            $<TARGET_FILE_DIR:MicSoundbar>
    )
    
    # Копирование папки plugins для работы Qt
    add_custom_command(TARGET MicSoundbar POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "C:/Qt/6.9.0/mingw_64/plugins"
            "$<TARGET_FILE_DIR:MicSoundbar>/plugins"
    )
endif()