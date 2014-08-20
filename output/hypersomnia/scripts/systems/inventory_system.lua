inventory_system = inherits_from (processing_system)

function inventory_system:constructor(owner_scene)
	self.owner_scene = owner_scene
	self.owner_world = owner_scene.world_object
	
	processing_system.constructor(self)
end

function inventory_system:get_required_components()
	return { "inventory" }
end

function inventory_system:add_entity(new_entity)
	local inventory = new_entity.inventory
	local slot_num = 6
	
	if inventory.draw_as_owner then
		inventory.slots = {}
		
		for i=1, slot_num do
			local slot_pos = (self.owner_scene.world_camera.camera.size/2) - vec2(110, 550 - i*80)
			
			inventory.slots[i] = { 
				bg_entity = self.owner_world:create_entity {
					render = {
						layer = render_layers.INVENTORY_SLOTS,
						absolute_transform = true
					},
					
					transform = {
						pos = slot_pos
					}
				},
				
				entity = self.owner_world:create_entity {
					render = {
						layer = render_layers.INVENTORY_ITEMS,
						absolute_transform = true,
						flip_horizontally = true
					},
					
					transform = {
						pos = slot_pos
					}
				}
			}
		end
	end
	
	processing_system.add_entity(self, new_entity)
end

function inventory_system:remove_entity(removed_entity)
	local inventory = removed_entity.inventory
	local owner_world = removed_entity.cpp_entity.owner_world

	for i=1, #inventory.slots do
		owner_world:post_message(destroy_message(inventory.slots[i].bg_entity, nil))
	end
	
	processing_system.remove_entity(self, removed_entity)
end

function inventory_system:update()
	local msgs = self.owner_entity_system.messages["item_wielder_change"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		
		if msg.succeeded then
			local inventory = msg.subject.inventory
			local wield = msg.subject.wield
			
			if inventory then
				for j=1, #inventory.slots do
					local slot = inventory.slots[j]
					
					if not slot.stored_item then
						slot.stored_item = msg.item
						slot.entity.render.model = msg.item.item.item_model
						print "Slot:" print (j)
						msg.item.item.inventory_slot = j
						
						-- if wield.pri
						break
					end
				end
			end
		end
	end
	
	msgs = self.owner_world:get_messages_filter_components("intent_message", { "inventory" } )
	local client_sys = self.owner_entity_system.all_systems["client"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		
		local subject = msgs[i].subject.script
		local inventory = subject.inventory
		
		if msg.state_flag then
			local intent = msg.intent
			
			if intent == custom_intents.PICK_REQUEST then
				client_sys.net_channel:post_reliable("PICK_ITEM_REQUEST", {})
			elseif intent == custom_intents.DROP_REQUEST then
				local to_be_dropped = subject.item.wielder.wield.wielded_items[components.wield.keys.PRIMARY_WEAPON]
				
				if to_be_dropped then
					client_sys.net_channel:post_reliable("DROP_ITEM_REQUEST", {
						item_id = to_be_dropped.replication.id
					})		
				end
			
			else
				for j=1, #inventory.slots do
					if intent == custom_intents["SELECT_ITEM_" .. j] then
						local item = inventory.slots[j].stored_item
						
						if item then
							client_sys.net_channel:post_reliable("SELECT_ITEM_REQUEST", {
								item_id = item.replication.id
							})
						end
						
						break
					end
				end
			end
		end
	end
	
	for i=1, #self.targets do
		local inventory = self.targets[i].inventory
		
		if inventory.draw_as_owner then
			for i=1, #inventory.slots do
				inventory.slots[i].bg_entity.render.model = self.owner_scene.sprite_object_library["slot"]["inactive"]
			end
		end
	end
end

function inventory_system:handle_item_requests()

end
--
--function inventory_system:send_item_requests()
--	local msgs = self.world_object:get_messages_filter_components("intent_message", { "inventory" } )
--	
--	for i=1, #msgs do
--end