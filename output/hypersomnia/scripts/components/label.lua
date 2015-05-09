components.label = inherits_from()

components.label.positioning = create_enum {
	"OVER_HEALTH_BAR",
	"TOP_LEFT_BBOX",
	"DAMAGE_INDICATOR"
}

function components.label:constructor(init_table)
	rewrite(self, init_table)
end