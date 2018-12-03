# Hypersomnia

[![Build Status](https://travis-ci.org/TeamHypersomnia/Hypersomnia.svg?branch=master)](https://travis-ci.org/TeamHypersomnia/Hypersomnia)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/5aatwxv8hceaop56?svg=true)](https://ci.appveyor.com/project/geneotech/Hypersomnia)

Latest **Windows** binaries: https://ci.appveyor.com/project/geneotech/hypersomnia/build/artifacts  
The latest **Linux** binary has to [be built manually](#linux-instructions).

- [Hypersomnia](#hypersomnia)
- [Gallery](#gallery)
- [How to build](#how-to-build)
  - [Windows instructions](#windows-instructions)
  - [Linux instructions](#linux-instructions)
    - [Dependencies](#dependencies)
    - [One-shot launch](#one-shot-launch)
    - [Detailed instructions](#detailed-instructions)
    - [Editor integration](#editor-integration)
      - [Opening and saving files](#opening-and-saving-files)
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

Currently, Hypersomnia is only buildable using ``clang`` (7.0.0 or newer), both on Linux and Windows.  
Additionally, the system must be 64-bit.  

<!--
Formerly, the game was buildable under modern ``gcc`` versions,  
and also using ``MSVC`` (the Microsoft's compiler shipping with Visual Studio),  
but it quickly became too much of a hassle to support these compilers as we use **modern C++ constructs** throughout the entire codebase.  
``gcc``, for example, would sometimes simply crash on some really template-heavy code.  
-->

The project's ``CMakeLists.txt`` contains clauses for both ``MSVC`` and ``gcc``,  
so in the future, it might be possible to build the game under these compilers if they catch up with ``clang``.  


Irrespectively of the OS, you will need some dependencies installed to build Hypersomnia:  
 - The newest **CMake**.
 - **git** to clone the respository and later generate version information.
 - Optional: **Python 3.6** or newer for the script that prepares an archive with the executable.
 - Optional: **7-Zip** so that the **Release** configuration can automatically create a compressed archive with the executable and game resources, ready to be sent to someone. 
   - Note that Continuous Integration systems always upload the build artifacts anyway.

Once dependencies are installed, go to the directory where you wish to have your Hypersomnia project downloaded,
open git bash and paste:

```
git clone --depth 1 --recurse-submodules https://github.com/TeamHypersomnia/Hypersomnia
```

The ``--depth 1`` parameters forces a shallow clone which will drastically reduce the download size.
The ``--recurse-submodules`` is necessary to clone the submodules as well.

Once repository finishes downloading, create a ```build/``` folder next to ```CMakeLists.txt``` file.  
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
- **Visual Studio 2017 Preview** (Community) or newer.
- ``ninja`` installed somewhere in PATH.
- [LLVM](https://releases.llvm.org/) 7 toolchain (or newer).
  - For example, use this installer: https://releases.llvm.org/7.0.0/LLVM-7.0.0-win64.exe

Use your favorite shell to go into the newly created ```build/``` folder and run these commands:

```
call "C:\Program Files (x86)\Microsoft Visual Studio\Preview\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
set CONFIGURATION=Release
cmake -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER=lld-link -DARCHITECTURE="x64" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DGENERATE_DEBUG_INFORMATION=0 ..
ninja
```

Note: the path to ``vcvarsall.bat`` in the first line may differ if you're using a version of Visual Studio that is not **Visual Studio 2017 Preview Community**.

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

### Dependencies

- ``git``
- ``cmake``

#### Additional, distro-specific

Arch Linux:

- LLVM:
  - ``clang``
  - ``libc++``
  - ``lld``
- ``pkg-config``
- ``libx11``
- ``libxcb``
- ``xcb-util-keysyms``
- ``ninja``
- ``mbedtls``
- ``libsodium``

Ubuntu:

- LLVM:
  - ``clang-7``
  - ``lld-7``
  - ``libc++-7-dev``
  - ``libc++abi-7-dev``
- ``ninja-build``
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


<!--
#### Compiler toolchain

You will need 
You can go with:

- ``gcc 7.3`` or newer, or...
- ...[``llvm``](https://llvm.org/) toolchain. Required components:
	- ``clang 6.0.1`` or newer
	- ``libc++``
	- ``libc++abi``
	- ``libc++experimental``
	- ``lld``
		- This one is optional, but it speeds up the relink time *a great deal*.

If you don't know which one to choose, ``llvm`` is recommended.  
Here are some test full-rebuild timings for ``Intel(R) Core(TM) i7-4770K CPU @ 3.50GHz``:

```
gcc:
make all -j8 -C build/current  1115.80s user 55.62s system 660% cpu 2:57.26 total
clang with gnu ld:
make all -j8 -C build/current  789.00s user 34.62s system 668% cpu 2:03.24 total
clang with lld:
make all -j8 -C build/current  781.58s user 33.29s system 696% cpu 1:57.04 total
```

LLVM toolchain is expected to yield much faster build times, even on the order of minutes.  
The generated binary is also more performant (e.g. a simple benchmark yielded ``700 FPS`` versus ``800 FPS`` on a default main menu scene)
-->

### One-shot launch

Once the dependencies are all set, this is the complete script for building and launching the game from scratch, with RelWithDebInfo configuration:

```
git clone --depth 1 --recurse-submodules https://github.com/TeamHypersomnia/Hypersomnia
cd Hypersomnia
export CXX=clang++; export CC=clang;
cmake/build.sh RelWithDebInfo x64
ninja run -C build/current
```

#### Details
 
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
- Output a ``$PWD/cache/gen/last_file_path.txt`` file containg the result - which is the chosen path, e.g. ``/home/pbc/projects/my_map``.

# Contributing

Pull requests are welcomed, should they even be typo fixes, missing const guarantees or changes in nomenclature.
If you however plan to add a completely new feature, create a relevant [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues) so that everybody knows about it,
because the project is continuously in a very, very active development and may undergo a revolution when it is the least expected.

For documentation, please make sure to read the [wiki](https://wiki.hypersomnia.xyz) to learn about the game and the source code.

You will be added to [TeamHypersomnia](https://github.com/TeamHypersomnia) organization once at least one of your pull requests is accepted.

If you have questions or you fail to build Hypersomnia, create an [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues).
Or if you just can't wait to utter some brilliant ideas with regard to the game, please do so, too!
