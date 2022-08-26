#pragma once
#include "Math.h"
#include "Themes.h"
#include <vector>
#include <string>


struct PlatformSettings {
    std::string name;
    std::vector<s32> enabledVersions;
    std::vector<s32> enabledSwitches;
    std::vector<s32> enabledPreBuild;
    std::vector<s32> enabledPostBuild;
};

struct Settings {
    s32 platformSelection = 0;
    s32 colorSelection = {};
    s32 styleSelection = {};
#if 1
    std::string rootPath;
    std::string projectPath;
#else
    std::vector<std::string> rootPath;
    std::vector<std::string> projectPath;
#endif
    std::vector<std::string> versionOptions;
    std::vector<std::string> switchOptions;
    std::vector<std::string> preBuildEvents;
    std::vector<std::string> postBuildEvents;
    std::vector<PlatformSettings> platformOptions;
};


void SortConfig(Settings& settings);
void SaveConfig(Settings& settings);
bool LoadConfig(Settings& settings);
void LoadDefaults(Settings& settings);
void ClearConfig(Settings& settings);
bool ConfigIsSameAsLastLoad(const Settings& settings);
