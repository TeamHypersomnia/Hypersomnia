item_system = inherits_from (processing_system)

function item_system:constructor(world_object)
	self.world_object = world_object
	
	processing_system.constructor(self)
end

function item_system:get_required_components()
	return { "item" }
end

function components.item:set_wielder(new_owner)
	local item = self.entity
	local cpp_chase = nil
	
	if new_owner ~= nil then
		cpp_chase = new_owner.cpp_entity
		
		if item.cpp_entity.physics ~= nil then 
			item.cpp_entity:remove_physics()
		end
	elseif item.cpp_entity.physics == nil then
		-- drop it to the ground
		add_physics_component(item.cpp_entity, item.item.entity_archetype.physics)
		
		-- if we had a wielder, copy its transform
		if item.item.wielder ~= nil then
			item.cpp_entity.transform.current = item.item.wielder.cpp_entity.transform.current
		end
		
		if item.item.on_drop then
			item.item.on_drop(item)
		end
	end
	
	if new_owner == nil then
		self.wielding_key = nil
	end
	
	item.cpp_entity.chase:set_target(cpp_chase)
	item.item.wielder = new_owner
	
	if item.item.on_wielder_changed then
		item.item.on_wielder_changed(item)
	end
end

function item_system:remove_entity(removed_entity)
	local wielder = removed_entity.item.wielder
	
	if wielder ~= nil then
		-- the same will happen both on the client and the server upon item deletion
		-- so there's no need to generate a	"wield_item" message for such an event
		components.wield.unwield_item(wielder, nil, removed_entity.item.wielding_key)
	end
end

function item_system:add_entity(new_entity)
	new_entity.item.entity = new_entity
	
	if new_entity.cpp_entity == nil then
		new_entity.cpp_entity = self.world_object:create_entity ( override({
			transform = {},
			chase = {}
		}, new_entity.item.entity_archetype) )
		
		new_entity.cpp_entity.script = new_entity
	end
	
	new_entity.item:set_wielder(nil)
	--processing_system.add_entity(self, new_entity)
end

