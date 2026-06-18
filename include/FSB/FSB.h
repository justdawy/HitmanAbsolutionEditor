#pragma once
#include <cstdint>
#include <istream>
#include <string>
namespace FSB
{
	enum class Format : unsigned int
	{
		None,
		PCM8,
		PCM16,
		PCM24,
		PCM32,
		PCMFloat,
		GCADPCM,
		IMAADPCM,
		VAG,
		HEVAG,
		XMA,
		MPEG,
		CELT,
		AT9,
		XWMA,
		Vorbis,
		Max,
	};
	struct Header
	{
		char id[4];
		unsigned int version = 0;
		unsigned int samples = 0;
		unsigned int headersSize = 0;
		unsigned int namesSize = 0;
		unsigned int dataSize = 0;
		Format mode;
		unsigned int unknown;
		unsigned int unknown2;
		std::int8_t guid[24];
	};
	struct Sample
	{
		std::string name;
		unsigned int frequency = 0;
		unsigned char channels = 0;
		std::size_t offset = 0;
		std::size_t size = 0;
		unsigned int vorbisCRC32 = 0;
		unsigned int loopStart = 0;
		unsigned int loopEnd = 0;
		unsigned int samples = 0;
	};
}