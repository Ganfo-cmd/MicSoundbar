cmake_minimum_required(VERSION 3.10)
project(MicSoundbar)

# 1. Указываем путь к библиотекам
link_directories(${CMAKE_SOURCE_DIR}/lib)

# 2. Добавляем исполняемый файл
add_executable(MicSoundbar src/main.cpp)

# 3. Указываем пути к заголовкам для конкретной цели
target_include_directories(MicSoundbar PRIVATE
    ${CMAKE_SOURCE_DIR}/include/mpg123
    ${CMAKE_SOURCE_DIR}/include/portaudio)   

# 4. Линкуем библиотеки 
target_link_libraries(MicSoundbar PRIVATE portaudio mpg123 winmm ole32 setupapi mmdevapi Shlwapi)