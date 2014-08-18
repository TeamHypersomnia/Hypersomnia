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
	print "adding!"
	if inventory.target_camera then
	print "adding camera!"
		local slot_entity = {
			render = {
				model = self.owner_scene.sprite_object_library["slot"]["active"],
				layer = render_layers.INVENTORY_SLOTS
			},
			
			transform = {},
			
			chase = {
				target = inventory.target_camera,
				offset = vec2(200, 300)
			}
		}
		
		self.owner_world:create_entity (slot_entity)
	end
	
	processing_system.add_entity(self, new_entity)
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