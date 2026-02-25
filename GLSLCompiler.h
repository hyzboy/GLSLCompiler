#pragma once

/*
 * GLSLCompiler.h — Public C++ API header for GLSLCompiler
 *
 * Include this file in any code that links against GLSLCompiler.dll/.so
 * or GLSLCompilerStatic.  Also includes SPVParseFlat.h so consumers get
 * the zero-dependency flat-file loader in the same include.
 */

#include <stdint.h>
#include "SPVParseFlat.h"

/* -----------------------------------------------------------------------
 * Source language & compilation parameters
 * These are C++ types and must be used from C++ callers.
 * --------------------------------------------------------------------- */

enum class ShaderLanguageType
{
    GLSL = 0,
    HLSL = 1,
    MAX  = 0xff
};

struct CompileInfo
{
    ShaderLanguageType  shader_type;     /* source language                  */
    const char *        entrypoint;      /* HLSL entry-point name (or NULL)  */
    uint32_t            includes_count;  /* number of include search paths   */
    const char **       includes;        /* include search-path array        */
    const char *        preamble;        /* text prepended before source     */
    uint32_t            vulkan_version;  /* glslang EShTargetVulkan_x_y      */
    uint32_t            spv_version;     /* glslang EShTargetSpv_x_y         */
};

/* -----------------------------------------------------------------------
 * Opaque result handles — never dereference directly.
 * Use the accessor / API functions below.
 * --------------------------------------------------------------------- */
typedef struct SPVData     SPVData;
typedef struct SPVParseData SPVParseData;

/* -----------------------------------------------------------------------
 * C-linkage API (callable from C++ with no name-mangling concerns)
 * --------------------------------------------------------------------- */
extern "C"
{

/* Lifecycle ------------------------------------------------------------ */
bool InitShaderCompiler();
void CloseShaderCompiler();

/* SPVData field accessors ---------------------------------------------- */
bool        SPVDataGetResult  (const SPVData *d);
const char *SPVDataGetLog     (const SPVData *d);
const char *SPVDataGetDebugLog(const SPVData *d);

/* Compilation ---------------------------------------------------------- */
SPVData *Shader2SPV    (uint32_t stage, const char *source, const CompileInfo *ci);
SPVData *CompileFromPath(uint32_t stage, const char *path,  const CompileInfo *ci);
void     FreeSPVData   (SPVData *spv);

/* Reflection (requires SPIRV-Cross at link time) ----------------------- */
SPVParseData *ParseSPV    (SPVData *spv);
void          FreeSPVParse(SPVParseData *spv);

/* Unified shader flat file --------------------------------------------- */
uint8_t *CreateShaderFlat(uint32_t stage, const char *entrypoint,
                           const SPVData *spv, const SPVParseData *parsed,
                           uint32_t *out_size);
void     FreeShaderFlat  (uint8_t *flat_data);

/* Utilities ------------------------------------------------------------ */
uint32_t GetShaderStageFlagByExtName(const char *ext_name);

} /* extern "C" */
