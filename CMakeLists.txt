cmake_minimum_required(VERSION 3.0)
project(openai-c)

set(CMAKE_C_STANDARD 11)

# Add include directory
include_directories(include)

# Find required libraries
find_package(CURL REQUIRED)
find_package(CJSON REQUIRED)

# Add source files
set(SOURCES
    openai.c
)

# Add executable for each example
file(GLOB EXAMPLES examples/*.c)
foreach(EXAMPLE ${EXAMPLES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE} ${SOURCES})
    target_link_libraries(${EXAMPLE_NAME} ${CURL_LIBRARIES} ${CJSON_LIBRARIES})
    target_include_directories(${EXAMPLE_NAME} PRIVATE ${CURL_INCLUDE_DIRS} ${CJSON_INCLUDE_DIRS})
endforeach()
