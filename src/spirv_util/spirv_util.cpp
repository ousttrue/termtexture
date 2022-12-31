#include "spirv_util.h"
#include <glslang/Public/ResourceLimits.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <vector>
// #include <fstream>
// #include <stdexcept>

// static std::vector<char> ReadAllBytes(char const *filename) {
//   std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
//   auto pos = ifs.tellg();
//   std::vector<char> buffer(pos);
//   ifs.seekg(0, std::ios::beg);
//   ifs.read(buffer.data(), pos);
//   return buffer;
// }
// static EShLanguage translateShaderStage(std::string_view filepath) {
//   if (filepath.ends_with("vert"))
//     return EShLangVertex;
//   else if (filepath.ends_with("frag"))
//     return EShLangFragment;
//   else if (filepath.ends_with("comp"))
//     return EShLangCompute;
//   else if (filepath.ends_with("rgen"))
//     return EShLangRayGenNV;
//   else if (filepath.ends_with("rmiss"))
//     return EShLangMissNV;
//   else if (filepath.ends_with("rchit"))
//     return EShLangClosestHitNV;
//   else if (filepath.ends_with("rahit"))
//     return EShLangAnyHitNV;
//   else
//     throw std::runtime_error("Unknown shader stage");
// }

struct SpirvCompiler {
  glslang::TShader shader_;
  std::vector<unsigned int> spirv_;

  SpirvCompiler(EShLanguage stage) : shader_(stage) {
    glslang::TShader shader(stage);
  }

  const unsigned int *compile(const char *src, unsigned int *out_size) {
    shader_.setStrings(&src, 1);

    shader_.setEnvClient(glslang::EShClientOpenGL,
                         glslang::EShTargetOpenGL_450);
    shader_.setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);

    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
    if (!shader_.parse(GetDefaultResources(), 100, false, messages)) {
      return nullptr;
    }
    glslang::GlslangToSpv(*shader_.getIntermediate(), spirv_);
    *out_size = spirv_.size();
    return spirv_.data();
  }

  const char *get_error() { return shader_.getInfoLog(); }
};

SPIRV_UTIL_API void SPIRV_Initialize() { glslang::InitializeProcess(); }
SPIRV_UTIL_API void SPIRV_Finalize() { glslang::FinalizeProcess(); }
SPIRV_UTIL_API SpirvCompiler *SPIRV_COMPILER_Create_VS() {
  return new SpirvCompiler(EShLangVertex);
}
SPIRV_UTIL_API SpirvCompiler *SPIRV_COMPILER_Create_FS() {
  return new SpirvCompiler(EShLangFragment);
}
SPIRV_UTIL_API void SPIRV_COMPILER_Destroy(SpirvCompiler *context) {
  delete context;
}
SPIRV_UTIL_API const unsigned int *
SPIRV_COMPILER_Compile(SpirvCompiler *context, const char *src,
                       unsigned int *out_size) {
  return context->compile(src, out_size);
}
SPIRV_UTIL_API const char *SPIRV_COMPILER_GetError(SpirvCompiler *context) {
  return context->get_error();
}
// struct SpirvLinker {
//   glslang::TProgram program_;

//   bool Link(SpirvCompiler *vs, SpirvCompiler *fs) {
//     program_.addShader(&vs->shader_);
//     program_.addShader(&fs->shader_);
//     EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
//     if (!program_.link(messages)) {
//       // throw std::runtime_error(shader.getInfoLog());
//       return false;
//     }

//     glslang::GlslangToSpv(*program_.getIntermediate(stage), spvShader);
//     glslang::FinalizeProcess();
//     return true;
//   }
// };

// SPIRV_UTIL_API SpirvLinker *SPIRV_LINER_Create() {}
// SPIRV_UTIL_API void SPIRV_LINER_Destroy();
// SPIRV_UTIL_API int SPIRV_LINKER_Compile(SpirvCompiler *context, const char
// *src,
//                                         unsigned int *out_size);
