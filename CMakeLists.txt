cmake_minimum_required(VERSION 3.10)

# Set the project name
project(rusolar_cam_v2)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add OpenCV package
find_package(OpenCV REQUIRED)

# Add the source files
add_executable(cam src/main.cpp)

# Link the OpenCV libraries
target_link_libraries(cam ${OpenCV_LIBS})

# Add the include directory to the target
target_include_directories(cam PRIVATE src/headers)