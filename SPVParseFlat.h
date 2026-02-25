#pragma once

/*
 * SPVParseFlat.h  —  "Shader Flat File" format, version 2
 *
 * PURPOSE
 *   A single flat binary that bundles the raw SPIRV binary together with all
 *   SPIRV-Cross reflection metadata.  The file is produced offline (by the
 *   compiler tool that links against glslang + SPIRV-Cross) and loaded at
 *   engine start-up with zero additional dependencies.
 *
 * DESIGN GOALS
 *   • One malloc / file-read into a contiguous buffer — nothing more.
 *   • ShaderFlatLoad() sets up a zero-copy view with raw pointers into that
 *     buffer; the caller never needs to copy or allocate.
 *   • Pure C99 header: only <stdint.h> and <string.h> required.
 *
 * FLAT BINARY LAYOUT
 *
 *   [ ShaderFlatHeader  84 B ]
 *   [ SPIRV binary           ]   header.spv_size bytes  (SPIRV uint32 words)
 *   [ input  section         ]   uint32_t count  +  count × SFAttribute
 *   [ output section         ]   uint32_t count  +  count × SFAttribute
 *   [ resource[0..10]        ]   11 × (uint32_t count  +  count × SFDescriptor)
 *   [ push_constant section  ]   uint32_t count  +  count × SFPushConstant
 *   [ subpass_input section  ]   uint32_t count  +  count × SFSubpassInput
 *
 *   Every section starts with a uint32_t item count followed immediately by
 *   that many fixed-size POD items.  No pointers, no padding, no alignment
 *   gaps (all structs are declared with #pragma pack(1)).
 *
 * POD TYPE SIZES (pack=1)
 *   SFAttribute    36 B   name[32] + location + basetype + vec_size + col_count
 *   SFDescriptor   36 B   name[32] + set + binding + descriptor_type + array_size
 *   SFPushConstant 40 B   name[32] + offset(u32) + size(u32)
 *   SFSubpassInput 34 B   name[32] + input_attachment_index + binding
 *   ShaderFlatHeader 84 B magic + version + total_size + shader_stage +
 *                          spv_size + entrypoint[64]
 *
 * USAGE
 *
 *   // load file into buf/buf_size (one read, no secondary parse)
 *   ShaderFlatView view;
 *   if (ShaderFlatLoad(buf, buf_size, &view)) {
 *       // use view.spv_words[0..spv_word_count-1]  for vkCreateShaderModule
 *       // use view.inputs[0..input_count-1]         for vertex layout
 *       // use view.resources[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER][...]
 *       // etc.
 *   }
 */

#include <stdint.h>
#include <string.h>

/* -----------------------------------------------------------------------
 * Format constants
 * --------------------------------------------------------------------- */

/* magic: 'S','P','V','F' in little-endian = 0x46565053 */
#define SF_MAGIC    0x46565053u
#define SF_VERSION  2u

#define SF_NAME_MAX_LENGTH       32u
#define SF_ENTRYPOINT_MAX_LENGTH 64u
#define SF_DESCRIPTOR_TYPE_COUNT 11u   /* VK_DESCRIPTOR_TYPE_SAMPLER (0)
                                          .. VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT (10) */

/* -----------------------------------------------------------------------
 * VkDescriptorType symbolic constants (mirrors Vulkan spec, no vulkan.h needed)
 * --------------------------------------------------------------------- */
#define SF_DESCRIPTOR_TYPE_SAMPLER                0u
#define SF_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER 1u
#define SF_DESCRIPTOR_TYPE_SAMPLED_IMAGE          2u
#define SF_DESCRIPTOR_TYPE_STORAGE_IMAGE          3u
#define SF_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER   4u
#define SF_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER   5u
#define SF_DESCRIPTOR_TYPE_UNIFORM_BUFFER         6u
#define SF_DESCRIPTOR_TYPE_STORAGE_BUFFER         7u
#define SF_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC 8u
#define SF_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC 9u
#define SF_DESCRIPTOR_TYPE_INPUT_ATTACHMENT       10u

/* VertexAttribBaseType values (mirrors glsl2spv.cpp VertexAttribBaseType) */
#define SF_BASETYPE_BOOL   0u
#define SF_BASETYPE_INT    1u
#define SF_BASETYPE_UINT   2u
#define SF_BASETYPE_FLOAT  3u
#define SF_BASETYPE_DOUBLE 4u

/* -----------------------------------------------------------------------
 * Fixed-size POD types written into the flat binary.
 * MUST stay byte-identical to the corresponding internal structs in
 * glsl2spv.cpp; static_assert guards are placed there.
 * --------------------------------------------------------------------- */

#pragma pack(push, 1)

/* Vertex/varying I/O attribute (stage input or output) */
typedef struct SFAttribute {
    char    name[SF_NAME_MAX_LENGTH]; /* null-terminated resource name       */
    uint8_t location;                 /* SPIR-V Location decoration           */
    uint8_t basetype;                 /* SF_BASETYPE_* constant               */
    uint8_t vec_size;                 /* row count: 1=scalar, 2-4=vec/mat row */
    uint8_t col_count;                /* col count: 1=scalar/vec, >1=matrix   */
} SFAttribute;                        /* 36 bytes                             */

/* Descriptor-set resource (uniform buffer, sampler, storage image, etc.) */
typedef struct SFDescriptor {
    char    name[SF_NAME_MAX_LENGTH]; /* null-terminated resource name        */
    uint8_t set;                      /* descriptor set index                 */
    uint8_t binding;                  /* binding point                        */
    uint8_t descriptor_type;          /* SF_DESCRIPTOR_TYPE_* constant        */
    uint8_t array_size;               /* 0 = non-array; >0 = array dimension  */
} SFDescriptor;                       /* 36 bytes                             */

/* Push-constant block */
typedef struct SFPushConstant {
    char     name[SF_NAME_MAX_LENGTH]; /* null-terminated resource name       */
    uint32_t offset;                   /* byte offset in the push-constant blk*/
    uint32_t size;                     /* byte size of the block              */
} SFPushConstant;                      /* 40 bytes                            */

/* Subpass input attachment */
typedef struct SFSubpassInput {
    char    name[SF_NAME_MAX_LENGTH]; /* null-terminated resource name        */
    uint8_t input_attachment_index;   /* InputAttachmentIndex decoration      */
    uint8_t binding;                  /* binding point                        */
} SFSubpassInput;                     /* 34 bytes                             */

/* File header — always the first 84 bytes */
typedef struct ShaderFlatHeader {
    uint32_t magic;                          /* SF_MAGIC                      */
    uint32_t version;                        /* SF_VERSION                    */
    uint32_t total_size;                     /* entire buffer size in bytes   */
    uint32_t shader_stage;                   /* VkShaderStageFlagBits         */
    uint32_t spv_size;                       /* SPIRV binary size in bytes    */
    char     entrypoint[SF_ENTRYPOINT_MAX_LENGTH]; /* entry point name        */
} ShaderFlatHeader;                          /* 84 bytes                      */

#pragma pack(pop)

/* -----------------------------------------------------------------------
 * Zero-copy view produced by ShaderFlatLoad().
 * All pointers point directly into the caller-owned buffer.
 * The buffer must remain valid as long as the view is used.
 * --------------------------------------------------------------------- */

typedef struct ShaderFlatView {
    /* From the header */
    uint32_t         shader_stage;    /* VkShaderStageFlagBits               */
    const char      *entrypoint;      /* points into header.entrypoint[]     */

    /* Raw SPIRV binary — pass directly to vkCreateShaderModule */
    const uint32_t  *spv_words;       /* pointer to SPIRV uint32 words       */
    uint32_t         spv_word_count;  /* number of 32-bit words              */

    /* Stage inputs / outputs */
    uint32_t           input_count;
    const SFAttribute *inputs;

    uint32_t           output_count;
    const SFAttribute *outputs;

    /* Descriptor resources, indexed by SF_DESCRIPTOR_TYPE_* (0-10) */
    uint32_t            resource_count[SF_DESCRIPTOR_TYPE_COUNT];
    const SFDescriptor *resources[SF_DESCRIPTOR_TYPE_COUNT];

    /* Push-constant blocks */
    uint32_t             push_constant_count;
    const SFPushConstant *push_constants;

    /* Subpass inputs */
    uint32_t             subpass_input_count;
    const SFSubpassInput *subpass_inputs;
} ShaderFlatView;

/* -----------------------------------------------------------------------
 * ShaderFlatLoad
 *
 * Parse a flat shader binary buffer and populate a ShaderFlatView.
 * All view pointers point into [data, data+size); no allocation is done.
 *
 * Returns 1 on success, 0 on error (bad magic, wrong version, truncated).
 * --------------------------------------------------------------------- */

static inline int ShaderFlatLoad(
    const void    *data,
    uint32_t       size,
    ShaderFlatView *view)
{
    const uint8_t           *p;
    const uint8_t           *end;
    const ShaderFlatHeader  *hdr;
    uint32_t                 i;

    if (!data || !view || size < (uint32_t)sizeof(ShaderFlatHeader))
        return 0;

    hdr = (const ShaderFlatHeader *)(const void *)data;
    if (hdr->magic   != SF_MAGIC)   return 0;
    if (hdr->version != SF_VERSION) return 0;
    if (hdr->total_size > size)     return 0;

    memset(view, 0, sizeof(ShaderFlatView));

    /* Header-derived fields */
    view->shader_stage = hdr->shader_stage;
    view->entrypoint   = hdr->entrypoint;

    /* Advance past header */
    p   = (const uint8_t *)data + sizeof(ShaderFlatHeader);
    end = (const uint8_t *)data + hdr->total_size;

    /* SPIRV binary */
    if (hdr->spv_size == 0 || (hdr->spv_size & 3u) != 0) return 0; /* must be non-zero and 4-byte aligned */
    if (p + hdr->spv_size > end) return 0;
    view->spv_words      = (const uint32_t *)(const void *)p;
    view->spv_word_count = hdr->spv_size / 4u;
    p += hdr->spv_size;

/* Helper: read uint32_t count, then set items pointer. */
#define SF_READ_SECTION(count_out, items_out, ItemType)              \
    do {                                                              \
        if (p + sizeof(uint32_t) > end) return 0;                   \
        (count_out) = *(const uint32_t *)(const void *)p;            \
        p += sizeof(uint32_t);                                        \
        (items_out) = (const ItemType *)(const void *)p;             \
        p += (count_out) * (uint32_t)sizeof(ItemType);               \
        if (p > end) return 0;                                        \
    } while (0)

    SF_READ_SECTION(view->input_count,  view->inputs,  SFAttribute);
    SF_READ_SECTION(view->output_count, view->outputs, SFAttribute);

    for (i = 0; i < SF_DESCRIPTOR_TYPE_COUNT; i++)
        SF_READ_SECTION(view->resource_count[i], view->resources[i], SFDescriptor);

    SF_READ_SECTION(view->push_constant_count, view->push_constants, SFPushConstant);
    SF_READ_SECTION(view->subpass_input_count, view->subpass_inputs, SFSubpassInput);

#undef SF_READ_SECTION

    return 1;
}
