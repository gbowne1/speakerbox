#include "utils.h"
#include "sha256.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace speakerbox {

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void initLogging() {
    google::InitGoogleLogging("speakerbox");
    google::SetLogDestination(google::INFO, "data/speakerbox.log");
}

std::string hashSHA256(const std::string& input) {
    SHA256 sha;
    sha.update(reinterpret_cast<const unsigned char*>(input.c_str()), input.length());
    unsigned char digest[SHA256::DIGEST_SIZE];
    sha.final(digest);
    std::stringstream ss;
    for (int i = 0; i < SHA256::DIGEST_SIZE; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return ss.str();
}

}  // namespace speakerbox
