function create_weapons(scene, include_render)
	local weapons = {}
	
	weapons.m4a1 = {
		current_rounds = 300,
		is_automatic = true,
		bullets_once = 1,
		bullet_speed = minmax(5000, 5000),
		
		shooting_interval_ms = 100,
		spread_degrees = 0,
		shake_radius = 9.5,
		shake_spread_degrees = 45,
		
		bullet_barrel_offset = vec2(50, 0),
		
		bullet_entity = {
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
		},				
		
		max_bullet_distance = 2000	
	}
	
	if include_render == true then
		weapons.m4a1.bullet_entity.render = {
			model = scene.bullet_sprite,
			layer = render_layers.BULLETS
		}
	end
	
	scene.weapons = weapons
end