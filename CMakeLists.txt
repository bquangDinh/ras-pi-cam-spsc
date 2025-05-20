cmake_minimum_required(VERSION 3.10)

# Set the project name
project(rusolar_cam_v2)

# Set build binary name
set(BINARY_NAME rusolar_cam)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add OpenCV package
find_package(OpenCV REQUIRED)

# Add the source files
add_executable(${BINARY_NAME} src/main.cpp)

# Link the OpenCV libraries
target_link_libraries(${BINARY_NAME} ${OpenCV_LIBS})

target_include_directories(${BINARY_NAME} PRIVATE src/headers)