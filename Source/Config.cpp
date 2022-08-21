#include "Config.h"
#include "Windows.h"

#include "json.hpp"

#include <fstream>


const char* configFileName          = "UATHelperConfig.json";
const char* platformSelectionText   = "Platform Selection";
const char* rootPathText            = "Root Path";
const char* projectPathText         = "Project Path";
const char* versionOptionsText      = "Version Options";
const char* switchOptionsText       = "Switch Options";
const char* preBuildEventsText      = "Pre Build Events";
const char* postBuildEventsText     = "Post Build Events";
const char* platformOptionsText     = "Platform Settings";
const char* versionText             = "Version";
const char* enabledVersionsText     = "Enabled Versions";
const char* enabledSwitchesName     = "Enabled Switches";
const char* enabledPreBuildName     = "Enabled Pre Build";
const char* enabledPostBuildName    = "Enabled Post Build";
const s32 currentVersion = 1;



s32 ComparisonFunction(const void* a, const void* b)
{
    return  *(s32*)b - *(s32*)a;
}
void SortData(std::vector<s32>& data)
{
    QuickSort((u8*)data.data(), (s32)data.size(), sizeof(data[0]), ComparisonFunction);
}
void SortConfig(Settings& set)
{
    for (s32 op = 0; op < set.platformOptions.size(); op++)
    {
        auto& plat = set.platformOptions[op];
        SortData(plat.enabledVersions);
        SortData(plat.enabledSwitches);
        SortData(plat.enabledPreBuild);
        SortData(plat.enabledPostBuild);
    }
}


void AddParentAndChildrenInt(nlohmann::json& root, const std::string& option, std::vector<s32>& data, const std::vector<std::string>& names)
{
    if (!data.size())
        return;
    for (s32 i = 0; i < data.size(); i++)
    {
#if 1
        root[option].push_back(names[data[i]]);
#else
        root[option].push_back(data[i]);
        //root[i] = std::to_string(data[i]);
#endif
    }
}
void SaveConfig(Settings& settings)
{
    SortConfig(settings);
    nlohmann::json j;

    j[versionText]          = currentVersion;
    j[platformSelectionText] = settings.platformSelection;
    j[rootPathText]          = settings.rootPath;
    j[projectPathText]       = settings.projectPath;
    j[versionOptionsText]    = settings.versionOptions;
    j[switchOptionsText]     = settings.switchOptions;
    j[preBuildEventsText]   = settings.preBuildEvents;
    j[postBuildEventsText]  = settings.postBuildEvents;

    for (s32 i = 0; i < settings.platformOptions.size(); i++)
    {
        PlatformSettings& set = settings.platformOptions[i];
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledVersionsText,      set.enabledVersions,     settings.versionOptions);
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledSwitchesName,      set.enabledSwitches,     settings.switchOptions);
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledPreBuildName,     set.enabledPreBuild,     settings.preBuildEvents);
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledPostBuildName,    set.enabledPostBuild,    settings.postBuildEvents);
    }


    std::ofstream o(configFileName);
    o << std::setw(4) << j << std::endl;
}

void GetChildrenString(nlohmann::json& j, const std::string& name, std::vector<std::string>& data)
{
    if (!j[name].is_null())
        data = j[name];
}

s32 FindStringInArray(const std::string s, std::vector<std::string> data)
{
    for (s32 i = 0; i < data.size(); i++)
    {
        if (data[i] == s)
            return i;
    }
    return INT_MAX;
}

void LoadPlatformSettingsChildren(const std::string& optionsName, const auto& src, std::vector<s32>& dest, const std::vector<std::string>& optionNames)
{
    if (src.contains(optionsName))
    {
        const auto& data = src[optionsName];
        for (auto it = data.begin(); it != data.end(); it++)
        {
            s32 index = FindStringInArray(it.value(), optionNames);
            if (index == INT_MAX)
            {
                assert(false);
                ShowErrorWindow("String Not Found In Array", ToString("\'%s\' not found in \'%s\'", it.value().get<std::string>().c_str(), optionsName.c_str()));
                continue;
            }
            dest.push_back(index);
        }
    }
}

bool LoadConfig(Settings& settings)
{
    settings = {};
    std::ifstream i(configFileName);
    if (i.fail())
        return false;
    nlohmann::json j;
    i >> j;

#define NOT_NULL_AND_DO_THING(root, name, type, var) \
    if (!root[name].is_null()) \
    {\
        var = j[name].get<type>();\
    }\
    nullptr

    if (!j[versionText].is_null())
    {
        if (j[versionText].get<s32>() != currentVersion)
            return false;
    }
    NOT_NULL_AND_DO_THING(j, platformSelectionText, s32, settings.platformSelection);
    NOT_NULL_AND_DO_THING(j, rootPathText, std::string, settings.rootPath);
    NOT_NULL_AND_DO_THING(j, projectPathText, std::string, settings.projectPath);

    GetChildrenString(j, versionOptionsText,   settings.versionOptions);
    GetChildrenString(j, switchOptionsText,    settings.switchOptions);
    GetChildrenString(j, preBuildEventsText,  settings.preBuildEvents);
    GetChildrenString(j, postBuildEventsText, settings.postBuildEvents);

    assert(!j[platformOptionsText].is_null());

    for (auto it = j[platformOptionsText].begin(); it != j[platformOptionsText].end(); it++)
    {
        auto& po = settings.platformOptions;
        po.push_back({ it.key() });
        if (it.value().is_null())
            continue;

        LoadPlatformSettingsChildren(enabledVersionsText,    it.value(), po[po.size() - 1].enabledVersions,  settings.versionOptions);
        LoadPlatformSettingsChildren(enabledSwitchesName,    it.value(), po[po.size() - 1].enabledSwitches,  settings.switchOptions);
        LoadPlatformSettingsChildren(enabledPreBuildName,   it.value(), po[po.size() - 1].enabledPreBuild,  settings.preBuildEvents);
        LoadPlatformSettingsChildren(enabledPostBuildName,  it.value(), po[po.size() - 1].enabledPostBuild, settings.postBuildEvents);
    }

    if (settings.platformSelection >= settings.platformOptions.size())
        settings.platformSelection = s32(settings.platformOptions.size() - 1);

    return true;
}

void LoadDefaults(Settings& settings)
{
    settings.platformOptions.push_back({ "Win64" });
    settings.platformOptions.push_back({ "XboxOne" });
    settings.platformOptions.push_back({ "XboxOneGDK" });
    settings.platformOptions.push_back({ "XSX" });
    settings.platformOptions.push_back({ "PS4" });
    settings.platformOptions.push_back({ "PS5" });

    settings.versionOptions.push_back({ "Shipping" });
    settings.versionOptions.push_back({ "Test" });
    settings.versionOptions.push_back({ "Development" });
    settings.versionOptions.push_back({ "Debug" });

    settings.switchOptions.push_back({ "AdditionalCookerOptions=\"-ddc=noshared\"" });
    settings.switchOptions.push_back({ "AdditionalCookerOptions=\"-ddc=ddcreadonly\"" });
    settings.switchOptions.push_back({ "distribution" });
    settings.switchOptions.push_back({ "MapIniSectionsToCook=DevCookMaps" });
    settings.switchOptions.push_back({ "cook" });
    settings.switchOptions.push_back({ "NoP4" });
    settings.switchOptions.push_back({ "manifests" });
    settings.switchOptions.push_back({ "pak" });
    settings.switchOptions.push_back({ "build" });
    settings.switchOptions.push_back({ "stage" });
    settings.switchOptions.push_back({ "compress" });
    settings.switchOptions.push_back({ "dedicatedserver" });
    settings.switchOptions.push_back({ "package" });
    settings.switchOptions.push_back({ "skipcook" });
    settings.switchOptions.push_back({ "skipbuild" });
}