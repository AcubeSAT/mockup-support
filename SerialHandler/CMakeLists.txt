cmake_minimum_required(VERSION 3.5)
project(LinuxReceiver)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Debug)

find_package(glfw3 REQUIRED)
#find_package(glew REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost 1.50 COMPONENTS system REQUIRED )

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ libzmq REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/lib/imgui" "${PROJECT_SOURCE_DIR}/lib")
add_subdirectory (lib)

set(SOURCE_FILES main.cpp imgui_impl_glfw.cpp gl3w.c)
add_executable(LinuxReceiver ${SOURCE_FILES})
target_link_libraries (LinuxReceiver ImGui ImGuiDemo MadgwickAHRS glfw GL GLEW dl)
target_link_libraries (LinuxReceiver mysqlcppconn)
target_link_libraries (LinuxReceiver pthread ${Boost_LIBRARIES})
target_link_libraries (LinuxReceiver ${ZMQ_LIBRARIES})

add_executable(ZMQTestReceiver zmq_test_receiver.cpp)
target_link_libraries(ZMQTestReceiver ${ZMQ_LIBRARIES})

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "-O3")
