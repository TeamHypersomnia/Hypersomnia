During a playtest, you can deterministically record and replay your gameplay.

This is useful if you want to replay some server session,
troubleshoot simulation determinism problems,
or you want to record a scene for the main menu.

**ToDo: Implement opening and saving recordings.**

## Controls

- While paused, press + on your numpad to move a simulate a single step forward.
- While paused, press - on your numpad to move a move a single step backward.
- Press i to start recording the game controlling the chosen character.
	- As the game advances, this (obviously) overwrites any game inputs made on the controlled entity.
- Press l to start replaying the in **read-only mode**, thus making no changes at all.
	- The property tweaks (of flavours, entities, rulesets etc.) will also be disabled whenever you replay.
- Press numbers on your numpad: 0, 1, 2, 3, 4, 5 to control the game's speed.
	- 0 resets speed back to 1x.
	- 1 sets speed to 0.01x.
	- 2 sets speed to 0.05x.
	- 3 sets speed to 0.1x.
	- 4 sets speed to 0.5x.
	- 5 sets speed to 2.0x.

Whenever recording or replaying,  
the camera will follow the character only if it is fixed on the player position by pressing HOME. 

## Player dialog

You can use the player dialog to perform additional actions.

- Use the timeline to seek to any moment of the recording. It might take a while if you seek backwards.
	- The timeline will always be trimmed to the moment where the last input was detected.
	  it will disappear if no inputs were made at all.

## Behaviour of history of changes

In order to always be able to deterministically replay any game session,
the editor player has to take into account all tweaks to properties that you make while playtesting.
Therefore, any change done during a playtest will be tied to the current step of the editor player.
	- e.g. a change in fire rate of a gun or a tweak to some ruleset.
This means that if you ever undo or redo a change from history, 
the editor player will seek to the world step when the change was initially made.
This also means that if you seek the editor player, history commands will be accordingly undone or redone.

## Snapshots

The sole purpose of snapshots is to be able to go back in time,
since the game logic is only implemented to move forward.
From time to time, the editor player will clone the entire game state and store it 
if you'd ever want to seek the player backwards.

To configure the snapshot interval,
go to Editor settings->Player.
The lesser the interval, the more snapshots will reside in memory but the faster the backward seeks will be.
The greater the interval, the less snapshots will reside in memory but the longer the backward seeks will be

The Player dialog will show a reasonable estimate of how much RAM space is currently occupied by the stored snapshots.

## Input overwrite options

The options to overwrite only particular kinds of inputs let you record mouse movements and keyboard presses in separate stages.

For example, if you'd want to choreograph a firefight, you might want to record the first character shooting air,
then make the second character walk into that fire while also shooting the first character,
and then only adjust the crosshair movements of both characters so that they actually shoot each other correctly.

'Intents' stand for actions tied to button presses, like shots, reloads and character movements.
'Motions' stand for actions tied to mouse movement, like aiming with the crosshair.
'Mode commands' stand for actions like item purchases or team selections.
The rest of inputs are:
	- spell casts,
	- wielding requests,
	- item transfers.
