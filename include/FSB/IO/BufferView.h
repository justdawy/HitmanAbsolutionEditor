#pragma once
#include <cstddef>
#include <cstdint>
#include <stdexcept>
namespace FSB
{
    namespace IO
    {
        class BufferView
        {
        public:
            BufferView(const char* buffer, std::size_t length);
            BufferView(const char* begin, const char* end);
            const char* Begin() const;
            const char* End() const;
            const char* Current() const;
            bool Empty() const;
            std::size_t Size() const;
            std::size_t Remaining() const;
            std::size_t Offset() const;
            void SetOffset(std::size_t offset);
            void Skip(std::size_t length);
            const char* Read(std::size_t length);
            char ReadChar();
            unsigned char ReadUInt8();
            unsigned short ReadUInt16();
            unsigned int ReadUInt24();
            unsigned int ReadUInt32();
            unsigned long long ReadUInt64();
        private:
            const char* begin;
            const char* end;
            const char* current;
        };
    }
}