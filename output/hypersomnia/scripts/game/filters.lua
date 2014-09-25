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
		"BULLET", "CHARACTER"
	},
	
	BULLET = {
		"STATIC_OBJECT", "REMOTE_CHARACTER"
	},
	
	REMOTE_BULLET = {
		"STATIC_OBJECT"
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

query_filter_category = 0
	
for k, v in pairs(all_categories) do
	query_filter_category = bitor(v, query_filter_all_categories)
end

function create_query_filter(entries)
	local mask = 0
	
	for i=1, #entries do
		mask = bitor(all_categories[entries[i]], mask)
	end
	
	return create(b2Filter, {
		categoryBits = query_filter_category,
		maskBits = mask
	})
end