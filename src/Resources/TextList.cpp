#include "Resources/TextList.h"
#include "Utility/StringUtility.h"
#include "Registry/TextListHashRegistry.h"
#include <rapidjson/document.h>
#include "IO/BinaryWriter.h"
void TextList::Deserialize()
{
	BinaryReader binaryReader = BinaryReader(resourceData, resourceDataSize);
	const unsigned int entryCount = binaryReader.Read<unsigned int>();

	entries.reserve(entryCount);

	for (unsigned int i = 0; i < entryCount; ++i)
	{
		Entry entry;

		const unsigned int key = binaryReader.Read<unsigned int>();
		const unsigned int textLength = binaryReader.Read<unsigned int>();
		const std::string text = binaryReader.ReadString(static_cast<size_t>(textLength));
		const std::string name = TextListHashRegistry::GetInstance().GetName(key);

		entry.key = key;
		entry.text = text;

		if (name.empty())
		{
			entry.name = StringUtility::ConvertValueToHexString(key);
		}
		else
		{
			entry.name = name;
		}

		entries.push_back(entry);
	}

	isResourceDeserialized = true;
}

void TextList::Export(const std::string& outputPath, const std::string& exportOption)
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

std::vector<TextList::Entry>& TextList::GetEntries()
{
	return entries;
}

void TextList::SerializeToJson(const std::string& outputFilePath)
{
	rapidjson::StringBuffer stringBuffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(stringBuffer);

	writer.StartObject();

	writer.String("entries");
	writer.StartArray();

	for (size_t i = 0; i < entries.size(); ++i)
	{
		writer.StartObject();

		writer.String("hash");
		writer.String(StringUtility::ConvertValueToHexString(entries[i].key, 8).c_str());

		writer.String("name");

		const std::string& name = TextListHashRegistry::GetInstance().GetName(entries[i].key);

		if (!name.empty())
		{
			writer.String(entries[i].name.c_str());
		}
		else
		{
			writer.Null();
		}

		writer.String("text");
		writer.String(entries[i].text.c_str());

		writer.EndObject();
	}

	writer.EndArray();

	writer.EndObject();

	std::ofstream outputFileStream = std::ofstream(outputFilePath);

	outputFileStream << stringBuffer.GetString();

	outputFileStream.close();
}

void TextList::ImportFromJson(const std::string& inputFilePath)
{
	std::ifstream inputFileStream(inputFilePath);
	std::string jsonString((std::istreambuf_iterator<char>(inputFileStream)), std::istreambuf_iterator<char>());
	inputFileStream.close();

	rapidjson::Document document;
	document.Parse(jsonString.c_str());

	if (document.HasParseError() || !document.IsObject() || !document.HasMember("entries") || !document["entries"].IsArray())
	{
		return; // Or throw an error
	}

	entries.clear();

	const rapidjson::Value& entriesArray = document["entries"];
	for (rapidjson::SizeType i = 0; i < entriesArray.Size(); i++)
	{
		const rapidjson::Value& entryObj = entriesArray[i];
		Entry entry;
		
		std::string hashString = entryObj["hash"].GetString();
		entry.key = static_cast<unsigned int>(std::stoul(hashString, nullptr, 16));
		
		if (entryObj["name"].IsString())
		{
			entry.name = entryObj["name"].GetString();
		}
		
		entry.text = entryObj["text"].GetString();
		
		entries.push_back(entry);
	}
	
	SerializeToBuffer();
}

void TextList::SerializeToBuffer()
{
	BinaryWriter binaryWriter(1024); // OutputMemoryStream backed
	
	binaryWriter.Write<unsigned int>(static_cast<unsigned int>(entries.size()));
	
	for (size_t i = 0; i < entries.size(); ++i)
	{
		binaryWriter.Write<unsigned int>(entries[i].key);
		binaryWriter.Write<unsigned int>(static_cast<unsigned int>(entries[i].text.length()));
		binaryWriter.WriteString(entries[i].text);
	}
	
	if (resourceData)
	{
		delete[] resourceData;
	}
	
	resourceDataSize = static_cast<unsigned int>(binaryWriter.GetPosition());
	resourceData = new char[resourceDataSize];
	memcpy(resourceData, binaryWriter.GetBuffer(), resourceDataSize);
}
