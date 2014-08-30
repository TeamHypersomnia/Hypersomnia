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
	
	local function create_weapon(weapon_name, weapon_type, weapon_properties)
		weapons[weapon_name] = {
			weapon_info = weapon_properties,
			
			item_info = {
				outfit_type = weapon_type,
				
				item_sprite = scene.sprite_library[weapon_name]["world"],
			
				can_be_unwielded = function(object)
					return object.weapon:can_be_unwielded()
				end,
				
				on_wielder_changed = function(object, new_wielder)
					object.weapon.trigger = components.weapon.triggers.NONE
	
					if new_wielder then
						object.cpp_entity.render.model = nil
					else
						object.cpp_entity.render.model = scene.sprite_object_library[weapon_name]["world"]
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
							rect_size = scene.sprite_library[weapon_name]["world"].size,
							
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
	end
	
	create_weapon("m4a1", "rifle", {
		current_rounds = 30,
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
	})
	
	create_weapon("shotgun", "rifle", {
		current_rounds = 6,
		is_automatic = false,
		bullets_once = 12,
		bullet_damage = 12,
		bullet_speed = minmax(2000, 4000),
		
		shooting_interval_ms = 400,
		spread_degrees = 5,
		shake_radius = 1.5,
		shake_spread_degrees = 45,
		
		bullet_barrel_offset = vec2(50, 0),
		
		bullet_entity = basic_bullet_entity,				
		
		max_lifetime_ms = 500	
	})
	
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