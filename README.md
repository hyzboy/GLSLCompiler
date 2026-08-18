# GLSLCompiler

一个跨平台的 GLSL/HLSL → SPIR-V 编译与反射库。  
基于 [glslang](https://github.com/KhronosGroup/glslang) 进行着色器编译，基于 [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) 进行着色器反射，并提供一套零依赖的统一二进制文件格式（Shader Flat File），可直接供引擎在运行时使用，无需二次解析。

---

## 目录

1. [功能概述](#功能概述)
2. [架构总览](#架构总览)
3. [构建说明](#构建说明)
   - [依赖关系](#依赖关系)
   - [CMake 选项](#cmake-选项)
   - [子模块方式（推荐）](#子模块方式推荐)
   - [系统 / Vulkan SDK / vcpkg 方式](#系统--vulkan-sdk--vcpkg-方式)
4. [C API 参考](#c-api-参考)
   - [生命周期管理](#生命周期管理)
   - [数据类型与常量](#数据类型与常量)
   - [编译函数](#编译函数)
   - [反射函数](#反射函数)
   - [统一 Flat 文件函数](#统一-flat-文件函数)
   - [工具函数](#工具函数)
   - [插件接口 GLSLCompilerInterface](#插件接口-glslcompilerinterface)
5. [Shader Flat File 格式](#shader-flat-file-格式)
   - [设计目标](#设计目标)
   - [二进制布局](#二进制布局)
   - [POD 类型详解](#pod-类型详解)
   - [零依赖加载器 ShaderFlatLoad](#零依赖加载器-shaderFlatload)
6. [完整使用示例](#完整使用示例)
   - [离线编译工具（需要 GLSLCompiler 库）](#离线编译工具需要-glslcompiler-库)
   - [引擎运行时加载（零依赖）](#引擎运行时加载零依赖)
7. [文件清单](#文件清单)
8. [支持的着色器阶段](#支持的着色器阶段)
9. [支持的描述符类型](#支持的描述符类型)
10. [注意事项与限制](#注意事项与限制)

---

## 功能概述

| 功能 | 说明 |
|------|------|
| GLSL → SPIR-V | 支持所有 Vulkan 着色器阶段，含光线追踪与网格着色器 |
| HLSL → SPIR-V | 通过 glslang HLSL 前端支持，需指定入口函数名 |
| `#include` 支持 | 通过 `DirStackFileIncluder` 实现嵌套 include 搜索 |
| Preamble 注入 | 支持在编译前注入宏定义等预处理文本 |
| 着色器反射 | 输入/输出属性、描述符、Push Constant、Subpass Input 全覆盖 |
| 统一二进制格式 | `CreateShaderFlat()` 将 SPIR-V + 反射数据打包成一个文件 |
| 零依赖加载 | `ShaderFlatLoad()`（纯 C99，仅需 `<stdint.h>` + `<string.h>`）|
| 跨平台 | Windows (MSVC/MinGW) / Linux / macOS / Android |
| 插件式 API | `GetInterface()` 导出函数指针表，可动态加载 |

---

## 架构总览

```
┌─────────────────────────────────────────────────────────────┐
│                      GLSLCompiler.dll/.so                   │
│                                                             │
│  ┌──────────────┐   ┌─────────────────┐   ┌─────────────┐  │
│  │  glsl2spv.cpp│   │  VKShaderParse.h│   │SPVParseFlat │  │
│  │              │   │                 │   │    .h       │  │
│  │  编译器前端   │──▶│  SPIRV-Cross    │──▶│  Flat 序列化│  │
│  │  (glslang)   │   │  反射封装       │   │  & 加载器   │  │
│  └──────────────┘   └─────────────────┘   └─────────────┘  │
│          │                                        │         │
│          ▼                                        ▼         │
│       SPVData                              ShaderFlatFile   │
│  (SPIR-V 二进制)                         (SPIR-V + 反射)   │
└─────────────────────────────────────────────────────────────┘
         ▲
         │ GetInterface() → GLSLCompilerInterface*
         │
┌────────┴───────────┐
│   调用方 / 引擎     │
│  (动态加载或静态链接)│
└─────────────────────┘
```

**两阶段工作流：**

```
[离线 / 构建阶段]                        [运行时]
GLSL/HLSL 源码
    │
    ▼ Shader2SPV() / CompileFromPath()
SPVData (SPIR-V 字节码)
    │
    ▼ ParseSPV()
SPVParseData (反射数据)
    │
    ▼ CreateShaderFlat()
.spvf 文件 ──────────────────────────▶ ShaderFlatLoad()
                                           │
                                           ▼
                                      ShaderFlatView
                                      (零拷贝视图)
```

---

## 构建说明

### 依赖关系

| 依赖 | 用途 | 获取方式 |
|------|------|----------|
| [glslang](https://github.com/KhronosGroup/glslang) | GLSL/HLSL 编译 | 子模块 / Vulkan SDK / vcpkg |
| [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross) | SPIR-V 反射 | 子模块 / vcpkg / apt / brew |
| [SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools) | SPIR-V 优化（可选） | 子模块 / Vulkan SDK / vcpkg |
| CMake ≥ 3.10 | 构建系统 | — |

> **注意**：标准 Vulkan SDK 中 **不包含** SPIRV-Cross。使用 SDK 方式时需单独安装：
> - Windows：`vcpkg install spirv-cross`
> - Linux：`apt install libspirv-cross-dev`
> - macOS：`brew install spirv-cross`

### CMake 选项

| 选项 | 默认 | 说明 |
|------|------|------|
| `USE_GLSLANG_SUBMODULE` | `ON` | `ON`：从 git 子模块编译；`OFF`：使用系统/SDK/vcpkg 预编译库 |
| `MSVC_USE_DLL` | `ON` | （仅 MSVC）使用动态运行时 (`/MD`) 还是静态运行时 (`/MT`) |
| `USE_CLANG` | `ON`（Apple）| 在非 Android 的 Unix 系统上强制使用 Clang |
| `USE_CHAR8_T` | `OFF` | 为非 Windows 平台启用 `-fchar8_t` 扩展 |

### 子模块方式（推荐）

```bash
git clone https://github.com/hyzboy/GLSLCompiler.git
cd GLSLCompiler
git submodule update --init --recursive   # 初始化 SPIRV-Cross 及其内部子模块

mkdir build && cd build
cmake .. -DUSE_GLSLANG_SUBMODULE=ON
cmake --build . --config Release
```

输出文件位于 `build/out/<Platform>_<Bits>_Release/`，例如：
- Windows 64 位：`out/Windows_64_Release/GLSLCompiler.dll`
- Linux 64 位：`out/Linux_64_Release/libGLSLCompiler.so`
- macOS 64 位：`out/Darwin_64_Release/libGLSLCompiler.dylib`

### 系统 / Vulkan SDK / vcpkg 方式

```bash
# 使用 Vulkan SDK（设置了 $VULKAN_SDK 环境变量）
cmake .. -DUSE_GLSLANG_SUBMODULE=OFF

# 使用 vcpkg
cmake .. -DUSE_GLSLANG_SUBMODULE=OFF \
         -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake

# 使用系统包（Linux，已安装 apt 包）
sudo apt install glslang-dev libspirv-cross-dev spirv-tools
cmake .. -DUSE_GLSLANG_SUBMODULE=OFF
```

CMake 的搜索优先级（先匹配先用）：
1. `$VULKAN_SDK` 环境变量指向的 SDK 目录
2. vcpkg 工具链（通过 `CMAKE_TOOLCHAIN_FILE`）
3. 系统包路径（`/usr`, `/usr/local`, Homebrew prefix 等）

---

## C API 参考

所有公开函数均以 `extern "C"` 导出，可从 C 或 C++ 调用。  
在 Windows 上通过 `__declspec(dllexport)` 导出 `GetInterface()`；  
在 Linux/macOS 上符号默认可见（ELF/Mach-O 默认导出）。

### 生命周期管理

#### `bool InitShaderCompiler()`

初始化 glslang 全局状态并设置默认硬件能力限制。  
**必须**在调用任何编译函数之前调用，且只能调用一次。

```c
bool ok = InitShaderCompiler();
assert(ok);
```

#### `void CloseShaderCompiler()`

释放 glslang 全局资源。程序退出前调用。

```c
CloseShaderCompiler();
```

---

### 数据类型与常量

#### `CompileInfo`

控制编译行为的参数结构体，传给 `Shader2SPV()` / `CompileFromPath()`。

```c
typedef struct CompileInfo {
    ShaderLanguageType  shader_type;     // GLSL=0, HLSL=1
    const char *        entrypoint;      // HLSL 入口函数名（GLSL 填 NULL）
    uint32_t            includes_count;  // include 搜索路径数量
    const char **       includes;        // include 搜索路径数组
    const char *        preamble;        // 注入到着色器最前面的文本（NULL 跳过）
    const uint32_t      vulkan_version;  // 目标 Vulkan 版本，如 glslang::EShTargetVulkan_1_2
    const uint32_t      spv_version;     // 目标 SPIR-V 版本，如 glslang::EShTargetSpv_1_5
} CompileInfo;
```

**`shader_type` 取值：**

| 值 | 含义 |
|----|------|
| `0` (`ShaderLanguageType::GLSL`) | GLSL 源码 |
| `1` (`ShaderLanguageType::HLSL`) | HLSL 源码 |

#### `SPVData`

保存编译结果或错误信息。

```c
struct SPVData {
    bool      result;      // true = 编译成功；false = 编译失败
    char     *log;         // 编译器信息日志（失败时含错误信息）
    char     *debug_log;   // 调试日志（通常与 log 相同）
    uint32_t *spv_data;    // SPIR-V 字节码（result=false 时为 NULL）
    uint32_t  spv_length;  // SPIR-V 字节数（不是字数）
};
```

使用完毕后**必须**调用 `FreeSPVData()` 释放。

#### `SPVParseData`

保存 SPIRV-Cross 反射结果。字段均为动态分配的数组。

```c
struct SPVParseData {
    ShaderStageIO      stage_io;           // 输入/输出属性
    ShaderDescriptorResource resource[11]; // 11 类描述符资源
    ShaderResourceData<PushConstant> push_constant;
    ShaderResourceData<SubpassInput> subpass_input;
};
```

使用完毕后**必须**调用 `FreeSPVParse()` 释放。

---

### 编译函数

#### `SPVData* Shader2SPV(uint32_t shader_stage, const char* shader_source, const CompileInfo* compile_info)`

从内存中的字符串编译着色器。

| 参数 | 说明 |
|------|------|
| `shader_stage` | `VkShaderStageFlagBits` 之一（见[支持的着色器阶段](#支持的着色器阶段)） |
| `shader_source` | 以 `\0` 结尾的 GLSL/HLSL 源码字符串 |
| `compile_info` | 编译参数，不能为 NULL |

返回 `SPVData*`，调用方拥有所有权，必须调用 `FreeSPVData()` 释放。  
`result==false` 时可从 `log` 字段获取错误信息。

```c
CompileInfo ci = {0};
ci.shader_type   = 0;            // GLSL
ci.entrypoint    = NULL;
ci.vulkan_version = 100;         // Vulkan 1.0
ci.spv_version    = 0x00010000;  // SPIR-V 1.0

SPVData *spv = Shader2SPV(VK_SHADER_STAGE_VERTEX_BIT, glsl_source, &ci);
if (!spv->result) {
    fprintf(stderr, "编译失败：%s\n", spv->log);
}
FreeSPVData(spv);
```

#### `SPVData* CompileFromPath(uint32_t shader_stage, const char* shader_path, const CompileInfo* compile_info)`

从文件路径读取并编译着色器。与 `Shader2SPV()` 等价，但自动读取文件。

```c
SPVData *spv = CompileFromPath(
    VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/pbr.frag", &ci);
```

#### `void FreeSPVData(SPVData* spv)`

释放 `Shader2SPV()` / `CompileFromPath()` 返回的对象。

---

### 反射函数

#### `SPVParseData* ParseSPV(SPVData* spv_data)`

对编译成功的 SPIR-V 进行反射，提取所有资源绑定信息。  
要求 `spv_data->result == true`。

```c
SPVParseData *parsed = ParseSPV(spv);
// 使用 parsed->stage_io.input / parsed->resource[...] 等
FreeSPVParse(parsed);
```

提取的信息包括：

| 字段 | 内容 |
|------|------|
| `stage_io.input` | 顶点/片元等阶段输入属性（含 location、类型、向量维度、矩阵列数） |
| `stage_io.output` | 阶段输出属性 |
| `resource[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER]` | UBO 列表 |
| `resource[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER]` | SSBO 列表 |
| `resource[VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER]` | 组合图像采样器 |
| `resource[VK_DESCRIPTOR_TYPE_SAMPLER]` | 独立采样器 |
| `resource[VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE]` | 独立纹理 |
| `resource[VK_DESCRIPTOR_TYPE_STORAGE_IMAGE]` | 存储图像 |
| `push_constant` | Push Constant 块（含 offset、精确字节大小） |
| `subpass_input` | Subpass Input Attachment |

#### `void FreeSPVParse(SPVParseData* spv)`

释放 `ParseSPV()` 返回的对象。

---

### 统一 Flat 文件函数

#### `uint8_t* CreateShaderFlat(uint32_t shader_stage, const char* entrypoint, const SPVData* spv, const SPVParseData* parsed, uint32_t* out_size)`

将 SPIR-V 二进制和反射数据**合并**成一个连续内存块（"Shader Flat File"）。

| 参数 | 说明 |
|------|------|
| `shader_stage` | `VkShaderStageFlagBits`，记录到文件头 |
| `entrypoint` | 入口函数名（最多 63 字符），GLSL 通常为 `"main"` |
| `spv` | `Shader2SPV()` 返回的编译结果，`result` 必须为 `true` |
| `parsed` | `ParseSPV()` 返回的反射结果 |
| `out_size` | 输出参数，接收返回缓冲区的字节大小 |

返回堆分配的字节缓冲区，调用方拥有所有权，用 `FreeShaderFlat()` 释放。  
失败（参数非法）时返回 `nullptr`。

```c
uint32_t flat_size;
uint8_t *flat = CreateShaderFlat(
    VK_SHADER_STAGE_VERTEX_BIT, "main", spv, parsed, &flat_size);

// 写入文件
FILE *f = fopen("shader.spvf", "wb");
fwrite(flat, 1, flat_size, f);
fclose(f);

FreeShaderFlat(flat);
```

#### `void FreeShaderFlat(uint8_t* flat_data)`

释放 `CreateShaderFlat()` 返回的缓冲区。

---

### 工具函数

#### `uint32_t GetShaderStageFlagByExtName(const char* ext_name)`

根据文件扩展名返回对应的 `VkShaderStageFlagBits`。大小写不敏感。  
找不到对应扩展名时返回 `0`。

| 扩展名 | 返回值 |
|--------|--------|
| `vert` | `VK_SHADER_STAGE_VERTEX_BIT` |
| `tesc` | `VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT` |
| `tese` | `VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT` |
| `geom` | `VK_SHADER_STAGE_GEOMETRY_BIT` |
| `frag` | `VK_SHADER_STAGE_FRAGMENT_BIT` |
| `comp` | `VK_SHADER_STAGE_COMPUTE_BIT` |
| `task` | `VK_SHADER_STAGE_TASK_BIT_NV` |
| `mesh` | `VK_SHADER_STAGE_MESH_BIT_NV` |
| `rgen` | `VK_SHADER_STAGE_RAYGEN_BIT_KHR` |
| `rahit` | `VK_SHADER_STAGE_ANY_HIT_BIT_KHR` |
| `rchit` | `VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR` |
| `rmiss` | `VK_SHADER_STAGE_MISS_BIT_KHR` |
| `rint` | `VK_SHADER_STAGE_INTERSECTION_BIT_KHR` |
| `rcall` | `VK_SHADER_STAGE_CALLABLE_BIT_KHR` |

```c
uint32_t stage = GetShaderStageFlagByExtName("frag"); // → 0x10
```

#### `bool GetLimit(TBuiltInResource* bir, int size)` / `bool SetLimit(TBuiltInResource* bir, int size)`

读取/设置编译器的硬件能力限制表（`TBuiltInResource`）。  
`size` 必须等于 `sizeof(TBuiltInResource)`，否则返回 `false`。

---

### 插件接口 GLSLCompilerInterface

`GetInterface()` 返回一个指向函数指针表的指针，适合动态加载场景（`dlopen` / `LoadLibrary`）。

```c
// 动态加载示例（Linux）
void *lib = dlopen("libGLSLCompiler.so", RTLD_NOW);
typedef GLSLCompilerInterface* (*GetInterface_fn)();
GetInterface_fn get_iface = (GetInterface_fn)dlsym(lib, "GetInterface");
GLSLCompilerInterface *iface = get_iface();

iface->Init();
SPVData *spv = iface->Compile(stage, source, &ci);
// ...
iface->Close();
```

函数指针表结构：

```c
struct GLSLCompilerInterface {
    bool        (*Init)();
    void        (*Close)();
    bool        (*GetLimit)(TBuiltInResource*, int);
    bool        (*SetLimit)(TBuiltInResource*, int);
    uint32_t    (*GetType)(const char *ext_name);
    SPVData*    (*Compile)(uint32_t stage, const char *source, const CompileInfo*);
    SPVData*    (*CompileFromPath)(uint32_t stage, const char *path, const CompileInfo*);
    void        (*Free)(SPVData*);
    SPVParseData* (*ParseSPV)(SPVData*);
    void        (*FreeParseSPVData)(SPVParseData*);
    uint8_t*    (*CreateShaderFlat)(uint32_t stage, const char *entrypoint,
                                     const SPVData*, const SPVParseData*, uint32_t *out_size);
    void        (*FreeShaderFlat)(uint8_t *flat_data);
};
```

---

## Shader Flat File 格式

头文件 `SPVParseFlat.h`，纯 C99，仅依赖 `<stdint.h>` 和 `<string.h>`。

### 设计目标

- **一次加载即可直接使用**：整个文件读入内存后，调用 `ShaderFlatLoad()` 只需设置指针，不做任何内存分配。
- **零依赖运行时**：引擎侧只包含 `SPVParseFlat.h`，无需 glslang、SPIRV-Cross 或 Vulkan 头文件。
- **自校验**：Magic + Version + total_size 三重检查，截断或损坏的文件可被可靠拒绝。
- **无填充**：所有结构体使用 `#pragma pack(1)`，布局与平台无关。

### 二进制布局

```
偏移    大小          内容
─────────────────────────────────────────────────────────────────
0       84 B          ShaderFlatHeader
                        magic       = 0x46565053 ('SPVF')
                        version     = 2
                        total_size  = 整个文件字节数
                        shader_stage = VkShaderStageFlagBits
                        spv_size    = SPIR-V 字节数
                        entrypoint  = 入口函数名（最多 63 字符）
─────────────────────────────────────────────────────────────────
84      spv_size      SPIR-V 二进制（uint32 字数组）
                      可直接传给 vkCreateShaderModule
─────────────────────────────────────────────────────────────────
+0      4 B           输入属性数量 (N)
+4      N × 36 B      N 个 SFAttribute（输入属性）
─────────────────────────────────────────────────────────────────
        4 B           输出属性数量 (M)
        M × 36 B      M 个 SFAttribute（输出属性）
─────────────────────────────────────────────────────────────────
×11     (4 + K×36) B  11 个描述符资源段（对应 VkDescriptorType 0-10）
                      每段：uint32_t count + count × SFDescriptor
─────────────────────────────────────────────────────────────────
        4 B           Push Constant 数量 (P)
        P × 40 B      P 个 SFPushConstant
─────────────────────────────────────────────────────────────────
        4 B           Subpass Input 数量 (S)
        S × 34 B      S 个 SFSubpassInput
─────────────────────────────────────────────────────────────────
```

### POD 类型详解

#### `SFAttribute`（36 字节）

顶点/varying 阶段 I/O 属性，对应 GLSL 中的 `in`/`out` 变量。

```c
typedef struct SFAttribute {
    char    name[32];     // 资源名称（null 结尾）
    uint8_t location;     // SPIR-V Location 装饰值
    uint8_t basetype;     // SF_BASETYPE_* 常量（见下表）
    uint8_t vec_size;     // 行数：1=标量/向量，2-4=矩阵行数
    uint8_t col_count;    // 列数：1=标量/向量，>1=矩阵列数
} SFAttribute;
```

**`basetype` 取值（`SF_BASETYPE_*`）：**

| 常量 | 值 | GLSL 类型示例 |
|------|----|---------------|
| `SF_BASETYPE_BOOL` | 0 | `bool` |
| `SF_BASETYPE_INT` | 1 | `int`, `ivec2`, `int64_t` |
| `SF_BASETYPE_UINT` | 2 | `uint`, `uvec3`, `uint64_t` |
| `SF_BASETYPE_FLOAT` | 3 | `float`, `vec4`, `mat4`, `half` |
| `SF_BASETYPE_DOUBLE` | 4 | `double`, `dvec3` |

**矩阵示例：**

| GLSL 类型 | `vec_size` | `col_count` |
|-----------|-----------|-------------|
| `float` | 1 | 1 |
| `vec4` | 4 | 1 |
| `mat4` | 4 | 4 |
| `mat3x4` | 4 | 3 |

#### `SFDescriptor`（36 字节）

描述符集资源（UBO、采样器、存储图像等）。

```c
typedef struct SFDescriptor {
    char    name[32];          // 资源名称（null 结尾）
    uint8_t set;               // 描述符集编号
    uint8_t binding;           // binding 点
    uint8_t descriptor_type;   // SF_DESCRIPTOR_TYPE_* 常量
    uint8_t array_size;        // 0=非数组；>0=数组第一维大小
} SFDescriptor;
```

**`descriptor_type` 取值（`SF_DESCRIPTOR_TYPE_*`）：**

| 常量 | 值 | 说明 |
|------|----|------|
| `SF_DESCRIPTOR_TYPE_SAMPLER` | 0 | 独立采样器 |
| `SF_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER` | 1 | 组合图像采样器 |
| `SF_DESCRIPTOR_TYPE_SAMPLED_IMAGE` | 2 | 独立采样纹理 |
| `SF_DESCRIPTOR_TYPE_STORAGE_IMAGE` | 3 | 存储图像 |
| `SF_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER` | 4 | Uniform Texel Buffer |
| `SF_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER` | 5 | Storage Texel Buffer |
| `SF_DESCRIPTOR_TYPE_UNIFORM_BUFFER` | 6 | Uniform Buffer（UBO） |
| `SF_DESCRIPTOR_TYPE_STORAGE_BUFFER` | 7 | Storage Buffer（SSBO） |
| `SF_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC` | 8 | 动态 UBO |
| `SF_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC` | 9 | 动态 SSBO |
| `SF_DESCRIPTOR_TYPE_INPUT_ATTACHMENT` | 10 | Subpass Input Attachment |

#### `SFPushConstant`（40 字节）

Push Constant 块信息。

```c
typedef struct SFPushConstant {
    char     name[32];   // 资源名称
    uint32_t offset;     // 在 push constant 区域中的字节偏移
    uint32_t size;       // 块的字节大小（可超过 255）
} SFPushConstant;
```

#### `SFSubpassInput`（34 字节）

Subpass Input Attachment（仅用于 tile-based 渲染的帧内传递）。

```c
typedef struct SFSubpassInput {
    char    name[32];
    uint8_t input_attachment_index;  // InputAttachmentIndex 装饰值
    uint8_t binding;                 // binding 点
} SFSubpassInput;
```

#### `ShaderFlatHeader`（84 字节）

文件头，固定位于文件最开始。

```c
typedef struct ShaderFlatHeader {
    uint32_t magic;          // 必须为 0x46565053u ('SPVF')
    uint32_t version;        // 当前为 2
    uint32_t total_size;     // 整个缓冲区字节数
    uint32_t shader_stage;   // VkShaderStageFlagBits
    uint32_t spv_size;       // SPIR-V 数据字节数（4 字节对齐）
    char     entrypoint[64]; // 入口函数名（null 结尾）
} ShaderFlatHeader;          // 20 + 64 = 84 字节
```

### 零依赖加载器 ShaderFlatLoad

```c
int ShaderFlatLoad(const void *data, uint32_t size, ShaderFlatView *view);
```

**参数：**
- `data`：指向文件内容的内存缓冲区
- `size`：缓冲区字节数（通常等于文件大小）
- `view`：输出参数，接收解析结果

**返回值：** 成功返回 `1`，以下情况返回 `0`：
- `data` / `view` 为 NULL
- 缓冲区小于 84 字节
- Magic 不匹配（0x46565053）
- Version 不等于 2
- `total_size > size`（文件被截断）
- SPIR-V 大小为 0 或不是 4 的倍数
- 任意 section 超出缓冲区边界

**`ShaderFlatView` 结构（无需释放，所有指针指向原始缓冲区）：**

```c
typedef struct ShaderFlatView {
    uint32_t         shader_stage;
    const char      *entrypoint;

    const uint32_t  *spv_words;       // SPIR-V 字数组
    uint32_t         spv_word_count;  // 字数（字节数 = word_count * 4）

    uint32_t           input_count;
    const SFAttribute *inputs;

    uint32_t           output_count;
    const SFAttribute *outputs;

    uint32_t            resource_count[11];
    const SFDescriptor *resources[11];

    uint32_t             push_constant_count;
    const SFPushConstant *push_constants;

    uint32_t              subpass_input_count;
    const SFSubpassInput *subpass_inputs;
} ShaderFlatView;
```

> **重要**：`ShaderFlatView` 内的所有指针直接指向传入的 `data` 缓冲区。  
> 必须保证缓冲区在 `view` 使用期间保持有效，**不要**在使用 `view` 之前释放缓冲区。

---

## 完整使用示例

### 离线编译工具（需要 GLSLCompiler 库）

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 假设通过动态加载方式使用
// 也可以静态链接后直接调用函数

// -- 初始化 --
if (!InitShaderCompiler()) {
    fprintf(stderr, "初始化失败\n");
    return -1;
}

// -- 编译参数 --
CompileInfo ci;
memset(&ci, 0, sizeof(ci));
ci.shader_type    = 0;       // GLSL
ci.entrypoint     = NULL;
ci.includes_count = 1;
const char *inc   = "shaders/include";
ci.includes       = &inc;
ci.preamble       = "#define ENABLE_SHADOWS 1\n";
ci.vulkan_version = 100;     // glslang::EShTargetVulkan_1_0
ci.spv_version    = 0x00010000; // SPIR-V 1.0

// -- 从文件编译顶点着色器 --
SPVData *spv = CompileFromPath(
    VK_SHADER_STAGE_VERTEX_BIT, "shaders/mesh.vert", &ci);

if (!spv || !spv->result) {
    fprintf(stderr, "编译错误：%s\n", spv ? spv->log : "未知错误");
    FreeSPVData(spv);
    CloseShaderCompiler();
    return -1;
}

// -- 反射 --
SPVParseData *parsed = ParseSPV(spv);

printf("顶点属性输入数：%u\n", parsed->stage_io.input.count);
for (uint32_t i = 0; i < parsed->stage_io.input.count; i++) {
    ShaderAttribute *a = &parsed->stage_io.input.items[i];
    printf("  [%u] %s  location=%u  basetype=%u  vec=%u  col=%u\n",
           i, a->name, a->location, a->basetype, a->vec_size, a->col_count);
}

uint32_t ubo_idx = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // = 6
printf("UBO 数量：%u\n", parsed->resource[ubo_idx].count);
for (uint32_t i = 0; i < parsed->resource[ubo_idx].count; i++) {
    Descriptor *d = &parsed->resource[ubo_idx].items[i];
    printf("  [%u] %s  set=%u  binding=%u  array=%u\n",
           i, d->name, d->set, d->binding, d->array_size);
}

// -- 打包成 Flat 文件 --
uint32_t flat_size;
uint8_t *flat = CreateShaderFlat(
    VK_SHADER_STAGE_VERTEX_BIT, "main", spv, parsed, &flat_size);

FILE *f = fopen("mesh.vert.spvf", "wb");
fwrite(flat, 1, flat_size, f);
fclose(f);

printf("已写出 mesh.vert.spvf  大小=%u 字节\n", flat_size);

// -- 释放 --
FreeShaderFlat(flat);
FreeSPVParse(parsed);
FreeSPVData(spv);
CloseShaderCompiler();
```

### 引擎运行时加载（零依赖）

```c
// 只需包含 SPVParseFlat.h，无需任何第三方库
#include "SPVParseFlat.h"
#include <stdio.h>
#include <stdlib.h>

// -- 读取文件 --
FILE *f = fopen("mesh.vert.spvf", "rb");
fseek(f, 0, SEEK_END);
long file_size = ftell(f);
fseek(f, 0, SEEK_SET);

uint8_t *buf = (uint8_t *)malloc(file_size);
fread(buf, 1, file_size, f);
fclose(f);

// -- 零拷贝加载（不做任何分配） --
ShaderFlatView view;
if (!ShaderFlatLoad(buf, (uint32_t)file_size, &view)) {
    fprintf(stderr, "加载失败（格式错误或文件损坏）\n");
    free(buf);
    return -1;
}

// -- 直接创建 VkShaderModule --
VkShaderModuleCreateInfo smci = {0};
smci.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
smci.codeSize = view.spv_word_count * sizeof(uint32_t);
smci.pCode    = view.spv_words;

VkShaderModule shader_module;
vkCreateShaderModule(device, &smci, NULL, &shader_module);

// -- 构建顶点布局 --
printf("着色器阶段：0x%X  入口：%s\n", view.shader_stage, view.entrypoint);
for (uint32_t i = 0; i < view.input_count; i++) {
    const SFAttribute *a = &view.inputs[i];
    // 根据 a->location / a->basetype / a->vec_size / a->col_count
    // 填写 VkVertexInputAttributeDescription
}

// -- 绑定 UBO --
for (uint32_t i = 0; i < view.resource_count[SF_DESCRIPTOR_TYPE_UNIFORM_BUFFER]; i++) {
    const SFDescriptor *d = &view.resources[SF_DESCRIPTOR_TYPE_UNIFORM_BUFFER][i];
    printf("UBO: %s  set=%u  binding=%u\n", d->name, d->set, d->binding);
    // 创建对应的 VkDescriptorSetLayoutBinding
}

// -- 处理 Push Constant --
for (uint32_t i = 0; i < view.push_constant_count; i++) {
    const SFPushConstant *pc = &view.push_constants[i];
    printf("PushConst: %s  offset=%u  size=%u\n", pc->name, pc->offset, pc->size);
}

// buf 在 view 使用期间必须保持有效
// 不再需要时 free(buf)
free(buf);
```

---

## 文件清单

| 文件 | 说明 |
|------|------|
| `glsl2spv.cpp` | 编译器与反射的核心实现，导出所有 C API 函数 |
| `VKShaderParse.h` | 对 SPIRV-Cross `spirv_cross::Compiler` 的薄封装，供 `glsl2spv.cpp` 内部使用 |
| `SPVParseFlat.h` | Shader Flat File 格式定义 + 零依赖加载器（C99，可单独分发给引擎） |
| `DirStackFileIncluder.h` | glslang 的 `#include` 解析器（来自 glslang 示例代码） |
| `CMakeLists.txt` | 构建脚本，同时构建动态库 `GLSLCompiler` 和静态库 `GLSLCompilerStatic` |
| `compiler.cmake` | 跨平台编译器选项（MSVC / MinGW / Clang / GCC） |
| `system_bit.cmake` | 检测目标平台位宽（32/64），定义 `HGL_32_BITS` / `HGL_64_BITS` |
| `output_path.cmake` | 统一输出目录规则 (`out/<Platform>_<Bits>_<Config>/`) |
| `FindVulkan.cmake` | 自定义 Vulkan SDK 查找脚本（补充 CMake 内置脚本的不足） |
| `SPIRV-Cross/` | git 子模块，包含 glslang + SPIRV-Tools + SPIRV-Cross |

---

## 支持的着色器阶段

| `VkShaderStageFlagBits` 值 | 文件扩展名 | 说明 |
|---------------------------|-----------|------|
| `0x00000001` | `.vert` | 顶点着色器 |
| `0x00000002` | `.tesc` | 细分控制着色器 |
| `0x00000004` | `.tese` | 细分求值着色器 |
| `0x00000008` | `.geom` | 几何着色器 |
| `0x00000010` | `.frag` | 片元着色器 |
| `0x00000020` | `.comp` | 计算着色器 |
| `0x00000040` | `.task` | 任务着色器（NV_mesh_shader） |
| `0x00000080` | `.mesh` | 网格着色器（NV_mesh_shader） |
| `0x00000100` | `.rgen` | 光线生成着色器（KHR/NV） |
| `0x00000200` | `.rahit` | 任意命中着色器 |
| `0x00000400` | `.rchit` | 最近命中着色器 |
| `0x00000800` | `.rmiss` | 未命中着色器 |
| `0x00001000` | `.rint` | 交叉着色器 |
| `0x00002000` | `.rcall` | 可调用着色器 |

---

## 支持的描述符类型

反射结果中 `resource` 数组仅填充以下 6 种（其余 5 种预留位置，count 保持 0）：

| 索引 | 类型 | GLSL 示例 |
|------|------|----------|
| 0 | `SAMPLER` | `layout(set=0, binding=0) uniform sampler s;` |
| 1 | `COMBINED_IMAGE_SAMPLER` | `layout(set=0, binding=1) uniform sampler2D tex;` |
| 2 | `SAMPLED_IMAGE` | `layout(set=0, binding=2) uniform texture2D t;` |
| 3 | `STORAGE_IMAGE` | `layout(set=0, binding=3, rgba8) uniform image2D img;` |
| 6 | `UNIFORM_BUFFER` | `layout(set=0, binding=4) uniform UBO { ... };` |
| 7 | `STORAGE_BUFFER` | `layout(set=0, binding=5) buffer SSBO { ... };` |

---

## 注意事项与限制

1. **线程安全**：`InitShaderCompiler()` / `CloseShaderCompiler()` 是全局操作，不能并发调用。编译函数本身（`Shader2SPV` 等）可在多线程中并发使用，但需保证 `InitShaderCompiler` 已完成。

2. **名称长度**：所有资源名称（属性名、UBO 名等）截断为 31 个有效字符（固定字段 32 字节含 null 终止符）。

3. **数组大小**：`SFDescriptor.array_size` 和 `array_size` 字段均为 `uint8_t`，最大值为 255。超大数组需自行处理。

4. **Subpass Input**：`SFSubpassInput` 结构存储 `input_attachment_index` 和 `binding`，均为 `uint8_t`，最大值 255，符合 Vulkan 规范限制。

5. **HLSL 编译**：使用 HLSL 时必须在 `CompileInfo.entrypoint` 中指定入口函数名，并将 `shader_type` 设为 `ShaderLanguageType::HLSL`（值为 1）。

6. **文件格式版本**：当前 `SF_VERSION = 2`。`ShaderFlatLoad()` 会严格校验版本号，旧版（v1）文件不被接受。

7. **字节序**：文件格式使用本机字节序写入。在大端与小端平台之间交换文件时需额外处理（当前主流平台均为小端，一般无需关注）。

8. **SPIR-V 大小对齐**：`ShaderFlatLoad()` 要求 `spv_size` 不为 0 且必须是 4 的倍数，否则拒绝加载。由 `CreateShaderFlat()` 生成的文件始终满足此要求。
