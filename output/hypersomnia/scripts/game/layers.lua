world_render_layers = {
	"EFFECTS",
	"OBJECTS",
	"HEADS",
	"WIELDED_GUNS",
	"PLAYERS",
	"BULLETS",
	"WIELDED",
	"LEGS",
	"SHELLS",
	"ON_GROUND",
	"UNDER_CORPSES",
	"GROUND"
}

hud_render_layers = {
	"CROSSHAIRS",
	"INVENTORY_SLOTS",
	"INVENTORY_ITEMS",
	"HEALTH_BARS"
}

render_layers = create_enum (table.concatenate {
	hud_render_layers,
	world_render_layers 
})