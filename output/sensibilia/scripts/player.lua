player_sprite = create_sprite {
	image = images.blank,
	size = vec2(30, 100)
}

debug_sensor = create_sprite {
	image = images.blank,
	size = vec2(40, 5),
	color = rgba(0, 0, 255, 122)
}

player_debug_circle = simple_create_polygon (reversed(gen_circle_vertices(60, 5)))
map_uv_square(player_debug_circle, images.blank)

function get_self(entity)
	return entity.scriptable.script_data
end

player_scriptable_info = create_scriptable_info {
	scripted_events = {
		[scriptable_component.INTENT_MESSAGE] = function (message) 
			if message.intent == custom_intents.JUMP then
				get_self(message.subject):jump(message.state_flag)
			else 
				return true
			end
			
			return false
		end,
		
		[scriptable_component.LOOP] = function (subject)
			get_self(subject):loop()
			render_system:push_line(debug_line(subject.transform.current.pos + get_self(subject).foot_sensor_p1, subject.transform.current.pos + get_self(subject).foot_sensor_p2, rgba(255, 0, 0, 255)))
		end
	}
}

player = spawn_npc {
	body = {
		render = {
			model = player_sprite
		},
		
		transform = {
			pos = vec2(20000, -10000)
		},
		
		input = {
			--intent_message.MOVE_FORWARD,
			--intent_message.MOVE_BACKWARD,
			custom_intents.JUMP,
			intent_message.MOVE_LEFT,
			intent_message.MOVE_RIGHT
		},
		
		scriptable = {
			available_scripts = player_scriptable_info
		}
	}
}


get_self(player.body):set_foot_sensor_from_sprite(player_sprite, 3)
--get_self(player.body):set_foot_sensor_from_circle(60, 6)
world_camera.chase:set_target(player.body)
world_camera.camera.player:set(player.body)
--world_camera.camera.crosshair:set(player.crosshair:get())