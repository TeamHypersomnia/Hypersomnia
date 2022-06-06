# Separate setup

- Actually let's just have a separate setup: playtest_setup
	- Test scene will be useful for local testing out weapons and training, we don't wanna meddle with that
	- If we really need hot reload this bad...
			- We can always make the playtesting setup hot-reload from disk, remembering the character's position and placing it on the scene
		- Screw recording for now... editor_setup will be the tool for that
			- Actually let's later just move recording utility to the playtest setup
				- q will exit from pause to main menu?

- We playtest in test_mode ALWAYS anyway, without a notion of competitive etc
	- true game modes and rules will only be finally tested in production


# Playtesting character

- Well we can have a special entity type like "playtest character" for playtests too on some small scenes with prefab rooms for example

# Game mode

- Will we ever launch a non-test game mode for a playtest?
	- I don't think it's necessary.
		- The warmups will finally never annoy us.

- All mechanics (bomb plants, flag captures) should be possible to test in isolation, without the context of a game mode.
	- Which is how it currently works with the bomb.
	- Should work the same with flags, e.g. we should be able to just give a flag to the character (over back)

# Hot reloads

- They are not mission critical for now
	- We'll implement the setup of initial conditions to such an extent that it won't be needed
		- Resetting the world will be instant anyway
		- And it's a good retry flow, because after a modificaiton cycle our character will be somewhere else anyway
