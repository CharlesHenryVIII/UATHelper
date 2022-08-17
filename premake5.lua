workspace "UATHelper"
   configurations { "Debug", "Profile", "Release" }
   platforms { "x64" }

project "UATHelper"
   --symbolspath '$(OutDir)$(TargetName).pdb'
   kind "WindowedApp" --WindowedApp --ConsoleApp
   language "C++"
   cppdialect "C++latest"
   targetdir "Build/%{cfg.platform}/%{cfg.buildcfg}"
   objdir "Build/obj/%{cfg.platform}/%{cfg.buildcfg}"
   editandcontinue "Off"
   characterset "ASCII"
   links {
       "SDL2",
       "SDL2main",
       "OpenGL32",
   }

   libdirs {
       "Contrib/SDL/lib/%{cfg.platform}/",
       "Contrib/imgui",
       --"Contrib/**",
   }
   includedirs {
       "Contrib/*",
       "Contrib/imgui/backends",
       "Contrib/SDL/include",
       --"Contrib/**"
   }
   flags {
       "MultiProcessorCompile",
       "FatalWarnings",
       "NoPCH",
   }
   defines {
       "_CRT_SECURE_NO_WARNINGS",
   }
   files {
       "Source/*.h",
       "Source/*.c",
       "Source/*.cpp",
       "Source/*.hpp",
       "Contrib/imgui/*.cpp",
       "Contrib/imgui/*.h",
       "Contrib/imgui/backends/imgui_impl_opengl3.cpp",
       "Contrib/imgui/backends/imgui_impl_sdl.cpp",
   }



    postbuildcommands
    {
        "{COPY} Contrib/SDL/lib/%{cfg.platform}/SDL2.dll %{cfg.targetdir}"
    }


   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols  "On"
      optimize "Off"

   filter "configurations:Profile"
      defines { "NDEBUG" }
      symbols  "on"
      optimize "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      symbols  "off"
      optimize "On"
