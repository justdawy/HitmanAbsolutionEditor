#include "Resources/MultiLanguage.h"
#include <rapidjson/document.h>
#include "IO/BinaryWriter.h"

void MultiLanguage::Deserialize()
{
	BinaryReader binaryReader = BinaryReader(resourceData, resourceDataSize);
	localizationCategory = binaryReader.Read<char>();
	const unsigned int count = (GetResourceDataSize() - 1) >> 3;
	const char* data = static_cast<const char*>(GetResourceData());

	for (unsigned int i = 0; i < count; ++i)
	{
		const int index = *reinterpret_cast<const int*>(&data[8 * i + 5]);
		const std::string locale = &data[8 * i + 1];

		indices.push_back(index);
		locales.push_back(locale);
	}

	isResourceDeserialized = true;
}

void MultiLanguage::Export(const std::string& outputPath, const std::string& exportOption)
{
	if (exportOption.starts_with("Raw"))
	{
		ExportRawData(outputPath);
	}
	else
	{
		SerializeToJson(outputPath);
	}
}

char MultiLanguage::GetLocalizationCategory()
{
	return localizationCategory;
}

std::vector<int>& MultiLanguage::GetIndices()
{
	return indices;
}

std::vector<std::string>& MultiLanguage::GetLocales()
{
	return locales;
}

void MultiLanguage::SerializeToJson(const std::string& outputFilePath)
{
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(stringBuffer);

	writer.StartObject();

	writer.String("localizationCategory");
	writer.Uint(localizationCategory);

	writer.String("locales");
	writer.StartArray();

	for (size_t i = 0; i < locales.size(); ++i)
	{
		writer.String(locales[i].c_str());
	}

	writer.EndArray();

	writer.String("indices");
	writer.StartArray();

	for (size_t i = 0; i < indices.size(); ++i)
	{
		writer.Int(indices[i]);
	}

	writer.EndArray();

	writer.EndObject();

	std::ofstream outputFileStream = std::ofstream(outputFilePath);

	outputFileStream << stringBuffer.GetString();

	outputFileStream.close();
}

void MultiLanguage::ImportFromJson(const std::string& inputFilePath)
{
	std::ifstream inputFileStream(inputFilePath);
	std::string jsonString((std::istreambuf_iterator<char>(inputFileStream)), std::istreambuf_iterator<char>());
	inputFileStream.close();

	rapidjson::Document document;
	document.Parse(jsonString.c_str());

	if (document.HasParseError() || !document.IsObject())
	{
		return;
	}

	if (document.HasMember("localizationCategory"))
	{
		localizationCategory = static_cast<char>(document["localizationCategory"].GetUint());
	}

	locales.clear();
	indices.clear();

	if (document.HasMember("locales") && document["locales"].IsArray())
	{
		const rapidjson::Value& localesArray = document["locales"];
		for (rapidjson::SizeType i = 0; i < localesArray.Size(); i++)
		{
			locales.push_back(localesArray[i].GetString());
		}
	}

	if (document.HasMember("indices") && document["indices"].IsArray())
	{
		const rapidjson::Value& indicesArray = document["indices"];
		for (rapidjson::SizeType i = 0; i < indicesArray.Size(); i++)
		{
			indices.push_back(indicesArray[i].GetInt());
		}
	}

	SerializeToBuffer();
}

void MultiLanguage::SerializeToBuffer()
{
	// Format: 1 byte category + N * 8 bytes (4 byte locale string padded + 4 byte int index)
	unsigned int count = static_cast<unsigned int>(locales.size());
	unsigned int newSize = 1 + count * 8;

	char* newData = new char[newSize];
	memset(newData, 0, newSize);

	newData[0] = localizationCategory;

	for (unsigned int i = 0; i < count; ++i)
	{
		// Copy locale string (up to 4 chars, null-padded)
		size_t len = locales[i].length();
		if (len > 4) len = 4;
		memcpy(&newData[8 * i + 1], locales[i].c_str(), len);

		// Write index
		memcpy(&newData[8 * i + 5], &indices[i], sizeof(int));
	}

	if (resourceData)
	{
		delete[] resourceData;
	}

	resourceDataSize = newSize;
	resourceData = newData;
}
