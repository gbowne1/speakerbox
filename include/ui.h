#pragma once

#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>

namespace speakerbox {

class UI {
public:
    UI(bool use_color = true);
    ~UI();

    void showSplash() const;
    void clearScreen() const;
    void drawBox(const std::string& title, const std::vector<std::string>& content) const;
    int showMenu(const std::string& title, const std::vector<std::string>& options) const;
    std::string getInput(const std::string& prompt, bool password = false) const;
    void showProgress(int duration_ms) const;
    void displayResult(const EnclosureResult& result) const;
    void showHelp() const;
    void showWarning(const std::string& msg) const;
    void showError(const std::string& msg) const;
    bool checkTerminalSize() const;

private:
    bool use_color_;
    std::map<std::string, std::string> colors_;
    std::mutex mutex_;

    std::string color(const std::string& code) const;
    char getKey() const;  // For arrow input
};

}  // namespace speakerbox
