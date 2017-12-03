# Hypersomnia

[![Build Status](https://travis-ci.org/TeamHypersomnia/Hypersomnia.svg?branch=master)](https://travis-ci.org/TeamHypersomnia/Hypersomnia)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/5aatwxv8hceaop56?svg=true)](https://ci.appveyor.com/project/geneotech/Hypersomnia)

- [Hypersomnia](#hypersomnia)
- [Gallery](#gallery)
- [How to build](#how-to-build)
  - [Windows](#windows)
  - [Linux](#linux)
    - [One-shot launch](#one-shot-launch)
    - [Detailed instructions](#detailed-instructions)
- [Contributing](#contributing)

Hypersomnia is an upcoming community-centered shooter released as free software,
with an aspiration to one day become an MMO with elements of RPG.

Set in a hypothetical afterlife reality, it shall provide joy through altruistic behaviours, fierce fights, exploration, fulfillment of elaborate social roles or sowing panic as a traitor to benevolent ones.

Decide upon your allegiance to one of the three warring factions whose apple of discord is a disparity between prevailing notions of moral excellence.
**Metropolis. Atlantis. Resistance.**

See the game's [wiki](http://wiki.hypersomnia.pl) to get familiar with the universe.  
To understand the repository's folder structure, make sure to read the [documentation](http://wiki.hypersomnia.pl/docs).

## Gallery

Watch gameplays on YouTube:

[![IMAGE ALT TEXT](http://img.youtube.com/vi/f0cHnds9UuU/0.jpg)](http://www.youtube.com/watch?v=f0cHnds9UuU "Video Title")
[![IMAGE ALT TEXT](http://img.youtube.com/vi/XsSKj6hJH0w/0.jpg)](http://www.youtube.com/watch?v=XsSKj6hJH0w "Video Title")

![enter image description here][1]
![enter image description here][8]
![enter image description here][3]
![enter image description here][4]

  [1]: http://hypersomnia.pl/pics/summary.png
  [8]: https://gifyu.com/images/16.main_menu_reup.png
  [3]: http://gifyu.com/images/23.light.png
  [4]: http://gifyu.com/images/30.smoke.png

# How to build
To build Hypersomnia, you will need some dependencies installed on your system:
 - **CMake 3.2** or newer.
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

Hypersomnia has currently been tested on:
- Arch Linux with i3 window manager 

Additional dependencies:
- gcc 7.2 or newer
- libx11
- libxcb
- xcb-util-keysyms

### One-shot launch

```
git clone https://github.com/TeamHypersomnia/Hypersomnia --recursive
cd Hypersomnia
cmake/build.sh Release x64
make run -j4 -C build/Release-x64
```

### Detailed instructions
 
Use your favorite shell to enter the repository's directory.
Then run:

```
cmake/build.sh [Debug|Release|RelWithDebInfo|MinSizeRel] [x86|x64] ["ADDITIONAL CMAKE FLAGS"]
```
For example:

```
cmake/build.sh Debug x64
```
After which, the resultant Makefile should appear in the build/Debug-x64 directory.
There are several additional make targets defined:

```
make run
```
Launches the game normally.

```
make tests
```
Launches unit tests only and exits cleanly.

```
make debug
```
Launches the game through ```cgdb```.

```
make memdeb
```
Launches the game through ```valgrind```.

All the above targets set the working directory automatically to ```${PROJECT_SOURCE_DIR}/hypersomnia```.
Remember to pass ``-j4`` or so to speed up the build.

If, for some reason, some step fails, refer to the latest working Travis build and the relevant ```travis.yml``` file.

If the game fails to launch, it should automatically open a log file with the relevant message using ```$VISUAL``` executable.

# Contributing

Pull requests are welcomed, should they even be typo fixes, missing const guarantees or changes in nomenclature.
If you however plan to add a completely new feature, create a relevant [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues) so that everybody knows about it,
because the project is continuously in a very, very active development and may undergo a revolution when it is the least expected.

For documentation, please make sure to read the [wiki](http://wiki.hypersomnia.pl) to learn about the game and the source code.

You will be added to [TeamHypersomnia](https://github.com/TeamHypersomnia) organization once at least one of your pull requests is accepted.

If you have questions or you fail to build Hypersomnia, create an [issue](https://github.com/TeamHypersomnia/Hypersomnia/issues).
Or if you just can't wait to utter some brilliant ideas with regard to the game, please do so, too!
