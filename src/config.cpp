#include "config.h"
#include <iostream>
#include <sstream>
#include <regex>
#include <stdexcept>

namespace speakerbox {

Config::Config() = default;

Config::~Config() = default;

bool Config::load(const std::string& file) {
    std::ifstream ifs(file);
    if (!ifs) return false;
    std::string line;
    while (std::getline(ifs, line)) {
        line = std::regex_replace(line, std::regex("^ +| +$|( )+"), "$1");  // Trim
        if (line.empty() || line[0] == ';') continue;
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string val = line.substr(eq + 1);
            data_[key] = val;
        }
    }
    ifs.close();
    return true;
}

bool Config::save(const std::string& file) const {
    std::ofstream ofs(file);
    if (!ofs) return false;
    for (const auto& pair : data_) {
        ofs << pair.first << "=" << pair.second << std::endl;
    }
    ofs.close();
    std::filesystem::permissions(file, std::filesystem::perms::owner_read | std::filesystem::perms::owner_write);  // Security
    return true;
}

void Config::set(const std::string& key, const std::string& value) {
    data_[key] = value;
}

std::string Config::get(const std::string& key, const std::string& default_val) const {
    auto it = data_.find(key);
    return it != data_.end() ? it->second : default_val;
}

void Config::loadTSParameters(TSParameters& params) {
    params.fs = std::stod(get("fs", "0.0"));
    params.qts = std::stod(get("qts", "0.0"));
    // ... load all
}

void Config::saveTSParameters(const TSParameters& params) {
    set("fs", std::to_string(params.fs));
    // ... save all
}

void initDataDir() {
    std::filesystem::create_directories("data");
}

}  // namespace speakerbox
