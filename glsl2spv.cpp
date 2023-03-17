#include<glslang/SPIRV/GlslangToSpv.h>
#include<glslang/Include/ResourceLimits.h>
#include"SPIRV-Cross/spirv_common.hpp"
#include"DirStackFileIncluder.h"
#include"VKShaderParse.h"
#include<vector>
#include<iostream>
#include<fstream>
#include<string>

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

typedef enum VkDescriptorType {
    VK_DESCRIPTOR_TYPE_SAMPLER = 0,
    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER = 1,
    VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2,
    VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3,
    VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER = 4,
    VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER = 5,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7,
    VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC = 8,
    VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC = 9,
    VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10,
} VkDescriptorType;

constexpr uint32_t VK_DESCRIPTOR_TYPE_COUNT =VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
                                            -VK_DESCRIPTOR_TYPE_SAMPLER+1;

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
        case VK_SHADER_STAGE_VERTEX_BIT:                    return EShLangVertex;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT:      return EShLangTessControl;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT:   return EShLangTessEvaluation;
        case VK_SHADER_STAGE_GEOMETRY_BIT:                  return EShLangGeometry;
        case VK_SHADER_STAGE_FRAGMENT_BIT:                  return EShLangFragment;

        case VK_SHADER_STAGE_COMPUTE_BIT:                   return EShLangCompute;

        case VK_SHADER_STAGE_TASK_BIT_NV:                   return EShLangTaskNV;
        case VK_SHADER_STAGE_MESH_BIT_NV:                   return EShLangMeshNV;

        case VK_SHADER_STAGE_RAYGEN_BIT_NV:                 return EShLangRayGenNV;
        case VK_SHADER_STAGE_ANY_HIT_BIT_NV:                return EShLangAnyHitNV;
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV:            return EShLangClosestHitNV;
        case VK_SHADER_STAGE_MISS_BIT_NV:                   return EShLangMissNV;
        case VK_SHADER_STAGE_INTERSECTION_BIT_NV:           return EShLangIntersectNV;
        case VK_SHADER_STAGE_CALLABLE_BIT_NV:               return EShLangCallableNV;

        default:                                            return EShLangVertex;
    }
}

enum class ShaderLanguageType
{
    GLSL=0,
    HLSL,

    MAX=0xff
};//enum class ShaderType

struct CompileInfo
{
    ShaderLanguageType  shader_type;
    const char *        entrypoint;
    uint32_t            includes_count;
    const char **       includes;
    const char *        preamble;
};

enum class VertexAttribBaseType
{
    Bool=0,
    Int,
    UInt,
    Float,
    Double,

    MAX=0xff
};//enum class VertexAttribBaseType

VertexAttribBaseType FromSPIRType(const spirv_cross::SPIRType::BaseType type)
{
    if(type==spirv_cross::SPIRType::BaseType::Boolean)  return VertexAttribBaseType::Bool;
    if(type==spirv_cross::SPIRType::BaseType::SByte
     ||type==spirv_cross::SPIRType::BaseType::Short
     ||type==spirv_cross::SPIRType::BaseType::Int
     ||type==spirv_cross::SPIRType::BaseType::Int64)    return VertexAttribBaseType::Int;
    if(type==spirv_cross::SPIRType::BaseType::UByte
     ||type==spirv_cross::SPIRType::BaseType::UShort
     ||type==spirv_cross::SPIRType::BaseType::UInt
     ||type==spirv_cross::SPIRType::BaseType::UInt64)   return VertexAttribBaseType::UInt;
    if(type==spirv_cross::SPIRType::BaseType::Half
     ||type==spirv_cross::SPIRType::BaseType::Float)    return VertexAttribBaseType::Float;
    if(type==spirv_cross::SPIRType::BaseType::Double)   return VertexAttribBaseType::Double;

    return VertexAttribBaseType::MAX;
}
    
char *new_strcpy(const char *src)
{
    size_t len=1+strlen(src);

    char *str=new char[len];
            
    memcpy(str,src,len);

    return str;
}

constexpr size_t SHADER_RESOURCE_NAME_MAX_LENGTH=32;

struct ShaderAttribute
{
    char name[SHADER_RESOURCE_NAME_MAX_LENGTH];
    uint8_t location;
    uint8_t basetype;
    uint8_t vec_size;
};//

struct ShaderAttributeData
{
    uint32_t count;
    ShaderAttribute *items;
};

struct Descriptor
{
    char name[SHADER_RESOURCE_NAME_MAX_LENGTH];
    uint8_t set;
    uint8_t binding;
};

struct PushConstant
{
    char name[SHADER_RESOURCE_NAME_MAX_LENGTH];
    uint8_t offset;
    uint8_t size;
};

struct SubpassInput
{
    char name[SHADER_RESOURCE_NAME_MAX_LENGTH];
    uint8_t input_attachment_index;
    uint8_t binding;
};

template<typename T>
struct ShaderResourceData
{
    uint32_t count;
    T *items;
};

using ShaderDescriptorResource=ShaderResourceData<Descriptor>[VK_DESCRIPTOR_TYPE_COUNT];

struct SPVData
{
    bool result;
    char *log;
    char *debug_log;

    uint32_t *spv_data;
    uint32_t spv_length;

    ShaderAttributeData                 input,
                                        output;
    ShaderDescriptorResource            resource;
    ShaderResourceData<PushConstant>    push_constant;
    ShaderResourceData<SubpassInput>    subpass_input;

    void Init()
    {
        memset(this,0,sizeof(SPVData));
    }

public:

    SPVData(const char *l,const char *dl)
    {
        Init();

        result=false;

        log=new_strcpy(l);
        debug_log=new_strcpy(dl);

        spv_data=nullptr;
    }

    SPVData(const char* log) : SPVData(log, log)
    {
    }

    SPVData(const std::vector<uint32_t> &spirv)
    {
        Init();

        result=true;

        log=nullptr;
        debug_log=nullptr;

        spv_length=(uint32_t)spirv.size();
        spv_data=new uint32_t[spv_length];
        spv_length*=sizeof(uint32_t);
        memcpy(spv_data,spirv.data(),spv_length);
    }

    ~SPVData()
    {   
        for(uint32_t i=0;i<VK_DESCRIPTOR_TYPE_COUNT;i++)
            delete[] resource[i].items;

        delete[] push_constant.items;
        delete[] subpass_input.items;

        delete[] input.items;
        delete[] output.items;

        delete[] log;
        delete[] debug_log;
        delete[] spv_data;
    }
};//struct SPVData

void OutputShaderAttributes(ShaderAttributeData *ssd,ShaderParse *sp,const SPVResVector &stages)
{
    size_t attr_count=stages.size();

    ssd->count=(uint32_t)attr_count;

    if(attr_count<=0)return;

    spirv_cross::SPIRType::BaseType base_type;
    uint8_t vec_size;
    std::string name;

    ssd->items=new ShaderAttribute[attr_count];
    ShaderAttribute *ss=ssd->items;

    for(const spirv_cross::Resource &si:stages)
    {
        sp->GetFormat(si,&base_type,&vec_size);

        ss->basetype   =(uint8_t)FromSPIRType(base_type);
        ss->vec_size    =vec_size;
        ss->location    =sp->GetLocation(si);

        strcpy(ss->name,sp->GetName(si).c_str());

        ++ss;
    }
}

void OutputShaderResource(ShaderResourceData<Descriptor> *ssd,ShaderParse *sp,const SPVResVector &res)
{
    size_t count=res.size();

    if(count<=0)return;

    ssd->count=(uint32_t)count;
    ssd->items=new Descriptor[count];
    Descriptor *sr=ssd->items;

    for(const spirv_cross::Resource &obj:res)
    {
        strcpy(sr->name,sp->GetName(obj).c_str());
        sr->set = sp->GetDescriptorSet(obj);
        sr->binding=sp->GetBinding(obj);       

        ++sr;
    }
}

void OutputPushConstant(ShaderResourceData<PushConstant> *ssd, ShaderParse* sp, const SPVResVector& res)
{
    size_t count = res.size();

    if (count <= 0)return;

    ssd->count = (uint32_t)count;
    ssd->items = new PushConstant[count];
    PushConstant *sr=ssd->items;

    for (const spirv_cross::Resource& obj : res)
    {
        strcpy(sr->name, sp->GetName(obj).c_str());
        sr->offset = sp->GetOffset(obj);
        sr->size = sp->GetBufferSize(obj);

        ++sr;
    }
}

void OutputSubpassInput(ShaderResourceData<SubpassInput> *ssd, ShaderParse* sp, const SPVResVector& res)
{
    size_t count = res.size();

    if (count <= 0)return;

    ssd->count = (uint32_t)count;
    ssd->items = new SubpassInput[count];
    SubpassInput *sr = ssd->items;

    for (const spirv_cross::Resource& obj : res)
    {
        strcpy(sr->name, sp->GetName(obj).c_str());
        sr->input_attachment_index = sp->GetInputAttachmentIndex(obj);
        sr->binding=sp->GetBinding(obj);

        ++sr;
    }
}

extern "C" 
{
    bool InitShaderCompiler()
    {
        init_default_build_in_resource();

        return glslang::InitializeProcess();
    }

    void CloseShaderCompiler()
    {
        glslang::FinalizeProcess();
    }

    bool GetLimit(TBuiltInResource *bir,const int size)
    {
        if(!bir)return(false);
        if(size!=sizeof(TBuiltInResource))return(false);

        memcpy(bir,&default_build_in_resource,size);
        return(true);
    }

    bool SetLimit(TBuiltInResource *bir,const int size)
    {
        if(!bir)return(false);
        if(size!=sizeof(TBuiltInResource))return(false);

        memcpy(&default_build_in_resource,bir,size);
        return(true);
    }

    void FreeSPVData(SPVData *spv)
    {
        delete spv;
    }
    
    SPVData *Shader2SPV(
        const uint32_t      shader_stage,
        const char *        shader_source,
        const CompileInfo * compile_info)
    {
        EShLanguage stage = FindLanguage((VkShaderStageFlagBits)shader_stage);

        glslang::EShSource      source = glslang::EShSourceGlsl;

        glslang::TShader        shader(stage);
        glslang::TProgram       program;
        DirStackFileIncluder    includer;

        const char *shaderStrings[1];
        TBuiltInResource Resources=default_build_in_resource;

        // Enable SPIR-V and Vulkan rules when parsing GLSL/HLSL
        EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
        
        if (compile_info != nullptr)
        {
            if (compile_info->shader_type == ShaderLanguageType::HLSL)
            {
                messages = (EShMessages)(EShMsgReadHlsl | EShMsgSpvRules | EShMsgVulkanRules);

                // HLSL Request!
                shader.setEntryPoint(compile_info->entrypoint);

                source = glslang::EShSourceHlsl;
            }

            for (uint32_t i = 0; i < compile_info->includes_count; i++)
            {
                includer.pushExternalLocalDirectory(compile_info->includes[i]);
            }
        }

        if(compile_info->preamble)
        shader.setPreamble(compile_info->preamble);

        shaderStrings[0] = shader_source;
        shader.setStrings(shaderStrings, 1);

//        shader.setEnvInput(source,stage,glslang::EShClientVulkan,);
//        shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
//        shader.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

        if (!shader.parse(&Resources, 
                          110,          // use 100 for ES environment, 110 for desktop; this is the GLSL version, not SPIR-V or Vulkan
                          false, 
                          messages, 
                          includer))
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

        SPVData *spv=new SPVData(spirv);
        
        {
            ShaderParse sp(spirv.data(),(uint32_t)spirv.size()*sizeof(uint32_t));
            
            OutputShaderAttributes(&(spv->input),&sp,sp.GetStageInputs());
            OutputShaderAttributes(&(spv->output),&sp,sp.GetStageOutputs());

            OutputShaderResource(spv->resource+VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,           &sp,sp.GetUBO());
            OutputShaderResource(spv->resource+VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,           &sp,sp.GetSSBO());
            OutputShaderResource(spv->resource+VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,   &sp,sp.GetSampledImages());
            OutputShaderResource(spv->resource+VK_DESCRIPTOR_TYPE_SAMPLER,                  &sp,sp.GetSeparateSamplers());
            OutputShaderResource(spv->resource+VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,            &sp,sp.GetSeparateImages());
            OutputShaderResource(spv->resource+VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,            &sp,sp.GetStorageImages());

            OutputPushConstant  (&(spv->push_constant),                                     &sp,sp.GetPushConstant());
            OutputSubpassInput  (&(spv->subpass_input),                                     &sp,sp.GetSubpassInputs());
        }

        return(spv);
    }

    SPVData *CompileFromPath(
        const uint32_t      shader_stage,
        const char *        shader_path,
        const CompileInfo * compile_info)
    {
        std::ifstream is(shader_path, std::ios::binary | std::ios::ate); // std::ios::binary is REQUEST!
        SPVData *data;

        if (is.is_open())
        {
            size_t size = is.tellg();
            if (size == -1)
                return new SPVData("Error: std::istream::tellg return -1!");

            is.seekg(0, std::ios::beg);
            char* shaderCode = new char[size + 1];
            is.read(shaderCode, size);
            is.close();

            shaderCode[size] = 0; // End of char* string.

            data = Shader2SPV(shader_stage, shaderCode, compile_info);

            delete[] shaderCode;

            return data;
        }
        else return new SPVData(("Error: Could not open shader file \"" + std::string(shader_path) + "\"!").data());
    }  
        
    uint32_t GetShaderStageFlagByExtName(const char *ext_name)
    {
        if (_stricmp(ext_name,"vert") == 0)return VK_SHADER_STAGE_VERTEX_BIT; else
        if (_stricmp(ext_name,"tesc") == 0)return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT; else
        if (_stricmp(ext_name,"tese") == 0)return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT; else
        if (_stricmp(ext_name,"geom") == 0)return VK_SHADER_STAGE_GEOMETRY_BIT; else
        if (_stricmp(ext_name,"frag") == 0)return VK_SHADER_STAGE_FRAGMENT_BIT; else

        if (_stricmp(ext_name,"comp") == 0)return VK_SHADER_STAGE_COMPUTE_BIT; else

        if (_stricmp(ext_name,"task") == 0)return VK_SHADER_STAGE_TASK_BIT_NV; else
        if (_stricmp(ext_name,"mesh") == 0)return VK_SHADER_STAGE_MESH_BIT_NV; else

        if (_stricmp(ext_name,"rgen") == 0)return VK_SHADER_STAGE_RAYGEN_BIT_KHR; else
        if (_stricmp(ext_name,"rahit") == 0)return VK_SHADER_STAGE_ANY_HIT_BIT_KHR; else
        if (_stricmp(ext_name,"rchit") == 0)return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR; else
        if (_stricmp(ext_name,"rmiss") == 0)return VK_SHADER_STAGE_MISS_BIT_KHR; else
        if (_stricmp(ext_name,"rint") == 0)return VK_SHADER_STAGE_INTERSECTION_BIT_KHR; else
        if (_stricmp(ext_name,"rcall") == 0)return VK_SHADER_STAGE_CALLABLE_BIT_KHR; else
        {
            return 0;
        }
    }
        
    struct GLSLCompilerInterface
    {
        bool        (*Init)();
        void        (*Close)();

        bool        (*GetLimit)(TBuiltInResource *,const int);
        bool        (*SetLimit)(TBuiltInResource *,const int);

        uint32_t    (*GetType)(const char *ext_name);
        SPVData *   (*Compile)(const uint32_t stage,const char *shader_source, const CompileInfo *compile_info);
        SPVData *   (*CompileFromPath)(const uint32_t stage,const char *shader_filename, const CompileInfo *compile_info);

        void        (*Free)(SPVData *);
    };

    static GLSLCompilerInterface plug_in_interface
    {
        &InitShaderCompiler,
        &CloseShaderCompiler,
        &GetLimit,
        &SetLimit,
        &GetShaderStageFlagByExtName,
        &Shader2SPV,
        &CompileFromPath,
        &FreeSPVData
    };
    
#ifdef WIN32
    #define EXPORT_FUNC __declspec(dllexport)
#else
    #define EXPORT_FUNC 
#endif

    EXPORT_FUNC GLSLCompilerInterface *GetInterface()
    {
        return &plug_in_interface;
    }
}//extern "C"