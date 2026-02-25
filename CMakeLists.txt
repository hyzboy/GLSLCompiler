cmake_minimum_required(VERSION 3.10)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

project(GLSLCompiler)

include("compiler.cmake")
include("system_bit.cmake")
include("output_path.cmake")

# ---------------------------------------------------------------------------
# USE_GLSLANG_SUBMODULE (default ON)
#   ON  – build glslang / SPIRV-Cross / SPIRV-Tools from the git submodule.
#         No external dependencies required; works out-of-the-box after
#         `git submodule update --init --recursive`.
#   OFF – find pre-built libraries.  Search order (first match wins):
#           1. Standalone Vulkan SDK  ($VULKAN_SDK environment variable)
#           2. vcpkg                  (automatic when CMAKE_TOOLCHAIN_FILE points
#                                      to vcpkg; works on Windows / Linux / macOS)
#           3. System package paths   (Linux: apt/dnf/pacman; macOS: Homebrew/MacPorts)
# ---------------------------------------------------------------------------
option(USE_GLSLANG_SUBMODULE
    "Build glslang and SPIRV-Cross from the git submodule (ON) or use system/SDK/vcpkg (OFF)"
    ON)

IF(USE_GLSLANG_SUBMODULE)

    # -------------------------------------------------------------------
    # Submodule build – compile everything from source
    # -------------------------------------------------------------------
    add_subdirectory(SPIRV-Cross)
    add_subdirectory(SPIRV-Cross/external/glslang)
    add_subdirectory(SPIRV-Cross/external/spirv-tools)

    include_directories(SPIRV-Cross/external)

    set(VULKAN_SPIRV_LIBS
        GenericCodeGen
        glslang
        MachineIndependent
        OSDependent
        SPIRV
        SPIRV-Tools
        SPIRV-Tools-opt
        spirv-cross-core)

ELSE()

    # -------------------------------------------------------------------
    # System / SDK / vcpkg build
    # -------------------------------------------------------------------

    # 1. If $VULKAN_SDK is set, prepend it to the CMake search path so that
    #    find_package / find_library / find_path prefer the SDK over the OS.
    if(DEFINED ENV{VULKAN_SDK})
        message(STATUS "Vulkan SDK detected: $ENV{VULKAN_SDK}")
        list(INSERT CMAKE_PREFIX_PATH 0 "$ENV{VULKAN_SDK}")
        if(WIN32)
            include_directories("$ENV{VULKAN_SDK}/Include")
            link_directories("$ENV{VULKAN_SDK}/Lib")
        else()
            include_directories("$ENV{VULKAN_SDK}/include")
            link_directories("$ENV{VULKAN_SDK}/lib")
        endif()
    endif()

    # -------------------------------------------------------------------
    # 2. glslang  (provides GlslangToSpv + SPIRV compilation)
    #    cmake config available from: Vulkan SDK ≥ 1.2, vcpkg, system
    # -------------------------------------------------------------------
    find_package(glslang CONFIG QUIET)
    if(glslang_FOUND)
        message(STATUS "glslang found via CMake config (version ${glslang_VERSION})")
        if(TARGET glslang::glslang)
            # Modern namespaced targets (vcpkg, Vulkan SDK ≥ 1.3, recent distros)
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
        message(STATUS "glslang CMake config not found – falling back to manual search")
        find_path(GLSLANG_INCLUDE_DIR
            NAMES glslang/SPIRV/GlslangToSpv.h
            HINTS "$ENV{VULKAN_SDK}/include" "$ENV{VULKAN_SDK}/Include")
        if(GLSLANG_INCLUDE_DIR)
            include_directories("${GLSLANG_INCLUDE_DIR}")
        endif()
        find_library(GLSLANG_LIB_glslang        NAMES glslang)
        find_library(GLSLANG_LIB_SPIRV          NAMES SPIRV)
        find_library(GLSLANG_LIB_OSDependent    NAMES OSDependent)
        find_library(GLSLANG_LIB_MachineIndep   NAMES MachineIndependent)
        find_library(GLSLANG_LIB_GenericCodeGen NAMES GenericCodeGen)
        set(GLSLANG_LIBS
            ${GLSLANG_LIB_glslang}
            ${GLSLANG_LIB_SPIRV}
            ${GLSLANG_LIB_OSDependent}
            ${GLSLANG_LIB_MachineIndep}
            ${GLSLANG_LIB_GenericCodeGen})
    endif()

    # -------------------------------------------------------------------
    # 3. SPIRV-Cross  (provides spirv_cross.hpp + spirv-cross-core)
    #    Note: NOT bundled in the standard Vulkan SDK; install separately
    #    via vcpkg (`vcpkg install spirv-cross`), apt (`libspirv-cross-dev`),
    #    or Homebrew (`brew install spirv-cross`).
    # -------------------------------------------------------------------
    find_package(spirv_cross_core CONFIG QUIET)
    if(spirv_cross_core_FOUND)
        message(STATUS "spirv-cross-core found via CMake config")
        set(SPIRV_CROSS_LIBS spirv-cross-core)
    else()
        message(STATUS "spirv_cross_core CMake config not found – falling back to manual search")
        find_library(SPIRV_CROSS_LIB NAMES spirv-cross-core spirv_cross_core)
        set(SPIRV_CROSS_LIBS ${SPIRV_CROSS_LIB})
        # Locate headers: system installs put them in spirv_cross/ subdirectory
        find_path(SPIRV_CROSS_INCLUDE_DIR
            NAMES spirv_cross.hpp spirv_cross/spirv_cross.hpp)
        if(SPIRV_CROSS_INCLUDE_DIR)
            include_directories("${SPIRV_CROSS_INCLUDE_DIR}")
        endif()
    endif()

    # -------------------------------------------------------------------
    # 4. SPIRV-Tools  (optimizer; bundled in Vulkan SDK and vcpkg)
    # -------------------------------------------------------------------
    find_package(SPIRV-Tools CONFIG QUIET)
    if(SPIRV-Tools_FOUND)
        message(STATUS "SPIRV-Tools found via CMake config")
        set(SPIRV_TOOLS_LIBS SPIRV-Tools SPIRV-Tools-opt)
    else()
        find_library(SPIRV_TOOLS_LIB     NAMES SPIRV-Tools)
        find_library(SPIRV_TOOLS_OPT_LIB NAMES SPIRV-Tools-opt)
        if(SPIRV_TOOLS_LIB)
            set(SPIRV_TOOLS_LIBS ${SPIRV_TOOLS_LIB} ${SPIRV_TOOLS_OPT_LIB})
            message(STATUS "SPIRV-Tools found via manual search")
        else()
            set(SPIRV_TOOLS_LIBS "")
            message(WARNING "SPIRV-Tools not found; optimisation passes will be unavailable")
        endif()
    endif()

    set(VULKAN_SPIRV_LIBS
        ${GLSLANG_LIBS}
        ${SPIRV_CROSS_LIBS}
        ${SPIRV_TOOLS_LIBS})

ENDIF()

add_library(GLSLCompiler SHARED glsl2spv.cpp VKShaderParse.h)

target_link_libraries(GLSLCompiler PRIVATE ${VULKAN_SPIRV_LIBS})

add_library(GLSLCompilerStatic STATIC glsl2spv.cpp VKShaderParse.h)

target_link_libraries(GLSLCompilerStatic PRIVATE ${VULKAN_SPIRV_LIBS})
