# Building Hypersomnia

- [Windows instructions](#windows-instructions)
	- [Method 1: Visual Studio project files](#visual-studio-project-files)
	- [Method 2: CMake with ninja](#cmake-with-ninja)
- [Linux instructions](#linux-instructions)
- [Distribution-specific dependencies](#distribution-specific-dependencies)
- [One-shot launch](#one-shot-launch)
- [Detailed instructions](#detailed-instructions)
- [File dialogs integration](#file-dialogs-integration)
- [MacOS instructions](#macos-instructions)

Currently, *Hypersomnia* is only buildable using `clang` or `MSVC`.
Your operating system must be 64-bit.  

<!--
	Formerly, the game was buildable under modern ``gcc`` versions,  
	and also using ``MSVC`` (the Microsoft's compiler shipping with Visual Studio),  
	but it quickly became too much of a hassle to support these compilers as we use **modern C++ constructs** throughout the entire codebase.  
	``gcc``, for example, would sometimes simply crash on some really template-heavy code.  

	The project's ``CMakeLists.txt`` contains clauses for both ``MSVC`` and ``gcc``,  
	so in the future, it might be possible to build the game under these compilers if they catch up to ``clang``.  
-->

Irrespectively of the OS, you will need the following software in order to build *Hypersomnia*:  

> [!TIP]
> You may skip installing CMake, LLVM and Ninja if you choose the [convenient method with the Windows `.sln` files](#visual-studio-project-files)

- **git** to clone the respository and later generate version information.
- The newest **CMake**.
- [**ninja**](https://ninja-build.org/) to carry out the build.
- [LLVM](https://releases.llvm.org/) toolchain version 17.0.6.
	- For Windows, you can use [this installer](https://github.com/llvm/llvm-project/releases/download/llvmorg-17.0.6/LLVM-17.0.6-win64.exe), or a newer one. 
	- For Linux, use your distro-specific package. Make sure to install ```libc++```, ```libc++abi``` and ```lld``` as well.
	- For MacOS, the version that comes pre-installed with **Xcode** is good enough.
- [OpenSSL](https://www.openssl.org/) needed by the auto-updater to download latest game binaries over HTTPS.
  - On Windows, you can get the appropriate installer here: https://slproweb.com/download/Win64OpenSSL-3_2_2.msi
    - Link broken? Other OpenSSL versions can be found here: https://slproweb.com/products/Win32OpenSSL.html
      - The game should generally build with any OpenSSL version 3.0 or later.
    - For easy access, install it to ``C:\OpenSSL-v32-Win64``.
    - If CMake can't find OpenSSL (despite passing proper ``-DOPENSSL_ROOT_DIR=C:\OpenSSL-v32-Win64``, **make sure to update CMake**. There were recently many issues with CMake not finding OpenSSL libraries, but newest CMake should already work fine.

Once dependencies are installed, go to the directory where you wish to have your *Hypersomnia* project downloaded,
open git bash and paste:

```
git clone --recurse-submodules https://github.com/TeamHypersomnia/Hypersomnia
```

The ``--recurse-submodules`` is necessary to clone the submodules as well.

Wait for the download to complete.
Next steps depend on the platform you are on.

On all platforms, you can choose among three building configurations:

- ``Debug`` - the fastest to build and provides debug information.   
	Recommended for day-to-day development.
- ``Release`` - No debug information. Use only for production builds. Specifies ``IS_PRODUCTION_BUILD=1`` C++ preprocessor define that disables assertions in performance-critical areas.
- ``RelWithDebInfo`` - Same as ``Release`` but with debug info and with many assertions ("ensures") compiled-in.
	Preferred choice for testing the developed game while full speed is required.

## Windows instructions

**If it doesn't work, refer to [Windows_build.yml](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/.github/workflows/Windows_build.yml) file for the up-to-date building procedure.**

Prerequisites:
- **Visual Studio 2022 Community** or newer.

### Visual Studio project files

For convenience, the CI/CD now includes pre-generated `.sln` and `.vcxproj` files for building Hypersomnia.
You can download them here:

https://nightly.link/TeamHypersomnia/Hypersomnia/workflows/Windows_build/master/Hypersomnia-sln.zip

If the link is dead, this means there was no commit in the last 90 days - you might want to just fork the project and run the GitHub action yourself.

### CMake with ninja

Open up the standard Windows ``cmd`` prompt (it **won't work** with PowerShell or others). Setup the environment:

```
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

(If the file cannot be found, it means you are not using Visual Studio 2022 Community. You will have to look for a corresponding ``vcvarsall.bat`` location on your own.)

Next, run these commands:

```
cd Hypersomnia
mkdir build
cd build
set CONFIGURATION=RelWithDebInfo
cmake -G Ninja -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DCMAKE_LINKER=lld-link -DARCHITECTURE="x64" -DCMAKE_BUILD_TYPE=%CONFIGURATION% -DGENERATE_DEBUG_INFORMATION=0 -DOPENSSL_ROOT_DIR=C:\OpenSSL-Win64 ..
ninja
```

This will build a non-Steam client by default.
To build a Steam client, add a ``-DLINK_STEAM_INTEGRATION=1`` flag to the ``cmake`` command.

Note: your computer **might start lagging heavily** for the duration of the build as ``ninja`` will use all available cores for compilation.

If you intend to develop the game, it is best to use "Debug" configuration for the fastest builds.

If you want to somehow customize your build, e.g. disable certain game features, refer to the beginning of ```CMakeLists.txt``` to see which options you can pass to the ```cmake``` command.

If the game builds successfully, run this command to launch it:

```
ninja run
```

If you built with ``-DLINK_STEAM_INTEGRATION=1``, **don't forget to create a ``hypersomnia/steam_appid.txt`` file** with ``2660970`` in it.
Otherwise the game will try to restart itself through Steam.

<!-- Note that the 64-bit version is more likely to be kept up to date. -->

<!--
Resultant ```.sln``` and ```.vcxproj``` files should appear in the ```build/``` directory.
Open ```Hypersomnia.sln``` file, select **Release** configuration and hit **F7** to build the game.
**F5** should launch it.
-->

If, for some reason, some step fails, refer to the latest working Appveyor build and the relevant ```appveyor.yml``` file.

## Linux instructions

Refer to [Linux_build.yml](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/.github/workflows/Linux_build.yml) file for up-to-date building procedure - it is constantly in flux. A short overview of the process now follows.

Current platforms are actively tested and supported:

- Arch Linux with i3 window manager - the developer's machine.
- Ubuntu, as this is where the dedicated server is deployed.

#### Distribution-specific dependencies

Arch Linux:

- ``libc++ lld pkg-config libx11 libxcb xcb-util-keysyms libsodium``
- Might need more - [let us know](https://github.com/TeamHypersomnia/Hypersomnia/issues) if this list is missing something.

Ubuntu:

```
sudo apt-get install cmake ninja-build libxcb-keysyms1 libxcb-keysyms1-dev libxi6 libxi-dev alsa-oss osspd-alsa osspd libasound2 libasound2-dev p7zip p7zip-full libgl1-mesa-dev libxcb-glx0-dev libx11-xcb-dev
```

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

If, for some reason, some step fails, refer to the latest working ```Linux_build.yml``` file.

If the game fails to launch, it should automatically open a log file with the relevant message using ```$VISUAL``` executable.

### File dialogs integration

On Linux, the *Hypersomnia* editor uses standard tools for file dialogs and revealing files in the file manager:

- **File dialogs** (open file, save file, choose directory): Uses ``zenity`` (GTK) or ``kdialog`` (KDE) if available. Most desktop environments include at least one of these.
- **Reveal in file manager**: Uses ``xdg-open`` to open the parent directory of a file.

To ensure file dialogs work, install at least one of the following:
```
# For GNOME/GTK-based desktops:
sudo apt install zenity

# For KDE-based desktops:
sudo apt install kdialog
```

## MacOS instructions

Refer to [MacOS_build.yml](https://github.com/TeamHypersomnia/Hypersomnia/blob/master/.github/workflows/MacOS_build.yml) file for up-to-date instructions.

