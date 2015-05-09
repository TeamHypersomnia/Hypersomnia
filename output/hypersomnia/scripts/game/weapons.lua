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
	
	local function create_weapon(weapon_name, weapon_type, weapon_properties, use_world_sprite_to_wielded)
		weapons[weapon_name] = {
			weapon_info = weapon_properties,
			
			item_info = {
				outfit_type = weapon_type,
				
				item_sprite = scene.sprite_library[weapon_name]["world"],
			
				can_be_unwielded = function(object)
					return object.weapon:can_be_unwielded()
				end,
				
				on_wielder_changed = function(object, new_wielder)
					if object.weapon then
						object.weapon.trigger = components.weapon.triggers.NONE
					end
					
					-- following is only view
					if include_render then
						object.weapon.barrel_smoke_group.particle_group.pause_emission = false
					
						if new_wielder then
							if new_wielder.inventory then
								object.cpp_entity.render.model = nil
								
								if object.weapon then
									object.weapon.barrel_smoke_group.particle_group.pause_emission = true
								end
							else
								local target_sprite;
								
								if use_world_sprite_to_wielded then
									target_sprite = scene.sprite_object_library[weapon_name]["world"]
								else
									target_sprite = scene.sprite_object_library[weapon_name]["wield"]
								end
								
								object.cpp_entity.render.model = target_sprite
								
								if object.weapon then
									object.weapon.barrel_smoke_group.chase.rotation_orbit_offset = vec2(target_sprite.size.x/2, 0)
								end
								
								if weapon_properties.is_melee then
									object.cpp_entity.render.layer = render_layers.WIELDED_MELEE
								else
									object.cpp_entity.render.layer = render_layers.WIELDED_GUNS
								end
							end
						else
							object.cpp_entity.render.model = scene.sprite_object_library[weapon_name]["world"]
							object.cpp_entity.render.layer = render_layers.ON_GROUND
							
							if object.weapon then
								object.weapon.barrel_smoke_group.chase.rotation_orbit_offset = weapon_properties.world_barrel_offset
							end
						end
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
		current_rounds = 3000,
		is_automatic = true,
		bullets_once = 1,
		bullet_damage = 35,
		bullet_speed = minmax(5000, 6000),
		
		shooting_interval_ms = 100,
		spread_degrees = 0,
		shake_radius = 9.5,
		shake_spread_degrees = 45,
		
		world_barrel_offset = vec2(49, -8),
		bullet_barrel_offset = vec2(70, 10),
		
		bullet_entity = basic_bullet_entity,				
		
		max_lifetime_ms = 500,
		
		swing_duration = 100,
		swing_interval_ms = 400,
		swing_angle = 20,
		swing_radius = 70,
		swing_damage = 35,
		hits_per_swing = 1
	})
	
	create_weapon("pistol", "rifle", {
		current_rounds = 3000,
		is_automatic = true,
		bullets_once = 1,
		bullet_damage = 27,
		bullet_speed = minmax(4000, 5000),
		
		shooting_interval_ms = 100,
		spread_degrees = 1,
		shake_radius = 2.5,
		shake_spread_degrees = 45,
		
		world_barrel_offset = vec2(51, -5),
		bullet_barrel_offset = vec2(72, 10),
		
		bullet_entity = basic_bullet_entity,				
		
		max_lifetime_ms = 500,
		swing_duration = 100,
		swing_interval_ms = 400,
		swing_angle = 20,
		swing_radius = 70,
		swing_damage = 30,
		hits_per_swing = 1
	})	

	create_weapon("shotgun", "rifle", {
		current_rounds = 3000,
		is_automatic = false,
		bullets_once = 12,
		bullet_damage = 9,
		bullet_speed = minmax(4000, 5000),
		
		shooting_interval_ms = 400,
		spread_degrees = 5,
		shake_radius = 1.5,
		shake_spread_degrees = 45,
		
		world_barrel_offset = vec2(51, -5),
		bullet_barrel_offset = vec2(72, 10),
		
		bullet_entity = basic_bullet_entity,				
		
		max_lifetime_ms = 500,
		swing_duration = 100,
		swing_interval_ms = 400,
		swing_angle = 20,
		swing_radius = 70,
		swing_damage = 30,
		hits_per_swing = 1
	})	
	
	create_weapon("fireaxe", "melee", {
		is_melee = true,
	
		current_rounds = 0,
		is_automatic = true,
		bullets_once = 0,
		bullet_damage = 0,
		bullet_speed = minmax(0, 0),
		
		shooting_interval_ms = 0,
		spread_degrees = 0,
		shake_radius = 0,
		shake_spread_degrees = 0,
		
		world_barrel_offset = vec2(0, 0),
		bullet_barrel_offset = vec2(0, 0),
		
		bullet_entity = basic_bullet_entity,				
		
		max_bullet_distance = 0,
		
		max_lifetime_ms = 0,
		
		swing_duration = 100,
		swing_interval_ms = 400,
		swing_angle = 140,
		swing_radius = 90,
		swing_damage = 75,
		hits_per_swing = 1
	}, true)
	
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

npc_wield_offsets = {
	HEAD = {
		shot = {
			{ pos = vec2(0, 0)  },
			{ pos = vec2(-1, 0)  },
			{ pos = vec2(-2, 0)  },
			{ pos = vec2(-4, 0)  },
			{ pos = vec2(-6, 0)  }
		},
		
		walk = {
			{ pos = vec2(0, 0) },
			{ pos = vec2(-1, 0) },
			{ pos = vec2(-2, 0) },
			{ pos = vec2(-3, 0) },
			{ pos = vec2(-4, 0) }
		},
		
		walk_cw = {
			{ pos = vec2(0, 0)  },
			{ pos = vec2(1, 0)  },
			{ pos = vec2(2, 0)  },
			{ pos = vec2(3, 0)  },
			{ pos = vec2(4, 0)  }
		},
		
		swing = {
			{ pos = vec2(0, 0)   },
			{ pos = vec2(0, 1)   },
			{ pos = vec2(1, 2)  },
			{ pos = vec2(0, 3)   },
			{ pos = vec2(0, 4)   }
		},
		
		swing_cw = {
			{ pos = vec2(0, 0) },
			{ pos = vec2(0, -1) },
			{ pos = vec2(-1, -2) },
			{ pos = vec2(0,  -3) },
			{ pos = vec2(0,  -4) }
		}
	},
	
	melee = {
		walk = {
			{ rotation = 180-90, pos = vec2(7, 24), flip = true  },
			{ rotation = 180-90, pos = vec2(6, 24), flip = true  },
			{ rotation = 180-90, pos = vec2(7, 23), flip = true  },
			{ rotation = 180-90, pos = vec2(6, 24), flip = true  },
			{ rotation = 180-90, pos = vec2(7, 24), flip = true  }
		},
		
		walk_cw = {
			{ rotation = 0-90, pos = vec2(7, -24)},
			{ rotation = 0-90, pos = vec2(6, -24)},
			{ rotation = 0-90, pos = vec2(7, -23)},
			{ rotation = 0-90, pos = vec2(6, -24)},
			{ rotation = 0-90, pos = vec2(7, -24)}
		},
		
		swing = {
			{ rotation = 150-90, pos = vec2(-5, 5)   + vec2.rotated(vec2(7, 24), vec2(17, -10), -30 ) , flip = true },
			{ rotation = 120-90, pos = vec2(-8, 7)   + vec2.rotated(vec2(7, 24), vec2(17, -10), -60 ) , flip = true },
			{ rotation = 90-90,  pos = vec2(-10, 10) + vec2.rotated(vec2(7, 24), vec2(17, -10), -90 ) , flip = true  },
			{ rotation = 60-90,  pos = vec2(-12, 12) + vec2.rotated(vec2(7, 24), vec2(17, -10), -120 ), flip = true   },
			{ rotation = 30-90,  pos = vec2(-15, 15) + vec2.rotated(vec2(7, 24), vec2(17, -10), -150 ), flip = true   }
		},
		
		swing_cw = {
			{ rotation = 30-90,  pos = vec2(-15, -15) + vec2.rotated(vec2(7, 24), vec2(17, -10), -150) },
			{ rotation = 60-90,  pos = vec2(-12, -12) + vec2.rotated(vec2(7, 24), vec2(17, -10), -120) },
			{ rotation = 90-90,  pos = vec2(-10, -10) + vec2.rotated(vec2(7, 24), vec2(17, -10), -90 ) },
			{ rotation = 120-90, pos = vec2(-8, -7)   + vec2.rotated(vec2(7, 24), vec2(17, -10), -60 )},
			{ rotation = 150-90, pos = vec2(-5, -5)   + vec2.rotated(vec2(7, 24), vec2(17, -10), -30 )}
		}
	},
	
	rifle = {
		walk = {
			{ rotation = 0, pos = vec2(35, 10) },
			{ rotation = 0, pos = vec2(36, 10) },
			{ rotation = 0, pos = vec2(37, 10) },
			{ rotation = 0, pos = vec2(39, 10) },
			{ rotation = 0, pos = vec2(40, 10) }
		},
		
		shot = {
			{ rotation = 0, pos = vec2(34, 10)  },
			{ rotation = 0, pos = vec2(33, 10)  },
			{ rotation = 0, pos = vec2(32, 10)  },
			{ rotation = 0, pos = vec2(30, 10)  },
			{ rotation = 0, pos = vec2(28, 10)  }
		}
	}
}