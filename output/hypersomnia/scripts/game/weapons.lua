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
		}
	}
	
	weapons.shotgun = {
		weapon_info = {
			current_rounds = 300,
			is_automatic = false,
			bullets_once = 12,
			bullet_damage = 12,
			bullet_speed = minmax(3000, 4000),
			
			shooting_interval_ms = 400,
			spread_degrees = 12,
			shake_radius = 1.5,
			shake_spread_degrees = 45,
			
			bullet_barrel_offset = vec2(50, 0),
			
			bullet_entity = basic_bullet_entity,				
			
			max_lifetime_ms = 500
		}
	}
	
	
	scene.weapons = weapons
end