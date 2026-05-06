workspace "AzureFlare"
    location "./build/"
    startproject "psobb"

    targetdir "%{wks.location}/bin/%{cfg.buildcfg}/"
    objdir "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}/"
    buildlog "%{wks.location}/obj/%{cfg.buildcfg}/%{prj.name}.log"

    toolset "gcc"
    architecture "x86"

    --[[
        Set the prefix only in Linux, as we build using the
        MinGW32 console on Windows
    ]]--
    filter "system:not windows"
        gccprefix "i686-w64-mingw32-"
    filter {}

    configurations { "Debug", "Release" }
    platforms { "x86" }

    buildoptions {
        "-std=c++17",
        "-msse2",                   -- was vectorextensions "sse2"
        "-finput-charset=UTF-8",    -- was /utf-8
        "-fexec-charset=UTF-8",
        "-march=i486",              -- XP hardware safety floor
    }

    links {
        "dbghelp",
    }

    linkoptions {
        "-Wl,--large-address-aware",   -- /LARGEADDRESSAWARE
        "-Wa,-mbig-obj",
        "-Wl,--major-subsystem-version,5,--minor-subsystem-version,1",
        "-Wl,--major-os-version,5,--minor-os-version,1",

        -- Static instructions
        "-static",
        "-static-libgcc",
        "-static-libstdc++",
        "-lpthread",
        
        "-Wl,-Map=%{wks.location}/bin/%{cfg.buildcfg}/%{cfg.targetname}.map",
    }

    defines {
        "NOMINMAX",
        "WIN32_LEAN_AND_MEAN",
        "UNICODE",
        "_UNICODE",
    }

    filter "configurations:Release"
        defines "NDEBUG"
        optimize "Size"
        symbols "Off"
        buildoptions { "-ffunction-sections", "-fdata-sections", "-flto" }
        linkoptions { 
            "-flto",                       -- /LTCG (Link phase)
            "-Wl,--gc-sections",           -- /OPT:REF
        --    "-Wl,--icf=all",               -- /OPT:ICF
            "-Wl,--no-keep-memory",
        --    "-v",
        }
    filter {}

    filter "configurations:Debug"
        defines "DEBUG"
        optimize "Debug"
        symbols "On"
    filter {}

project "psobb"
    targetname "wsock32"
    language "C++"
    kind "SharedLib"
    warnings "Off"
    targetextension ".dll"

    forceincludes { "stdafx.hh" }

    links {
        "minhook",
        "polar_common",
    }

    linkoptions {
        path.getabsolute("./src/psobb/resources/wsock32.def"),
        "-Wl,--enable-stdcall-fixup", -- Required for the server redirection
    }
    
    files { "./src/psobb/**/*.cc", "./src/psobb/resources/wsock32.def" }
    includedirs {
        "./src/psobb/",
        "./src/common/include/",
        "./deps/minhook/include/",
    }

    filter "configurations:Release"
        linkoptions {
            "-Wl,--exclude-all-symbols",
        }
    filter {}

    -- Include the license on build
    postbuildcommands {
        "cp %[LICENSE.txt] %{wks.location}/bin/%{cfg.buildcfg}/LICENSE.txt",
        "cp %[psobb.cfg] %{wks.location}/bin/%{cfg.buildcfg}/psobb.cfg",
    }

project "polar_common"
    kind "StaticLib"
    language "C++"
    warnings "Off"

    forceincludes { "header/stdafx.hh" }
    
    files {
        "./src/common/**/*.cc",
    }

    links {
        "dbghelp"
    }
    
    includedirs {
        "./src/common/include/"
    }

project "minhook"
    kind "StaticLib"
    language "C++"
    warnings "Off"

    files {
        "./deps/minhook/src/**.c"
    }

    -- Required by the library
    postbuildcommands { "cp %[deps/minhook/LICENSE.txt] %{wks.location}/bin/%{cfg.buildcfg}/LICENSE.minhook.txt" }