cmake_minimum_required(VERSION 3.5)
project(computer_vision_project)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(OpenCV REQUIRED)

set(SOURCE_FILES main.cpp)
add_executable(computer_vision_project ${SOURCE_FILES})

include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries(computer_vision_project ${OpenCV_LIBS})