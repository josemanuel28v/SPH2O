cmake_minimum_required(VERSION 3.2)

project(SPH2O CXX)
set(CMAKE_CXX_STANDARD 20)

# Define symbol __STDC_LIB_EXT1__ for stb_image to use the secure version of sprint
add_definitions(-D__STDC_LIB_EXT1__)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/inc
    # Extern
    ${CMAKE_SOURCE_DIR}/extern/
    ${CMAKE_SOURCE_DIR}/extern/glm/inc
    ${CMAKE_SOURCE_DIR}/extern/glfw/include
    ${CMAKE_SOURCE_DIR}/extern/glfw/deps
    ${CMAKE_SOURCE_DIR}/extern/eigen # Needed for RegularTriangleSampling
    ${CMAKE_SOURCE_DIR}/extern/pugixml
)

# Source files to compile
file(GLOB_RECURSE cpu_source_files 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/GUI/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/SPH2O/*.cpp"
    "${CMAKE_SOURCE_DIR}/extern/RegularTriangleSampling/*.cpp")

add_executable(SPH2O ${cpu_source_files})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release build")
else()
    message(STATUS "No build mode selected")
endif()

# Check for MSVC compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(SPH2O
            PRIVATE $<$<COMPILE_LANGUAGE:CXX>: /arch:AVX2 /fp:fast>
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(SPH2O
            PRIVATE $<$<COMPILE_LANGUAGE:CXX>: /O2 /arch:AVX2 /fp:fast>
        )
    endif()

# Check for GCC compiler
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # Path to the binary in Debug build
        set(RELEASE_DIRECTORY ${CMAKE_BINARY_DIR}/Debug)
        file(MAKE_DIRECTORY ${RELEASE_DIRECTORY})
        set_target_properties(SPH2O PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${RELEASE_DIRECTORY})
        target_compile_options(SPH2O
            PRIVATE $<$<COMPILE_LANGUAGE:CXX>: -march=native -ffast-math>
        )
    elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        # Path to the binary in Release build
        set(RELEASE_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
        file(MAKE_DIRECTORY ${RELEASE_DIRECTORY})
        set_target_properties(SPH2O PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${RELEASE_DIRECTORY})

        target_compile_options(SPH2O
            PRIVATE $<$<COMPILE_LANGUAGE:CXX>: -O3 -march=native -ffast-math>
        )
    endif()
endif()

# Avoid link warnings in Windows
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") 
    target_link_options(SPH2O PRIVATE "/NODEFAULTLIB:LIBCMT")
endif()    

# Add subdirectory to build glfw library
add_subdirectory(${CMAKE_SOURCE_DIR}/extern/glfw)

# Find OpenMP
find_package(OpenMP REQUIRED)

# Add llvm support for OpenMP: unsigned int in loops and max in reduction
if(OpenMP_CXX_FOUND AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(SPH2O PRIVATE $<$<COMPILE_LANGUAGE:CXX>: /openmp:llvm>)
endif()

target_link_libraries(SPH2O glfw)