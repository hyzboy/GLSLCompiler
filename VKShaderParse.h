#pragma once
#include"SPIRV-Cross/spirv_cross.hpp"
#include<string>

using SPVResVector=spirv_cross::SmallVector<spirv_cross::Resource>;

class ShaderParse
{
    spirv_cross::Compiler *compiler;
    spirv_cross::ShaderResources resource;

public:

    ShaderParse(const void *spv_data,const uint32_t spv_size)
    {
        compiler=new spirv_cross::Compiler((const uint32_t *)spv_data,spv_size/sizeof(uint32_t));

        resource=compiler->get_shader_resources();
    }

    ~ShaderParse()
    {
        delete compiler;
    }

#define SHADER_PARSE_GET_RESOURCE(name,buf_name)    const SPVResVector &Get##name()const{return resource.buf_name;}

    SHADER_PARSE_GET_RESOURCE(UBO,          uniform_buffers)
    SHADER_PARSE_GET_RESOURCE(SSBO,         storage_buffers)
    SHADER_PARSE_GET_RESOURCE(StageInputs,  stage_inputs)
    SHADER_PARSE_GET_RESOURCE(StageOutputs, stage_outputs)
    SHADER_PARSE_GET_RESOURCE(Sampler,      sampled_images)
    SHADER_PARSE_GET_RESOURCE(Subpass,      subpass_inputs)

    //SmallVector<Resource> storage_images;
    //SmallVector<Resource> atomic_counters;
    //SmallVector<Resource> acceleration_structures;
    //SmallVector<Resource> push_constant_buffers;
    //SmallVector<Resource> separate_images;
    //SmallVector<Resource> separate_samplers;

#undef SHADER_PARSE_GET_RESOURCE

public:

    const std::string &GetName(const spirv_cross::Resource &res)const
    {
        return compiler->get_name(res.id);
    }

    const uint32_t GetSet(const spirv_cross::Resource &res)const
    {
        return compiler->get_decoration(res.id,spv::DecorationDescriptorSet);
    }

    const uint32_t GetBinding(const spirv_cross::Resource &res)const
    {
        return compiler->get_decoration(res.id,spv::DecorationBinding);
    }

    const uint32_t GetLocation(const spirv_cross::Resource &res)const
    {
        return compiler->get_decoration(res.id,spv::DecorationLocation);
    }

    void GetFormat(const spirv_cross::Resource &res,spirv_cross::SPIRType::BaseType *base_type,uint8_t *vecsize)
    {
        const spirv_cross::SPIRType &type=compiler->get_type(res.type_id);

        *base_type  =type.basetype;
        *vecsize    =type.vecsize;
    }
};//class ShaderParse
