synchronization_system = inherits_from (processing_system)

function synchronization_system:constructor(owner_scene)
	self.object_by_id = {}
	self.owner_scene = owner_scene

	processing_system.constructor(self)
end

function synchronization_system:read_object_state(object, input_bs)
	-- read what modules the entity has or have changed
	local modules = object.synchronization.modules
			
	for i=1, #protocol.module_mappings do
		input_bs:name_property("has_module " .. i)
		if input_bs:ReadBit() then
			local module_name = protocol.module_mappings[i]
			
			if modules[module_name] == nil then
				modules[module_name] = sync_modules[module_name]:create()
			end
			
			modules[module_name]:read_state(input_bs)
		end
	end
end

function synchronization_system:read_object_stream(object, input_bs)
	-- assume all modules are streaming data in module_mappings order
	-- some modules may specify they won't stream data for the moment and will send a notification via reliable state change
	-- in which case both sides will know about it by the time altered stream data arrives
	
	local modules = object.synchronization.modules
	
	for i=1, #protocol.module_mappings do
		local module_name = protocol.module_mappings[i]
			
		if modules[module_name] ~= nil then
			modules[module_name]:read_stream(object, input_bs, self.my_sync_id)
		end
	end
end

function synchronization_system:update_states_from_bitstream(msg)
	local input_bs = msg.input_bs
	
	for i=1, msg.data.object_count do
		input_bs:name_property("object_id")
		local incoming_id = input_bs:ReadUshort()
		
		local is_object_new;
		local object = self.object_by_id[incoming_id]
		
		if object == nil then
			is_object_new = true
			
			-- create space for modules
			object = { synchronization = { modules = {}, id = incoming_id } }
		end	
		
		self:read_object_state(object, input_bs)
		
		if is_object_new then
			-- if the object was just created, create an entity before updating object state
			
			-- for now, simply create remote player object or player object if the id is ours
			local new_entity;
			if incoming_id == self.my_sync_id then
				new_entity = components.create_components {
					cpp_entity = self.owner_scene.player.body,
					input_prediction = {
						simulation_entity = self.owner_scene.simulation_player
					}
				}
				
				new_entity.cpp_entity.script = new_entity
			else
				print(incoming_id)
				local new_remote_player = create_remote_player(self.owner_scene)
				
				new_entity = components.create_components {
					cpp_entity = new_remote_player
				}
			end
			
			-- save synchronization data (not as component)
			new_entity.synchronization = object.synchronization
			
			-- save the newly created entity
			self.object_by_id[incoming_id] = new_entity
			self.owner_entity_system:add_entity(new_entity)
		end
		
		for k, v in pairs (object.synchronization.modules) do
			v:update_game_object(object)
		end
	end
end


function synchronization_system:update_streams_from_bitstream(msg)
	local input_bs = msg.input_bs

	for i=1, msg.data.object_count do
		input_bs:name_property("object_id")
		-- we never read streams if there was reliable data attached and it got mismatched, so it is
		-- safe to assume that streams always contain existing ids
		self:read_object_stream(self.object_by_id[input_bs:ReadUshort()], input_bs)
	end
end

function synchronization_system:handle_variable_message(msg)
	if msg.info.name == "STATE_UPDATE" then
		self:update_states_from_bitstream(msg)
	elseif msg.info.name == "STREAM_UPDATE" then
		self:update_streams_from_bitstream(msg)
	elseif msg.info.name == "ASSIGN_SYNC_ID" then
		self.my_sync_id = msg.data.sync_id
	elseif msg.info.name == "DELETE_OBJECT" then
		local id = msg.data.removed_id
		self.owner_entity_system:remove_entity(self.object_by_id[id])
		
		self.object_by_id[id] = nil
	end
end
