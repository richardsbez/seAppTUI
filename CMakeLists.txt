cmake_minimum_required(VERSION 3.10)
project(minilauncher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)

add_executable(minilauncher
    main.cpp
    app_searcher.cpp
    tui.cpp
)

target_include_directories(minilauncher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(minilauncher ${CURSES_LIBRARIES})

