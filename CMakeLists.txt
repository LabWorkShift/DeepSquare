
cmake_minimum_required(VERSION 3.10)
project(SuperChessEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O3 -march=native)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB SOURCES 
    "module/*.cpp"
    "main.cpp"
)

add_executable(chess_engine ${SOURCES})

if(UNIX)
    target_link_libraries(chess_engine pthread)
endif() 
