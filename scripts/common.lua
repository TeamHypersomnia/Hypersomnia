function rewrite(component, entry, omit_properties)
	if omit_properties == nil then
		for key, val in pairs(entry) do
			component[key] = val
		end
	else
		for key, val in pairs(entry) do
			if omit_properties[key] == nil then
				component[key] = val
			end
		end
	end
end

function rewrite_ptr(component, entry, properties, entities_lookup)
	if properties == nil then return end
	
	for key, val in pairs(properties) do
		if type(entry[key]) == "string" then
			component[key]:set(entities_lookup[entry[key]])
		else
			component[key]:set(entry[key])
		end
	end
end

function recursive_write(entries, final_entries)
	for key, entry in pairs(entries) do
		if type(entry) == "table" then
			if final_entries[key] == nil then final_entries[key] = {} end
			recursive_write(entry, final_entries[key])
		else
			final_entries[key] = entry
		end
	end
end

function entries_from_archetypes(archetype, entries, final_entries)
	recursive_write(archetype, final_entries)
	recursive_write(entries, final_entries)
end

function archetyped(archetype, entries)
	local final_entries = {}
	entries_from_archetypes(archetype, entries, final_entries)
	return final_entries
end