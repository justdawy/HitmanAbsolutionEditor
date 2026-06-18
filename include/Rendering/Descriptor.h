#pragma once
#include <string>
#include "ImageLayout.h"
class Descriptor
{
public:
    enum class Type
    {
        Sampler,
        Texture,
        TextureStorage,
        ConstantBuffer,
        StructuredBuffer,
        Undefined
    };
    Descriptor() = default;
    Descriptor(const Descriptor& descriptor);
    Descriptor(const std::string& name, const Type type, const ImageLayout layout, const unsigned int slot, const unsigned int array_size, const unsigned int stage);
    bool IsStorage() const;
    bool IsArray() const;
    Type type = Type::Undefined;
    unsigned int slot = 0;
    unsigned int stage = 0;
    unsigned int arraySize = 0;
    unsigned int dynamicOffset = 0;
    uint64_t range = 0;
    unsigned int mip = 0;
    unsigned int mipRange = 0;
    void* data = nullptr;
    ImageLayout layout = ImageLayout::Undefined;
    std::string name;
};