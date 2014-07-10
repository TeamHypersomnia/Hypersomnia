function recursive_write(final_entries, entries, omit_names)
	if omit_names == nil then omit_names = {} end
	
	for key, entry in pairs(entries) do
		if omit_names[key] == nil then
			if type(entry) == "table" then
				if final_entries[key] == nil then final_entries[key] = {} end
				recursive_write(final_entries[key], entry)
			else
				final_entries[key] = entry
			end
		end
	end
end

function entries_from_archetypes(archetype, entries, final_entries)
	recursive_write(final_entries, archetype)
	recursive_write(final_entries, entries)
end

function override(archetype, entries)
	local final_entries = {}
	entries_from_archetypes(archetype, entries, final_entries)
	return final_entries
end

function clone_table(entries)
	return override(entries, {})
end