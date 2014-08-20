function create_weapons(scene, include_render)
	local weapons = {}
	
	local basic_bullet_entity = {
		physics = {
			body_type = Box2D.b2_dynamicBody,
	
			body_info = {
				filter = filters.BULLET,
				shape_type = physics_info.RECT,
				rect_size = vec2(35, 19*0.05),
				fixed_rotation = true,
				density = 0.1,
				bullet = true
			}
		}
	}
	
	if include_render == true then
		basic_bullet_entity.render = {
			model = scene.bullet_sprite,
			layer = render_layers.BULLETS
		}
	end
	
	weapons.m4a1 = {
		weapon_info = {
			current_rounds = 300,
			is_automatic = true,
			bullets_once = 1,
			bullet_damage = 5,
			bullet_speed = minmax(3000, 3000),
			
			shooting_interval_ms = 100,
			spread_degrees = 0,
			shake_radius = 9.5,
			shake_spread_degrees = 45,
			
			bullet_barrel_offset = vec2(50, 0),
			
			bullet_entity = basic_bullet_entity,				
			
			max_lifetime_ms = 500	
		},
		
		item_info = {
			outfit_type = "rifle",
			
			item_model = scene.sprite_object_library["m4a1"]["world"],
		
			on_wielder_changed = function(object, new_wielder)
				if new_wielder then
					object.cpp_entity.render.model = nil
				else
					object.cpp_entity.render.model = scene.sprite_object_library["m4a1"]["world"]
					object.cpp_entity.render.layer = render_layers.ON_GROUND
				end
			end,
				
			entity_archetype = {
				render = {
					
				},
				
				physics = {
					body_type = Box2D.b2_dynamicBody,
					
					body_info = {
						filter = filters.DROPPED_ITEM,
						shape_type = physics_info.RECT,
						rect_size = vec2(98, 36),
						
						linear_damping = 4,
						angular_damping = 4,
						fixed_rotation = false,
						density = 0.1,
						friction = 0,
						restitution = 0.4,
						sensor = false
					}
				}
			}
		}
	}
	
	--weapons.shotgun = {
	--	weapon_info = {
	--		current_rounds = 300,
	--		is_automatic = false,
	--		bullets_once = 12,
	--		bullet_damage = 12,
	--		bullet_speed = minmax(3000, 4000),
	--		
	--		shooting_interval_ms = 400,
	--		spread_degrees = 12,
	--		shake_radius = 1.5,
	--		shake_spread_degrees = 45,
	--		
	--		bullet_barrel_offset = vec2(50, 0),
	--		
	--		bullet_entity = basic_bullet_entity,				
	--		
	--		max_lifetime_ms = 500
	--	}
	--}
	
	if not include_render then
		for k, v in pairs (weapons) do
			v.item_info.render = nil
			v.item_info.on_drop = nil
			v.item_info.on_pick = nil
		end
	end
	
	scene.weapons = weapons
end