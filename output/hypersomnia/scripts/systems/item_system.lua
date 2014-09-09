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
		if item.item.entity_archetype ~= nil then
			item.cpp_entity.transform.current.rotation = 0
			
			add_physics_component(item.cpp_entity, item.item.entity_archetype.physics)
		end
		
		-- if we had a wielder, copy its transform
		if item.item.wielder ~= nil then
			item.cpp_entity.transform.current = item.item.wielder.cpp_entity.transform.current
		end
	end
	
	if new_owner == nil then
		self.wielding_key = nil
	end
	
	item.cpp_entity.chase:set_target(cpp_chase)
	item.item.wielder = new_owner
	
	if item.item.on_wielder_changed then
		item.item.on_wielder_changed(item, new_owner)
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
		local archetype = new_entity.item.entity_archetype
		if archetype == nil then archetype = {} end
		
		new_entity.cpp_entity = self.world_object:create_entity ( override({
			transform = {},
			chase = {
				chase_type = chase_component.ORBIT,
				chase_rotation = true,
				relative = false
			}
		}, archetype) )
		
	else
		if new_entity.cpp_entity.transform == nil then
			new_entity.cpp_entity:add(transform_component())
		end
		if new_entity.cpp_entity.chase == nil then
			new_entity.cpp_entity:add(chase_component())
		end
	end
	
	new_entity.cpp_entity.script = new_entity
	
	components.item.set_wielder(new_entity.item, nil)
	--new_entity.item:set_wielder(nil)
	--processing_system.add_entity(self, new_entity)
end

