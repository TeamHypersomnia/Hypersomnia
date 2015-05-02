light_system = inherits_from (processing_system)

function light_system:constructor(blank_texture)
	self.blank_texture = blank_texture
	self.light_fbo = framebuffer_object(config_table.resolution_w, config_table.resolution_h)
	processing_system.constructor(self)

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

	local light_fragment_shader_code = [[
	#version 330
	smooth in vec4 theColor;
	in vec2 theTexcoord;
	
	out vec4 outputColor;
	
	uniform vec2 light_pos;
	uniform vec3 light_attenuation;

	void main() 
	{	
		float light_distance = length(gl_FragCoord.xy - light_pos);
		vec4 final_color = theColor;
		final_color.a *= 1.0/(light_attenuation.x+light_attenuation.y*light_distance+light_attenuation.z*light_distance*light_distance); 

		outputColor = final_color;
	}
	]]

	self.my_vertex_shader = GLSL_shader(GL.GL_VERTEX_SHADER, vertex_shader_code)
	self.my_fragment_shader = GLSL_shader(GL.GL_FRAGMENT_SHADER, light_fragment_shader_code)

	self.my_light_program = GLSL_program()
	self.my_light_program:attach(self.my_vertex_shader)
	self.my_light_program:attach(self.my_fragment_shader)
	self.my_light_program:use()

	self.light_pos_uniform = GL.glGetUniformLocation(self.my_light_program.id, "light_pos")
	self.light_attenuation_uniform = GL.glGetUniformLocation(self.my_light_program.id, "light_attenuation")

	GL.glUniform3f(self.light_attenuation_uniform, 1, 0.00002, 0.00004)

	self.projection_matrix_uniform = GL.glGetUniformLocation(self.my_light_program.id, "projection_matrix")

	self.delta_timer = timer()

end

function light_system:get_required_components()
	return { "light" }
end

function light_system:process_entities(renderer, camera_draw_input)
	renderer:call_triangles()
	renderer:clear_triangles()

	self.light_fbo:use()

	GL.glClearColor(0, 0.1, 0.0, 1.0)
	GL.glClear(GL.GL_COLOR_BUFFER_BIT)
	GL.glClearColor(0, 0, 0, 0)

	local visible_area = camera_draw_input.visible_area

	self.my_light_program:use()

	GL.glUniformMatrix4fv(
	self.projection_matrix_uniform, 
	1, 
	GL.GL_FALSE, 
	orthographic_projection(0, visible_area.x, visible_area.y, 0, 0, 1):data()
	)

	local delta = self.delta_timer:extract_seconds()

	for i=1, #self.targets do
		local target = self.targets[i]
		local light = target.light
		local pos = target.cpp_entity.transform.current.pos


		for i=1, #light.attenuation_variations do
			local variation = light.attenuation_variations[i]

			variation.value = variation.value + delta * randval(-variation.change_speed, variation.change_speed)
		
			if variation.value < variation.min_value then
				variation.value = variation.min_value
			end
	
			if variation.value > variation.max_value then
				variation.value = variation.max_value
			end
		end

		local lighting_layer = target.cpp_entity.visibility:get_layer(visibility_layers.BASIC_LIGHTING)
		local my_light_poly = simple_create_polygon(vector_to_table(lighting_layer:get_polygon(1, pos, 0.0)))

		map_texture_to_polygon(my_light_poly, self.blank_texture, uv_mapping_mode.STRETCH)
		set_polygon_color(my_light_poly, light.color)

		camera_draw_input.transform.pos = vec2()
		camera_draw_input.transform.rotation = 0
		camera_draw_input.additional_info = nil

		local screen_pos = pos - camera_draw_input.camera_transform.pos
		screen_pos.x = screen_pos.x +0.5*config_table.resolution_w
		screen_pos.y = config_table.resolution_h-(screen_pos.y +0.5*config_table.resolution_h)
		
		local light_displacement = vec2(light.attenuation_variations[4].value, light.attenuation_variations[5].value)
		GL.glUniform2f(self.light_pos_uniform, screen_pos.x + light_displacement.x, screen_pos.y + light_displacement.y)

		my_light_poly:draw(camera_draw_input)
		
		GL.glUniform3f(self.light_attenuation_uniform, 1 + light.attenuation_variations[1].value, 0.00002 + light.attenuation_variations[2].value, 0.00007 + light.attenuation_variations[3].value)
		renderer:call_triangles()
		renderer:clear_triangles()

		GL.glUniform3f(self.light_attenuation_uniform, 1 + light.attenuation_variations[1].value, 0.00002 + light.attenuation_variations[2].value, 0.00017 + light.attenuation_variations[3].value)
		renderer:draw_layer(camera_draw_input, render_layers.OBJECTS)
	
		renderer:call_triangles()
		renderer:clear_triangles()
	end

	framebuffer_object.use_default()

	GL.glActiveTexture(GL.GL_TEXTURE2)
	GL.glBindTexture(GL.GL_TEXTURE_2D, self.light_fbo:get_texture_id())
	GL.glActiveTexture(GL.GL_TEXTURE0)
end