cmake_minimum_required(VERSION 3.0)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

project(GLSLCompiler)

if(MSVC)
    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MD")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")

    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

include("FindVulkan.cmake")

include_directories(${Vulkan_INCLUDE_DIRS})
link_directories(${Vulkan_LIBRARIES_DIR})

set(VULKAN_SPIRV_LIBS   GenericCodeGen
                        glslang
                        HLSL
                        MachineIndependent
                        OGLCompiler
                        OSDependent
                        SPIRV
                        SPIRV-Tools
                        SPIRV-Tools-opt
                        spirv-cross-core)

add_library(GLSLCompiler SHARED glsl2spv.cpp)

target_link_libraries(GLSLCompiler PRIVATE ${VULKAN_SPIRV_LIBS} ${Vulkan_LIBRARY})
