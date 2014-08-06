item_system = inherits_from (processing_system)

function item_system:constructor(add_render, world_object)
	self.add_render = add_render
	self.world_object = world_object
	
	processing_system.constructor(self)
end

function item_system:get_required_components()
	return { "item" }
end

function components.item:set_ownership(new_owner)
	local item = self.entity
	local cpp_chase = nil
	
	if new_owner ~= nil then
		cpp_chase = new_owner.cpp_entity
	end
	
	item.cpp_entity.chase:set_target(cpp_chase)

	if new_owner ~= nil and item.cpp_entity.physics ~= nil then
		item.cpp_entity:remove_physics()
	elseif new_owner == nil and item.cpp_entity.physics == nil then
		-- drop it to the ground
		add_physics_component(item.cpp_entity, item.item.physics_table)
		
		-- if we had an owner, copy its transform
		if item.item.ownership ~= nil then
			item.cpp_entity.transform.current = item.item.ownership.cpp_entity.transform.current
		end
	end
	
	item.item.ownership = new_owner
end


function item_system:remove_entity(removed_entity)
	local owner = removed_entity.item.ownership
	
	if owner ~= nil then
		local wield = owner.wield

		if wield.on_drop ~= nil then
			wield.on_drop(owner, wield.wielded_item)
		end
		
		wield.wielded_item = nil
	end
end

function item_system:add_entity(new_entity)
	new_entity.item.entity = new_entity
	
	if new_entity.cpp_entity == nil then
		local cpp_entity_table = {
			transform = {},
			chase = {}
		}
		
		if self.add_render then
			cpp_entity_table.render = {}
		end
	
		new_entity.cpp_entity = self.world_object:create_entity (cpp_entity_table)
		new_entity.cpp_entity.script = new_entity
	end
	
	new_entity.item:set_ownership(nil)
	--processing_system.add_entity(self, new_entity)
end

