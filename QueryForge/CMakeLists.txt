
cmake_minimum_required(VERSION 3.16)

project(QueryForge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(QueryForge
    src/main.cpp
    src/core/Column.cpp
    src/core/Value.cpp
    src/core/Record.cpp
    src/core/Table.cpp
    src/core/Database.cpp
    src/execution/QueryExecutor.cpp
)

