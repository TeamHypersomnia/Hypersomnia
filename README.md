# Hypersomnia

[![Build Status](https://travis-ci.org/TeamHypersomnia/Hypersomnia.svg?branch=master)](https://travis-ci.org/TeamHypersomnia/Hypersomnia)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/5aatwxv8hceaop56?svg=true)](https://ci.appveyor.com/project/geneotech/Hypersomnia)

*([Our CI is due to be overhauled](https://github.com/TeamHypersomnia/Hypersomnia/issues/258). Currently, the game is only buildable under clang 6.0.0 and gcc 7.3.1; or newer ones.)*
- [Hypersomnia](#hypersomnia)
- [Gallery](#gallery)
- [How to build](#how-to-build)
  - [Windows](#windows)
  - [Linux](#linux)
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

See the game's [wiki](http://wiki.hypersomnia.xyz) to get familiar with the universe.  
To understand the repository's folder structure, make sure to read the [documentation](http://wiki.hypersomnia.xyz/docs).

## Gallery

Watch gameplays on YouTube:

[![IMAGE ALT TEXT](http://img.youtube.com/vi/mxvuWvD53tY/0.jpg)](http://www.youtube.com/watch?v=mxvuWvD53tY "Video Title")
[![IMAGE ALT TEXT](http://img.youtube.com/vi/f0cHnds9UuU/0.jpg)](http://www.youtube.com/watch?v=f0cHnds9UuU "Video Title")
[![IMAGE ALT TEXT](http://img.youtube.com/vi/XsSKj6hJH0w/0.jpg)](http://www.youtube.com/watch?v=XsSKj6hJH0w "Video Title")

![enter image description here][8]
![enter image description here][3]
![enter image description here][4]

  [8]: https://gifyu.com/images/16.main_menu_reup.png
  [3]: http://gifyu.com/images/23.light.png
  [4]: http://gifyu.com/images/30.smoke.png

# How to build
To build Hypersomnia, you will need some dependencies installed on your system:
 - The newest **CMake**.
 - **git** to clone the respository and later generate version information.
 - Optional: **7-Zip** so that the **Release** configuration can automatically create a compressed archive with the executable and game resources, ready to be sent to someone. 
 - Optional: **Python 3.6** or newer for the script that prepares an archive with the executable.

Once installed, go to the directory where you wish to have your Hypersomnia project downloaded,
open git bash and paste:

```
git clone https://github.com/TeamHypersomnia/Hypersomnia.git --recursive
```

The repository will start downloading. Once complete, create a ```build/``` folder next to ```CMakeLists.txt``` file.  
Next steps depend on the platform you are on.

## Windows

You will need **Visual Studio 2017 Preview** or newer.
Use your favorite shell to go into the newly created ```build/``` folder and run:

```
cmake ..
```

If you want to somehow customize your build, refer to the beginning of ```CMakeLists.txt``` to see which options you can pass to the ```cmake``` command.

Resultant ```.sln``` and ```.vcxproj``` files should appear in the ```build/``` directory.
Open ```Hypersomnia.sln``` file, select **Release** configuration and hit **F7** to build the game.
**F5** should launch it.

If, for some reason, some step fails, refer to the latest working Appveyor build and the relevant ```appveyor.yml``` file.

## Linux

Current platforms are actively supported:
- Arch Linux with i3 window manager
- Ubuntu 14.04 trusty, but it was only tested via Travis - the unit tests all pass.

### Dependencies

#### Tools

- ``git``
- ``cmake``
- ``ninja``

#### Build dependencies

- ``pkg-config``
- ``libx11``
- ``libxcb``
- ``xcb-util-keysyms``

On Ubuntu:
- ``alsa-oss``
- ``libxcb-keysyms1-dev``

#### Compiler toolchain

You can go with:

- ``gcc 7.3`` or newer, or...
- ...[``llvm``](http://llvm.org/) toolchain. Required components:
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

### One-shot launch

```
git clone https://github.com/TeamHypersomnia/Hypersomnia --recursive
cd Hypersomnia
cmake/build.sh RelWithDebInfo x64
ninja run -C build/current
```

### Detailed instructions
 
Use your favorite shell to enter the repository's directory.
Then run:

```
cmake/build.sh [Debug|Release|RelWithDebInfo|MinSizeRel] [x86|x64] [C_COMPILER CXX_COMPILER] ["ADDITIONAL CMAKE FLAGS"]
```
For example:

```
cmake/build.sh Debug x64 clang clang++
```
After which, the resultant Makefile should appear in the build/Debug-x64-clang directory.

Example for gcc:

```
cmake/build.sh Debug x64 gcc g++
```

After which, the resultant Makefile should appear in the build/Debug-x64-gcc directory.

#### Invoking ninja

If you are building with ``clang``, make sure to call these exports before invoking ``ninja``:

```
export CXX=clang++; export CC=clang;
```

This is because some third-party libraries - freetype, for example - generate their Makefiles only after calling ``ninja`` on the Hypersomnia's ``build.ninja`` itself.
(if GCC build fails for some reason, add respective exports as well.)

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

The Hypersomnia editor can open files for editing and save them.  
On Windows, this is accomplished through [GetOpenFileName](https://msdn.microsoft.com/en-us/library/windows/desktop/ms646927(v=vs.85).aspx).  
Needless to say, such a thing does not exist on Linux.  
Hypersomnia provides bash scripts for common file managers in ``hypersomnia/scripts/unix/managers``.  
Choose one for opening and one for saving, then ``cd`` to ``hypersomnia/scripts/unix`` and, assuming you want to use ``ranger`` as your file manager, create symlinks as such: 

- ``ln -s managers/save_file_ranger.zsh save_file.local``
- ``ln -s managers/open_file_ranger.zsh open_file.local``
- ``ln -s managers/choose_directory_ranger.zsh choose_directory.local``
- ``ln -s managers/select_file_ranger.zsh select_file.local`` (e.g. for when there is a need to reveal the file in explorer)

The symlinks will not be tracked by git.  

Currently, the following file managers are supported:
- [ranger](https://github.com/ranger/ranger) through ``--choosefile`` option

The scripts use ``$TERMINAL`` variable for file managers that need a terminal to run on. Ensure your terminal supports ``-e`` flag that passes the commands to launch on startup. 

If you want to implement your own script, the only thing it is required to do is creating a ``$PWD/cache/gen/last_file_path.txt`` file containg the path to the file to be opened or saved to by the editor.

# Contributing

Pull requests are welcomed, should they even be typo fixes, missing const guarantees or changes in nomenclature.
If you however plan to add a completely new feature, create a relevant [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues) so that everybody knows about it,
because the project is continuously in a very, very active development and may undergo a revolution when it is the least expected.

For documentation, please make sure to read the [wiki](http://wiki.hypersomnia.xyz) to learn about the game and the source code.

You will be added to [TeamHypersomnia](https://github.com/TeamHypersomnia) organization once at least one of your pull requests is accepted.

If you have questions or you fail to build Hypersomnia, create an [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues).
Or if you just can't wait to utter some brilliant ideas with regard to the game, please do so, too!
