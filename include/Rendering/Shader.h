#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include "VertexType.h"
#include "Descriptor.h"
#include "InputLayout.h"
class Shader
{
public:
    enum Type : unsigned char
    {
        Unknown = 0,
        Vertex = 1,
        Pixel = 2,
        Compute = 4,
    };
    enum class CompilationState
    {
        Succeeded,
        Failed
    };
    Shader() = default;
    ~Shader();
    void* Compile();
    void Compile(const Type shaderType, const std::string& filePath, const VertexType vertexType = VertexType::Undefined);
    void CompileShader();
    CompilationState GetCompilationState() const;
    bool IsCompiled() const;
    void LoadSource(const std::string& filePath);
    const std::vector<std::string>& GetNames() const;
    const std::vector<std::string>& GetFilePaths() const;
    const std::vector<std::string>& GetSources() const;
    void SetSource(const unsigned int index, const std::string& source);
    void AddDefine(const std::string& define, const std::string& value = "1");
    auto& GetDefines() const;
    unsigned int GetVertexSize() const;
    const std::vector<Descriptor>& GetDescriptors() const;
    const std::shared_ptr<InputLayout>& GetInputLayout() const;
    const auto& GetFilePath() const;
    const char* GetEntryPoint() const;
    const char* GetTargetProfile() const;
    void* GetResource() const;
private:
    void PreprocessIncludeDirectives(const std::string& filePath);
    std::string filePath;
    std::string preprocessedSource;
    std::vector<std::string> names;
    std::vector<std::string> filePaths;
    std::vector<std::string> sources;
    std::vector<std::string> filePathsMultiple;
    std::unordered_map<std::string, std::string> defines;
    std::vector<Descriptor> descriptors;
    std::shared_ptr<InputLayout> inputLayout;
    CompilationState compilationState;
    Type shaderType;
    VertexType vertexType;
    void* resource;
};