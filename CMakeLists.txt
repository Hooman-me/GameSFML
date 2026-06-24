cmake_minimum_required(VERSION 3.16)
project(TowerDefense)

set(CMAKE_CXX_STANDARD 17)

# Source files
set(SOURCES
    src/Main.cpp
    src/Map.cpp
    src/Enemy.cpp
    src/Troops.cpp
    src/Projectile.cpp
    src/Playstate.cpp
    src/Startstate.cpp
    src/Endingstate.cpp
)

add_executable(TowerDefense ${SOURCES})

# Memberitahu compiler di mana mencari file .hpp (termasuk nlohmann/json.hpp)
target_include_directories(TowerDefense PRIVATE include)

# SFML - Otomatis mendeteksi hasil dari 'apt install libsfml-dev'
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
target_link_libraries(TowerDefense sfml-graphics sfml-window sfml-system)

# Menyalin folder assets ke dalam folder build secara otomatis
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})