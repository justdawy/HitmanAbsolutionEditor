#include "Utility/ResourcePatcher.h"
#include "Logger.h"

#include <fstream>
#include <vector>
#include <filesystem>

bool ResourcePatcher::BackupFile(const std::string& filePath)
{
    std::string backupPath = filePath + ".bak";
    if (!std::filesystem::exists(backupPath))
    {
        try
        {
            std::filesystem::copy_file(filePath, backupPath, std::filesystem::copy_options::overwrite_existing);
            return true;
        }
        catch (const std::exception& e)
        {
            Logger::GetInstance().Log(Logger::Level::Error, "Failed to create backup: " + std::string(e.what()));
            return false;
        }
    }
    return true; // Backup already exists
}

bool ResourcePatcher::PatchResourceLibrary(const std::string& resourceLibraryPath, const std::string& headerLibraryPath, unsigned int offsetInResourceLibrary, unsigned int offsetInHeaderLibrary, const void* newData, unsigned int newDataSize)
{
    if (!BackupFile(resourceLibraryPath))
    {
        return false;
    }

    if (!BackupFile(headerLibraryPath))
    {
        return false;
    }

    // Read original size from header library
    unsigned int originalSize = 0;
    std::ifstream headerFileIn(headerLibraryPath, std::ios::binary);
    if (!headerFileIn.is_open())
    {
        Logger::GetInstance().Log(Logger::Level::Error, "Failed to open header library to read original size.");
        return false;
    }
    headerFileIn.seekg(offsetInHeaderLibrary + 12, std::ios::beg);
    headerFileIn.read(reinterpret_cast<char*>(&originalSize), sizeof(unsigned int));
    headerFileIn.close();

    if (newDataSize == originalSize)
    {
        // Simple in-place replacement
        std::fstream file(resourceLibraryPath, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open())
        {
            Logger::GetInstance().Log(Logger::Level::Error, "Failed to open resource library for patching.");
            return false;
        }

        file.seekp(offsetInResourceLibrary, std::ios::beg);
        file.write(static_cast<const char*>(newData), newDataSize);
        file.close();
        
        Logger::GetInstance().Log(Logger::Level::Info, "Patched resource in place successfully.");
        return true;
    }
    else
    {
        // Full rebuild required
        Logger::GetInstance().Log(Logger::Level::Info, "Size changed from " + std::to_string(originalSize) + " to " + std::to_string(newDataSize) + ". Rebuilding resource library...");
        return RebuildResourceLibrary(resourceLibraryPath, headerLibraryPath, offsetInResourceLibrary, offsetInHeaderLibrary, originalSize, newData, newDataSize);
    }
}

bool ResourcePatcher::RebuildResourceLibrary(const std::string& resourceLibraryPath, const std::string& headerLibraryPath, unsigned int offsetInResourceLibrary, unsigned int offsetInHeaderLibrary, unsigned int originalSize, const void* newData, unsigned int newDataSize)
{
    // 1. Rebuild the Resource Library
    std::ifstream inFile(resourceLibraryPath, std::ios::binary);
    if (!inFile.is_open())
    {
        Logger::GetInstance().Log(Logger::Level::Error, "Failed to open resource library for reading.");
        return false;
    }

    std::ofstream outFile(resourceLibraryPath + ".tmp", std::ios::binary | std::ios::trunc);
    if (!outFile.is_open())
    {
        Logger::GetInstance().Log(Logger::Level::Error, "Failed to open temporary resource library for writing.");
        return false;
    }

    constexpr size_t bufferSize = 16 * 1024 * 1024; // 16 MB chunks
    std::vector<char> buffer(bufferSize);

    // Copy pre-data
    size_t preRemaining = offsetInResourceLibrary;
    while (preRemaining > 0)
    {
        size_t toRead = (std::min)(preRemaining, bufferSize);
        inFile.read(buffer.data(), toRead);
        outFile.write(buffer.data(), toRead);
        preRemaining -= toRead;
    }

    // Write new data
    outFile.write(static_cast<const char*>(newData), newDataSize);

    // Skip original size in the input file
    inFile.seekg(offsetInResourceLibrary + originalSize, std::ios::beg);

    // Copy post-data
    inFile.seekg(0, std::ios::end);
    size_t totalSize = inFile.tellg();
    size_t postRemaining = totalSize - (offsetInResourceLibrary + originalSize);
    
    inFile.seekg(offsetInResourceLibrary + originalSize, std::ios::beg);

    while (postRemaining > 0)
    {
        size_t toRead = (std::min)(postRemaining, bufferSize);
        inFile.read(buffer.data(), toRead);
        outFile.write(buffer.data(), toRead);
        postRemaining -= toRead;
    }

    inFile.close();
    outFile.close();

    // Replace the original resource library with the temporary one
    try
    {
        std::filesystem::rename(resourceLibraryPath + ".tmp", resourceLibraryPath);
    }
    catch (const std::exception& e)
    {
        Logger::GetInstance().Log(Logger::Level::Error, "Failed to replace resource library: " + std::string(e.what()));
        return false;
    }

    // 1.5. Update the resource library's own header (m_nDataSize at offset 12)
    {
        size_t newTotalFileSize = offsetInResourceLibrary + newDataSize + postRemaining;
        unsigned int newTotalDataSize = static_cast<unsigned int>(newTotalFileSize - 0x18);

        std::fstream resLibHeader(resourceLibraryPath, std::ios::in | std::ios::out | std::ios::binary);
        if (resLibHeader.is_open())
        {
            resLibHeader.seekp(12, std::ios::beg);
            resLibHeader.write(reinterpret_cast<const char*>(&newTotalDataSize), sizeof(unsigned int));
            resLibHeader.close();
        }
        else
        {
            Logger::GetInstance().Log(Logger::Level::Error, "Failed to update resource library header.");
        }
    }

    // 2. Update the Header Library
    std::fstream headerFile(headerLibraryPath, std::ios::in | std::ios::out | std::ios::binary);
    if (!headerFile.is_open())
    {
        Logger::GetInstance().Log(Logger::Level::Error, "Failed to open header library for updating.");
        return false;
    }

    // Read the old data size to see if memory requirements matched it
    unsigned int oldDataSize = 0;
    headerFile.seekg(offsetInHeaderLibrary + 12, std::ios::beg);
    headerFile.read(reinterpret_cast<char*>(&oldDataSize), sizeof(unsigned int));

    unsigned int sysMemReq = 0;
    headerFile.seekg(offsetInHeaderLibrary + 16, std::ios::beg);
    headerFile.read(reinterpret_cast<char*>(&sysMemReq), sizeof(unsigned int));

    // Update data size
    headerFile.seekp(offsetInHeaderLibrary + 12, std::ios::beg);
    headerFile.write(reinterpret_cast<const char*>(&newDataSize), sizeof(unsigned int));

    // If system memory requirement equaled the old data size, update it too
    if (sysMemReq == oldDataSize || sysMemReq == originalSize)
    {
        headerFile.seekp(offsetInHeaderLibrary + 16, std::ios::beg);
        headerFile.write(reinterpret_cast<const char*>(&newDataSize), sizeof(unsigned int));
    }

    headerFile.close();

    return true;
}
