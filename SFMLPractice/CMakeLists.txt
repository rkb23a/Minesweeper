cmake_minimum_required(VERSION 3.2)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SFMLPractice)

set(SFML_DIR "C:/Users/rachk/Downloads/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1")

find_package(SFML 2.5.1 COMPONENTS system window graphics audio REQUIRED)
add_executable(SFMLPractice main.cpp)
target_link_libraries(SFMLPractice sfml-system sfml-window sfml-graphics sfml-audio)