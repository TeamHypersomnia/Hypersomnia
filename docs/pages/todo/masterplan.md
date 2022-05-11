---
title: The masterplan
tags: [planning]
hide_sidebar: true
permalink: masterplan
summary: We need to set our priorities straight.
---

Top-level topics that are most important.

# Builder

## Map format

### Safety

**Choice of the text serialization format matters ONLY INSOFAR AS STORING ON HDD/VERSIONING IS CONCERNED.**
Look - even if you have two separate data formats:
a) One for the map scheme from which the proper binary is built;
b) and the low-level binary one that is understood by cosmos.cpp

And you need to send a) - you can always send the in-memory binary representation.
The game will then safely be able to convert it back to yaml for its own purpose.

Therefore saving to/loading from yaml will be only relevant when saving maps that are easily versioned.

### Preserving compatibility  

Our main issue with storing the maps binary was that any change in the binary structure would break them.
This is why 
a) We will have a separate generator format from which the proper binary maps will be generated in runtime
b) We will store the map files *TEXTUALLY* (in yaml probably) to be able to easily convert the maps
Being able to nicely version the maps is a plus.

We will always ask the user if they want to convert the map to the higher version,
if a new version of the game is detected.

We might need incremental update procedures. 
Perhaps we won't do this with just a simple find/replace (or it might affect the string),
but with some yaml node traversal logic so that only keys are affected.
- Anyways, this is for later! Once we actually have lots of community maps.

# Better controls

