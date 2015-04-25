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
	
	new_entity.health.under_bar_outline_sprite = create_sprite {
		image = self.owner_scene.sprite_library["blank"],
		color = rgba(11, 23, 35, 255)
	}
	
	new_entity.health.under_bar_sprite = create_sprite {
		image = self.owner_scene.sprite_library["blank"],
		color = rgba(11, 23, 35, 255)
	}	
	
	local bar_entity = {
		render = {
			layer = render_layers.HEALTH_BARS
		},
		
		transform = {},
		
		chase = {
			target = new_entity.cpp_entity,
			offset = vec2(0, -20)
		}
	}
	
	new_entity.health.under_bar_outline_entity = self.owner_world:create_entity (override(bar_entity, { 
		render = { 
			model = new_entity.health.under_bar_outline_sprite
		}
	}))
	
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
	local owner_world = removed_entity.cpp_entity.owner_world
	owner_world:post_message(destroy_message(removed_entity.health.under_bar_entity, nil))
	owner_world:post_message(destroy_message(removed_entity.health.health_bar_entity, nil))
	owner_world:post_message(destroy_message(removed_entity.health.under_bar_outline_entity, nil))
	
	processing_system.remove_entity(self, removed_entity)
end

function health_system:update()
	local msgs = self.owner_entity_system.messages["DAMAGE_MESSAGE"]
	
	for i=1, #msgs do
		local health = self.owner_entity_system.all_systems["replication"].object_by_id[msgs[i].data.victim_id].health
		health.hp = health.hp - msgs[i].data.amount
	end
	
	local bar_width = 50
	for i=1, #self.targets do
		local health = self.targets[i].health
		
		health.under_bar_sprite.size = vec2(bar_width + 2, 3)
		health.under_bar_outline_sprite.size = vec2(bar_width + 4, 5)
		health.health_bar_sprite.size = vec2(health.hp*bar_width/100, 1)
		local mult = health.hp/bar_width
		health.health_bar_sprite.color = rgba(255*(1-mult*mult), 255*mult*mult, 255*mult, 255)--rgba(255*(1-health.hp/100), 194*health.hp/100, 0, 255)
		health.under_bar_outline_sprite.color = health.health_bar_sprite.color
		--health.health_bar_sprite.color.r = health.health_bar_sprite.color.r*0.7
		--health.health_bar_sprite.color.g = health.health_bar_sprite.color.g*0.7
		--health.health_bar_sprite.color.b = health.health_bar_sprite.color.b*0.7
		health.health_bar_entity.chase.offset.x = - (bar_width-health.hp*bar_width/100)/2
	end
end