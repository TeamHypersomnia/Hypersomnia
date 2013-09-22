function create_render_info(entries)
	local final_entries = {}
	entries_from_archetypes(entries, final_entries)

	local object = sprite()
	rewrite(object, entries)
	
	if entries.size == nil then 
		object:update_size()
	end
	
	return object
end