#pragma once
#include "app_searcher.hpp"
#include <vector>
#include <string>

class Launcher {
public:
    Launcher();
    void run();

private:
    void draw();
    void update_list(const std::vector<AppInfo>& newList);
    bool launchApp(const AppInfo& app); // Retorna true se deve sair
    bool handleKey(int ch); // Retorna true se deve sair

    std::vector<AppInfo> allApps;
    std::vector<AppInfo> filtered;
    std::string query;
    int selected = 0;
};
