/*#include "DataManager.h"
#include <fstream>

json DataManager::loadJSON(const std::string& filename) {
    std::ifstream file(filename);
    json data;
    if (file.is_open()) {
        file >> data;
    }
    return data;
}

void DataManager::saveJSON(const std::string& filename, const json& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << data.dump(4);
    }
}*/
