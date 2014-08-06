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
		
		local object = self.object_by_id[new_object.id]
		local replica = {}
		
		-- read what modules the replica has
		for i=1, #protocol.module_mappings do
			input_bs:name_property("has_module " .. i)
			if input_bs:ReadBit() then
				local module_name = protocol.module_mappings[i]
				
				replica[module_name] = replication_module:create(protocol.replication_tables[module_name])
			end
		end
			
		if object == nil then
			-- create space for modules
			object = { replication = { ["modules"] = replica, id = new_object.id } }
			
			local new_entity;
			
			-- resolve the archetype
			if archetype_name == "CONTROLLED_PLAYER" then
				local player_cpp_entity = create_controlled_player(
				self.owner_scene,
				self.owner_scene.teleport_position, 
				self.owner_scene.world_camera, 
				self.owner_scene.crosshair_sprite)
	
				new_entity = components.create_components {
					cpp_entity = player_cpp_entity.body,
					input_prediction = {
						simulation_entity = self.owner_scene.simulation_player
					},
					
					orientation = {
						receiver = false,
						crosshair_entity = player_cpp_entity.crosshair
					},
					
					health = {},

					wield = {}
				}
				
				new_entity.wield.on_pickup = function(this)
					local picked = this.wield.wielded_item
					
					if picked.weapon ~= nil then
						picked.weapon.transmit_bullets = true
						picked.weapon.constrain_requested_bullets = true
						picked.weapon.bullet_entity.physics.body_info.filter = filters.BULLET
					end
				end
				
			elseif archetype_name == "REMOTE_PLAYER" then
				local new_remote_player = create_remote_player(self.owner_scene, self.owner_scene.crosshair_sprite)
				
				new_remote_player.body.animate.available_animations = self.owner_scene.torso_sets["white"]["rifle"].set
				new_remote_player.legs.animate.available_animations = self.owner_scene.legs_sets["white"].set
	
				new_entity = components.create_components {
					cpp_entity = new_remote_player.body,
					interpolation = {},
					
					orientation = {
						receiver = true,
						crosshair_entity = new_remote_player.crosshair
					},
					
					health = {},
					
					wield = {}
				}
				
				
				new_entity.wield.on_pickup = function(this)
					local picked = this.wield.wielded_item
					
					if picked.weapon ~= nil then
						picked.weapon.transmit_bullets = false
						picked.weapon.constrain_requested_bullets = false
						picked.weapon.bullet_entity.physics.body_info.filter = filters.REMOTE_BULLET
					end
				end
			elseif archetype_name == "m4a1" then
				new_entity = components.create_components {
					item = {
						physics_table = {
							body_type = Box2D.b2_dynamicBody,
							
							body_info = {
								filter = filters.DROPPED_ITEM,
								shape_type = physics_info.RECT,
								rect_size = vec2(98, 36),
								
								linear_damping = 4,
								angular_damping = 4,
								fixed_rotation = false,
								density = 0.1,
								friction = 0,
								restitution = 0.4,
								sensor = false
							}
						}
					},
				
					weapon = self.owner_scene.weapons.m4a1
				}
				
				new_entity.weapon:create_smoke_group(self.owner_scene.world_object.world)
			end
			
			-- save replication data (not as a component; just a table)
			new_entity.replication = object.replication
			
			-- save the newly created entity
			print "adding"
			print(new_object.id)
			self.object_by_id[new_object.id] = new_entity
			self.owner_entity_system:add_entity(new_entity)
			
			for i=1, #protocol.module_mappings do
				local module_name = protocol.module_mappings[i]
				if replica[module_name] ~= nil then
					replica[module_name]:read_initial_state(new_entity, input_bs)
				end
			end
			
			-- after all initial data is read, call construction callbacks
			if archetype_name == "m4a1" then
				if new_entity.item.is_owned then
					self.owner_entity_system:post_table("item_ownership", {
						subject = self.object_by_id[new_entity.item.ownership_id],
						item = new_entity,
						pick = true
					})
				end
			end
		else
			print "WARNING! Recreating an existing object (not implemented)"
		end
		
	end
end

function replication_system:get_variable_message_size(msg)
	if msg.info.name == "STATE_UPDATE" then
		return msg.data.bits
	elseif msg.info.name == "NEW_OBJECTS" then
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
end
