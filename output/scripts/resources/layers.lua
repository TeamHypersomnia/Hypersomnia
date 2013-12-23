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
	"STEERING_REQUEST",
	"EXPLORING_REQUEST",
	"INSTANT_SLOWDOWN",
	"SPEED_INCREASE",
	"SPEED_DECREASE",
	"QUIT",
	"DROP_WEAPON"
}

-- PHYSICS COLLISION LAYERS --
create_options { 
	"CHARACTERS", 
	"OBJECTS", 
	"STATIC_OBJECTS",
	"BULLETS", 
	"CORPSES",
	"ITEMS"
}


filter_nothing = {
	categoryBits = 0,
	maskBits = 0
}

local mask_all = bitor(OBJECTS, STATIC_OBJECTS, BULLETS, CHARACTERS, CORPSES, ITEMS)

filter_static_objects = {
	categoryBits = STATIC_OBJECTS,
	maskBits = mask_all
}

filter_objects = {
	categoryBits = OBJECTS,
	maskBits = bitor(OBJECTS, STATIC_OBJECTS, BULLETS, CHARACTERS, CORPSES)
}

filter_characters = {
	categoryBits = CHARACTERS,
	maskBits = bitor(OBJECTS, STATIC_OBJECTS, BULLETS, CHARACTERS)
}

filter_characters_separation = {
	categoryBits = CHARACTERS,
	maskBits = bitor(CHARACTERS)
}

filter_bullets = {
	categoryBits = BULLETS,
	maskBits = bitor(OBJECTS, STATIC_OBJECTS, CHARACTERS)
}

filter_corpses = {
	categoryBits = CORPSES,
	maskBits = bitor(OBJECTS, STATIC_OBJECTS)
}

filter_items = {
	categoryBits = ITEMS,
	maskBits = bitor(OBJECTS, STATIC_OBJECTS)
}

filter_pick_up_items = {
	categoryBits = mask_all,
	maskBits = ITEMS
}

filter_pathfinding_visibility = {
	categoryBits = bitor(OBJECTS, STATIC_OBJECTS, BULLETS, CHARACTERS, CORPSES),
	maskBits = bitor(STATIC_OBJECTS)
}

filter_obstacle_visibility = {
	categoryBits = bitor(OBJECTS, STATIC_OBJECTS, BULLETS, CHARACTERS, CORPSES),
	maskBits = bitor(OBJECTS, STATIC_OBJECTS, CHARACTERS)
}