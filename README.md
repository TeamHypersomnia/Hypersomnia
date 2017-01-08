# Hypersomnia
Community-centered shooter/MMORPG released as free software.

from: http://hypersomnia.pl/summary/

Hypersomnia is an upcoming futuristic universe with elements of fast-paced shooter, stealth and role-playing game.
Set in a hypothetical afterlife reality, it shall provide joy through altruistic behaviours and fulfillment of elaborate social roles,
including, but not limited to, sowing panic as a traitor to benevolent ones.



Decide upon your allegiance to one of the three warring factions whose apple of discord is a disparity between prevailing notions of moral excellence.
**Metropolis. Atlantis. Resistance.**

![enter image description here][1]
![enter image description here][2]
![enter image description here][3]
![enter image description here][4]

  [1]: http://hypersomnia.pl/pics/summary.png
  [2]: http://gifyu.com/images/16.main_menu.png
  [3]: http://gifyu.com/images/23.light.png
  [4]: http://gifyu.com/images/30.smoke.png
  
# Opening the to-do list

**As of currently, I advise against modifying the files inside ToDo/. It is because even the smallest change yields a completely different file which makes it impossible to merge, should a conflict arise. 
We will switch to some other solution soon.** However, feel free to read the list.

The official to-do list of this project is located in ToDo/todo.tdl.

To properly view it, you have to download this hierarchical task manager:
http://www.codeproject.com/Articles/5371/ToDoList-An-effective-and-flexible-way-to-keep-on

Of importance are:
- **Hypersomnia->Doing->Current tasks** - tasks in progress.
- **Hypersomnia->Doing->ToDo** - tasks that may be undertaken at any moment.
- **Hypersomnia->Doing->Scratchpad** - low priority things that will need to be done eventually in some distant future.
- **Hypersomnia->Doing->Done** - completed tasks.
  
# How to build
from: http://hypersomnia.pl/join/

Go to the directory where you wish to have your Hypersomnia project downloaded,
open git bash and paste:

```
git clone https://github.com/TeamHypersomnia/Hypersomnia.git --recursive
```

The repository will start downloading. Once complete, open Hypersomnia.sln file, select **Release x86** configuration and hit **F7** to build.

As it currently stands, the game can only be built in Visual Studio 2015 or higher and can only run on Windows systems.

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

**Explanation of values:**

*launch_mode*:
- **LOCAL** - Will launch the game locally without networking at all.
- **LOCAL_DETERMINISM_TEST** - Same as **LOCAL**, but will launch *determinism_test_cloned_cosmoi_count* copies of the world running in parallel to whom applied are exactly the same inputs. If the worlds differ at some point, the game will hit an assertion.
- **ONLY_CLIENT** - Will use *connect_address* and *connect_port* to connect to a remote host and start the multiplayer simulation.
- **ONLY_SERVER** - Will use *server_port* to setup a listenserver without a game client.
- **CLIENT_AND_SERVER** - **ONLY_SERVER** and **ONLY_CLIENT** in the same process.
- **TWO_CLIENTS_AND_SERVER** - **ONLY_SERVER** and two clients on split-screen. For debugging purposes. The server will use *alternative_port* for the second connection.

*determinism_test_cloned_cosmoi_count*:
- See **LOCAL_DETERMINISM_TEST**.

*window_name,
fullscreen,
window_border,
window_x,
window_y,
bpp,
resolution_w,
resolution_h,
doublebuffer:*
- Self explanatory.

*debug_disable_cursor_clipping*:
- Flag. **1** disables the cursor clipping so that it is easier to mark a breakpoint, for example. **0** is for normal playing.

*mouse_sensitivity*:
- vec2. Sensitivity of mouse movement in-game.

*connect_address, connect_port, server_port, alternative_port*:
- Network variables. See *launch_mode* for details.

*nickname*:
- Client-chosen nickname of the controlled character.

*debug_second_nickname*:
- Client-chosen nickname of the second controlled character used in **launch_mode.TWO_CLIENTS_AND_SERVER**.

*tickrate*:
- Frequency of the simulation. **1/tickrate** equals the fixed delta time in seconds, so tickrate = 60 means that the logical step advances the simulation about around **16 milliseconds**.

*jitter_buffer_ms*:
- Client-side jitter buffer time to preserve smooth display of the past. The bigger the value, the bigger the lag.

*client_commands_jitter_buffer_ms*:
- Server-side jitter buffer time for client commands. Useful for lag simulation.

*debug_var*:
- Reserved for experimental use, don't touch.

*debug_randomize_entropies_in_client_setup*:
- Used by the server to inject random inputs to the other players to examine and test lag compensation strategies.

*debug_randomize_entropies_in_client_setup_once_every_steps*:
- How often the above input injection happens. The less it is, the more erratic the movements are.

*server_launch_http_daemon*:
- Flag. **1** will launch a http daemon on the localhost in a separate thread which samples the server statistics. Used as a widget on http://hypersomnia.pl

*server_http_daemon_port*:
- What port to open the web daemon on. Recommended value: **80**.

*server_http_daemon_html_file_path*:
- Format of the broadcasted widget.
