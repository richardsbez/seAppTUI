#pragma once
#include <string>
#include <vector>

struct AppInfo {
    std::string name;
    std::string exec;
    std::string lowerName; // pré-processado para acelerar busca
};

class AppSearcher {
public:
    std::vector<AppInfo> loadApps();
    std::vector<AppInfo> filter(const std::vector<AppInfo>& apps, const std::string& text);
};

