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
    s32 version = 2;
    s32 platformSelection = 0;
    std::string rootPath;
    std::string projectPath;
    std::vector<std::string> versionOptions;
    std::vector<std::string> switchOptions;
    BuildEvents preBuildEvents;
    BuildEvents postBuildEvents;
    std::vector<PlatformSettings> platformOptions;
};

struct AppSettings {
    s32 majorRev = 1;
    s32 minorRev = 3;
    float UPS = 60.0f; //updates per second
    s32 colorSelection = {};
    s32 styleSelection = {};
    s32 currentFileNameIndex = -1;
    std::vector<std::string> fileNames;
    std::string configDirectory;
};

void SortConfig(Settings& settings);
void SaveConfig(Settings& settings, const std::string& filename);
void LoadConfig(Settings& settings, const AppSettings& appSettings);
void LoadConfigDefaults(Settings& settings);
void ClearConfig(Settings& settings);
bool ConfigIsSameAsLastLoad(const Settings& settings);

void SaveAppSettings(AppSettings& settings);
void LoadAppSettings(AppSettings& settings);
void LoadDefaultAppSettings(AppSettings& appSet);
void ScanDirectoryForConfigs(AppSettings& settings);
