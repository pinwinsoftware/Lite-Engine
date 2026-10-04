#pragma once
#include <cstdint>
#include <string>
#include <vector>

#pragma pack(push, 1)

struct LedHeader {
    char magic[4];
    uint32_t fileCount;
};

struct LedFileEntry {
    char name[64];
    uint32_t offset;
    uint32_t size;
};

#pragma pack(pop)

class LedReader {
public:
    bool Open(const std::string& archivePath);
    bool ReadFile(const std::string& fileName, std::vector<char>& data) const;
    bool ReadTextFile(const std::string& fileName, std::string& text) const;
    bool ContainsFile(const std::string& fileName) const;

    const std::vector<LedFileEntry>& GetEntries() const {
        return entries;
    }

private:
    std::string path;
    std::vector<LedFileEntry> entries;
};