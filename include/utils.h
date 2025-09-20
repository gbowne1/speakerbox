#pragma once

#include <string>
#include <chrono>
#include <ctime>
#include <glog/logging.h>

namespace speakerbox {

std::string trim(const std::string& str);

std::string getTimestamp();

void initLogging();

std::string hashSHA256(const std::string& input);

// Other utils...

}  // namespace speakerbox
