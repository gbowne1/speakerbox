#include "ui.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <cstdio>

namespace speakerbox {

UI::UI(bool use_color) : use_color_(use_color) {
    colors_ = {
        {"black", "\033[30m"},
        {"white", "\033[37m"},
        {"blue", "\033[34m"},
        {"gray", "\033[90m"},
        {"yellow", "\033[33m"},
        {"green", "\033[32m"},
        {"cyan", "\033[36m"},
        {"red", "\033[31m"},
        {"reset", "\033[0m"},
        {"inverse", "\033[7m"},
        {"flash", "\033[5m"},
        {"bg_blue", "\033[44m"}  // Example bg
    };
    // Check env
    if (std::getenv("NO_COLOR")) use_color_ = false;
}

UI::~UI() = default;

void UI::showSplash() const {
    clearScreen();
    std::cout << color("yellow") << "SpeakerBox v0.0.1" << color("reset") << std::endl;
    std::cout << "Copyright 2025" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(3));
    clearScreen();
}

void UI::clearScreen() const {
    std::cout << "\033[2J\033[H" << std::flush;
}

void UI::drawBox(const std::string& title, const std::vector<std::string>& content) const {
    std::cout << color("blue") << "┌─── " << title << " ───┐" << color("reset") << std::endl;
    for (const auto& line : content) {
        std::cout << "│ " << line << std::endl;
    }
    std::cout << "└──────────────┘" << std::endl;
}

int UI::showMenu(const std::string& title, const std::vector<std::string>& options) const {
    int selected = 0;
    while (true) {
        clearScreen();
        drawBox(title, {});
        for (size_t j = 0; j < options.size(); ++j) {
            if (static_cast<int>(j) == selected) {
                std::cout << color("inverse") << "> " << options[j] << color("reset") << std::endl;
            } else {
                std::cout << "  " << options[j] << std::endl;
            }
        }
        char key = getKey();
        if (key == 'A' || key == 'w' || key == 'i' || key == 'a') selected = (selected - 1 + options.size()) % options.size();  // Up
        else if (key == 'B' || key == 's' || key == 'k' || key == 'd') selected = (selected + 1) % options.size();  // Down
        else if (key == '\n' || key == '\r') return selected;
    }
    return -1;
}

std::string UI::getInput(const std::string& prompt, bool password) const {
    std::cout << prompt << ": " << std::flush;
    std::string input;
    if (password) {
        termios oldt;
        tcgetattr(STDIN_FILENO, &oldt);
        termios newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        std::getline(std::cin, input);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        std::cout << std::endl;
    } else {
        std::getline(std::cin, input);
    }
    return input;
}

void UI::showProgress(int duration_ms) const {
    std::atomic<bool> done(false);
    std::thread t([&]() {
        const char spinner[] = {'-', '\\', '|', '/'};
        int i = 0;
        while (!done) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                std::cout << "\rProgress: " << spinner[i % 4] << std::flush;
            }
            i++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "\rProgress: Done   " << std::endl;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    done = true;
    t.join();
}

void UI::displayResult(const EnclosureResult& result) const {
    std::vector<std::string> content;
    content.push_back("Type: " + result.type);
    content.push_back("Vb: " + std::to_string(result.vb) + " L");
    content.push_back("Fc/Fb: " + std::to_string(result.fc_or_fb) + " Hz");
    content.push_back("Response: " + result.freq_response);
    if (result.port_length > 0.0) {
        content.push_back("Port Length: " + std::to_string(result.port_length) + " cm");
        content.push_back("Port Diameter: " + std::to_string(result.port_diameter) + " cm");
        content.push_back("Air Velocity: " + std::to_string(result.air_velocity) + " m/s");
    }
    content.push_back("Dimensions (WxHxD cm): " + std::to_string(result.width) + "x" + std::to_string(result.height) + "x" + std::to_string(result.depth));
    content.push_back("Within Xmax: " + std::string(result.within_xmax ? "Yes" : "No"));
    if (!result.warnings.empty()) {
        for (const auto& w : result.warnings) {
            content.push_back("Warning: " + w);
        }
    }
    drawBox("Result", content);
    std::cout << "Press enter to continue..." << std::endl;
    std::cin.get();
}

void UI::showHelp() const {
    std::vector<std::string> content = {
        "SpeakerBox Help",
        "Enter T/S params when prompted.",
        "Choose enclosure type.",
        "Save/load configs from menu."
    };
    drawBox("Help", content);
    std::cout << "Press enter..." << std::endl;
    std::cin.get();
}

void UI::showWarning(const std::string& msg) const {
    std::cout << color("yellow") << "Warning: " << msg << color("reset") << std::endl;
}

void UI::showError(const std::string& msg) const {
    std::cout << color("red") << "Error: " << msg << color("reset") << std::endl;
}

bool UI::checkTerminalSize() const {
    struct winsize ws;
    if (ioctl(STDIN_FILENO, TIOCGWINSZ, &ws) == -1) return false;
    if (ws.ws_col < 80 || ws.ws_row < 25) {
        showWarning("Terminal too small (min 80x25)");
        return false;
    }
    return true;
}

std::string UI::color(const std::string& code) const {
    if (!use_color_) return "";
    auto it = colors_.find(code);
    return it != colors_.end() ? it->second : "";
}

char UI::getKey() const {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    char c = getchar();
    if (c == '\033') {  // Escape
        getchar();  // [
        c = getchar();  // A/B/C/D for arrows
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return c;
}

}  // namespace speakerbox
