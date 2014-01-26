dofile "sensibilia\\scripts\\input.lua"
dofile "sensibilia\\scripts\\camera.lua"

current_zoom_level = 2000
set_zoom_level(world_camera)

function set_color(poly, col)
	for i = 0, poly:get_vertex_count()-1 do
		poly:get_vertex(i).color = col
	end
end

environment_archetype = {
	physics = {
		body_type = Box2D.b2_staticBody,
		
		body_info = {
			shape_type = physics_info.POLYGON,
			filter = filter_static_objects,
			density = 1,
			friction = 0.1
		}
	},
	
	render = {
		layer = render_layers.BACKGROUND
	},
	
	transform = {
	
	}
}

ground_poly = simple_create_polygon (reversed {
	vec2(0, 500) + vec2(-800, 0),
	vec2(0, 500) + vec2(500, 0),
	vec2(0, 500) + vec2(900, -200),
	vec2(0, 500) + vec2(1400, -300),
	vec2(0, 500) + vec2(3000, -300),
	vec2(0, 500) + vec2(3000, 200),
	vec2(0, 500) + vec2(-800, 200)
})

map_uv_square(ground_poly, images.blank)
set_color(ground_poly, rgba(0, 255, 0, 255))

environment_entity = create_entity (archetyped(environment_archetype, {
	render = {
		model = ground_poly
	}
}))

dofile "sensibilia\\scripts\\npc.lua"
dofile "sensibilia\\scripts\\player.lua"

loop_only_info = create_scriptable_info {
	scripted_events = {
		[scriptable_component.INTENT_MESSAGE] = 
			function(message)
				if message.intent == custom_intents.QUIT then
					input_system.quit_flag = 1
				elseif message.intent == custom_intents.RESTART then
						set_world_reloading_script(reloader_script)
				elseif message.intent == custom_intents.INSTANT_SLOWDOWN then
					physics_system.timestep_multiplier = 0.00001
				elseif message.intent == custom_intents.SPEED_INCREASE then
					physics_system.timestep_multiplier = physics_system.timestep_multiplier + 0.05
				elseif message.intent == custom_intents.SPEED_DECREASE then
					physics_system.timestep_multiplier = physics_system.timestep_multiplier - 0.05
					
					if physics_system.timestep_multiplier < 0.01 then
						physics_system.timestep_multiplier = 0.01
					end
				end
				
				return false
			end,
				
		[scriptable_component.LOOP] = function(subject)
			my_atlas:bind()
		end
	}
}


create_entity {
	input = {
			custom_intents.SPEED_INCREASE,
			custom_intents.SPEED_DECREASE,
			custom_intents.INSTANT_SLOWDOWN,
			custom_intents.QUIT,
			custom_intents.RESTART
	},
		
	scriptable = {
		available_scripts = loop_only_info
	}	
}


player.body.name = "player_body"
environment_entity.name = "environment_entity"

physics_system.b2world:SetGravity(b2Vec2(0, 100))