#pragma once
#include <string>

bool NewMap();
bool OpenMap(const std::string& filename);
bool SaveMap(const std::string& filename);
bool SaveMapAs(std::string& filename);