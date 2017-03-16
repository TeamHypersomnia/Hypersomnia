# Hypersomnia
Community-centered shooter/MMORPG released as free software.

from: http://hypersomnia.pl/summary/

Hypersomnia is an upcoming futuristic universe with elements of fast-paced shooter, stealth and role-playing game.
Set in a hypothetical afterlife reality, it shall provide joy through altruistic behaviours and fulfillment of elaborate social roles,
including, but not limited to, sowing panic as a traitor to benevolent ones.



Decide upon your allegiance to one of the three warring factions whose apple of discord is a disparity between prevailing notions of moral excellence.
**Metropolis. Atlantis. Resistance.**

Watch gameplay with all current features on YouTube:

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
from: http://hypersomnia.pl/join/

Go to the directory where you wish to have your Hypersomnia project downloaded,
open git bash and paste:

```
git clone https://github.com/TeamHypersomnia/Hypersomnia.git --recursive
```

The repository will start downloading. Once complete, open Hypersomnia.sln file, select **Release x86** configuration and hit **F7** to build.

As it currently stands, the game can only be built in Visual Studio 2017 or higher (compilers from Visual Studio 2015 do not support some of the language features I'm using) and can only run on Windows systems.

If you dream about creating an entirely new mechanic, I recommend that you be familiar with component-based entity architecture beforehand.
Better yet, tell me about your plans! Together we can deal with problems more swiftly.

You will be added to our TeamHypersomnia organization once we accept at least one of your pull requests.

If you have questions, just ask me via mail: patryk.czachurski@gmail.com

Or if you just can't wait to utter some brilliant suggestions regarding the game, please do so, too!

# Contributing

We fondly welcome every pull request, should it even be a typo fix or a missing const guarantee.

If you however plan to add a completely new feature, please notice us in advance, because the project is continuously in a very, very active development and may undergo a revolution when it is the least expected.

[Mail](mailto:patryk.czachurski@gmail.com) or [Steam](http://steamcommunity.com/id/hypersomnialeaddev/).

# Launching

You might want to properly configure some variables before launching the game.
- Clone the **config.lua file** and name it **config.local.lua** so that it stays unversioned and unique to your filesystem, if for example you want to preserve your original window resolution and coordinates across further commits.
- The game will try to read **config.local.lua** and if there is no such file, it shall try loading **config.lua**.

**Explanation of config values is found within config.lua file.**

After that, just go to output/ folder and open Hypersomnia.exe or Hypersomnia-Debug.exe for debugging.
