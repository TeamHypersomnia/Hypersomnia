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
- **LOCAL** - will launch the game locally without networking at all.
- **LOCAL_DETERMINISM_TEST** - same as **LOCAL**, but will launch *determinism_test_cloned_cosmoi_count* copies of the world running in parallel to whom applied are exactly the same inputs. If the worlds differ at some point, the game will hit an assertion.
- **ONLY_CLIENT** - will use value *connect_address* and *connect_port* to connect to a remote host and start the multiplayer simulation.
- **ONLY_SERVER** - will use *server_port* to setup a listenserver without a game client.
- **CLIENT_AND_SERVER** - **ONLY_SERVER** and **ONLY_CLIENT** in the same process.
- **TWO_CLIENTS_AND_SERVER** - **ONLY_SERVER** and two clients on split-screen. For debugging purposes. The server will use *alternative_server_port* for the second connection.

*debug_disable_cursor_clipping*:
- Flag. **1** disables the cursor clipping so that it is easier to mark a breakpoint, for example. **0** for normal playing.

*tickrate*:
- Frequency of the simulation. **1/tickrate** equals the fixed delta time in seconds, so tickrate = 60 means that the logical step advances the simulation about around **16 milliseconds**.
