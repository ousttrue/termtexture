#pragma once
#include "spirv_util.h"

#ifdef SPIRV_UTIL_BUILD
#define SPIRV_UTIL_API extern "C" __declspec(dllexport)
#else
#define SPIRV_UTIL_API extern "C"
#endif

struct SpirvCompiler;
SPIRV_UTIL_API void SPIRV_Initialize();
SPIRV_UTIL_API void SPIRV_Finalize();

SPIRV_UTIL_API SpirvCompiler *SPIRV_COMPILER_Create_VS();
SPIRV_UTIL_API SpirvCompiler *SPIRV_COMPILER_Create_FS();
SPIRV_UTIL_API SpirvCompiler *SPIRV_COMPILER_Create_GS();
SPIRV_UTIL_API void SPIRV_COMPILER_Destroy(SpirvCompiler *context);
SPIRV_UTIL_API const unsigned int *
SPIRV_COMPILER_Compile(SpirvCompiler *context, const char *src,
                       unsigned int *out_size);
SPIRV_UTIL_API const char *SPIRV_COMPILER_GetError(SpirvCompiler *context);

// struct SpirvLinker;
// SPIRV_UTIL_API SpirvLinker *SPIRV_LINER_Create();
// SPIRV_UTIL_API void SPIRV_LINER_Destroy();
// SPIRV_UTIL_API int SPIRV_LINKER_Compile(SpirvCompiler *context, const char
// *src,
//                                   unsigned int *out_size);
