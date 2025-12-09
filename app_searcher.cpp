#include "app_searcher.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>

static AppInfo parseDesktop(const std::string& path) {
    AppInfo info;
    std::ifstream file(path);
    if (!file.is_open()) return info;

    std::string line;
    while (std::getline(file, line)) {

        if (line.rfind("Name=", 0) == 0) {
            info.name = line.substr(5);
        }
        else if (line.rfind("Exec=", 0) == 0) {
            info.exec = line.substr(5);
        }

        // já leu o suficiente
        if (!info.name.empty() && !info.exec.empty())
            break;
    }

    // cria versão lowercase para busca mais rápida
    info.lowerName = info.name;
    std::transform(info.lowerName.begin(), info.lowerName.end(),
                   info.lowerName.begin(), ::tolower);

    return info;
}

std::vector<AppInfo> AppSearcher::loadApps() {
    std::vector<AppInfo> result;

    const char* home = getenv("HOME");
    std::vector<std::string> dirs = {
        "/usr/share/applications",
        std::string(home ? home : "") + "/.local/share/applications"
    };

    for (const auto& d : dirs) {
        if (!std::filesystem::exists(d)) continue;

        for (const auto& p : std::filesystem::directory_iterator(d)) {
            if (p.path().extension() == ".desktop") {
                AppInfo info = parseDesktop(p.path().string());
                if (!info.name.empty() && !info.exec.empty()) {
                    result.push_back(info);
                }
            }
        }
    }

    return result;
}

static bool fuzzyMatch(const std::string& name, const std::string& text) {
    int n = name.size();
    int m = text.size();
    int i = 0, j = 0;

    while (i < n && j < m) {
        if (name[i] == text[j])
            j++;
        i++;
    }
    return j == m;
}


std::vector<AppInfo> AppSearcher::filter(const std::vector<AppInfo>& apps, const std::string& text) {
    if (text.empty()) return apps;

    std::string lowText = text;
    std::transform(lowText.begin(), lowText.end(), lowText.begin(), ::tolower);

    std::vector<AppInfo> starts; // prioridade 1
    std::vector<AppInfo> contains; // prioridade 2
    std::vector<AppInfo> fuzzy; // prioridade 3

    for (const auto& a : apps) {

        if (a.lowerName.rfind(lowText, 0) == 0) {
            starts.push_back(a);
        } 
        else if (a.lowerName.find(lowText) != std::string::npos) {
            contains.push_back(a);
        }
        else if (fuzzyMatch(a.lowerName, lowText)) {
            fuzzy.push_back(a);
        }
    }

    // Junta com prioridade
    std::vector<AppInfo> final;
    final.reserve(starts.size() + contains.size() + fuzzy.size());

    final.insert(final.end(), starts.begin(), starts.end());
    final.insert(final.end(), contains.begin(), contains.end());
    final.insert(final.end(), fuzzy.begin(), fuzzy.end());

    return final;
}

