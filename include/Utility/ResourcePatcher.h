#pragma once

#include <string>

class ResourcePatcher
{
public:
	static bool PatchResourceLibrary(const std::string& resourceLibraryPath, const std::string& headerLibraryPath, unsigned int offsetInResourceLibrary, unsigned int offsetInHeaderLibrary, const void* newData, unsigned int newDataSize);
	static bool BackupFile(const std::string& filePath);

private:
	static bool RebuildResourceLibrary(const std::string& resourceLibraryPath, const std::string& headerLibraryPath, unsigned int offsetInResourceLibrary, unsigned int offsetInHeaderLibrary, unsigned int originalSize, const void* newData, unsigned int newDataSize);
};
