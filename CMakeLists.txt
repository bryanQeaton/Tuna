cmake_minimum_required(VERSION 3.29)
project(ChessBot)

set(CMAKE_CXX_STANDARD 20)

# Base directory relative includes
include_directories(.)
include_directories(./src/)

# Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -pedantic -pedantic-errors -Wextra -Wcast-align -Wcast-qual -Wchar-subscripts -Wcomment -Wdisabled-optimization -Wfloat-equal -Wformat -Wformat=2 -Wformat-nonliteral -Wformat-security -Wformat-y2k -Wimport -Winit-self -Winline -Winvalid-pch -Wmissing-braces -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wodr -Wpacked -Wparentheses -Wpointer-arith -Wredundant-decls -Wreturn-type -Wsequence-point -Wshadow -Wsign-compare -Wstack-protector -Wstrict-aliasing -Wstrict-aliasing=2 -Wswitch -Wswitch-default -Wswitch-enum -Wtrigraphs -Wuninitialized -Wunknown-pragmas -Wunreachable-code -Wunused -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value -Wunused-variable -Wvariadic-macros -Wvolatile-register-var -Wwrite-strings")
set(CMAKE_CXX_FLAGS_DEBUG "-O3")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE)
set(CMAKE_EXE_LINKER_FLAGS "-static")

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_library(
        libchess_obj
        OBJECT
        src/attackers.cpp
        src/checkers.cpp
        src/check_evasions.cpp
        src/count_moves.cpp
        src/get_fen.cpp
        src/is_legal.cpp
        src/king_allowed.cpp
        src/legal_captures.cpp
        src/legal_moves.cpp
        src/legal_noncaptures.cpp
        src/makemove.cpp
        src/movegen.cpp
        src/perft.cpp
        src/pinned.cpp
        src/predict_hash.cpp
        src/set_fen.cpp
        src/square_attacked.cpp
        src/squares_attacked.cpp
        src/undomove.cpp
        src/valid.cpp
        src/zobrist.cpp
        main.cpp
        evaluation.h
        Params.h
        uci.h
        root.h
        search.h
        tt.h
        eval_tuner.h
)
# Add the static library
add_library(
        libchess_static
        STATIC
        $<TARGET_OBJECTS:libchess_obj>
        main.cpp
        evaluation.h
        Params.h
)

# Add the shared library
add_library(
        libchess_shared
        SHARED
        $<TARGET_OBJECTS:libchess_obj>
        main.cpp
        evaluation.h
        Params.h
)

target_include_directories(
        libchess_static
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(
        libchess_shared
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


# Add example
add_executable(
        perft
        examples/perft.cpp
)

# Add example
add_executable(
        ttperft
        examples/ttperft.cpp
)

# Add example
add_executable(
        split
        examples/split.cpp
)

# Add example
add_executable(
        suite
        examples/suite.cpp
)

# Add example
add_executable(
        ttsuite
        examples/ttsuite.cpp
)

# Add example
add_executable(
        suite960
        examples/suite960.cpp
)
add_executable(
        main
        main.cpp
)
if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-subsystem,console")
endif()
set_target_properties(
        libchess_static
        PROPERTIES
        OUTPUT_NAME "chess"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/static"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/static"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/static"
)

set_target_properties(
        libchess_shared
        PROPERTIES
        OUTPUT_NAME "chess"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/shared"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/shared"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/shared"
)

target_link_libraries(main libchess_static)
target_link_libraries(perft libchess_static)
target_link_libraries(ttperft libchess_static)
target_link_libraries(split libchess_static)
target_link_libraries(suite libchess_static)
target_link_libraries(ttsuite libchess_static)
target_link_libraries(suite960 libchess_static)