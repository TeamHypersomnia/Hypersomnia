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
	inventory.slots = 6
	
	if inventory.draw_as_owner then
		inventory.slot_entities = {}
		
		for i=1, inventory.slots do
			inventory.slot_entities[i] = self.owner_world:create_entity {
				render = {
					layer = render_layers.INVENTORY_SLOTS,
					absolute_transform = true
				},
				
				transform = {
					pos = (self.owner_scene.world_camera.camera.size/2) - vec2(110, 550 - i*80)
				}
			}
		end
	end
	
	processing_system.add_entity(self, new_entity)
end

function inventory_system:remove_entity(removed_entity)
	local inventory = removed_entity.inventory
	local owner_world = removed_entity.cpp_entity.owner_world

	for i=1, #inventory.slot_entities do
		owner_world:post_message(destroy_message(inventory.slot_entities[i], nil))
	end
	
	processing_system.remove_entity(self, removed_entity)
end

function inventory_system:update()
	for i=1, #self.targets do
		local inventory = self.targets[i].inventory
		
		if inventory.draw_as_owner then
			for i=1, #inventory.slot_entities do
				inventory.slot_entities[i].render.model = self.owner_scene.sprite_object_library["slot"]["inactive"]
			end
		end
	end
end

--function inventory_system:handle_item_requests()
--	local msgs = self.owner_entity_system.messages["PICK_ITEM_REQUEST"]
--end
--
--function inventory_system:send_item_requests()
--	local msgs = self.world_object:get_messages_filter_components("intent_message", { "inventory" } )
--	
--	for i=1, #msgs do
--end