function create_world_camera_entity(owner_world, blank_sprite)
	local camera_archetype = {
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
			averages_per_sec = 25,
			
			crosshair = nil,
			player = nil,
		
			orbit_mode = camera_component.LOOK,
			max_look_expand = vec2(config_table.resolution_w/2, config_table.resolution_h/2),
			angled_look_length = 10
		},
		
		chase = {
			relative = false
			--offset = vec2(config_table.resolution_w/(-2), config_table.resolution_h/(-2))
		}
	}

	
	local smoke_fbo = framebuffer_object(config_table.resolution_w, config_table.resolution_h)
	
	local fullscreen_vertex_shader_code = [[
	#version 330
	layout(location = 0) in vec2 position;
	
	out vec2 theTexcoord;
	
	void main() 
	{
		vec4 output_vert;
		output_vert.x = position.x * 2.0 - 1.0;		
		output_vert.y = position.y * 2.0 - 1.0;				
		output_vert.z = 0.0f;						
		output_vert.w = 1.0f;
		
		gl_Position = output_vert;
		theTexcoord = position;
	}
	]]

	local smoke_fragment_shader_code = [[
	#version 330
	in vec2 theTexcoord;
	out vec4 outputColor;
	
	uniform sampler2D smoke_texture;
	uniform sampler2D light_texture;
	const int levels = 4;
	const int level_step = 255/levels + 1;

	vec3 rgb2hsv(vec3 c)
	{
	    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
	
	    float d = q.x - min(q.w, q.y);
	    float e = 1.0e-10;
	    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
	}
	
	vec3 hsv2rgb(vec3 c)
	{
	    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
	}

	void main() 
	{	
		vec4 pixel =  texture(smoke_texture, theTexcoord);

		float intensity = max(max(pixel.r, pixel.g), pixel.b);
		if(intensity == 0.0) discard;

		vec3 pixel_hsv = rgb2hsv(pixel.rgb);
		int discrete_intensity = int(intensity * 255.0);		
		int add_one = discrete_intensity > 0.0 ? 1 : 0;
		int level = discrete_intensity / level_step + add_one;
		intensity = float(level_step * level) / 255.0;
		
		vec2 texcoord = gl_FragCoord.xy;
		texcoord.x /= ]] .. config_table.resolution_w .. [[; 
		texcoord.y /= ]] .. config_table.resolution_h .. [[; 

		vec4 light_pixel =  texture(light_texture, texcoord);
		float light_intensity = max(max(light_pixel.r, light_pixel.g), light_pixel.b);
		vec3 light_hsv =  rgb2hsv(light_pixel.rgb);

		vec4 final_pixel = vec4(intensity, intensity, intensity, level>2?1.0:0.0);
		vec3 final_pixel_hsv = rgb2hsv(final_pixel.rgb);
		//final_pixel_hsv.xy = pixel_hsv.xy;
		final_pixel_hsv.xy = vec2(mix(pixel_hsv.x, light_hsv.x, 0.2), pixel_hsv.y);

		final_pixel.rgb = hsv2rgb(final_pixel_hsv.rgb);
		//final_pixel.rgb *= min(1, light_intensity+0.3);

		outputColor = final_pixel;
	}
	]]

	local vertex_shader_code = [[
	#version 330
	
	uniform mat4 projection_matrix;
	layout(location = 0) in vec2 position;
	layout(location = 1) in vec2 texcoord;
	layout(location = 2) in vec4 color;
	
	smooth out vec4 theColor;
	out vec2 theTexcoord;
	
	void main() 
	{
		vec4 output_vert;
		output_vert.x = position.x;		
		output_vert.y = position.y;				
		output_vert.z = 0.0f;						
		output_vert.w = 1.0f;
		
		gl_Position = projection_matrix*output_vert;
		theColor = color;
		theTexcoord = texcoord;
	}
	
	]]
	
	local my_vertex_shader = GLSL_shader(GL.GL_VERTEX_SHADER, vertex_shader_code)
	local fullscreen_vertex_shader = GLSL_shader(GL.GL_VERTEX_SHADER, fullscreen_vertex_shader_code)
	
	local my_fragment_shader = GLSL_shader(GL.GL_FRAGMENT_SHADER, [[
	#version 330
	smooth in vec4 theColor;
	in vec2 theTexcoord;
	
	out vec4 outputColor;
	
	uniform sampler2D basic_texture;
	
	void main() 
	{
		vec4 pixel = theColor * texture(basic_texture, theTexcoord);
		outputColor = pixel;
	}
	
	]])	


	local illuminated_fragment_shader = GLSL_shader(GL.GL_FRAGMENT_SHADER, [[
	#version 330
	smooth in vec4 theColor;
	in vec2 theTexcoord;
	
	out vec4 outputColor;
	
	uniform sampler2D basic_texture;
	uniform sampler2D light_texture;
	
	const int levels = 4;
	const int step = 255/levels;

	void main() 
	{
		vec2 texcoord = gl_FragCoord.xy;
		texcoord.x /= ]] .. config_table.resolution_w .. [[; 
		texcoord.y /= ]] .. config_table.resolution_h .. [[;

		vec4 light = texture(light_texture, texcoord);
		//light.r = float(step * (int(light.r * 255.0) / step)) / 255.0;
		//light.g = float(step * (int(light.g * 255.0) / step)) / 255.0;
		//light.b = float(step * (int(light.b * 255.0) / step)) / 255.0;
		//light.a = float(step * (int(light.a * 255.0) / step)) / 255.0;

		float intensity = max(max(light.r, light.g), light.b);
		intensity = float(
			
			step * (int(intensity * 255.0) / step + levels)

			) / 255.0;
		light.rgb *= intensity;

		vec4 pixel = theColor * texture(basic_texture, theTexcoord) * light;
		outputColor = pixel;
	}
	
	]])



	local highlights_fragment_shader = GLSL_shader(GL.GL_FRAGMENT_SHADER, [[
	#version 330
	smooth in vec4 theColor;
	in vec2 theTexcoord;
	
	out vec4 outputColor;
	
	uniform sampler2D basic_texture;
	uniform sampler2D light_texture;
	
	const int levels = 4;
	const int step = 255/levels;

	void main() 
	{
		vec2 texcoord = gl_FragCoord.xy;
		texcoord.x /= ]] .. config_table.resolution_w .. [[; 
		texcoord.y /= ]] .. config_table.resolution_h .. [[;

		vec4 light = texture(light_texture, texcoord);
		float intensity = max(max(light.r, light.g), light.b);
		int level = int(intensity * 255.0) / step + levels;
		intensity = float(
			
			step * (level)

			) / 255.0;
		light.rgb *= intensity;
		if (level < 5) discard;
		light.rgb *= intensity * intensity;

		vec4 pixel = theColor * texture(basic_texture, theTexcoord) * light;
		outputColor = pixel;
	}
	
	]])

	local highlights_vertex_shader = GLSL_shader(GL.GL_VERTEX_SHADER, vertex_shader_code)

	local illuminated_vertex_shader = GLSL_shader(GL.GL_VERTEX_SHADER, vertex_shader_code)

	local smoke_fragment_shader = GLSL_shader(GL.GL_FRAGMENT_SHADER, smoke_fragment_shader_code)
	
	local illuminated_shader_program = GLSL_program()
	illuminated_shader_program:attach(illuminated_vertex_shader)
	illuminated_shader_program:attach(illuminated_fragment_shader)
	illuminated_shader_program:use()

	local illuminated_projection_matrix_uniform = GL.glGetUniformLocation(illuminated_shader_program.id, "projection_matrix")
	local illuminated_basic_texture_uniform = GL.glGetUniformLocation(illuminated_shader_program.id, "basic_texture")
	local illuminated_light_texture_uniform = GL.glGetUniformLocation(illuminated_shader_program.id, "light_texture")
	
	GL.glUniform1i(illuminated_basic_texture_uniform, 0)
	GL.glUniform1i(illuminated_light_texture_uniform, 2)



	local highlights_shader_program = GLSL_program()
	highlights_shader_program:attach(highlights_vertex_shader)
	highlights_shader_program:attach(highlights_fragment_shader)
	highlights_shader_program:use()

	local highlights_projection_matrix_uniform = GL.glGetUniformLocation(highlights_shader_program.id, "projection_matrix")
	local highlights_basic_texture_uniform = GL.glGetUniformLocation(highlights_shader_program.id, "basic_texture")
	local highlights_light_texture_uniform = GL.glGetUniformLocation(highlights_shader_program.id, "light_texture")
	
	GL.glUniform1i(highlights_basic_texture_uniform, 0)
	GL.glUniform1i(highlights_light_texture_uniform, 2)



	local my_shader_program = GLSL_program()
	my_shader_program:attach(my_vertex_shader)
	my_shader_program:attach(my_fragment_shader)
	my_shader_program:use()
	
	
	local projection_matrix_uniform = GL.glGetUniformLocation(my_shader_program.id, "projection_matrix")
	local basic_texture_uniform = GL.glGetUniformLocation(my_shader_program.id, "basic_texture")
	
	GL.glUniform1i(basic_texture_uniform, 0)
	
	local my_smoke_program = GLSL_program()
	my_smoke_program:attach(fullscreen_vertex_shader)
	my_smoke_program:attach(smoke_fragment_shader)
	my_smoke_program:use()
	
	local smoke_texture_uniform = GL.glGetUniformLocation(my_smoke_program.id, "smoke_texture")
	local smoke_light_uniform = GL.glGetUniformLocation(my_smoke_program.id, "light_texture")
	GL.glUniform1i(smoke_texture_uniform, 1)
	GL.glUniform1i(smoke_light_uniform, 2)
	
	local blink_timer = timer()

	return owner_world:create_entity (override(camera_archetype, {
		transform = {
			pos = vec2(),
			rotation = 0
		},
	
		camera = {
			screen_rect = rect_xywh(0, 0, config_table.resolution_w, config_table.resolution_h),
			size = vec2(config_table.resolution_w, config_table.resolution_h),
			
			drawing_callback = function (subject, camera_draw_input, mask)

				subject.script.owner_scene.all_atlas:bind()
				-- now assuming that the atlas is already bound upon setting this scene to current
			
				local renderer = camera_draw_input.output
				local visible_area = camera_draw_input.visible_area
				renderer:generate_layers(mask)
				

				subject.script.owner_scene.owner_client_screen.systems.light:process_entities(renderer, camera_draw_input)

				my_shader_program:use()

				GL.glUniformMatrix4fv(
				projection_matrix_uniform, 
				1, 
				GL.GL_FALSE, 
				orthographic_projection(0, visible_area.x, visible_area.y, 0, 0, 1):data()
				)

				
				smoke_fbo:use()
				GL.glClear(GL.GL_COLOR_BUFFER_BIT)
				
				renderer:draw_layer(camera_draw_input, render_layers.SMOKES)
				renderer:call_triangles()
				renderer:clear_triangles()
				
				framebuffer_object.use_default()
				
				
				illuminated_shader_program:use()
				
				GL.glUniformMatrix4fv(
				illuminated_projection_matrix_uniform, 
				1, 
				GL.GL_FALSE, 
				orthographic_projection(0, visible_area.x, visible_area.y, 0, 0, 1):data()
				)


				renderer:draw_layer(camera_draw_input, render_layers.GROUND)
				renderer:draw_layer(camera_draw_input, render_layers.UNDER_CORPSES)
				renderer:draw_layer(camera_draw_input, render_layers.ON_GROUND)
				renderer:draw_layer(camera_draw_input, render_layers.SHELLS)
				renderer:draw_layer(camera_draw_input, render_layers.LEGS)
				renderer:draw_layer(camera_draw_input, render_layers.WIELDED_MELEE)


				local shining_tiles = int_vector()
				shining_tiles:add(1)

				for m=1, 1 do
					blink_timer:reset()
					for i=1, #subject.script.owner_scene.tile_layers do
						camera_draw_input.transform.pos = vec2(0, 0)
						camera_draw_input.additional_info = nil

						local coordinate = get_random_coordinate_on_a_special_tile (subject.script.owner_scene.tile_layers[i], shining_tiles, camera_draw_input)
						if coordinate.x > -1 then


						coordinate = coordinate + vec2_i(randval(0, 32), randval(0, 32))

						local blink_entity = subject.script.owner_scene.world_object:create_entity {
							render = {
								model = nil,
								layer = render_layers.SPECULAR_HIGHLIGHTS
							},

							transform = {
								pos = vec2(coordinate.x, coordinate.y),
								rotation = randval(0, 0)
							},

							animate = {

							}
						}

						local msg = animate_message()

						msg.set_animation = subject.script.owner_scene.blink_animation
						msg.preserve_state_if_animation_changes = false
						msg.change_animation = true
						msg.change_speed = true
						msg.speed_factor = 1
						msg.subject = blink_entity
						msg.message_type = animate_message.START
						msg.animation_priority = 1
						
						subject.owner_world:post_message(msg)
						end

					end
				end




				renderer:call_triangles()
				renderer:clear_triangles()
				
				highlights_shader_program:use()

				GL.glUniformMatrix4fv(
				illuminated_projection_matrix_uniform, 
				1, 
				GL.GL_FALSE, 
				orthographic_projection(0, visible_area.x, visible_area.y, 0, 0, 1):data()
				)


				renderer:draw_layer(camera_draw_input, render_layers.SPECULAR_HIGHLIGHTS)


				renderer:call_triangles()
				renderer:clear_triangles()

				illuminated_shader_program:use()


				renderer:draw_layer(camera_draw_input, render_layers.PLAYERS)
				renderer:draw_layer(camera_draw_input, render_layers.WIELDED_GUNS)
				renderer:draw_layer(camera_draw_input, render_layers.OBJECTS)

				renderer:call_triangles()
				renderer:clear_triangles()
				
				GL.glActiveTexture(GL.GL_TEXTURE1)
				GL.glBindTexture(GL.GL_TEXTURE_2D, smoke_fbo:get_texture_id())
				GL.glActiveTexture(GL.GL_TEXTURE0)
				
				my_smoke_program:use()
				
				GL.glBegin(GL.GL_QUADS)	
					GL.glVertexAttrib2f(0,1,1)
					GL.glVertexAttrib2f(0,1,0)
					GL.glVertexAttrib2f(0,0,0)
					GL.glVertexAttrib2f(0,0,1)
				GL.glEnd()
				

				my_shader_program:use()

				renderer:draw_layer(camera_draw_input, render_layers.BULLETS)
				renderer:draw_layer(camera_draw_input, render_layers.EFFECTS)
				

				renderer:call_triangles()
				renderer:clear_triangles()

				illuminated_shader_program:use()

				renderer:draw_layer(camera_draw_input, render_layers.HEALTH_BARS)
				owner_world.owner_client_screen.systems.label:draw_labels(camera_draw_input)

				renderer:call_triangles()
				renderer:clear_triangles()
				
				my_shader_program:use()

				renderer:draw_layer(camera_draw_input, render_layers.INVENTORY_SLOTS)
				renderer:draw_layer(camera_draw_input, render_layers.INVENTORY_ITEMS)
				
				renderer:draw_layer(camera_draw_input, render_layers.CROSSHAIRS)
				

				
				owner_world.owner_client_screen.my_gui:draw_call(camera_draw_input)
				renderer:call_triangles()
				--renderer:draw_debug_info(camera_draw_input.visible_area, camera_draw_input.camera_transform, blank_sprite.tex)
				renderer:clear_triangles()
				
			end
		},
		
		input = {
			intent_message.SWITCH_LOOK,
			custom_intents.ZOOM_CAMERA
		},
		
		chase = {},
		
		script_class = camera_class
	}))
end