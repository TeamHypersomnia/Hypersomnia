# For developers

Linux: [![Build Status](https://travis-ci.org/TeamHypersomnia/Hypersomnia.svg?branch=master)](https://travis-ci.org/TeamHypersomnia/Hypersomnia)

Windows: [![Appveyor Build Status](https://ci.appveyor.com/api/projects/status/5aatwxv8hceaop56?svg=true)](https://ci.appveyor.com/project/geneotech/Hypersomnia)


**Tree structure:**

- ```cmake/``` - CMake scripts and source code generators. (e.g. generated introspectors or a source file with the commit number)
- ```hypersomnia/``` - all content needed by the Hypersomnia executable to run properly. Images, sounds, shaders, configs and so on. 
- ```src/``` - complete source code of Hypersomnia, along with 3rd party libraries.
  - ```src/3rdparty``` - 3rd party libraries, upon which the rest of ```src/``` depends.
  - ```src/augs/``` - abstraction of i/o; template code, utility functions, window management.
  - ```src/game/``` - Hypersomnia-specific code that implements the game world. Strictly, the **model** is present here, and nothing else.
  - ```src/view/``` - Code that is concerned with viewing the game world. Examples: viewables (meant for viewing only, as opposed to logical assets used by the model), state of particle systems, interpolation, playing of sounds, or rendering scripts that take the game world reference and speak directly to OpenGL.
  - ```src/test_scenes/``` - Code generating some simple test scenes, with their needed resources. It exists only in order to conveniently test new game features without relying on the editor. Can be excluded from compilation via BUILD_TEST_SCENES CMake flag.
  - ```src/application/``` - highest level abstraction. Examples: _setups_ implementation, the main menu or the ingame menu overlay, the main loop helpers, but also collateral things like http server code.
    - ```src/application/setups``` - _setups_ are objects that manage high-level functionality like a client, a server, an editor or a local test scene. They expose functions like ```get_viewed_cosmos()``` or ```get_viewed_character_id()``` that are in turn used by main.cpp to know what world to render and with which entity as the viewer.
  - ```main.cpp``` - that, which straps all of the above together. Initializes libraries, contextes, necessary resources, handles input, selects the setup to work with, keeps track of the single ```audiovisual_state```.
- ```todo/``` - a personal to-do list of the founder. At the moment, not meant to be understood by the public.

**Dependency graph of ```src/```:**

An arrow from node A to node B denotes that A includes files from B. An arrow to a cluster means that the node may include from all nodes in the cluster.

![enter image description here][2]

**Graph source (DOT language):**

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
- Modified Box2D files (from ```3rdparty```) include ```game/transcendental/entity_id.h``` in order to conveniently define a userdata type that contains the id of the entity to whom a ```b2Fixture``` or a ```b2Body``` belongs. Separating that dependency would otherwise involve a lot of alterations to Box2D in terms of code templatization, or unsafe reinterpret casts between ```void*``` and my types. 

# Hypersomnia
Community-centered shooter/MMORPG released as free software.

from: http://hypersomnia.pl/summary/

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
To build Hypersomnia, you will need **CMake 3.8** or newer.
You might also need **Python** installed on your system due to requirements of some CMake scripts.
Optionally, you can also install **7-Zip** so that the **Release** configuration can automatically create a compressed archive with the executable and game resources. 

Go to the directory where you wish to have your Hypersomnia project downloaded,
open git bash and paste:

```
git clone https://github.com/TeamHypersomnia/Hypersomnia.git --recursive
```

The repository will start downloading. Once complete, create a ```build/``` folder next to ```CMakeLists.txt``` file. 
Then, use your favorite shell to go into the newly created ```build/``` folder and run:

```
cmake ..
```

Alternatively, if you want to build the minimal possible Hypersomnia runtime, if you're for example trying to build for a different platform, run:

```
cmake -DHYPERSOMNIA_BARE=ON ..
```
This will disable all third party code and build Hypersomnia executable reliant exclusively on the standard C++. Refer to ```CMakeLists.txt``` to see which other options are available.

If you are on Windows, resultant ```.sln``` and ```.vcxproj``` files should appear in the ```build/``` directory.
Open ```Hypersomnia.sln``` file, select **Release** configuration and hit **F7** to build the game.

As it currently stands, the game is known to build successfully only with Visual Studio 2017 **Preview** under Windows systems. Compilers from Visual Studio 2015 do not support some of the C++ language features.

# Contributing

We fondly welcome every pull request, should it even be a typo fix or a missing const guarantee.

If you however plan to add a completely new feature, please notice us in advance, because the project is continuously in a very, very active development and may undergo a revolution when it is the least expected.
I also recommend that you be familiar with component-based entity architecture beforehand.

You will be added to our TeamHypersomnia organization once we accept at least one of your pull requests.

If you have questions or you fail to build Hypersomnia, ask via mail: patryk.czachurski@gmail.com

Or if you just can't wait to utter some brilliant suggestions regarding the game, please do so, too!

# Launching

You might want to properly configure some variables before launching the game.
- Clone the ```config.lua``` and name it ```config.local.lua``` so that it stays unversioned and unique to your filesystem, if for example you want to preserve your original window resolution and coordinates across further commits.
- The game will try to read ```config.local.lua``` and if there is no such file, it shall try loading ```config.lua```.
