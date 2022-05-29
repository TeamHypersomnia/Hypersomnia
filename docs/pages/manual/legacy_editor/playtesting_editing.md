You have paused the playtest session.
If you don't know what it means, 
press Ctrl+Shift+Backspace to return to normal editing and carefully read the tutorial.

Now you can edit the map without exiting the playtest.
This means that you can prototype your game world very quickly.

## Where has my project gone?

You will notice that your History GUI (Alt+H) now shows no commands. 
Worry not, you haven't lost anything.

The moment that you have entered the playtest for the first time,
your entire project (.int, .rulesets, .history and .view_ids) has been backed up
and resides within deep depths of the .player file.   

These files - and thus, your earlier project state - will all be perfectly restored 
the moment that you exit the playtest.  

You can also safely save your project during a playtest.
The .player file will still contain the backed up files,
and the .int, .rulesets, .history and .view_ids files on the disk 
will contain the newest state that is visible during the playtest, 
e.g. the .int file will contain the state of the played world,
possibly with some corpses and shells lying on the ground due to fired shots.
	The same .int file can be copied to another project and later be opened by the editor for normal editing,
	although it is not something you should be concerned with now.

## Exiting the playtest

- Press Ctrl+Shift+Backspace to return to normal editing
  and **discard all changes** done during the playtest.

- Press Ctrl+Shift+Enter to return to normal editing
  and **re-apply all changes** done during the playtest.
	- Note: when editing during a playtest, you might introduce some new entities to the world, 
	  for example by duplication, instantiation or mirroring. 
	  The implementation will try to instantiate them accordingly during re-application of changes, 
	  but there is currently no guarantee that it will succeed 
	  in creating and placing the new entities correctly.
	
	  Changes done to entities that have existed even BEFORE starting the playtest are perfectly fine.
	  Duplicating or creating assets or rulesets is also fine.

## Other controls

- You can select the character you want to control by selecting it and pressing O.
	- The currently controlled entity is always highlighted in bright yellow.
	- You can select any entity for controlling but only selecting the
	  Controlled character type of entities will yield any meaningful result.
- Press i to enter gameplay while controlling the selected character.

## Gameplay recording

You can deterministically record and replay your gameplay.
You can even choreograph movements of many characters to create some interesting scenes.
Press Alt+P for more information.
