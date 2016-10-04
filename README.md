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

  [1]: http://hypersomnia.pl/pics/summary.png
  [2]: http://hypersomnia.pl/pics/13.Green-charge.png
  
# How to build
from: http://hypersomnia.pl/join/

To build on Windows platforms, simply open Hypersomnia.sln file and build with Release x86 configuration.
As it currently stands, the project is not ported to any other platform.

If you dream about creating an entirely new mechanic, I recommend that you be familiar with component-based entity architecture beforehand.
Better yet, tell me about your plans! Together we can deal with problems more swiftly.

You will be added to our TeamHypersomnia organization once we accept at least one of your pull requests.

If you have questions, just ask me via mail: patryk.czachurski@gmail.com

Or if you just can't wait to utter some brilliant suggestions regarding the game, please do so, too!

# Launching

You might want to properly configure config.lua before launching the game.

**Explanation of values:**

*launch_mode*:
- **LOCAL** - Will launch the game locally without networking at all.
- **LOCAL_DETERMINISM_TEST** - Same as **LOCAL**, but will launch *determinism_test_cloned_cosmoi_count* copies of the world running in parallel to whom applied are exactly the same inputs. If the worlds differ at some point, the game will hit an assertion.
- **ONLY_CLIENT** - Will use *connect_address* and *connect_port* to connect to a remote host and start the multiplayer simulation.
- **ONLY_SERVER** - Will use *server_port* to setup a listenserver without a game client.
- **CLIENT_AND_SERVER** - **ONLY_SERVER** and **ONLY_CLIENT** in the same process.
- **TWO_CLIENTS_AND_SERVER** - **ONLY_SERVER** and two clients on split-screen. For debugging purposes. The server will use *alternative_server_port* for the second connection.

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

*test_var*:
- Reserved for experimental use, don't touch.

*test_randomize_entropies_in_client_setup*:
- Used by the server to inject random inputs to the other players to examine and test lag compensation strategies.

*test_randomize_entropies_in_client_setup_once_every_steps*:
- How often the above input injection happens. The less it is, the more erratic the movements are.

*server_launch_http_daemon*:
- Flag. **1** will launch a http daemon on the localhost in a separate thread which samples the server statistics. Used as a widget on http://hypersomnia.pl

*server_http_daemon_port*:
- What port to open the web daemon on. Recommended value: **80**.

*server_http_daemon_html_file_path*:
- Format of the broadcasted widget.
