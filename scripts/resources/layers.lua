render_layers = {
	GUI_OBJECTS = 0,
	EFFECTS = 1,
	OBJECTS = 2,
	PLAYERS = 3,
	BULLETS = 4,
	LEGS = 5,
	ON_GROUND = 6,
	GROUND = 7
}

custom_intents = create_inverse_enum {
	"ZOOM_CAMERA",
	"STEERING_REQUEST"
}

-- PHYSICS COLLISION LAYERS --
create_options { 
	"CHARACTERS", 
	"OBJECTS", 
	"BULLETS", 
	"CORPSES" 
}

filter_objects = {
	categoryBits = OBJECTS,
	maskBits = bitor(OBJECTS, BULLETS, CHARACTERS, CORPSES)
}

filter_characters = {
	categoryBits = CHARACTERS,
	maskBits = bitor(OBJECTS, BULLETS, CHARACTERS)
}

filter_bullets = {
	categoryBits = BULLETS,
	maskBits = bitor(OBJECTS, CHARACTERS)
}

filter_corpses = {
	categoryBits = CORPSES,
	maskBits = OBJECTS
}