cmake_minimum_required(VERSION 3.10)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

project(GLSLCompiler)

include("compiler.cmake")
include("system_bit.cmake")
include("output_path.cmake")

# -------------------------------------------------------------------
# SDK / vcpkg / system build
# -------------------------------------------------------------------

# 1. Vulkan SDK root: the -DVULKAN_SDK_DIR=... cache variable wins, otherwise
#    fall back to the VULKAN_SDK environment variable.  The root is
#    prepended to the CMake search path so that find_path / find_library
#    prefer the SDK over the OS.
set(VULKAN_SDK_DIR "" CACHE PATH "Vulkan SDK root directory (overrides the VULKAN_SDK env var)")
if(VULKAN_SDK_DIR)
    set(_vulkan_sdk_root "${VULKAN_SDK_DIR}")
else()
    set(_vulkan_sdk_root "$ENV{VULKAN_SDK}")
endif()

if(_vulkan_sdk_root)
    if(NOT EXISTS "${_vulkan_sdk_root}")
        message(WARNING "VULKAN_SDK points to a missing directory: ${_vulkan_sdk_root}")
    else()
        message(STATUS "Vulkan SDK detected: ${_vulkan_sdk_root}")
        list(INSERT CMAKE_PREFIX_PATH 0 "${_vulkan_sdk_root}")
        if(WIN32)
            set(SDK_INCLUDE_DIR "${_vulkan_sdk_root}/Include")
            set(SDK_LIB_DIR     "${_vulkan_sdk_root}/Lib")
        else()
            set(SDK_INCLUDE_DIR "${_vulkan_sdk_root}/include")
            set(SDK_LIB_DIR     "${_vulkan_sdk_root}/lib")
        endif()
        include_directories("${SDK_INCLUDE_DIR}")
        link_directories("${SDK_LIB_DIR}")
    endif()
endif()

# -------------------------------------------------------------------
# Helper: locate a pre-built library and assemble a linkable
# `<base>_LIBRARY` variable.  Release and debug names are searched
# separately - the Vulkan SDK appends a 'd' suffix to its debug libs
# (spirv-cross-core.lib vs spirv-cross-cored.lib) - and
# select_library_configurations() pairs them so Debug builds link the
# debug libs (avoids MSVC LNK2038 runtime-library mismatch).
# -------------------------------------------------------------------
include(SelectLibraryConfigurations)

function(sdk_find_library base)
    set(_release_names ${ARGN})
    set(_debug_names "")
    foreach(_n IN LISTS _release_names)
        list(APPEND _debug_names "${_n}d")
    endforeach()

    find_library(${base}_LIBRARY_RELEASE NAMES ${_release_names} HINTS ${SDK_LIB_DIR})
    find_library(${base}_LIBRARY_DEBUG   NAMES ${_debug_names}   HINTS ${SDK_LIB_DIR})

    select_library_configurations(${base})

    set(${base}_LIBRARY "${${base}_LIBRARY}" PARENT_SCOPE)
endfunction()

# -------------------------------------------------------------------
# 2. glslang  (provides GlslangToSpv + SPIR-V generation)
#    The Vulkan SDK ships glslang.lib / SPIRV.lib / OSDependent.lib /
#    MachineIndependent.lib / GenericCodeGen.lib (+ 'd'-suffixed debug
#    variants).  vcpkg's glslang port installs a CMake config instead,
#    which is picked up by find_package below.
# -------------------------------------------------------------------
find_package(glslang CONFIG QUIET)
if(glslang_FOUND)
    message(STATUS "glslang found via CMake config (version ${glslang_VERSION})")
    if(TARGET glslang::glslang)
        # Modern namespaced targets (vcpkg, recent distros)
        set(GLSLANG_LIBS
            glslang::glslang
            glslang::MachineIndependent
            glslang::GenericCodeGen
            glslang::OSDependent
            glslang::SPIRV)
        if(TARGET glslang::glslang-default-resource-limits)
            list(APPEND GLSLANG_LIBS glslang::glslang-default-resource-limits)
        endif()
    else()
        # Older cmake configs that export un-namespaced targets
        set(GLSLANG_LIBS glslang SPIRV OSDependent MachineIndependent GenericCodeGen)
    endif()
else()
    # Manual fallback: locate headers and libraries by path
    message(STATUS "glslang CMake config not found - searching for pre-built libraries")
    sdk_find_library(glslang             glslang)
    sdk_find_library(glslang_SPIRV       SPIRV)
    sdk_find_library(glslang_OSDependent OSDependent)
    sdk_find_library(glslang_MachineInd  MachineIndependent)
    sdk_find_library(glslang_GenericCode GenericCodeGen)
    set(GLSLANG_LIBS
        ${glslang_LIBRARY}
        ${glslang_SPIRV_LIBRARY}
        ${glslang_OSDependent_LIBRARY}
        ${glslang_MachineInd_LIBRARY}
        ${glslang_GenericCode_LIBRARY})
endif()

find_path(GLSLANG_INCLUDE_DIR
    NAMES glslang/SPIRV/GlslangToSpv.h
    HINTS ${SDK_INCLUDE_DIR})
if(GLSLANG_INCLUDE_DIR)
    include_directories("${GLSLANG_INCLUDE_DIR}")
endif()

# -------------------------------------------------------------------
# 3. SPIRV-Cross  (provides spirv_cross.hpp + spirv-cross-core)
#    Bundled in the Vulkan SDK (Include/spirv_cross/ +
#    Lib/spirv-cross-core[.d].lib).  This project only uses the
#    reflection API (spirv_cross::Compiler), so spirv-cross-core is
#    the only SPIRV-Cross library required.
#    Sources include it unprefixed (`#include "spirv_cross.hpp"`),
#    so the include directory must point at the spirv_cross/ folder.
# -------------------------------------------------------------------
sdk_find_library(spirv_cross_core spirv-cross-core)
set(SPIRV_CROSS_LIBS ${spirv_cross_core_LIBRARY})

if(DEFINED SDK_INCLUDE_DIR)
    set(_spirv_cross_hints "${SDK_INCLUDE_DIR}/spirv_cross")
endif()
find_path(SPIRV_CROSS_INCLUDE_DIR
    NAMES spirv_cross.hpp
    HINTS ${_spirv_cross_hints})
if(SPIRV_CROSS_INCLUDE_DIR)
    message(STATUS "SPIRV-Cross headers: ${SPIRV_CROSS_INCLUDE_DIR}")
    include_directories("${SPIRV_CROSS_INCLUDE_DIR}")
else()
    message(FATAL_ERROR
        "SPIRV-Cross headers (spirv_cross.hpp) not found. "
        "Set -DVULKAN_SDK_DIR=... or the VULKAN_SDK env var to a valid Vulkan SDK "
        "(current root: '${_vulkan_sdk_root}'), or install spirv-cross via "
        "vcpkg/system package.")
endif()

# -------------------------------------------------------------------
# 4. SPIRV-Tools  (optimizer)
#    glslang's SpvTools.obj references spvtools::Optimizer passes, so
#    SPIRV-Tools-opt is REQUIRED at link time even though this project
#    never calls the optimizer directly.
# -------------------------------------------------------------------
sdk_find_library(SPIRV_Tools     SPIRV-Tools)
sdk_find_library(SPIRV_Tools_opt SPIRV-Tools-opt)
set(SPIRV_TOOLS_LIBS ${SPIRV_Tools_LIBRARY} ${SPIRV_Tools_opt_LIBRARY})

set(VULKAN_SPIRV_LIBS
    ${GLSLANG_LIBS}
    ${SPIRV_CROSS_LIBS}
    ${SPIRV_TOOLS_LIBS})

add_library(GLSLCompiler SHARED glsl2spv.cpp VKShaderParse.h GLSLCompiler.h)

target_link_libraries(GLSLCompiler PRIVATE ${VULKAN_SPIRV_LIBS})

add_library(GLSLCompilerStatic STATIC glsl2spv.cpp VKShaderParse.h GLSLCompiler.h)

target_link_libraries(GLSLCompilerStatic PRIVATE ${VULKAN_SPIRV_LIBS})

# spvfc - command-line shader compiler (GLSL/HLSL -> .spvf)
add_executable(spvfc spvfc.cpp)
target_link_libraries(spvfc PRIVATE GLSLCompilerStatic ${VULKAN_SPIRV_LIBS})
