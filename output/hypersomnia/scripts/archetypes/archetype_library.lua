

--world_archetype_groups = {}
--world_archetype_groups.register = function(archetype_name, group_name)
--	if group_name ~= nil then
--		if world_archetype_groups[group_name] == nil then
--			world_archetype_groups[group_name] = {}
--		end
--		
--		world_archetype_groups[group_name][archetype_name] = true
--	end
--
--	--table.insert(protocol.archetype_by_id, archetype_name)
--	--table.sort(protocol.archetype_by_id)
--	--
--	--protocol.archetype_library = create_enum (protocol.archetype_by_id)
--end


world_archetype_groups = {
	guns = {
		"m4a1"
	},
	
	UNGROUPED = {
		"CONTROLLED_PLAYER",
		"REMOTE_PLAYER"
	}
}

world_archetype_callbacks = {}

--for k, v in pairs(groups) do
--	if k == "UNGROUPED" then
--		for entry_name in pairs(v) do
--			world_archetype_groups[k] = { entries = { entry_name } }
--		end
--	end
--	
--	world_archetype_groups[k] = {}
--end



protocol.archetype_by_id = {}

for k, v in spairs(world_archetype_groups) do
	protocol.archetype_by_id = table.concatenate( { protocol.archetype_by_id, v } )
end

protocol.archetype_library = create_enum (protocol.archetype_by_id)