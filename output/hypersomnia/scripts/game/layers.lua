world_render_layers = {
	"OBJECTS",
	"HEADS",
	"WIELDED_GUNS",
	"PLAYERS",
	"BULLETS",
	"WIELDED_MELEE",
	"LEGS",
	"SHELLS",
	"SPECULAR_HIGHLIGHTS",
	"ON_GROUND",
	"UNDER_CORPSES",
	"GROUND"
}

other_layers = {
	"EFFECTS",
	"SMOKES"
}

hud_render_layers = {
	"CROSSHAIRS",
	"INVENTORY_SLOTS",
	"INVENTORY_ITEMS",
	"HEALTH_BARS"
}

render_layers = create_enum (table.concatenate {
	hud_render_layers,
	other_layers,
	world_render_layers 
})

visibility_layers = {
	BASIC_LIGHTING = 998,
	
	
	LIGHT_BOUNCE = 999
	-- do not add anything below as these values are used by another light bounces
}