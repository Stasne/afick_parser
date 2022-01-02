cmake_minimum_required(VERSION 3.7)
project (AfickMonitoring)

include_directories(${Qt5Widgets_INCLUDE_DIRS} ${QtQml_INCLUDE_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS} ${QtQml_DEFINITIONS} ${Qt5Quick_DEFINITIONS})
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(nlohmann_json REQUIRED)

include_directories(${Qt5Widgets_INCLUDES})


set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fstrict-aliasing -Wno-deprecated-declarations -Wno-unused-variable")

file(GLOB GLOB_SRC "*.cpp" "*.h")

add_executable(${PROJECT_NAME} ${GLOB_SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

