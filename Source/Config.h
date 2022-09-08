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

struct BuildEvent {
    s32 id = {};
    std::string name;
};

struct BuildEvents {
    std::vector<BuildEvent> m_events;

    void RemoveNullElements();
    bool Get(s32& index, const std::string& s) const; 
    bool Get(BuildEvent& out, const s32 id) const;
    bool Get(BuildEvent& out, const std::string& s) const;
    BuildEvent* Add(const std::string& name);
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
    BuildEvents preBuildEvents;
    BuildEvents postBuildEvents;
    std::vector<PlatformSettings> platformOptions;
};


void SortConfig(Settings& settings);
void SaveConfig(Settings& settings);
bool LoadConfig(Settings& settings);
void LoadDefaults(Settings& settings);
void ClearConfig(Settings& settings);
bool ConfigIsSameAsLastLoad(const Settings& settings);
