#include "Config.h"
#include "Windows.h"

#include "json.hpp"

#include <fstream>


const char* configFileName          = "UATHelperConfig.json";
const char* platformSelectionText   = "Platform Selection";
const char* colorSelectionText      = "Color Selection";
const char* styleSelectionText      = "Style Selection";
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

Settings fileSettings;


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


void RemoveNullStrings(const std::vector<std::string>& strings, std::vector<s32>& vals)
{
    std::erase_if(vals,
        [strings](s32 val)
        {
            return strings[val].empty();
        });
}
void RemoveNullStrings(std::vector<std::string>& strings)
{
    std::erase_if(strings,
        [](std::string& s)
        {
            return s.empty();
        });
}
void AddParentAndChildrenInt(nlohmann::json& root, const std::string& option, std::vector<s32>& data, const std::vector<std::string>& names)
{
    if (!data.size())
        return;
    RemoveNullStrings(names, data);
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

    j[versionText]              = currentVersion;
    j[platformSelectionText]    = settings.platformSelection;
    j[colorSelectionText]       = settings.colorSelection;
    j[styleSelectionText]       = settings.styleSelection;
    j[rootPathText]             = settings.rootPath;
    j[projectPathText]          = settings.projectPath;

    for (s32 i = 0; i < settings.platformOptions.size(); i++)
    {
        PlatformSettings& set = settings.platformOptions[i];
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledVersionsText,      set.enabledVersions,     settings.versionOptions);
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledSwitchesName,      set.enabledSwitches,     settings.switchOptions);
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledPreBuildName,     set.enabledPreBuild,     settings.preBuildEvents);
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledPostBuildName,    set.enabledPostBuild,    settings.postBuildEvents);
    }

    RemoveNullStrings(settings.versionOptions);
    RemoveNullStrings(settings.switchOptions);
    RemoveNullStrings(settings.preBuildEvents);
    RemoveNullStrings(settings.postBuildEvents);
    j[versionOptionsText]       = settings.versionOptions;
    j[switchOptionsText]        = settings.switchOptions;
    j[preBuildEventsText]       = settings.preBuildEvents;
    j[postBuildEventsText]      = settings.postBuildEvents;

    fileSettings = settings;

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

void ValidateLoadConfig(Settings& settings)
{
    if (!settings.platformOptions.size())
    {
        ShowErrorWindow("Invalid Config", ToString("\'%s\' needs to be greater than 0", platformOptionsText));
    }
}

bool LoadConfig(Settings& settings)
{
    settings = {};
    std::ifstream file(configFileName);
    if (file.fail())
        return false;
    nlohmann::json j;
    file >> j;

    if (!j[versionText].is_null())
    {
        if (j[versionText].get<s32>() != currentVersion)
            return false;
    }

#define NOT_NULL_AND_DO_THING(root, name, type, var) \
    if (!root[name].is_null()) \
    {\
        var = j[name].get<type>();\
    }\
    nullptr

    NOT_NULL_AND_DO_THING(j, platformSelectionText, s32,            fileSettings.platformSelection);
    NOT_NULL_AND_DO_THING(j, colorSelectionText,    s32,            fileSettings.colorSelection);
    NOT_NULL_AND_DO_THING(j, styleSelectionText,    s32,            fileSettings.styleSelection);
    NOT_NULL_AND_DO_THING(j, rootPathText,          std::string,    fileSettings.rootPath);
    NOT_NULL_AND_DO_THING(j, projectPathText,       std::string,    fileSettings.projectPath);

    GetChildrenString(j, versionOptionsText,   fileSettings.versionOptions);
    GetChildrenString(j, switchOptionsText,    fileSettings.switchOptions);
    GetChildrenString(j, preBuildEventsText,  fileSettings.preBuildEvents);
    GetChildrenString(j, postBuildEventsText, fileSettings.postBuildEvents);

    assert(!j[platformOptionsText].is_null());

    for (auto it = j[platformOptionsText].begin(); it != j[platformOptionsText].end(); it++)
    {
        auto& po = fileSettings.platformOptions;
        po.push_back({ it.key() });
        if (it.value().is_null())
            continue;

        LoadPlatformSettingsChildren(enabledVersionsText,    it.value(), po[po.size() - 1].enabledVersions,  fileSettings.versionOptions);
        LoadPlatformSettingsChildren(enabledSwitchesName,    it.value(), po[po.size() - 1].enabledSwitches,  fileSettings.switchOptions);
        LoadPlatformSettingsChildren(enabledPreBuildName,   it.value(), po[po.size() - 1].enabledPreBuild,  fileSettings.preBuildEvents);
        LoadPlatformSettingsChildren(enabledPostBuildName,  it.value(), po[po.size() - 1].enabledPostBuild, fileSettings.postBuildEvents);
    }

    if (fileSettings.platformSelection >= fileSettings.platformOptions.size())
        fileSettings.platformSelection = s32(fileSettings.platformOptions.size() - 1);

    settings = fileSettings;

    Color_Set(settings.colorSelection);
    Style_Set(settings.styleSelection);
    ValidateLoadConfig(settings);
    
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

    Color_Set(settings.colorSelection);
    Style_Set(settings.styleSelection);
}

void ClearConfig(Settings& settings)
{
    settings = {};
}



bool ConfigIsSameAsLastLoad(const Settings& s)
{
#define ROOTCMP(__member)                       \
if (s. ## __member != fileSettings. ## __member)\
    return false

#define ARRCMP(__a, __b)                                        \
do {                                                            \
    if (__a ## .size() != __b ## .size())                       \
        return false;                                           \
    else                                                        \
    {                                                           \
            for (s32 index = 0; index < __a ## .size(); index++)\
            {                                                   \
                 if (__a ## [index] != __b ## [index])\
                    return false;                               \
            }}}                                                 \
while (0)

#define CHILDCMP(__index, __member)                      \
if (s.platformOptions[__index]. ## __member != fileSettings.platformOptions[__index]. ## __member)\
    return false


    ROOTCMP(platformSelection);
    ROOTCMP(colorSelection);
    ROOTCMP(styleSelection);
    ROOTCMP(rootPath);
    ROOTCMP(projectPath);

    ARRCMP(s.versionOptions, fileSettings.versionOptions);
    ARRCMP(s.switchOptions, fileSettings.switchOptions);
    ARRCMP(s.preBuildEvents, fileSettings.preBuildEvents);
    ARRCMP(s.postBuildEvents, fileSettings.postBuildEvents);

    if (s.platformOptions.size() != fileSettings.platformOptions.size())
        return false;
    else
    {
        for (s32 i = 0; i < s.platformOptions.size(); i++)
        {
            CHILDCMP(i, name);
            ARRCMP(s.platformOptions[i].enabledVersions, fileSettings.platformOptions[i].enabledVersions);
            ARRCMP(s.platformOptions[i].enabledSwitches, fileSettings.platformOptions[i].enabledSwitches);
            ARRCMP(s.platformOptions[i].enabledPreBuild, fileSettings.platformOptions[i].enabledPreBuild);
            ARRCMP(s.platformOptions[i].enabledPostBuild, fileSettings.platformOptions[i].enabledPostBuild);
        }
    }



    return true;
}
