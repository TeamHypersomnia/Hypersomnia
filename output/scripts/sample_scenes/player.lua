player_class = inherits_from(npc_class)

function player_class:init()
	self:take_weapon_item(bare_hands)
	print("player initialised")
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
			sensitivity = 5.5
		},
		
		chase = {
			target = "body",
			relative = true
		},
		
		input = {
			intent_message.AIM
		}
	}
}))

init_npc(player.body:get(), { 
	weapon_animation_sets = {
		BARE_HANDS = npc_animation_body_set,
		FIREAXE = npc_animation_body_set,
		ASSAULT_RIFLE = npc_animation_body_shotgun_set,
		SHOTGUN = npc_animation_body_shotgun_set
	},
	
	health_info = {
		hp = 100,
		
		corpse_entity = archetyped(corpse_archetype, {
			render = {
				model = corpse_sprite
			}
		})			
	}
})

get_scripted(player.body:get()):take_weapon_item(shotgun)

set_max_speed(player.body:get(), 7000)

main_context = create_input_context {
	intents = { 
		[mouse.raw_motion] 		= intent_message.AIM,
		[keys.W] 				= intent_message.MOVE_FORWARD,
		[keys.ESC] 				= custom_intents.QUIT,
		[keys.S] 				= intent_message.MOVE_BACKWARD,
		[keys.A] 				= intent_message.MOVE_LEFT,
		[keys.D] 				= intent_message.MOVE_RIGHT,
		[keys.R] 				= custom_intents.STEERING_REQUEST,
		[keys.E] 				= custom_intents.EXPLORING_REQUEST,
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

input_system:add_context(main_context)

world_camera.chase:set_target(player.body:get())
world_camera.camera.player:set(player.body:get())
world_camera.camera.crosshair:set(player.crosshair:get())
player.body:get().gun.target_camera_to_shake:set(world_camera)

set_zoom_level(world_camera)