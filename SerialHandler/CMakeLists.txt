cmake_minimum_required(VERSION 3.5)
project(LinuxReceiver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

find_package(glfw3 REQUIRED)
#find_package(glew REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Boost 1.50 COMPONENTS system REQUIRED )

find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ libzmq REQUIRED)

include_directories ("${PROJECT_SOURCE_DIR}/lib/imgui" "${PROJECT_SOURCE_DIR}/lib")
add_subdirectory (lib)

# ECSS services stuff
add_subdirectory(lib/ecss-services EXCLUDE_FROM_ALL)
include_directories(
        "${PROJECT_SOURCE_DIR}/lib/ecss-services/inc"
        "${PROJECT_SOURCE_DIR}/lib/ecss-services/lib/etl/include"
        "${PROJECT_SOURCE_DIR}/lib/freertos/include"
        "${PROJECT_SOURCE_DIR}/lib/freertos/portable/GCC/ARM_CM4F"
)

set(SOURCE_FILES main.cpp imgui_impl_glfw.cpp gl3w.c lib/mockup/ECSSObjects.cpp)
set(ECSS_PLATFORM lib/ecss-services/src/Platform/x86/Logger.cpp lib/ecss-services/src/Platform/x86/ErrorHandler.cpp)
set(VARIOUS lib/cobs/cobs.c)
add_executable(LinuxReceiver ${SOURCE_FILES} ${ECSS_PLATFORM} ${VARIOUS})
target_link_libraries (LinuxReceiver ImGui ImGuiDemo glfw GL GLEW dl)
target_link_libraries (LinuxReceiver pthread ${Boost_LIBRARIES})
target_link_libraries (LinuxReceiver ${ZMQ_LIBRARIES})
target_link_libraries (LinuxReceiver common)
set_target_properties(LinuxReceiver PROPERTIES COMPILE_DEFINITIONS LOGLEVEL_TRACE)


add_executable(ZMQTestReceiver zmq_test_receiver.cpp)
target_link_libraries(ZMQTestReceiver ${ZMQ_LIBRARIES})

set(CMAKE_VERBOSE_MAKEFILE ON)
#set(CMAKE_CXX_FLAGS "-O3")
