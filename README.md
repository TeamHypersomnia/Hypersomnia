# Hypersomnia

[![Build Status](https://travis-ci.org/TeamHypersomnia/Hypersomnia.svg?branch=master)](https://travis-ci.org/TeamHypersomnia/Hypersomnia)
[![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/5aatwxv8hceaop56?svg=true)](https://ci.appveyor.com/project/geneotech/Hypersomnia)

## Source tree structure

- ```cmake/``` - CMake scripts and source code generators. (Introspector-generator, version_file_generator)
- ```hypersomnia/``` -  directory for the executable. All content needed by Hypersomnia to run properly is present here: images, sounds, shaders, configs and so on. 
- ```src/``` - complete source code of Hypersomnia, along with 3rd party libraries.
  - ```src/3rdparty/``` - 3rd party libraries, upon which the rest of ```src/``` depends.
  - ```src/augs/``` - abstraction of i/o; template code, utility functions, window management.
  - ```src/game/``` - Hypersomnia-specific code that implements the game world. Strictly, just the **model** is present here.
  - ```src/view/``` - Code that is concerned with viewing the game world. Examples: viewables (meant for viewing only, as opposed to logical assets used by the model), state of particle systems, interpolation, playing of sounds, or rendering scripts that take the game world reference and speak directly to OpenGL.
  - ```src/test_scenes/``` - Code generating some simple test scenes, with their needed resources. It exists only in order to conveniently test new game features without relying on the editor. Can be excluded from compilation via BUILD_TEST_SCENES CMake flag.
  - ```src/application/``` - highest level abstraction. Examples: _setups_ implementation, the main menu or the ingame menu overlay, workspace file format, but also collateral things like http server code.
    - ```src/application/setups``` - _setups_ are objects that manage high-level functionality like a client, a server, an editor or a local test scene. They expose functions like ```get_viewed_cosmos()``` or ```get_viewed_character_id()``` that are in turn used by main.cpp to know what world to render and with which entity as the viewer.
  - ```main.cpp``` - that, which straps all of the above together. Initializes libraries, contextes, necessary resources, handles input, selects the setup to work with, keeps track of the single ```audiovisual_state```.
- ```todo/``` - a personal to-do list of the founder. At the moment, not meant to be understood by the public.

### Dependency graph of ```src/```

An arrow from node A to node B denotes that A includes files from B. An arrow to a cluster means that the node may include from all nodes in the cluster.

![enter image description here][2]

### Graph source (DOT language)

```
digraph G {
  compound=true;

  subgraph cluster0 {
    subgraph cluster1 {
      subgraph cluster2 {
        subgraph cluster3 {
          augs->"3rdparty"
          augs->std
          "3rdparty"->std[constraint=false];
        }
        game->augs[lhead=cluster3];
      }
      view->game[lhead=cluster2];
      test_scenes->game[lhead=cluster2];
      test_scenes->view[constraint=false];
    }
    application->test_scenes[lhead=cluster1];
  }
  "main.cpp"->application[lhead=cluster0];
}
```
**Exceptions:**
- Modified Box2D files (from ```3rdparty```) include ```game/transcendental/entity_id.h``` in order to conveniently define a userdata type that contains the id of the entity to whom a ```b2Fixture``` or a ```b2Body``` belongs. Separating that dependency would otherwise involve a lot of alterations to Box2D in terms of code templatization, or unsafe reinterpret casts between ```void*``` and ```entity_id``` types. 

# About
Hypersomnia is an upcoming community-centered shooter/MMORPG released as free software.

from http://hypersomnia.pl/summary/:

Hypersomnia is an upcoming futuristic universe with elements of fast-paced shooter, stealth and role-playing game.
Set in a hypothetical afterlife reality, it shall provide joy through altruistic behaviours and fulfillment of elaborate social roles,
including, but not limited to, sowing panic as a traitor to benevolent ones.


Decide upon your allegiance to one of the three warring factions whose apple of discord is a disparity between prevailing notions of moral excellence.
**Metropolis. Atlantis. Resistance.**

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
  [2]: https://i.imgur.com/SzYA3BA.png

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

One-shot launch:

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

We consider every pull request, should it even be typo fixes, missing const guarantees or changes in nomenclature.

If you however plan to add a completely new feature, please notice us in advance, because the project is continuously in a very, very active development and may undergo a revolution when it is the least expected.
I also recommend that you be familiar with component-based entity architecture beforehand.

You will be added to our TeamHypersomnia organization once we accept at least one of your pull requests.

If you have questions or you fail to build Hypersomnia, ask via mail: patryk.czachurski@gmail.com

Or if you just can't wait to utter some brilliant suggestions regarding the game, please do so, too!
