#include<glslang/SPIRV/GlslangToSpv.h>
#include<glslang/Include/ResourceLimits.h>
#include<vector>
#include<iostream>

typedef enum VkShaderStageFlagBits {
    VK_SHADER_STAGE_VERTEX_BIT = 0x00000001,
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT = 0x00000002,
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT = 0x00000004,
    VK_SHADER_STAGE_GEOMETRY_BIT = 0x00000008,
    VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010,
    VK_SHADER_STAGE_COMPUTE_BIT = 0x00000020,
    VK_SHADER_STAGE_ALL_GRAPHICS = 0x0000001F,
    VK_SHADER_STAGE_ALL = 0x7FFFFFFF,
    VK_SHADER_STAGE_RAYGEN_BIT_KHR = 0x00000100,
    VK_SHADER_STAGE_ANY_HIT_BIT_KHR = 0x00000200,
    VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR = 0x00000400,
    VK_SHADER_STAGE_MISS_BIT_KHR = 0x00000800,
    VK_SHADER_STAGE_INTERSECTION_BIT_KHR = 0x00001000,
    VK_SHADER_STAGE_CALLABLE_BIT_KHR = 0x00002000,
    VK_SHADER_STAGE_TASK_BIT_NV = 0x00000040,
    VK_SHADER_STAGE_MESH_BIT_NV = 0x00000080,
    VK_SHADER_STAGE_RAYGEN_BIT_NV = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
    VK_SHADER_STAGE_ANY_HIT_BIT_NV = VK_SHADER_STAGE_ANY_HIT_BIT_KHR,
    VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,
    VK_SHADER_STAGE_MISS_BIT_NV = VK_SHADER_STAGE_MISS_BIT_KHR,
    VK_SHADER_STAGE_INTERSECTION_BIT_NV = VK_SHADER_STAGE_INTERSECTION_BIT_KHR,
    VK_SHADER_STAGE_CALLABLE_BIT_NV = VK_SHADER_STAGE_CALLABLE_BIT_KHR,
    VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
} VkShaderStageFlagBits;

static TBuiltInResource default_build_in_resource;

void init_default_build_in_resource() 
{
    default_build_in_resource.maxLights = 32;
    default_build_in_resource.maxClipPlanes = 6;
    default_build_in_resource.maxTextureUnits = 32;
    default_build_in_resource.maxTextureCoords = 32;
    default_build_in_resource.maxVertexAttribs = 64;
    default_build_in_resource.maxVertexUniformComponents = 4096;
    default_build_in_resource.maxVaryingFloats = 64;
    default_build_in_resource.maxVertexTextureImageUnits = 32;
    default_build_in_resource.maxCombinedTextureImageUnits = 80;
    default_build_in_resource.maxTextureImageUnits = 32;
    default_build_in_resource.maxFragmentUniformComponents = 4096;
    default_build_in_resource.maxDrawBuffers = 32;
    default_build_in_resource.maxVertexUniformVectors = 128;
    default_build_in_resource.maxVaryingVectors = 8;
    default_build_in_resource.maxFragmentUniformVectors = 16;
    default_build_in_resource.maxVertexOutputVectors = 16;
    default_build_in_resource.maxFragmentInputVectors = 15;
    default_build_in_resource.minProgramTexelOffset = -8;
    default_build_in_resource.maxProgramTexelOffset = 7;
    default_build_in_resource.maxClipDistances = 8;
    default_build_in_resource.maxComputeWorkGroupCountX = 65535;
    default_build_in_resource.maxComputeWorkGroupCountY = 65535;
    default_build_in_resource.maxComputeWorkGroupCountZ = 65535;
    default_build_in_resource.maxComputeWorkGroupSizeX = 1024;
    default_build_in_resource.maxComputeWorkGroupSizeY = 1024;
    default_build_in_resource.maxComputeWorkGroupSizeZ = 64;
    default_build_in_resource.maxComputeUniformComponents = 1024;
    default_build_in_resource.maxComputeTextureImageUnits = 16;
    default_build_in_resource.maxComputeImageUniforms = 8;
    default_build_in_resource.maxComputeAtomicCounters = 8;
    default_build_in_resource.maxComputeAtomicCounterBuffers = 1;
    default_build_in_resource.maxVaryingComponents = 60;
    default_build_in_resource.maxVertexOutputComponents = 64;
    default_build_in_resource.maxGeometryInputComponents = 64;
    default_build_in_resource.maxGeometryOutputComponents = 128;
    default_build_in_resource.maxFragmentInputComponents = 128;
    default_build_in_resource.maxImageUnits = 8;
    default_build_in_resource.maxCombinedImageUnitsAndFragmentOutputs = 8;
    default_build_in_resource.maxCombinedShaderOutputResources = 8;
    default_build_in_resource.maxImageSamples = 0;
    default_build_in_resource.maxVertexImageUniforms = 0;
    default_build_in_resource.maxTessControlImageUniforms = 0;
    default_build_in_resource.maxTessEvaluationImageUniforms = 0;
    default_build_in_resource.maxGeometryImageUniforms = 0;
    default_build_in_resource.maxFragmentImageUniforms = 8;
    default_build_in_resource.maxCombinedImageUniforms = 8;
    default_build_in_resource.maxGeometryTextureImageUnits = 16;
    default_build_in_resource.maxGeometryOutputVertices = 256;
    default_build_in_resource.maxGeometryTotalOutputComponents = 1024;
    default_build_in_resource.maxGeometryUniformComponents = 1024;
    default_build_in_resource.maxGeometryVaryingComponents = 64;
    default_build_in_resource.maxTessControlInputComponents = 128;
    default_build_in_resource.maxTessControlOutputComponents = 128;
    default_build_in_resource.maxTessControlTextureImageUnits = 16;
    default_build_in_resource.maxTessControlUniformComponents = 1024;
    default_build_in_resource.maxTessControlTotalOutputComponents = 4096;
    default_build_in_resource.maxTessEvaluationInputComponents = 128;
    default_build_in_resource.maxTessEvaluationOutputComponents = 128;
    default_build_in_resource.maxTessEvaluationTextureImageUnits = 16;
    default_build_in_resource.maxTessEvaluationUniformComponents = 1024;
    default_build_in_resource.maxTessPatchComponents = 120;
    default_build_in_resource.maxPatchVertices = 32;
    default_build_in_resource.maxTessGenLevel = 64;
    default_build_in_resource.maxViewports = 16;
    default_build_in_resource.maxVertexAtomicCounters = 0;
    default_build_in_resource.maxTessControlAtomicCounters = 0;
    default_build_in_resource.maxTessEvaluationAtomicCounters = 0;
    default_build_in_resource.maxGeometryAtomicCounters = 0;
    default_build_in_resource.maxFragmentAtomicCounters = 8;
    default_build_in_resource.maxCombinedAtomicCounters = 8;
    default_build_in_resource.maxAtomicCounterBindings = 1;
    default_build_in_resource.maxVertexAtomicCounterBuffers = 0;
    default_build_in_resource.maxTessControlAtomicCounterBuffers = 0;
    default_build_in_resource.maxTessEvaluationAtomicCounterBuffers = 0;
    default_build_in_resource.maxGeometryAtomicCounterBuffers = 0;
    default_build_in_resource.maxFragmentAtomicCounterBuffers = 1;
    default_build_in_resource.maxCombinedAtomicCounterBuffers = 1;
    default_build_in_resource.maxAtomicCounterBufferSize = 16384;
    default_build_in_resource.maxTransformFeedbackBuffers = 4;
    default_build_in_resource.maxTransformFeedbackInterleavedComponents = 64;
    default_build_in_resource.maxCullDistances = 8;
    default_build_in_resource.maxCombinedClipAndCullDistances = 8;
    default_build_in_resource.maxSamples = 4;
    default_build_in_resource.maxMeshOutputVerticesNV = 256;
    default_build_in_resource.maxMeshOutputPrimitivesNV = 512;
    default_build_in_resource.maxMeshWorkGroupSizeX_NV = 32;
    default_build_in_resource.maxMeshWorkGroupSizeY_NV = 1;
    default_build_in_resource.maxMeshWorkGroupSizeZ_NV = 1;
    default_build_in_resource.maxTaskWorkGroupSizeX_NV = 32;
    default_build_in_resource.maxTaskWorkGroupSizeY_NV = 1;
    default_build_in_resource.maxTaskWorkGroupSizeZ_NV = 1;
    default_build_in_resource.maxMeshViewCountNV = 4;
    default_build_in_resource.limits.nonInductiveForLoops = 1;
    default_build_in_resource.limits.whileLoops = 1;
    default_build_in_resource.limits.doWhileLoops = 1;
    default_build_in_resource.limits.generalUniformIndexing = 1;
    default_build_in_resource.limits.generalAttributeMatrixVectorIndexing = 1;
    default_build_in_resource.limits.generalVaryingIndexing = 1;
    default_build_in_resource.limits.generalSamplerIndexing = 1;
    default_build_in_resource.limits.generalVariableIndexing = 1;
    default_build_in_resource.limits.generalConstantMatrixVectorIndexing = 1;
}

EShLanguage FindLanguage(const VkShaderStageFlagBits shader_type) 
{
    switch (shader_type) 
    {
        case VK_SHADER_STAGE_VERTEX_BIT:
            return EShLangVertex;

        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:
            return EShLangTessControl;

        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:
            return EShLangTessEvaluation;

        case VK_SHADER_STAGE_GEOMETRY_BIT:
            return EShLangGeometry;

        case VK_SHADER_STAGE_FRAGMENT_BIT:
            return EShLangFragment;

        case VK_SHADER_STAGE_COMPUTE_BIT:
            return EShLangCompute;

        case VK_SHADER_STAGE_TASK_BIT_NV:
            return EShLangTaskNV;

        case VK_SHADER_STAGE_MESH_BIT_NV:
            return EShLangMeshNV;

        default:
            return EShLangVertex;
    }
}

extern "C" 
{
#ifdef WIN32
    #define EXPORT_FUNC __declspec(dllexport)
#else
    #define EXPORT_FUNC 
#endif

    EXPORT_FUNC bool InitShaderCompiler()
    {
        init_default_build_in_resource();

        return glslang::InitializeProcess();
    }

    EXPORT_FUNC void CloseShaderCompiler()
    {
        glslang::FinalizeProcess();
    }

    struct SPVData
    {
        bool result;
        char *log;
        char *debug_log;

        uint32_t *spv_data;
        uint32_t spv_length;

    public:

        SPVData(const char *l,const char *dl)
        {
            result=false;

            log=new char[strlen(l)+1];
            strcpy(log,l);

            debug_log=new char[strlen(dl)+1];
            strcpy(debug_log,dl);

            spv_data=nullptr;
        }

        SPVData(const std::vector<uint32_t> &spirv)
        {
            result=true;

            log=nullptr;
            debug_log=nullptr;

            spv_length=spirv.size();
            spv_data=new uint32_t[spv_length];
            memcpy(spv_data,spirv.data(),spv_length*sizeof(uint32_t));
        }

        ~SPVData()
        {
            delete[] log;
            delete[] debug_log;
            delete[] spv_data;
        }
    };//struct SPVData

    EXPORT_FUNC void FreeSPVData(SPVData *spv)
    {
        delete spv;
    }

    EXPORT_FUNC SPVData *GLSL2SPV(const uint32_t shader_type,const char *shader_source)
    {
        EShLanguage stage = FindLanguage((VkShaderStageFlagBits)shader_type);

        glslang::TShader shader(stage);
        glslang::TProgram program;

        const char *shaderStrings[1];
        TBuiltInResource Resources=default_build_in_resource;

        // Enable SPIR-V and Vulkan rules when parsing GLSL
        EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

        shaderStrings[0] = shader_source;
        shader.setStrings(shaderStrings, 1);

        if (!shader.parse(&Resources, 100, false, messages))
            return(new SPVData(shader.getInfoLog(),shader.getInfoDebugLog()));

        program.addShader(&shader);

        //
        // Program-level processing...
        //

        if (!program.link(messages)) 
        {
            fflush(stdout);            
            return(new SPVData(shader.getInfoLog(),shader.getInfoDebugLog()));
        }

        std::vector<uint32_t> spirv;

        glslang::GlslangToSpv(*program.getIntermediate(stage),spirv);
        
        return(new SPVData(spirv));
    }
        
    EXPORT_FUNC uint32_t GetShaderStageFlagByExtName(const char *ext_name)
    {
        if (stricmp(ext_name,"vert") == 0)return VK_SHADER_STAGE_VERTEX_BIT; else
        if (stricmp(ext_name,"tesc") == 0)return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; else
        if (stricmp(ext_name,"tese") == 0)return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; else
        if (stricmp(ext_name,"geom") == 0)return VK_SHADER_STAGE_GEOMETRY_BIT; else
        if (stricmp(ext_name,"frag") == 0)return VK_SHADER_STAGE_FRAGMENT_BIT; else
        if (stricmp(ext_name,"comp") == 0)return VK_SHADER_STAGE_COMPUTE_BIT; else
        if (stricmp(ext_name,"task") == 0)return VK_SHADER_STAGE_TASK_BIT_NV; else
        if (stricmp(ext_name,"mesh") == 0)return VK_SHADER_STAGE_MESH_BIT_NV; else
        {
            return 0;
        }
    }
}//extern "C"