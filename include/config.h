#pragma once

#include <string>
#include <map>
#include <filesystem>
#include <fstream>

namespace speakerbox {

class Config {
public:
    Config();
    ~Config();

    bool load(const std::string& file);
    bool save(const std::string& file) const;
    void set(const std::string& key, const std::string& value);
    std::string get(const std::string& key, const std::string& default_val = "") const;

    void loadTSParameters(TSParameters& params);
    void saveTSParameters(const TSParameters& params);

private:
    std::map<std::string, std::string> data_;
};

void initDataDir();

}  // namespace speakerbox
