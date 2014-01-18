current_zoom_level = 0

function set_zoom_level(camera)
	local mult = 1 + (current_zoom_level / 1000)
	local new_w = config_table.resolution_w*mult
	local new_h = config_table.resolution_h*mult
	camera.camera.ortho = rect_ltrb(rect_xywh((config_table.resolution_w-new_w)/2, (config_table.resolution_h-new_h)/2, new_w, new_h))
	
	player.crosshair:get().crosshair.size_multiplier = vec2(mult, mult)
	target_entity.crosshair.size_multiplier = vec2(mult, mult)
end

scriptable_zoom = create_scriptable_info {
	scripted_events = {
		[scriptable_component.INTENT_MESSAGE] = function(message)
				if message.intent == custom_intents.ZOOM_CAMERA then
					current_zoom_level = current_zoom_level-message.wheel_amount
					set_zoom_level(message.subject)
				end
			return false
		end
	}
}

camera_archetype = {
	transform = {
		pos = vec2(),
		rotation = 0
	},

	camera = {
		enabled = true,
		
		layer = 0, -- 0 = topmost
		mask = render_component.WORLD,
		
		enable_smoothing = true,
		smoothing_average_factor = 0.5,
		averages_per_sec = 20,
		
		crosshair = nil,
		player = nil,
	
		orbit_mode = camera_component.LOOK,
		max_look_expand = vec2(config_table.resolution_w/2, config_table.resolution_h/2),
		angled_look_length = 100
	},
	
	chase = {
		relative = false,
		offset = vec2(config_table.resolution_w/(-2), config_table.resolution_h/(-2))
	}
}

scene_fbo = framebuffer_object(config_table.resolution_w, config_table.resolution_h)

world_camera = create_entity (archetyped(camera_archetype, {
	transform = {
		pos = vec2(),
		rotation = 0
	},

	camera = {
		screen_rect = rect_xywh(0, 0, config_table.resolution_w, config_table.resolution_h),
		ortho = rect_ltrb(0, 0, config_table.resolution_w, config_table.resolution_h),
		
		--drawing_callback = function (subject, renderer, visible_area, drawn_transform, target_transform, mask)
		--	renderer:generate_triangles(visible_area, drawn_transform, mask)
		--	local old_num = renderer:get_triangle_count()
		--	
		--	local drawn_dist = (drawn_transform.pos - target_transform.pos):length()
		--	if drawn_dist > 20 then
		--		target_transform.pos = target_transform.pos + vec2(randval(-drawn_dist/10, drawn_dist/10), randval(-drawn_dist/10, drawn_dist/10))
		--		renderer:generate_triangles(visible_area, target_transform, mask)
		--		
		--		for i=old_num, renderer:get_triangle_count()-1 do
		--			local my_tri = renderer:get_triangle(i)
		--			
		--			my_tri:get_vert(0).color.a = my_tri:get_vert(0).color.a * (drawn_dist / 300)
		--			my_tri:get_vert(1).color.a = my_tri:get_vert(1).color.a * (drawn_dist / 300)
		--			my_tri:get_vert(2).color.a = my_tri:get_vert(2).color.a * (drawn_dist / 300)
		--			
		--			my_tri:get_vert(0).color.r = my_tri:get_vert(0).color.r * (drawn_dist / 100)
		--			my_tri:get_vert(1).color.r = my_tri:get_vert(1).color.r * (drawn_dist / 100)
		--			my_tri:get_vert(2).color.r = my_tri:get_vert(2).color.r * (drawn_dist / 100)
		--			
		--			my_tri:get_vert(0).color.g = my_tri:get_vert(0).color.g * (drawn_dist / 200)
		--			my_tri:get_vert(1).color.g = my_tri:get_vert(1).color.g * (drawn_dist / 200)
		--			my_tri:get_vert(2).color.g = my_tri:get_vert(2).color.g * (drawn_dist / 200)
		--			
		--			my_tri:get_vert(0).color.b = my_tri:get_vert(0).color.b * (drawn_dist / 150)
		--			my_tri:get_vert(1).color.b = my_tri:get_vert(1).color.b * (drawn_dist / 150)
		--			my_tri:get_vert(2).color.b = my_tri:get_vert(2).color.b * (drawn_dist / 150)
		--		end
		--	end
		--	
		--	scene_fbo:use()
		--	
		--	GL.glClear(GL.GL_COLOR_BUFFER_BIT)
		--	
		--	renderer:call_triangles()
		--	
		--	framebuffer_object.use_default()
		--	
		--	GL.glColor4f(1, 1, 1, 1)
		--	GL.glBindTexture(GL.GL_TEXTURE_2D, scene_fbo:get_texture_id())
		--	
		--	GL.glGenerateMipmap(GL.GL_TEXTURE_2D)
		--	
		--	GL.glBegin(GL.GL_QUADS)
		--	
		--	GL.glTexCoord2f(0, 1); GL.glVertex2i(visible_area.x, visible_area.y)
		--	GL.glTexCoord2f(1, 1); GL.glVertex2i(visible_area.r, visible_area.y)
		--	GL.glTexCoord2f(1, 0); GL.glVertex2i(visible_area.r, visible_area.b)
		--	GL.glTexCoord2f(0, 0); GL.glVertex2i(visible_area.x, visible_area.b)
		--	
		--	GL.glEnd()
		--	
		--	renderer:clear_triangles()
		--	--renderer:default_render(visible_area)
		--end
	},
	
	input = {
		intent_message.SWITCH_LOOK,
		custom_intents.ZOOM_CAMERA
	},
	
	chase = {},
	
	scriptable = {
		available_scripts = scriptable_zoom
	}
}))
