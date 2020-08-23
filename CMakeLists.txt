cmake_minimum_required(VERSION 3.0)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

project(GLSLCompiler)

if(MSVC)
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include("system_bit.cmake")
include("output_path.cmake")

add_subdirectory(SPIRV-Cross)
add_subdirectory(SPIRV-Cross/external/glslang)
add_subdirectory(SPIRV-Cross/external/spirv-tools)

include_directories(SPIRV-Cross/external)

set(VULKAN_SPIRV_LIBS   GenericCodeGen
                        glslang
                        MachineIndependent
                        OGLCompiler
                        OSDependent
                        SPIRV
                        SPIRV-Tools
                        SPIRV-Tools-opt
                        spirv-cross-core)

add_library(GLSLCompiler SHARED glsl2spv.cpp VKShaderParse.h)

target_link_libraries(GLSLCompiler PRIVATE ${VULKAN_SPIRV_LIBS} ${Vulkan_LIBRARY})
