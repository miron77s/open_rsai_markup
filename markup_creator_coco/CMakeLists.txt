cmake_minimum_required(VERSION 3.5)

project(markup_creator_coco LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GDAL REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

include_directories(./include)
include_directories(${GDAL_INCLUDE_DIR})

set(HEADERS
    include/converter.h
)

set(SOURCES
    src/converter.cpp
    src/main.cpp
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_link_libraries(
                        ${PROJECT_NAME}
                        gdal
                        ${CURL_LIBRARIES}
                        ${TIFF_LIBRARIES}
                        Qt5::Core
                        Qt5::Gui
                        Qt5::Widgets
)


install(TARGETS ${PROJECT_NAME} DESTINATION)
