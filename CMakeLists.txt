cmake_minimum_required(VERSION 3.10)
project(chess_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check CPU architecture and OS
if(MSVC)
    add_compile_options(/W4 /arch:AVX2)
else()
    # Common flags for GCC/Clang
    add_compile_options(-Wall -Wextra)
    
    if(APPLE)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
            add_compile_options(-march=native)
        else()
            add_compile_options(-march=native -mavx2)
        endif()
    elseif(UNIX)
        # Linux specific configuration
        include(CheckCXXCompilerFlag)
        
        # Check for AVX2 support
        check_cxx_compiler_flag("-mavx2" COMPILER_SUPPORTS_AVX2)
        if(COMPILER_SUPPORTS_AVX2)
            add_compile_options(-march=native -mavx2)
        else()
            # Check for NEON support on ARM
            if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
                add_compile_options(-march=native)
            else()
                add_compile_options(-march=native)
            endif()
        endif()
        
        # Add threading support
        find_package(Threads REQUIRED)
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

# Link against threading library on Unix
if(UNIX AND NOT APPLE)
    target_link_libraries(chess_engine PRIVATE Threads::Threads)
endif()

target_include_directories(chess_engine PRIVATE include) 