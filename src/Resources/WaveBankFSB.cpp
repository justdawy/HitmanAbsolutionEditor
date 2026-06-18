#include <format>
#include <algorithm>
#include "Resources/WaveBankFSB.h"
#include "FSB/FSB.h"
#include "FSB/Container.h"
WaveBankFSB::AudioSample::~AudioSample()
{
	operator delete(data);
}
void WaveBankFSB::Export(const std::string& outputPath, const std::string& exportOption)
{
	if (exportOption.starts_with("Raw"))
	{
		ExportRawData(outputPath);
	}
	else if (exportOption.starts_with("FSB"))
	{
		void* fsb5Data = nullptr;
		unsigned int fsb5Size = 0;
		GetFSB5Data(fsb5Data, fsb5Size);
		BinaryWriter binaryWriter = BinaryWriter(outputPath);
		binaryWriter.Write(fsb5Data, fsb5Size);
	}
	else
	{
		const std::string extension = exportOption.substr(exportOption.find("(") + 1, 3);
		if (extension == "OGG")
		{
			ConvertFSB5ToOGG();
		}
		else
		{
			ConvertFSB5ToWAV();
		}
		for (size_t i = 0; i < audioSamples.size(); ++i)
		{
			std::string sanitizedSampleName = audioSamples[i]->name;
			std::string invalidChars = "<>:\"|?*/\\";
			for (char c : invalidChars)
			{
				std::replace(sanitizedSampleName.begin(), sanitizedSampleName.end(), c, '_');
			}
			const std::string outputFilePath = std::format("{}\\{}.{}", outputPath, sanitizedSampleName, extension);
			BinaryWriter binaryWriter = BinaryWriter(outputFilePath);
			binaryWriter.Write(audioSamples[i]->data, audioSamples[i]->dataSize);
		}
	}
}
const std::vector<std::shared_ptr<WaveBankFSB::AudioSample>>& WaveBankFSB::GetAudioSamples() const
{
	return audioSamples;
}
std::vector<std::shared_ptr<WaveBankFSB::AudioSample>>& WaveBankFSB::GetAudioSamples()
{
	return audioSamples;
}
void WaveBankFSB::ConvertFSB5ToOGG()
{
	void* fsb5Data = nullptr;
	unsigned int fsb5Size = 0;
	GetFSB5Data(fsb5Data, fsb5Size);
	if (!fsb5Data || fsb5Size == 0)
	{
		throw std::runtime_error("Empty or invalid FSB5 data");
	}
	BinaryReader binaryReader = BinaryReader(fsb5Data, fsb5Size);
	FSB::Container container = FSB::Container(binaryReader);
	audioSamples.clear();
	audioSamples.reserve(container.Samples().size());
	for (const FSB::Sample& sample : container.Samples())
	{
		BinaryWriter binaryWriter(sample.size + 8192); // Extra capacity for headers
		container.ExtractSample(sample, binaryWriter);
		std::shared_ptr<AudioSample> audioSample = std::make_shared<AudioSample>();
		audioSample->name = sample.name;
		audioSample->dataSize = static_cast<unsigned int>(binaryWriter.GetPosition());
		audioSample->data = operator new(audioSample->dataSize);
		memcpy(audioSample->data, binaryWriter.GetBuffer(), audioSample->dataSize);
		audioSamples.push_back(audioSample);
	}
}
void WaveBankFSB::ConvertFSB5ToWAV()
{
	void* fsb5Data = nullptr;
	unsigned int fsb5Size = 0;
	GetFSB5Data(fsb5Data, fsb5Size);
	if (!fsb5Data || fsb5Size == 0)
	{
		throw std::runtime_error("Empty or invalid FSB5 data");
	}
	BinaryReader binaryReader = BinaryReader(fsb5Data, fsb5Size);
	FSB::Container container = FSB::Container(binaryReader);
	audioSamples.clear();
	audioSamples.reserve(container.Samples().size());
	for (const FSB::Sample& sample : container.Samples())
	{
		BinaryWriter binaryWriter(sample.size + 8192); // Extra capacity for headers
		container.ExtractSample(sample, binaryWriter);
		std::shared_ptr<AudioSample> audioSample = std::make_shared<AudioSample>();
		audioSample->name = sample.name;
		audioSample->dataSize = static_cast<unsigned int>(binaryWriter.GetPosition());
		audioSample->data = operator new(audioSample->dataSize);
		memcpy(audioSample->data, binaryWriter.GetBuffer(), audioSample->dataSize);
		audioSamples.push_back(audioSample);
	}
}