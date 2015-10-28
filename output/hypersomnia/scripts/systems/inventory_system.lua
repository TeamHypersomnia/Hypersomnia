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
				},
				
				stored_sprite = create_sprite {}
			}
		end
	end
	
	processing_system.add_entity(self, new_entity)
end

function inventory_system:remove_entity(removed_entity)
	local inventory = removed_entity.inventory
	local owner_world = removed_entity.cpp_entity.owner_world

	for i=1, #inventory.slots do
		owner_world:post_message(destroy_message(inventory.slots[i].bg_entity, entity_id()))
	end
	
	processing_system.remove_entity(self, removed_entity)
end

function inventory_system:handle_picked_up_item(msg)
	if msg.succeeded and msg.wield then
		local inventory = msg.subject.inventory
		local wield = msg.subject.wield
		
		if inventory then
			local found_slot = msg.predefined_slot
			
			if found_slot == nil then
				for j=1, #inventory.slots do
					if not inventory.slots[j].stored_item then
						found_slot = j
						break
					end
				end
			end
					
			local slot = inventory.slots[found_slot]
			
			slot.stored_item = msg.item
			slot.stored_sprite = create_sprite { image = msg.item.item.item_sprite }
			slot.entity.render.model = slot.stored_sprite
			msg.item.item.inventory_slot = found_slot
		end
	end
end

function components.inventory:set_active(slot_index)
	self.last_item = self.active_item
	self.active_item = slot_index
end

function inventory_system:update()
	local msgs = self.owner_world:get_messages_filter_components("intent_message", { "inventory" } )
	local client_sys = self.owner_entity_system.all_systems["client"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		
		local subject_inventory = msg.subject.script
		local inventory = subject_inventory.inventory
		local wielder = subject_inventory.item.wielder
		
		if msg.state_flag then
			local intent = msg.intent
				
			-- some events may request selecting another item
			if intent == custom_intents.SELECT_LAST_ITEM then
				if inventory.last_item then
					intent = custom_intents["SELECT_ITEM_" .. inventory.last_item]
				else
					intent = custom_intents.HOLSTER_ITEM
				end
			end
			
			if intent == custom_intents.PICK_REQUEST then
				client_sys.net_channel:post_reliable("PICK_ITEM_REQUEST", {})
			elseif intent == custom_intents.DROP_REQUEST then
				if inventory.active_item then
					local item = inventory.slots[inventory.active_item].stored_item
					
					client_sys.net_channel:post_reliable("DROP_ITEM_REQUEST", {
						item_id = item.replication.id
					})

					-- predict the holstering on the client
					--self:holster_item(subject_inventory, wielder, item, inventory.active_item)
					
					local slot = inventory.slots[inventory.active_item]
					slot.stored_item = nil
					inventory:set_active(nil)

					self.owner_entity_system:post_table("item_wielder_change", { 
						unwield = true,
						subject = wielder,
						wielding_key = components.wield.keys.PRIMARY_WEAPON
					})
					
					slot.stored_sprite = create_sprite {}
					slot.entity.render.model = nil
					item.item.inventory_slot = nil
					
					for j=#inventory.slots, 1, -1 do
						if inventory.slots[j].stored_item then
							intent = custom_intents["SELECT_ITEM_" .. j]
						end
					end
				end
			elseif intent == custom_intents.HOLSTER_ITEM then
				if inventory.active_item then
					local item = inventory.slots[inventory.active_item].stored_item
					
					client_sys.net_channel:post_reliable("HOLSTER_ITEM", {
						item_id = item.replication.id
					})		
					
					-- predict the holstering on the client
					self:holster_item(subject_inventory, wielder, item, inventory.active_item)
					
					inventory:set_active(nil)
				end
			end
		
			
			for j=1, #inventory.slots do
				if intent == custom_intents["SELECT_ITEM_" .. j] then
					local item = inventory.slots[j].stored_item
					
					if item and inventory.active_item ~= j then
						client_sys.net_channel:post_reliable("SELECT_ITEM_REQUEST", {
							item_id = item.replication.id
						})
						
						-- predict the selecting on the client
						self:select_item(subject_inventory, wielder, item, inventory.active_item)
						
						
						inventory:set_active(j)
					end
					
					break
				end
			end
		end
	end
	
	for i=1, #self.targets do
		local inventory = self.targets[i].inventory
		
		if inventory.draw_as_owner then
			for j=1, #inventory.slots do
				local model = "inactive"
				
				if inventory.active_item == j then
					model = "active"
					inventory.slots[j].stored_sprite.color.a = 255
				else
					inventory.slots[j].stored_sprite.color.a = 120
				end
				
				inventory.slots[j].bg_entity.render.model = self.owner_scene.sprite_object_library["slot"][model]
			end
		end
	end
end

--
--function inventory_system:send_item_requests()
--	local msgs = self.world_object:get_messages_filter_components("intent_message", { "inventory" } )
--	
--	for i=1, #msgs do
--end