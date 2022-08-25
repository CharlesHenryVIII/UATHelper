#pragma once
#include "Math.h"

struct Theme {
    const char* name;
    void (*function)(void);
};


enum ThemeColor : s32 {
    Color_DefaultDark,
    Color_DefaultLight,
    Color_DefaultClassic,
    Color_GreenAccent,
    Color_RedAccent,
    Color_Grey,
    Color_Grey2,
    Color_WildCard,
    Color_Count,
};

enum ThemeStyle : s32 {
    Style_Basic,
    Style_Original,
    Style_SimpleRounding,
    Style_Grey,
    Style_Count,
};


extern Theme ColorOptions[Color_Count];
extern Theme StyleOptions[Style_Count];


void ThemesInit();
void Color_Set(s32 color);
void Style_Set(s32 style);
