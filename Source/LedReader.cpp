#include <fstream>
#include <algorithm>
#include <cstring>
#include <limits>

#include "LedReader.h"

bool LedReader::Open(const std::string& archivePath) {
    path = archivePath;
    entries.clear();

    std::ifstream led(path, std::ios::binary);

    if (!led) {
        return false;
    }

    LedHeader header{};

    led.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (!led) {
        return false;
    }

    if (std::memcmp(header.magic, "LED1", 4) != 0) {
        return false;
    }

    // Read file entries from the archive
    entries.reserve(header.fileCount);

    for (uint32_t i = 0; i < header.fileCount; ++i) {
        LedFileEntry entry{};

        led.read(reinterpret_cast<char*>(&entry), sizeof(entry));

        if (!led) {
            entries.clear();
            return false;
        }

        // Make sure name is terminated
        entry.name[sizeof(entry.name) - 1] = '\0';

        entries.push_back(entry);
    }
    return true;
}

bool LedReader::ReadFile(const std::string& fileName, std::vector<char>& data) const {
    data.clear();

    if (path.empty())
        return false;

    std::ifstream led(path, std::ios::binary);

    if (!led)
        return false;

    // Find the requested file entry
    const LedFileEntry* found = nullptr;

    for (const LedFileEntry& entry : entries) {
        std::string name(
            entry.name,
            strnlen(entry.name, sizeof(entry.name))
        );

        if (name == fileName) {
            found = &entry;
            break;
        }
    }

    if (!found)
        return false;

    // Get archive size
    led.seekg(0, std::ios::end);

    std::streamoff archiveSize = led.tellg();

    if (archiveSize < 0)
        return false;

    const uint64_t offset = found->offset;
    const uint64_t size = found->size;
    const uint64_t archiveSize64 = static_cast<uint64_t>(archiveSize);

    // Make sure the file starts inside the archive
    if (offset > archiveSize64)
        return false;

    // Make sure the complete file fits inside the archive
    if (size > archiveSize64 - offset)
        return false;

    // Make sure size can fit into a vector
    if (size > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
        return false;

    // Move to the file data
    led.clear();

    led.seekg(static_cast<std::streamoff>(offset), std::ios::beg);

    if (!led)
        return false;

    // Allocate space BEFORE reading
    data.resize(static_cast<size_t>(size));

    // Read file data.
    if (size > 0) {
        led.read(
            data.data(),
            static_cast<std::streamsize>(size)
        );

        if (!led) {
            data.clear();
            return false;
        }
    }

    return true;
}

// Read a file as text
bool LedReader::ReadTextFile(const std::string& fileName, std::string& text) const {
    text.clear();

    std::vector<char> data;

    if (!ReadFile(fileName, data))
        return false;

    text.assign(data.begin(), data.end());

    return true;
}

// Check whether a file exists in the archive.
bool LedReader::ContainsFile(const std::string& fileName) const {
    for (const LedFileEntry& entry : entries) {
        std::string name(entry.name, strnlen(entry.name, sizeof(entry.name)));

        if (name == fileName)
            return true;
    }

    return false;
}