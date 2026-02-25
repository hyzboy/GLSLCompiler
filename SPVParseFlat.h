#pragma once

/*
 * SPVParseFlat.h
 *
 * Flat binary serialization format for SPVParseData.
 *
 * PURPOSE
 *   After SPIRV-Cross reflection is run once to produce SPVParseData, call
 *   FlattenSPVParseData() (defined in glsl2spv.cpp) to write the result into
 *   a single contiguous byte buffer.  That buffer can be persisted to disk and
 *   reloaded later with SPVParseFlatLoad() — no SPIRV-Cross (or any other
 *   third-party library) is required at load time.
 *
 * FLAT BINARY LAYOUT
 *
 *   [ SPVParseFlatHeader  ]   12 bytes
 *   [ input  section      ]   4 + count * sizeof(SPVPFAttribute)
 *   [ output section      ]   4 + count * sizeof(SPVPFAttribute)
 *   [ resource[0..10]     ]   (4 + count * sizeof(SPVPFDescriptor)) * 11
 *   [ push_constant       ]   4 + count * sizeof(SPVPFPushConstant)
 *   [ subpass_input       ]   4 + count * sizeof(SPVPFSubpassInput)
 *
 *   Every section begins with a uint32_t item count followed by that many
 *   fixed-size POD items.  No padding, no pointers, no variable-length fields.
 *
 * LOADER USAGE
 *
 *   SPVParseFlatView view;
 *   if (SPVParseFlatLoad(buffer, size, &view)) {
 *       // view.inputs[0..input_count-1], etc. point directly into buffer
 *   }
 *
 * DEPENDENCIES
 *   This header requires only <stdint.h> and <string.h>.
 */

#include <stdint.h>
#include <string.h>

/* -----------------------------------------------------------------------
 * Format constants
 * --------------------------------------------------------------------- */

#define SPVPF_MAGIC    0x44565053u   /* 'S','P','V','D' in little-endian */
#define SPVPF_VERSION  1u

#define SPVPF_NAME_MAX_LENGTH      32u
#define SPVPF_DESCRIPTOR_TYPE_COUNT 11u   /* mirrors VK_DESCRIPTOR_TYPE_COUNT */

/* -----------------------------------------------------------------------
 * POD structs embedded in the flat binary (fixed-size, no pointers)
 * Layout must stay byte-identical to the originals in glsl2spv.cpp.
 * --------------------------------------------------------------------- */

#pragma pack(push, 1)

typedef struct SPVPFAttribute {
    char    name[SPVPF_NAME_MAX_LENGTH];
    uint8_t location;
    uint8_t basetype;
    uint8_t vec_size;
} SPVPFAttribute;                          /* 35 bytes */

typedef struct SPVPFDescriptor {
    char    name[SPVPF_NAME_MAX_LENGTH];
    uint8_t set;
    uint8_t binding;
} SPVPFDescriptor;                         /* 34 bytes */

typedef struct SPVPFPushConstant {
    char    name[SPVPF_NAME_MAX_LENGTH];
    uint8_t offset;
    uint8_t size;
} SPVPFPushConstant;                       /* 34 bytes */

typedef struct SPVPFSubpassInput {
    char    name[SPVPF_NAME_MAX_LENGTH];
    uint8_t input_attachment_index;
    uint8_t binding;
} SPVPFSubpassInput;                       /* 34 bytes */

typedef struct SPVParseFlatHeader {
    uint32_t magic;       /* SPVPF_MAGIC   */
    uint32_t version;     /* SPVPF_VERSION */
    uint32_t total_size;  /* size of the entire buffer in bytes */
} SPVParseFlatHeader;                      /* 12 bytes */

#pragma pack(pop)

/* -----------------------------------------------------------------------
 * Zero-copy view into a flat binary buffer.
 * All pointers point directly into the caller-owned buffer; no allocation
 * is performed.  The buffer must remain valid as long as the view is used.
 * --------------------------------------------------------------------- */

typedef struct SPVParseFlatView {
    /* stage I/O */
    uint32_t              input_count;
    const SPVPFAttribute *inputs;

    uint32_t              output_count;
    const SPVPFAttribute *outputs;

    /* descriptors indexed by VkDescriptorType (0 = SAMPLER … 10 = INPUT_ATTACHMENT) */
    uint32_t               resource_count[SPVPF_DESCRIPTOR_TYPE_COUNT];
    const SPVPFDescriptor *resources[SPVPF_DESCRIPTOR_TYPE_COUNT];

    /* push constants */
    uint32_t                 push_constant_count;
    const SPVPFPushConstant *push_constants;

    /* subpass inputs */
    uint32_t               subpass_input_count;
    const SPVPFSubpassInput *subpass_inputs;
} SPVParseFlatView;

/* -----------------------------------------------------------------------
 * SPVParseFlatLoad
 *
 * Map a flat binary buffer to a SPVParseFlatView.
 * Returns 1 on success, 0 on failure (bad magic / version / truncated data).
 * --------------------------------------------------------------------- */

static inline int SPVParseFlatLoad(
    const void       *data,
    uint32_t          size,
    SPVParseFlatView *view)
{
    const uint8_t *p;
    const uint8_t *end;
    const SPVParseFlatHeader *hdr;
    uint32_t i;

    if (!data || !view || size < (uint32_t)sizeof(SPVParseFlatHeader))
        return 0;

    hdr = (const SPVParseFlatHeader *)data;
    if (hdr->magic   != SPVPF_MAGIC)   return 0;
    if (hdr->version != SPVPF_VERSION) return 0;
    if (hdr->total_size > size)        return 0;

    memset(view, 0, sizeof(SPVParseFlatView));

    p   = (const uint8_t *)data + sizeof(SPVParseFlatHeader);
    end = (const uint8_t *)data + hdr->total_size;

/* Read one uint32_t count then set the items pointer into the buffer. */
#define SPVPF_READ_SECTION(count_out, items_out, ItemType)           \
    do {                                                              \
        if (p + sizeof(uint32_t) > end) return 0;                   \
        (count_out) = *(const uint32_t *)(const void *)p;            \
        p += sizeof(uint32_t);                                        \
        (items_out) = (const ItemType *)(const void *)p;             \
        p += (count_out) * (uint32_t)sizeof(ItemType);               \
        if (p > end) return 0;                                        \
    } while (0)

    SPVPF_READ_SECTION(view->input_count,  view->inputs,  SPVPFAttribute);
    SPVPF_READ_SECTION(view->output_count, view->outputs, SPVPFAttribute);

    for (i = 0; i < SPVPF_DESCRIPTOR_TYPE_COUNT; i++) {
        SPVPF_READ_SECTION(view->resource_count[i], view->resources[i],
                           SPVPFDescriptor);
    }

    SPVPF_READ_SECTION(view->push_constant_count, view->push_constants,
                       SPVPFPushConstant);
    SPVPF_READ_SECTION(view->subpass_input_count, view->subpass_inputs,
                       SPVPFSubpassInput);

#undef SPVPF_READ_SECTION

    return 1;
}
