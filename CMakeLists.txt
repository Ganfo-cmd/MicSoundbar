cmake_minimum_required(VERSION 3.10)
project(MicSoundbar)

# 1. Указываем пути к заголовкам и библиотеке
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

# 2. Добавляем исполняемый файл
add_executable(MicSoundbar src/main.cpp)

# 3. Линкуем библиотеки 
target_link_libraries(MicSoundbar PRIVATE portaudio mpg123 winmm ole32 setupapi mmdevapi Shlwapi)