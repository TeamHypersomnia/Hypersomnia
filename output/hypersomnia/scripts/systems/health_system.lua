health_system = inherits_from (processing_system)

function health_system:constructor(owner_scene)
	self.owner_scene = owner_scene
	self.owner_world = owner_scene.world_object
	
	processing_system.constructor(self)
end

function health_system:get_required_components()
	return { "health" }
end

function health_system:add_entity(new_entity)
	new_entity.health.health_bar_sprite = create_sprite {
		image = self.owner_scene.sprite_library["blank"],
		color = rgba(0, 194, 0, 255)
	}
	
	new_entity.health.under_bar_sprite = create_sprite {
		image = self.owner_scene.sprite_library["blank"],
		color = rgba(0, 0, 0, 255)
	}
	
	local bar_entity = {
		render = {
			layer = render_layers.GUI_OBJECTS
		},
		
		transform = {},
		
		chase = {
			target = new_entity.cpp_entity,
			offset = vec2(0, -40)
		}
	}
	
	new_entity.health.under_bar_entity = self.owner_world:create_entity (override(bar_entity, { 
		render = { 
			model = new_entity.health.under_bar_sprite
		}
	}))
	
	new_entity.health.health_bar_entity = self.owner_world:create_entity (override(bar_entity, { 
		render = { 
			model = new_entity.health.health_bar_sprite
		}
	}))
	
	processing_system.add_entity(self, new_entity)
end

function health_system:remove_entity(removed_entity)
	self.owner_world:delete_entity(removed_entity.health.under_bar_entity)
	self.owner_world:delete_entity(removed_entity.health.health_bar_entity)
	
	processing_system.remove_entity(self, removed_entity)
end

function health_system:update()
	for i=1, #self.targets do
		local health = self.targets[i].health
		
		health.under_bar_sprite.size = vec2(102, 5)
		health.health_bar_sprite.size = vec2(health.hp, 3)
		health.health_bar_sprite.color = rgba(255*(1-health.hp/100), 194*health.hp/100, 0, 255)
		health.health_bar_entity.chase.offset.x = - (100-health.hp)/2
	end
end