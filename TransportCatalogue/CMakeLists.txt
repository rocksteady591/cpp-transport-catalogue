cmake_minimum_required(VERSION 3.20)

project(transport_catalogue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(transport_catalogue
    main.cpp
    geo.cpp
    json.cpp
    json_reader.cpp
    map_renderer.cpp
    svg.cpp
    transport_catalogue.cpp
    json_builder.cpp
)

target_include_directories(transport_catalogue PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
