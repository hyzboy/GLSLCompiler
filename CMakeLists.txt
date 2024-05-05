cmake_minimum_required(VERSION 3.5)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

project(GLSLCompiler)

if(MSVC)
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include("compiler.cmake")
include("system_bit.cmake")
include("output_path.cmake")

OPTION(USE_VULKAN_SDK "Use Vulkan SDK" OFF)

IF(USE_VULKAN_SDK)

    include("FindVulkan.cmake")

    include_directories(${Vulkan_INCLUDE_DIRS})

    message("Vulkan Include: " ${Vulkan_INCLUDE_DIRS})
    message("Vulkan Library: " ${Vulkan_LIBRARIES_DIR})

    set(CMAKE_MODULE_PATH ${Vulkan_LIBRARIES}/cmake)
    set(CMAKE_MODULE_PATH ${Vulkan_LIBRARIES}/cmake/glslang)

    target_link_directories(GLSLCompiler PRIVATE ${Vulkan_LIBRARIES_DIR})

ELSE()

    add_subdirectory(SPIRV-Cross)
    add_subdirectory(SPIRV-Cross/external/glslang)
    add_subdirectory(SPIRV-Cross/external/spirv-tools)

    include_directories(SPIRV-Cross/external)

ENDIF()

set(VULKAN_SPIRV_LIBS   GenericCodeGen
                        glslang
                        MachineIndependent
                        OSDependent
                        SPIRV
                        SPIRV-Tools
                        SPIRV-Tools-opt
                        spirv-cross-core)

add_library(GLSLCompiler SHARED glsl2spv.cpp VKShaderParse.h)

target_link_libraries(GLSLCompiler PRIVATE ${VULKAN_SPIRV_LIBS})
