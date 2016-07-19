workspace "Hypersomnia"
  configurations { "Debug", "Release" }
  platforms { "Linux", "Win64" }

project "Augmentations"
  kind "SharedLib"
  language "C++"
  targetdir "bin/"

  includedirs { ".", "augs/**", "3rdparty" }
  files { "augs/**.h", "augs/**.cpp" }
  removefiles { "augs/gui/controls/**",
                "augs/gui/text_drawer.cpp",
                "game/detail/gui/**",
                "game/components/gui_element.cpp",
                "game/messages/gui_intents.cpp",
                "game/stateful_systems/gui_system.cpp",
  }
  --  flags { "C++14" }
  buildoptions { "-std=c++1z" }

  filter "configurations:Debug"
    defines { "DEBUG", "ENABLE_ENSURE" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

  filter { "platforms:Win64" }
    system "Windows"
    architecture "x64"
    defines { "PLATFORM_WINDOWS" }

  filter { "platforms:Linux" }
    system "Linux"
    architecture "x64"
    defines { "PLATFORM_LINUX" }
    includedirs { "/usr/include/freetype2", "/usr/include/freetype2/freetype" } -- should make it global (windows?)

  filter { "platforms:Linux", "configurations:Debug" }
    buildoptions { "-ferror-limit=1" }
