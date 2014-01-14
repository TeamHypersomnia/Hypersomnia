player_class = inherits_from(npc_class)

function player_class:init()
	self:take_weapon_item(bare_hands)
end

function player_class:loop()
	if not self:good_health() then
		
		self:drop_weapon(0.5)
		local player_corpse = self:throw_corpse()
		
		world_camera.chase:set_target(player_corpse)
		world_camera.camera.player:set(player_corpse)
		--world_camera.camera.crosshair:set(nil)
	end
end

player = ptr_create_entity_group (archetyped(character_archetype, {
	body = {
		transform = {},
		
		input = {
			intent_message.MOVE_FORWARD,
			intent_message.MOVE_BACKWARD,
			intent_message.MOVE_LEFT,
			intent_message.MOVE_RIGHT,
			intent_message.SHOOT
		},
		
		lookat = {
			target = "crosshair",
			look_mode = lookat_component.POSITION
		},
		
		scriptable = {
			script_data = player_class
		}
	},

	crosshair = { 
		transform = {
			pos = vec2(0, 0),
			rotation = 0
		},
		
		render = {
			layer = render_layers.GUI_OBJECTS,
			model = crosshair_sprite
		},
		
		crosshair = {
			sensitivity = config_table.sensitivity
		},
		
		chase = {
			target = "body",
			relative = true
		},
		
		input = {
			intent_message.AIM
		}
	},
	
	legs = {
		animate = {
			available_animations = player_animations.sets.legs
		}
	}
}))

player_corpse_sprite = create_sprite {
	image = player_images.dead_front
}

init_npc(player.body:get(), { 
	weapon_animation_sets = {
		BARE_HANDS = player_animations.sets.bare_hands,
		FIREAXE = player_animations.sets.melee,
		ASSAULT_RIFLE = player_animations.sets.firearm,
		SHOTGUN = player_animations.sets.firearm
	},
	
	health_info = {
		hp = 100,
		
		corpse_entity = archetyped(corpse_archetype, {
			render = {
				model = player_corpse_sprite
			}
		})			
	},
	
	wield_offsets = npc_wield_offsets,
	
	head_archetype =  {
		transform = {},
		
		chase = {
			--rotation_orbit_offset = vec2(2, 0)
		},
		
		render = {
			layer = layers.HEADS,
			model = head_walk_sprite
		}
	},
	
	weapon_bullet_filter = filter_bullets,
	weapon_melee_filter = filter_melee,
	weapon_melee_obstruction_filter = filter_melee_obstruction
	}
)


--print(vec2.rotated(vec2(7, 24), vec2(17, -10), 150 ).x, vec2.rotated(vec2(7, 24), vec2(17, -10), 150 ).y)
--print(get_scripted(player.body:get()).wield_offsets.FIREAXE.swing[1].pos.x, get_scripted(player.body:get()).wield_offsets.FIREAXE.swing[1].pos.y)

get_scripted(player.body:get()):take_weapon_item(fireaxe)
get_scripted(player.body:get()):drop_weapon(0.5)
get_scripted(player.body:get()):take_weapon_item(assault_rifle)
get_scripted(player.body:get()):drop_weapon(0.3)
get_scripted(player.body:get()):take_weapon_item(shotgun)
get_scripted(player.body:get()):drop_weapon(0.1)
--spawn_weapon(assault_rifle)
player.body:get().name = "player"

set_max_speed(player.body:get(), 7000)

main_context = create_input_context {
	intents = { 
		[mouse.raw_motion] 		= intent_message.AIM,
		[keys.W] 				= intent_message.MOVE_FORWARD,
		[keys.ESC] 				= custom_intents.QUIT,
		[keys.S] 				= intent_message.MOVE_BACKWARD,
		[keys.A] 				= intent_message.MOVE_LEFT,
		[keys.D] 				= intent_message.MOVE_RIGHT,
		[keys.R] 				= custom_intents.RESTART,
		[keys.V] 				= custom_intents.INSTANT_SLOWDOWN,
		
		[mouse.ldoubleclick] 	= intent_message.SHOOT,
		[mouse.ltripleclick] 	= intent_message.SHOOT,
		[mouse.ldown] 			= intent_message.SHOOT,
		
		[keys.LSHIFT] 			= intent_message.SWITCH_LOOK,
		[mouse.rdown] 			= custom_intents.DROP_WEAPON,
		[mouse.rdoubleclick] 	= custom_intents.DROP_WEAPON,
		[mouse.wheel]			= custom_intents.ZOOM_CAMERA,
		[keys.ADD] 				= custom_intents.SPEED_INCREASE,
		[keys.SUBTRACT] 		= custom_intents.SPEED_DECREASE
	}
}

input_system:clear_contexts()
input_system:add_context(main_context)

world_camera.chase:set_target(player.body:get())
world_camera.camera.player:set(player.body:get())
world_camera.camera.crosshair:set(player.crosshair:get())
--player.body:get().gun.target_camera_to_shake:set(world_camera)

set_zoom_level(world_camera)