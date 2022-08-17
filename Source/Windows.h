#pragma once
#include "Threading.h"
#include <string>

std::string ToString(const char* fmt, ...);
void        RunProcess(const char* applicationPath, const char* arguments);

struct StartProcessJob : Job
{
    std::string applicationPath;
    std::string arguments;
    virtual void RunJob() override;
};
