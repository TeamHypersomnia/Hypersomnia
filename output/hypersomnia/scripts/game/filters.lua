-- PHYSICS COLLISION LAYERS --
filters = {
	STATIC_OBJECT = "ALL",

	AVOIDANCE = {
		"STATIC_OBJECT"
	},
	
	DROPPED_ITEM = {
		"ITEM_PICK", "STATIC_OBJECT"
	},
	
	ITEM_PICK = {
		"DROPPED_ITEM"
	},
	
	CHARACTER = {
		"STATIC_OBJECT", "REMOTE_CHARACTER", "CHARACTER"
	},
	
	REMOTE_CHARACTER = {
		"BULLET", "CHARACTER", "SWING_HITSENSOR"
	},
	
	BULLET = {
		"STATIC_OBJECT", "REMOTE_CHARACTER"
	},
	
	REMOTE_BULLET = {
		"STATIC_OBJECT"
	},
	
	SWING_HITSENSOR = {
		"STATIC_OBJECT", "REMOTE_CHARACTER"
	}
}

-- processing

local all_keys = {}
local all_categories = {}

for k, v in pairs(filters) do
	table.insert(all_keys, k)
end

create_options(all_keys, all_categories)

for k, v in pairs(filters) do
	if type(v) == "string" and v == "ALL" then
		filters[k] = all_keys
	end
end

for k, v in pairs(filters) do
	local mask = 0
	
	for i=1, #filters[k] do
		mask = bitor(all_categories[filters[k][i]], mask)
	end
	
	filters[k] = create(b2Filter, {
		categoryBits = all_categories[k],
		maskBits = mask
	})
end
