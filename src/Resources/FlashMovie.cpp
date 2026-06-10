#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

#include "Resources/FlashMovie.h"
#include "Editor.h"
#include "Utility/ResourcePatcher.h"

void FlashMovie::Export(const std::string& outputPath, const std::string& exportOption)
{
	if (exportOption.starts_with("Raw"))
	{
		ExportRawData(outputPath);
	}
	else
	{
		void* textureData = nullptr;
		unsigned int textureDataSize;
		bool isDDSTexture;

		GetTextureData(textureData, textureDataSize, isDDSTexture);

		BinaryWriter binaryWriter = BinaryWriter(outputPath);

		binaryWriter.Write(textureData, textureDataSize);
	}
}

void FlashMovie::ImportFromSWF(const std::string& filePath)
{
	BinaryReader binaryReader = BinaryReader(resourceData, resourceDataSize);
	const unsigned char type = binaryReader.Read<unsigned char>();
	const unsigned int fileOffset = binaryReader.Read<unsigned int>();

	// The header is fileOffset + 1 bytes long
	unsigned int headerSize = fileOffset + 1;

	// Read the new SWF file
	BinaryReader swfReader = BinaryReader(filePath);
	unsigned int newSwfSize = swfReader.GetSize();
	void* newSwfData = swfReader.Read<void>(newSwfSize);

	// Create new buffer
	unsigned int newResourceDataSize = headerSize + newSwfSize;
	void* newResourceData = malloc(newResourceDataSize);

	// Copy header
	memcpy(newResourceData, resourceData, headerSize);
	
	// Copy new SWF data
	memcpy(static_cast<char*>(newResourceData) + headerSize, newSwfData, newSwfSize);

	// Free the temporary SWF data
	operator delete(newSwfData);

	// Free old resourceData if necessary
	if (resourceData)
	{
		free(resourceData);
	}

	// Assign new resourceData
	resourceData = newResourceData;
	resourceDataSize = newResourceDataSize;
}

bool FlashMovie::PatchBackToGame()
{
	std::string resourceLibPath = GetResourceLibraryFilePath();
	std::string headerLibPath = GetHeaderLibraryFilePath();
	unsigned int offsetInResLib = GetOffsetInResourceLibrary();
	unsigned int offsetInHeaderLib = GetOffsetInHeaderLibrary();

	return ResourcePatcher::PatchResourceLibrary(resourceLibPath, headerLibPath, offsetInResLib, offsetInHeaderLib, resourceData, resourceDataSize);
}

void FlashMovie::GetTextureData(void*& textureData, unsigned int& textureDataSize, bool& isDDSTexture)
{
	BinaryReader binaryReader = BinaryReader(resourceData, resourceDataSize);
	const unsigned char type = binaryReader.Read<unsigned char>();
	const unsigned int fileOffset = binaryReader.Read<unsigned int>();

	binaryReader.Seek(fileOffset + 1);

	size_t currentPosition = binaryReader.GetPosition();

	binaryReader.Seek(0, SeekOrigin::End);

	textureDataSize = binaryReader.GetPosition() - currentPosition;

	binaryReader.Seek(fileOffset + 1);

	textureData = binaryReader.GetBuffer(true);
	isDDSTexture = type == 'd';
}

void FlashMovie::CreateTextureFromMemory()
{
	void* textureData = nullptr;
	unsigned int textureDataSize;
	bool isDDSTexture;

	GetTextureData(textureData, textureDataSize, isDDSTexture);

	if (isDDSTexture)
	{
		DirectX::CreateDDSTextureFromMemory(Editor::GetInstance().GetDirectXRenderer()->GetD3D11Device(), static_cast<unsigned char*>(textureData), textureDataSize, &texture, &textureView);
	}
	else
	{
		DirectX::CreateWICTextureFromMemory(Editor::GetInstance().GetDirectXRenderer()->GetD3D11Device(), static_cast<unsigned char*>(textureData), textureDataSize, &texture, &textureView);
	}

	if (texture)
	{
		ID3D11Texture2D* texture2D = nullptr;
		D3D11_TEXTURE2D_DESC textureDesc;

		texture->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture2D));
		
		if (texture2D)
		{
			texture2D->GetDesc(&textureDesc);

			textureWidth = textureDesc.Width;
			textureHeight = textureDesc.Height;

			texture2D->Release();
		}
	}
}

const FlashMovie::Format FlashMovie::GetFormat() const
{
	const unsigned char type = *static_cast<unsigned char*>(resourceData);

	switch (type)
	{
		case 'd':
			return Format::DDS;
		case 'p':
			return Format::PNG;
		case 's':
			return Format::SWF;
		default:
			return Format::None;
	}
}

const unsigned int FlashMovie::GetTextureWidth() const
{
	return textureWidth;
}

const unsigned int FlashMovie::GetTextureHeight() const
{
	return textureHeight;
}

ID3D11Resource* FlashMovie::GetTexture() const
{
	return texture;
}

ID3D11ShaderResourceView* FlashMovie::GetTextureView() const
{
	return textureView;
}
