replication_system = inherits_from (processing_system)

function replication_system:constructor(owner_scene)
	self.object_by_id = {}
	self.owner_scene = owner_scene

	processing_system.constructor(self)
end

function replication_system:read_object_state(object, input_bs)
	-- read what modules have changed
	local replica = object.replication.modules
			
	for i=1, #protocol.module_mappings do
		local module_name = protocol.module_mappings[i]
		local module_object = replica[module_name]
	
		if module_object ~= nil then
			module_object:read_state(object, input_bs)
		end
	end
end

function replication_system:update_states_from_bitstream(msg)
	local input_bs = msg.input_bs
	
	for i=1, msg.data.object_count do
		input_bs:name_property("object_id")
		
		local id = input_bs:ReadUshort()
		protocol.LAST_READ_BITSTREAM = input_bs
		local object = self.object_by_id[id]
		
		self:read_object_state(object, input_bs)
	end
end

function replication_system:create_objects_or_change_modules(msg)
	local input_bs = msg.input_bs
	
	for i=1, msg.data.object_count do
		local new_object = {} 
		protocol.read_sig(protocol.new_object_signature, new_object, input_bs)
		
		local archetype_name = protocol.archetype_by_id[new_object.archetype_id]
		
		local is_object_new = self.object_by_id[new_object.id] == nil
		local replica = {}
		
		-- read what modules the replica has
		for i=1, #protocol.module_mappings do
			input_bs:name_property("has_module " .. i)
			if input_bs:ReadBit() then
				local module_name = protocol.module_mappings[i]
				
				replica[module_name] = replication_module:create(protocol.replication_tables[module_name])
			end
		end
			
		local new_entity;
		
		-- if the object is new
		if is_object_new then
			-- creation phase
			new_entity = world_archetype_callbacks[archetype_name].creation(self, new_object.id)
			
			-- save replication data (not as a component; just a table)
			new_entity.replication = { modules = replica, id = new_object.id }
			
			-- save the newly created entity
			self.object_by_id[new_object.id] = new_entity
			self.owner_entity_system:add_entity(new_entity)
		-- otherwise just update the replica
		else
			print "RECREATING OBJECT"
			new_entity = self.object_by_id[new_object.id]
			new_entity.replication.modules = replica
		end
		
		-- whole initial state is always read
		for i=1, #protocol.module_mappings do
			local module_name = protocol.module_mappings[i]
			if replica[module_name] ~= nil then
				replica[module_name]:read_initial_state(new_entity, input_bs)
			end
		end
			
		-- creation is necessary, though construction is not
		local construction_callback = world_archetype_callbacks[archetype_name].construction
		
		if construction_callback then 
			construction_callback (self, new_entity, is_object_new)
		end
	end
end

function replication_system:get_variable_message_size(msg)
	if msg.info.name == "STATE_UPDATE" then
		return msg.data.bits
	elseif msg.info.name == "NEW_OBJECTS" then
		return msg.data.bits
	elseif msg.info.name == "DELETE_OBJECTS" then
		return msg.data.bits
	end
	
	return 0
end


function replication_system:create_new_objects()
	local msgs = self.owner_entity_system.messages["NEW_OBJECTS"]
	
	for i=1, #msgs do
		self:create_objects_or_change_modules(msgs[i])
	end
end

function replication_system:update_object_states()
	local msgs = self.owner_entity_system.messages["ASSIGN_SYNC_ID"]
	
	for i=1, #msgs do
		self.my_sync_id = msgs[i].data.sync_id
	end
	
	msgs = self.owner_entity_system.messages["STATE_UPDATE"]
	
	for i=1, #msgs do
		self:update_states_from_bitstream(msgs[i])
	end
end

function replication_system:delete_objects()
	local msgs = self.owner_entity_system.messages["DELETE_OBJECT"]
	
	for i=1, #msgs do
		local id = msgs[i].data.removed_id
		
		self.owner_entity_system:post_remove(self.object_by_id[id])
		self.object_by_id[id] = nil
	end
	
	msgs = self.owner_entity_system.messages["DELETE_OBJECTS"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		local input_bs = msg.input_bs
		
		for j=1, msg.data.object_count do
			input_bs:name_property("deleted_object_id")
			local id = input_bs:ReadUshort()
			
			self.owner_entity_system:post_remove(self.object_by_id[id])
			self.object_by_id[id] = nil
		end
	end
end
