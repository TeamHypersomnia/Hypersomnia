# Hypersomnia

[![Build Status](https://travis-ci.org/TeamHypersomnia/Hypersomnia.svg?branch=master)](https://travis-ci.org/TeamHypersomnia/Hypersomnia)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/5aatwxv8hceaop56?svg=true)](https://ci.appveyor.com/project/geneotech/Hypersomnia)

**Download for:**

<a href="https://hypersomnia.xyz/builds/latest/Hypersomnia-for-Windows.zip"> <img src="https://hypersomnia.xyz/windows_icon.svg" height=70 hspace=1> </a>
<a href="https://hypersomnia.xyz/builds/latest/Hypersomnia-for-Linux.tar.gz"> <img src="https://hypersomnia.xyz/linux_icon.svg" height=70 hspace=1 vspace=20> </a>
<br>

*[(for older versions click here)](https://hypersomnia.xyz/builds)*

- [Hypersomnia](#hypersomnia)
- [Gallery](#gallery)
- [How to build](#how-to-build)
  - [Windows instructions](#windows-instructions)
  - [Linux instructions](#linux-instructions)
    - [Distribution-specific dependencies](#distribution-specific-dependencies)
    - [One-shot launch](#one-shot-launch)
    - [Detailed instructions](#detailed-instructions)
    - [Editor integration](#editor-integration)
      - [Opening and saving files](#opening-and-saving-files)
  - [MacOS instructions](#macos-instructions)
	- [Additional dependencies](#additional-dependencies)
	- [Details](#details)
- [Contributing](#contributing)

Hypersomnia is an upcoming community-centered shooter released as free software,
with an aspiration to one day become an MMO with elements of RPG.

Set in a hypothetical afterlife reality, it shall provide joy through altruistic behaviours, fierce fights, exploration, fulfillment of elaborate social roles or sowing panic as a traitor to benevolent ones.

Declare allegiance to one of the three factions whose apple of discord is a disparity between prevailing notions of moral excellence.
**Metropolis. Atlantis. Resistance.**

See the game's [wiki](https://wiki.hypersomnia.xyz) to get familiar with the universe.  
To understand the repository's folder structure, make sure to read the [documentation](https://wiki.hypersomnia.xyz/docs).

## Gallery

Watch gameplays on YouTube:

[![IMAGE ALT TEXT](https://img.youtube.com/vi/d_G1mRqKlb0/0.jpg)](https://www.youtube.com/watch?v=d_G1mRqKlb0 "Video Title")
[![IMAGE ALT TEXT](https://img.youtube.com/vi/0vlUOO5l0jw/0.jpg)](https://www.youtube.com/watch?v=0vlUOO5l0jw "Video Title")
[![IMAGE ALT TEXT](https://img.youtube.com/vi/mxvuWvD53tY/0.jpg)](https://www.youtube.com/watch?v=mxvuWvD53tY "Video Title")
[![IMAGE ALT TEXT](https://img.youtube.com/vi/f0cHnds9UuU/0.jpg)](https://www.youtube.com/watch?v=f0cHnds9UuU "Video Title")
[![IMAGE ALT TEXT](https://img.youtube.com/vi/XsSKj6hJH0w/0.jpg)](https://www.youtube.com/watch?v=XsSKj6hJH0w "Video Title")

![enter image description here][8]
![enter image description here][3]
![enter image description here][4]

  [8]: https://gifyu.com/images/16.main_menu_reup.png
  [3]: https://gifyu.com/images/23.light.png
  [4]: https://gifyu.com/images/30.smoke.png

# How to build

Currently, Hypersomnia is only buildable using ``clang`` both on Linux and Windows.  
Additionally, your operating system must be 64-bit.  

<!--
Formerly, the game was buildable under modern ``gcc`` versions,  
and also using ``MSVC`` (the Microsoft's compiler shipping with Visual Studio),  
but it quickly became too much of a hassle to support these compilers as we use **modern C++ constructs** throughout the entire codebase.  
``gcc``, for example, would sometimes simply crash on some really template-heavy code.  
-->

The project's ``CMakeLists.txt`` contains clauses for both ``MSVC`` and ``gcc``,  
so in the future, it might be possible to build the game under these compilers if they catch up to ``clang``.  

Irrespectively of the OS, you will need the following software in order to build Hypersomnia:  

- The newest **CMake**.
- **git** to clone the respository and later generate version information.
- [**ninja**](https://ninja-build.org/) to carry out the build.
- [LLVM](https://releases.llvm.org/) toolchain version 8 or newer.
	- For Windows, you can use [this installer](https://github.com/llvm/llvm-project/releases/download/llvmorg-8.0.1/LLVM-8.0.1-win64.exe), or a newer one. 
	- For Linux, use your distro-specific package. Make sure to install ```libc++``` and ```lld``` as well.
	- For MacOS, the version that comes pre-installed with **Xcode** is good enough.

Once dependencies are installed, go to the directory where you wish to have your Hypersomnia project downloaded,
open git bash and paste:

```
git clone --recurse-submodules https://github.com/TeamHypersomnia/Hypersomnia
```

The ``--recurse-submodules`` is necessary to clone the submodules as well.

Wait for the download to complete.
Next steps depend on the platform you are on.

On all platforms, you can choose among three building configurations:

- ``Debug`` - the fastest to build and provides debug information.   
	Recommended for normal development.
- ``Release`` - takes AWFULLY long to build because of link-time optimizations.  
	No debug information. Use only for production builds. Specifies ``IS_PRODUCTION_BUILD=1`` C++ preprocessor define.
- ``RelWithDebInfo`` - Same as ``Release`` but with debug info and without link-time optimizations.  
	Preferred choice for quickly testing how the game's mechanics play at normal speed, and also for debugging performance problems.

## Windows instructions

Prerequisites:
- **Visual Studio 2019 Preview** (Community) or newer.

Open up the terminal. Setup the environment:

```
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Preview\VC\Auxiliary\Build\vcvarsall.bat" x64
```

If the file cannot be found, you might need to look somewhere else for the ```vcvarsall.bat``` that comes with your particular Visual Studio installation. Another possible location is:

```
call "C:\Program Files (x86)\Microsoft Visual Studio\Preview\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

Next, run these commands:

```
cd Hypersomnia
mkdir build
cd build
set CONFIGURATION=Release
cmake -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER=lld-link -DARCHITECTURE="x64" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DGENERATE_DEBUG_INFORMATION=0 ..
ninja
```

Note: your computer **might start lagging heavily** for the duration of the build as ``ninja`` will use all available cores for compilation.

If you intend to develop the game, it is best to use "Debug" configuration for the fastest builds.

If you want to somehow customize your build, e.g. disable certain game features, refer to the beginning of ```CMakeLists.txt``` to see which options you can pass to the ```cmake``` command.

If the game builds successfully, issue these commands to launch it:

```
cd ../hypersomnia
"../build/Hypersomnia.exe"
```

<!-- Note that the 64-bit version is more likely to be kept up to date. -->

<!--
Resultant ```.sln``` and ```.vcxproj``` files should appear in the ```build/``` directory.
Open ```Hypersomnia.sln``` file, select **Release** configuration and hit **F7** to build the game.
**F5** should launch it.
-->

If, for some reason, some step fails, refer to the latest working Appveyor build and the relevant ```appveyor.yml``` file.

## Linux instructions

Current platforms are actively tested and supported:
- Arch Linux with i3 window manager.
- Ubuntu, but it is tested only through Travis builds.

#### Distribution-specific dependencies

Arch Linux:

- ``pkg-config``
- ``libx11``
- ``libxcb``
- ``xcb-util-keysyms``
- ``mbedtls``
- ``libsodium``

Ubuntu:

- ``libxcb-keysyms1``
- ``libxcb-keysyms1-dev``
- ``libxi6``
- ``libxi-dev``
- ``alsa-oss``
- ``osspd-alsa``
- ``osspd``
- ``libasound2``
- ``libasound2-dev``
- ``libmbedtls-dev``
- ``libsodium-dev``


### One-shot launch

Once the dependencies are all set, this is the complete script for building and launching the game from scratch, with RelWithDebInfo configuration:

```
git clone --depth 1 --recurse-submodules https://github.com/TeamHypersomnia/Hypersomnia
cd Hypersomnia
export CXX=clang++; export CC=clang;
cmake/build.sh RelWithDebInfo x64
ninja run -C build/current
```

#### Detailed instructions
 
Use your favorite shell to enter the repository's directory.
Then run:

```
cmake/build.sh [Debug|Release|RelWithDebInfo] [x86|x64] ["ADDITIONAL CMAKE FLAGS"]
```
For example:

```
export CXX=clang++; export CC=clang;
cmake/build.sh Debug x64
```
After which, the resultant ``build.ninja`` should appear in the build/Debug-x64-clang directory.

Example for gcc:

```
export CXX=g++; export CC=gcc;
cmake/build.sh Debug x64
```

After which, the resultant ``build.ninja`` should appear in the build/Debug-x64-gcc directory.

#### Invoking ninja

<!--
If you are building with ``clang``, make sure to call these exports before invoking ``ninja``:

```
export CXX=clang++; export CC=clang;
```

This is because some third-party libraries - freetype, for example - generate their Makefiles only after calling ``ninja`` on the Hypersomnia's ``build.ninja`` itself.
(if GCC build fails for some reason, add respective exports as well.)
-->

There are several additional ninja targets defined:

```
ninja run
```
Launches the game normally.

```
ninja tests
```
Launches unit tests only and exits cleanly.

The above targets set the working directory automatically to ```${PROJECT_SOURCE_DIR}/hypersomnia```.

If, for some reason, some step fails, refer to the latest working Travis build and the relevant ```travis.yml``` file.

If the game fails to launch, it should automatically open a log file with the relevant message using ```$VISUAL``` executable.

### Editor integration

If you plan to use the Hypersomnia editor on Linux, you might want to follow some additional configuration to make the experience better.

#### Opening and saving files

The Hypersomnia editor can choose directories for opening projects and saving them.  
On Windows, this is accomplished through an ``IFileDialog``.  
Needless to say, such a class does not exist on Linux.  
Hypersomnia provides shell scripts for common file managers in ``hypersomnia/scripts/unix/managers``.  
You'll need one for choosing a directory and one for revealing files in explorer.  
``cd`` to ``hypersomnia/scripts/unix`` and, assuming you want to use ``ranger`` as your file manager, create symlinks as such:  

```
ln -s managers/choose_directory_ranger.zsh choose_directory.local
ln -s managers/reveal_file_ranger.zsh reveal_file.local 
```

The framework also supports calling scripts for opening and saving files,  
though the project does not use this functionality yet:

```
ln -s managers/save_file_ranger.zsh save_file.local
ln -s managers/open_file_ranger.zsh open_file.local
```

None of the symlinks will be tracked by git.  

Currently, the following file managers are supported:
- [ranger](https://github.com/ranger/ranger) through ``--choosedir``, ``--choosefile`` and ``--selectfile`` options

To implement your own script for choosing a directory:

- Use a ``$TERMINAL`` variable for file managers that need a terminal to run on.  
  Ensure your terminal supports ``-e`` flag that passes the commands to launch on startup. 
- Output a ``$PWD/cache/last_file_path.txt`` file containg the result - which is the chosen path, e.g. ``/home/pbc/projects/my_map``.


## MacOS instructions

**WARNING: Building for MacOS is a work in progress! The window framework has yet to be ported.**

Generally, to build for MacOS, you can follow the instructions for Linux.
Here are the things you need to know beforehand:

### Prerequisites

- Xcode 11.1 or newer

### Additional dependencies

- ``mbedtls``
- ``libsodium``
- ``openssl``

### Details

To build, run from the repository's directory:

```
cmake/build.sh Release x64 -DBUILD_WINDOW_FRAMEWORK=0 -DBUILD_OPENGL=0 -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl
```

Note that the path to OpenSSL library must be set explicitly.

# Contributing

Pull requests are welcomed, should they even be typo fixes, missing const guarantees or changes in nomenclature.
If you however plan to add a completely new feature, create a relevant [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues) so that everybody knows about it,
because the project is continuously in a very, very active development and may undergo a revolution when it is the least expected.

For documentation, please make sure to read the [wiki](https://wiki.hypersomnia.xyz) to learn about the game and the source code.

You will be added to [TeamHypersomnia](https://github.com/TeamHypersomnia) organization once at least one of your pull requests is accepted.

If you have questions or you fail to build Hypersomnia, create an [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues).
Or if you just can't wait to utter some brilliant ideas with regard to the game, please do so, too!
