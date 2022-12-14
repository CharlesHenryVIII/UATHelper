#include "Config.h"
#include "Windows.h"

#include "json.hpp"

#include <fstream>


const char* appSettingsFileName     = "UATHelper.json";
const char* majorRevText            = "Major Revision";
const char* minorRevText            = "Minor Revision";
const char* UPSText                 = "Updates Per Second";
const char* colorSelectionText      = "Color Selection";
const char* styleSelectionText      = "Style Selection";
const char* currentFileText         = "Currently Loaded File";
const char* configDirectoryText     = "Config Directory";

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
const char* enabledSwitchesText     = "Enabled Switches";
const char* enabledPreBuildText     = "Enabled Pre Build";
const char* enabledPostBuildText    = "Enabled Post Build";


Settings fileSettings = {};
AppSettings appSettings = {};

void BuildEvents::RemoveNullElements()
{
    std::erase_if(m_events,
        [](const BuildEvent& be)
        {
            return be.name.empty();
        });
}
BuildEvent* BuildEvents::Add(const std::string& name)
{
    static s32 buildEventID = {};
    BuildEvent be;
    be.id = ++buildEventID;
    be.name = name;
    m_events.push_back(be);
    return &m_events[m_events.size() - 1];
}

//s32 FindStringInArray(const std::string& s, const std::vector<BuildEvent>& data)
bool BuildEvents::Get(s32& out_index, const std::string& s) const
{
    for (s32 i = 0; i < m_events.size(); i++)
    {
        if (m_events[i].name == s)
        {
            out_index = i;
            return true;
        }
    }
    return false;
}
//bool GetStringFromArray(std::string& out, const std::vector<BuildEvent>& events, const s32 id)
bool BuildEvents::Get(BuildEvent& out, const s32 id) const
{
    out.id = {};
    out.name.clear();
    for (s32 i = 0; i < m_events.size(); i++)
    {
        if (m_events[i].id == id)
        {
            out.id      = m_events[i].id;
            out.name    = m_events[i].name;
            return true;
        }
    }
    return false;
}
bool BuildEvents::Get(BuildEvent& out, const std::string& s) const
{
    out.id = {};
    out.name.clear();
    for (s32 i = 0; i < m_events.size(); i++)
    {
        if (m_events[i].name == s)
        {
            out.id      = m_events[i].id;
            out.name    = m_events[i].name;
            return true;
        }
    }
    return false;
}

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
void RemoveNullIDs(std::vector<s32>& IDs, const BuildEvents& be)
{
    std::erase_if(IDs,
        [be](s32& id)
        {
            BuildEvent b;
            if (be.Get(b, id))
            {
                return b.name.empty();
            }
            return true;
        });
}
void AddParentAndChildrenInt(nlohmann::json& root, const std::string& option, std::vector<s32>& IDs, const BuildEvents& be)
{
    if (!IDs.size())
        return;
    RemoveNullIDs(IDs, be);
    BuildEvent b;
    for (s32 i = 0; i < IDs.size(); i++)
    {
        if (be.Get(b, IDs[i]))
            root[option].push_back(b.name);
    }
}
void AddParentAndChildrenInt(nlohmann::json& root, const std::string& option, std::vector<s32>& data, const std::vector<std::string>& names)
{
    if (!data.size())
        return;
    RemoveNullStrings(names, data);
    for (s32 i = 0; i < data.size(); i++)
    {
        root[option].push_back(names[data[i]]);
    }
}
void AddBuildEvents(nlohmann::json& j, const char* name, const BuildEvents& events)
{
    for (s32 i = 0; i < events.m_events.size(); i++)
    {
        j[name].push_back(events.m_events[i].name);
    }
}
void SaveConfig(Settings& settings, const std::string& filename)
{
    SortConfig(settings);
    nlohmann::json j;

    j[versionText]              = settings.version;
    j[platformSelectionText]    = settings.platformSelection;
    j[rootPathText]             = settings.rootPath;
    j[projectPathText]          = settings.projectPath;

    for (s32 i = 0; i < settings.platformOptions.size(); i++)
    {
        if (settings.platformOptions[i].name.empty())
            continue;
        PlatformSettings& set = settings.platformOptions[i];
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledVersionsText,      set.enabledVersions,    settings.versionOptions);
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledSwitchesText,      set.enabledSwitches,    settings.switchOptions);
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledPreBuildText,     set.enabledPreBuild,     settings.preBuildEvents);
        AddParentAndChildrenInt(j[platformOptionsText][set.name], enabledPostBuildText,    set.enabledPostBuild,    settings.postBuildEvents);
    }

    RemoveNullStrings(settings.versionOptions);
    RemoveNullStrings(settings.switchOptions);
    settings.preBuildEvents.RemoveNullElements();
    settings.postBuildEvents.RemoveNullElements();
    j[versionOptionsText] = settings.versionOptions;
    j[switchOptionsText]  = settings.switchOptions;
    AddBuildEvents(j, preBuildEventsText,   settings.preBuildEvents);
    AddBuildEvents(j, postBuildEventsText,  settings.postBuildEvents);

    fileSettings = settings;

    std::ofstream o(filename);
    o << std::setw(4) << j << std::endl;
}

void GetChildrenString(nlohmann::json& j, const std::string& name, std::vector<std::string>& data)
{
    if (! j[name].is_null())
        data = j[name];
}
void GetChildrenString(nlohmann::json& j, const std::string& name, BuildEvents& be)
{
    if (!j[name].is_null())
    {
        for (auto it = j[name].begin(); it != j[name].end(); it++)
        {
            be.Add(it.value().get<std::string>());
        }
    }
}


s32 FindStringInArray(const std::string& s, const std::vector<std::string>& data)
{
    for (s32 i = 0; i < data.size(); i++)
    {
        if (data[i] == s)
            return i;
    }
    return INT_MAX;
}

void LoadPlatformSettingsChildren(const std::string& optionsName, const auto& src, std::vector<s32>& dest, const BuildEvents& be)
{
    if (src.contains(optionsName))
    {
        const auto& data = src[optionsName];
        BuildEvent b;
        for (auto it = data.begin(); it != data.end(); it++)
        {
            
            const std::string& s = it.value();
            if (be.Get(b, s))
            {
                dest.push_back(b.id);
            }
            else
            {
                assert(false);
                ShowErrorWindow("String Not Found In Array", ToString("\'%s\' not found in \'%s\'", it.value().get<std::string>().c_str(), optionsName.c_str()));
            }
        }
    }
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

bool Valid(const nlohmann::json root, const std::string& text)
{
    return (root.contains(text) && !root[text].is_null());
}

template <typename T>
void GetTypeFromValid(const nlohmann::json& root, const char* name, T& var)
{
    if (Valid(root, name))
    {
        var = root[name].get<T>();
    }
}

void LoadConfig(Settings& settings, const AppSettings& appSettings)
{
    fileSettings = {};
    settings = {};
    std::ifstream file(appSettings.fileNames[appSettings.currentFileNameIndex]);
    if (file.fail())
    {
        LoadConfigDefaults(settings);
        return;
    }
    nlohmann::json j;
    file >> j;

    if (!Valid(j, versionText) || j[versionText].get<s32>() != fileSettings.version)
    {
        LoadConfigDefaults(settings);
        return;
    }

    GetTypeFromValid<s32>(          j, platformSelectionText,   fileSettings.platformSelection);
    GetTypeFromValid<std::string>(  j, rootPathText,            fileSettings.rootPath);
    GetTypeFromValid<std::string>(  j, projectPathText,         fileSettings.projectPath);

    GetChildrenString(j, versionOptionsText,   fileSettings.versionOptions);
    GetChildrenString(j, switchOptionsText,    fileSettings.switchOptions);
    GetChildrenString(j, preBuildEventsText,  fileSettings.preBuildEvents);
    GetChildrenString(j, postBuildEventsText, fileSettings.postBuildEvents);

    //assert(Valid(j, platformOptionsText));

    for (auto it = j[platformOptionsText].begin(); it != j[platformOptionsText].end(); it++)
    {
        auto& po = fileSettings.platformOptions;
        po.push_back({ it.key() });
        if (it.value().is_null())
            continue;

        LoadPlatformSettingsChildren(enabledVersionsText,   it.value(), po[po.size() - 1].enabledVersions,  fileSettings.versionOptions);
        LoadPlatformSettingsChildren(enabledSwitchesText,   it.value(), po[po.size() - 1].enabledSwitches,  fileSettings.switchOptions);
        LoadPlatformSettingsChildren(enabledPreBuildText,   it.value(), po[po.size() - 1].enabledPreBuild,  fileSettings.preBuildEvents);
        LoadPlatformSettingsChildren(enabledPostBuildText,  it.value(), po[po.size() - 1].enabledPostBuild, fileSettings.postBuildEvents);
    }

    if (fileSettings.platformSelection >= fileSettings.platformOptions.size())
        fileSettings.platformSelection = s32(fileSettings.platformOptions.size() - 1);

    settings = fileSettings;

    ValidateLoadConfig(settings);

     
    
    return;
}

void LoadConfigDefaults(Settings& settings)
{
    settings.platformOptions.push_back({ "Win64" });
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
    settings.switchOptions.push_back({ "servertargetplatform=win64" });
    settings.switchOptions.push_back({ "serverconfig=Development" });
}

void ClearConfig(Settings& settings)
{
    settings = {};
}

bool ArraysAreTheSame(const std::vector<s32>& a, const BuildEvents& abes, const std::vector<s32>& b, const BuildEvents& bbes)
{
    if (a.size() != b.size())
    {
        return false;
    }
    else                                                        
    {                                                           
        BuildEvent abe;
        BuildEvent bbe;
        for (s32 i = 0; i < a.size(); i++)
        {
            abe.name.clear();
            bbe.name.clear();
            if (!abes.Get(abe, a[i]))
                return false;
            if (!bbes.Get(bbe, b[i]))
                return false;
            if (abe.name != bbe.name)
                return false;
        }
    }
    return true;
}
bool ArraysAreTheSame(const std::vector<s32>& a, const std::vector<std::string>& aStrings, const std::vector<s32>& b, const std::vector<std::string>& bStrings)
{
    if (a.size() != b.size())
    {
        return false;
    }
    else                                                        
    {                                                           
        for (s32 i = 0; i < a.size(); i++)
        {
            if (aStrings[a[i]] != bStrings[b[i]])
                return false;
        }
    }
    return true;
}
bool ArraysAreTheSame(const std::vector<std::string>& a, const std::vector<std::string>& b)
{
    if (a.size() != b.size())
    {
        return false;
    }
    else                                                        
    {                                                           
        for (s32 i = 0; i < a.size(); i++)
        {
            if (a[i] != b[i])
                return false;
        }
    }
    return true;
}
bool ArraysAreTheSame(const BuildEvents& a, const BuildEvents& b)
{
    if (a.m_events.size() != b.m_events.size())
    {
        return false;
    }
    else                                                        
    {                                                           
        for (s32 i = 0; i < a.m_events.size(); i++)
        {
            if (a.m_events[i].name != b.m_events[i].name)
                return false;
        }
    }
    return true;
}

bool ConfigIsSameAsLastLoad(const Settings& s)
{
#define ROOTCMP(__member)                       \
if (s. ## __member != fileSettings. ## __member)\
    return false

#define CHILDCMP(__index, __member)                      \
if (s.platformOptions[__index]. ## __member != fileSettings.platformOptions[__index]. ## __member)\
    return false

#define ARRAYS_ARE_DIFFERENT2(a, b) if (!ArraysAreTheSame(a, b))\
    return false
#define ARRAYSAREDIFFERENT4(a, b, c, d) if (!ArraysAreTheSame(a, b, c, d))\
    return false

    ROOTCMP(platformSelection);
    ROOTCMP(rootPath);
    ROOTCMP(projectPath);

    ARRAYS_ARE_DIFFERENT2(s.versionOptions,  fileSettings.versionOptions);
    ARRAYS_ARE_DIFFERENT2(s.switchOptions,   fileSettings.switchOptions);
    ARRAYS_ARE_DIFFERENT2(s.preBuildEvents,  fileSettings.preBuildEvents );
    ARRAYS_ARE_DIFFERENT2(s.postBuildEvents, fileSettings.postBuildEvents);

    if (s.platformOptions.size() != fileSettings.platformOptions.size())
        return false;
    else
    {
        for (s32 i = 0; i < s.platformOptions.size(); i++)
        {
            CHILDCMP(i, name);
            ARRAYSAREDIFFERENT4(s.platformOptions[i].enabledVersions,  s.versionOptions,   fileSettings.platformOptions[i].enabledVersions,    fileSettings.versionOptions );
            ARRAYSAREDIFFERENT4(s.platformOptions[i].enabledSwitches,  s.switchOptions,    fileSettings.platformOptions[i].enabledSwitches,    fileSettings.switchOptions  );
            ARRAYSAREDIFFERENT4(s.platformOptions[i].enabledPreBuild,  s.preBuildEvents,   fileSettings.platformOptions[i].enabledPreBuild,    fileSettings.preBuildEvents );
            ARRAYSAREDIFFERENT4(s.platformOptions[i].enabledPostBuild, s.postBuildEvents,  fileSettings.platformOptions[i].enabledPostBuild,   fileSettings.postBuildEvents);
        }
    }



    return true;
}

void SaveAppSettings(AppSettings& settings)
{
    nlohmann::json j;

    j[majorRevText]         = settings.majorRev;
    j[minorRevText]         = settings.minorRev;
    j[colorSelectionText]   = settings.colorSelection;
    j[styleSelectionText]   = settings.styleSelection;
    j[UPSText]              = settings.UPS;
    if (settings.fileNames.size() && settings.currentFileNameIndex >= 0 && settings.currentFileNameIndex < settings.fileNames.size())
        j[currentFileText] = settings.fileNames[settings.currentFileNameIndex];
    else
        j[currentFileText] = {};
    j[configDirectoryText]  = settings.configDirectory;

    std::ofstream o(appSettingsFileName);
    o << std::setw(4) << j << std::endl;
    LoadAppSettings(settings);
}

void Convert(s32 fromMajor, s32 fromMinor, AppSettings& settings, s32 toMajor, s32 toMinor)
{
    while (fromMajor != settings.majorRev || fromMinor != settings.minorRev)
    {
        if (fromMajor == 1)
        {
            switch (fromMinor)
            {
            case 3:
                [[fallthrough]];
            default:
                fromMinor++;
                //do nothing
            }
        }
    }
}

void LoadAppSettings(AppSettings& settings)
{
    appSettings = {};
    std::ifstream file(appSettingsFileName);
    if (file.fail())
    {
        LoadDefaultAppSettings(settings);
        return;
    }
    nlohmann::json j;
    file >> j;

    if (!Valid(j, majorRevText) || !Valid(j, minorRevText))
    {
        LoadDefaultAppSettings(settings);
        return;
    }
    else
    {
        s32 major = j[majorRevText].get<s32>();
        s32 minor = j[minorRevText].get<s32>();
        if (major != appSettings.majorRev || minor != appSettings.minorRev)
        {
            //TODO: is this ever neccessary for app settings?
            Convert(major, minor, settings, appSettings.majorRev, appSettings.minorRev);
        }
    }

    GetTypeFromValid<s32>(  j, majorRevText,        appSettings.majorRev);
    GetTypeFromValid<s32>(  j, minorRevText,        appSettings.minorRev);
    GetTypeFromValid<s32>(  j, colorSelectionText,  appSettings.colorSelection);
    GetTypeFromValid<s32>(  j, styleSelectionText,  appSettings.styleSelection);
    GetTypeFromValid<float>(j, UPSText,             appSettings.UPS);
    GetTypeFromValid<std::string>(j, configDirectoryText, appSettings.configDirectory);

    ScanDirectoryForConfigs(appSettings);
    appSettings.currentFileNameIndex = -1;
    if (Valid(j, currentFileText))
    {
        for (s32 i = 0; i < appSettings.fileNames.size(); i++)
        {
            if (appSettings.fileNames[i] == j[currentFileText])
            {
                appSettings.currentFileNameIndex = i;
            }
        }
    }

    settings = appSettings;

    Color_Set(settings.colorSelection);
    Style_Set(settings.styleSelection);
}

void LoadDefaultAppSettings(AppSettings& appSet)
{
    appSet = {};
}

void ScanDirectoryForConfigs(AppSettings& settings)
{
    //TODO: Add ability to work with case insensitivity
    std::vector<std::string> fileNames;
    ScanDirectoryForFileNames(settings.configDirectory, fileNames);
    for (s32 i = 0; i < fileNames.size(); i++)
    {
        std::string_view name = fileNames[i];
        if (name.size() > 9 + 5)
        {
            if (name.starts_with("UATHelper") && name.ends_with(".json"))
            {
                settings.fileNames.push_back(fileNames[i]);
            }
        }
    }
}
