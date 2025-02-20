cmake_minimum_required(VERSION 3.10)
project(chess_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /arch:AVX2)
else()
    if(APPLE)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
            add_compile_options(-Wall -Wextra -march=native)
        else()
            add_compile_options(-Wall -Wextra -march=native -mavx2)
        endif()
    else()
        add_compile_options(-Wall -Wextra -march=native -mavx2)
    endif()
endif()

set(SOURCES
    main.cpp
    module/board.cpp
    module/engine.cpp
    module/evaluation.cpp
    module/move.cpp
    module/nnue.cpp
    module/piece.cpp
    module/uci.cpp
    module/simd_utils.cpp
)

add_executable(chess_engine ${SOURCES})

target_include_directories(chess_engine PRIVATE include) 