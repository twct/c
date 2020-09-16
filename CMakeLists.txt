cmake_minimum_required (VERSION 3.17)
project (c)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories (${CMAKE_SOURCE_DIR}/include)

add_executable (${PROJECT_NAME}
    src/main.cpp
)