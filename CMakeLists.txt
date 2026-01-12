cmake_minimum_required(VERSION 3.16)
project(reg LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

add_executable(main
    src/main.cpp
    src/core.cpp
    src/io.cpp
    src/render.cpp
    src/utils.cpp
)

target_include_directories(main PRIVATE
    include
)

target_link_libraries(main PRIVATE ${OpenCV_LIBS} Eigen3::Eigen)
