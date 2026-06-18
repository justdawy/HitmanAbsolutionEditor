#pragma once
#include <cstdint>
#include <iosfwd>
#include "ogg/ogg.h"
#include "vorbis/codec.h"
#include "../FSB.h"
#include "IO/BinaryWriter.h"
namespace FSB
{
    namespace Vorbis
    {
        class OGGPacketHolder
        {
        public:
            OGGPacketHolder();
            ~OGGPacketHolder();
            operator ogg_packet* ()
            {
                return &value;
            }
            operator ogg_packet& ()
            {
                return value;
            }
            ogg_packet* operator->()
            {
                return &value;
            }
            void Assign(const unsigned char* buffer, std::size_t buffer_size);
            void Assign(const char* buffer, std::size_t buffer_size);
            void Clear();
        private:
            OGGPacketHolder(const OGGPacketHolder& rhs) = delete;
            OGGPacketHolder& operator=(const OGGPacketHolder& rhs) = delete;
            ogg_packet value;
        };
        class OGGOuputStream
        {
        public:
            OGGOuputStream(int serialNumber, std::ostream& output);
            ~OGGOuputStream();
            void WritePacket(ogg_packet& packet);
            void FlushPackets();
        private:
            void WritePage(const ogg_page& page);
            OGGOuputStream(const OGGOuputStream& rhs) = delete;
            OGGOuputStream& operator=(const OGGOuputStream& rhs) = delete;
            std::ostream& output;
            ogg_stream_state stream_state;
        };
        class OGGBinaryWriter
        {
        public:
            OGGBinaryWriter(int serialNumber, BinaryWriter& binaryWriter);
            ~OGGBinaryWriter();
            void WritePacket(ogg_packet& packet);
            void FlushPackets();
        private:
            void WritePage(const ogg_page& page);
            OGGBinaryWriter(const OGGBinaryWriter& rhs) = delete;
            OGGBinaryWriter& operator=(const OGGBinaryWriter& rhs) = delete;
            BinaryWriter& binaryWriter;
            ogg_stream_state streamState;
        };
        class VorbisInfoHolder
        {
        public:
            VorbisInfoHolder();
            ~VorbisInfoHolder();
            operator vorbis_info* ()
            {
                return &value;
            }
            operator vorbis_info& ()
            {
                return value;
            }
            vorbis_info* operator->()
            {
                return &value;
            }
        private:
            VorbisInfoHolder(const VorbisInfoHolder& rhs) = delete;
            VorbisInfoHolder& operator=(const VorbisInfoHolder& rhs) = delete;
            vorbis_info value;
        };
        class VorbisCommentHolder
        {
        public:
            VorbisCommentHolder();
            void AddTag(const char* tag, const char* contents);
            ~VorbisCommentHolder();
            operator vorbis_comment* ()
            {
                return &value;
            }
            operator vorbis_comment& ()
            {
                return value;
            }
            vorbis_comment* operator->()
            {
                return &value;
            }
        private:
            VorbisCommentHolder(const VorbisCommentHolder& rhs) = delete;
            VorbisCommentHolder& operator=(const VorbisCommentHolder& rhs) = delete;
            vorbis_comment value;
        };
    }
}