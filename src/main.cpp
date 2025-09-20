#include "ui.h"
#include "calculator.h"
#include "config.h"
#include "utils.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include <glog/logging.h>
#include <atomic>
#include <thread>
#include <cstdlib>
#include <getopt.h>

namespace speakerbox {

int main(int argc, char** argv) {
    bool use_color = true;
    bool debug = false;

    // Parse flags
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"version", no_argument, 0, 'v'},
        {"no-color", no_argument, 0, 'n'},
        {"debug", no_argument, 0, 'd'},
        {0, 0, 0, 0}
    };
    int opt;
    while ((opt = getopt_long(argc, argv, "hvnd", long_options, nullptr)) != -1) {
        switch (opt) {
            case 'h': std::cout << "Help: speakerbox [options]" << std::endl; return 0;
            case 'v': std::cout << "0.0.1" << std::endl; return 0;
            case 'n': use_color = false; break;
            case 'd': debug = true; break;
            default: return 1;
        }
    }

    initDataDir();
    initLogging();
    LOG(INFO) << "Started at " << getTimestamp();

    UI ui(use_color);
    if (!ui.checkTerminalSize()) return 1;
    ui.showSplash();

    // Login/Register
    int choice = ui.showMenu("Welcome", {"Login", "Register"});
    std::string user = ui.getInput("Username");
    std::string pass = ui.getInput("Password", true);
    std::string hash = hashSHA256(pass);
    Config user_config;
    std::string user_file = "data/" + user + ".dat";
    if (choice == 1) {  // Register
        user_config.set("hash", hash);
        if (!user_config.save(user_file)) {
            ui.showError("Failed to register");
            return 1;
        }
    } else {  // Login
        if (!user_config.load(user_file) || user_config.get("hash") != hash) {
            ui.showError("Invalid login");
            return 1;
        }
    }
    LOG(INFO) << "User " << user << " logged in";

    Calculator calc;
    Config app_config;
    app_config.load("data/config.cfg");

    while (true) {
        int menu = ui.showMenu("Main Menu", {"Calculate", "Load Config", "Save Config", "Settings", "Help", "Exit"});
        if (menu == 5) break;
        if (menu == 4) ui.showHelp();
        if (menu == 3) {
            // Settings: toggle color, etc.
            use_color = !use_color;
            ui.use_color_ = use_color;
            app_config.set("use_color", use_color ? "true" : "false");
            app_config.save("data/config.cfg");
        }
        if (menu == 1 || menu == 2) {
            std::string file = ui.getInput("Config file");
            if (menu == 1) app_config.load(file);
            else app_config.save(file);
        }
        if (menu == 0) {
            TSParameters params;
            // Input params
            params.fs = std::stod(ui.getInput("Fs (Hz)"));
            params.qts = std::stod(ui.getInput("Qts"));
            // ... input all, with validation
            try {
                std::string rec = calc.recommendType(params.qts);
                ui.showWarning("Recommended: " + rec);
            } catch (const std::exception& e) {
                ui.showError(e.what());
                continue;
            }

            std::vector<std::string> types = {"Sealed", "Ported", "Bandpass", "Transmission Line", "Passive Radiator"};
            int type_idx = ui.showMenu("Enclosure Type", types);
            EnclosureType type = static_cast<EnclosureType>(type_idx);

            ui.showProgress(1000);  // Fake calc time

            EnclosureResult res = calc.calculate(params, type);
            ui.displayResult(res);

            // Save log
            LOG(INFO) << "Calculation: " << res.type << " Vb=" << res.vb;
        }
    }

    LOG(INFO) << "Exited at " << getTimestamp();
    return 0;
}

}  // namespace speakerbox

int main(int argc, char** argv) {
    return speakerbox::main(argc, argv);
}
