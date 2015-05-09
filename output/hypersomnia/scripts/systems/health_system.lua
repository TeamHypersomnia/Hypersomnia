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
		local victim = self.owner_entity_system.all_systems["replication"].object_by_id[msgs[i].data.victim_id]
		local health = victim.health
		health.hp = health.hp - msgs[i].data.amount

		local new_indicator_entity = self.owner_scene.world_object:create_entity {
			transform = {
				pos = victim.pos + vec2(randval(-40, 40), -randval(0, 15))
			}
		}

		local indicator_color = rgba(255, 0, 0, 255)
		local drawn_number = msgs[i].data.amount
		drawn_number = -drawn_number

		local preffix = ""
		local rising_speed = 80

		if drawn_number > 0 then
			preffix = "+"
			rising_speed = 120
			indicator_color = rgba(0, 255, 0, 255)
		end
		


		local new_indicator = components.create_components {
			cpp_entity = new_indicator_entity,
			label = {
				position = components.label.positioning.DAMAGE_INDICATOR,
				default_font = self.owner_scene.font_by_name["kubasta"],

				["rising_speed"] = rising_speed,
				
				text = {
					{
						str = preffix .. drawn_number,
						color = indicator_color
					}
				}
			}
		}

		self.owner_scene.owner_client_screen.entity_system_instance:add_entity(new_indicator)
	end
	
	local bar_width = 50
	for i=1, #self.targets do
		local health = self.targets[i].health
		
		health.under_bar_sprite.size = vec2(bar_width + 2, 3)
		health.under_bar_outline_sprite.size = vec2(bar_width + 4, 5)
		health.health_bar_sprite.size = vec2(health.hp*bar_width/100, 1)
		local mult = health.hp/components.health.max_hp
		--health.health_bar_sprite.color = rgba(255*(1-mult*mult), 255*mult*mult, 255*mult, 255)--rgba(255*(1-health.hp/100), 194*health.hp/100, 0, 255)
		health.last_mult = mult

		local red_hsv = rgba(255, 0, 0, 255):get_hsv()
		local cyan_hsv = rgba(0, 255, 255, 255):get_hsv()
		local target_hsv = hsv(0, 0, 0)
		math.lerp_object (target_hsv, red_hsv, cyan_hsv, math.sqrt(mult), "hsv")

		local target_color = rgba(0, 0, 0, 255)
		target_color:set_hsv(target_hsv)

		health.health_bar_sprite.color = target_color--rgba(255*(1-health.hp/100), 194*health.hp/100, 0, 255)
		health.under_bar_outline_sprite.color = health.health_bar_sprite.color
		--health.health_bar_sprite.color.r = health.health_bar_sprite.color.r*0.7
		--health.health_bar_sprite.color.g = health.health_bar_sprite.color.g*0.7
		--health.health_bar_sprite.color.b = health.health_bar_sprite.color.b*0.7
		health.health_bar_entity.chase.offset.x = - (bar_width-health.hp*bar_width/100)/2
	end
end