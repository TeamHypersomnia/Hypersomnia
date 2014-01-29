current_zoom_level = 0

function set_zoom_level(camera)
	local mult = 1 + (current_zoom_level / 1000)
	local new_w = config_table.resolution_w*mult
	local new_h = config_table.resolution_h*mult
	camera.camera.ortho = rect_ltrb(rect_xywh((config_table.resolution_w-new_w)/2, (config_table.resolution_h-new_h)/2, new_w, new_h))
	
	--player.crosshair:get().crosshair.size_multiplier = vec2(mult, mult)
	--target_entity.crosshair.size_multiplier = vec2(mult, mult)
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
		averages_per_sec = 5,
		
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
film_grain_vertex_shader = GLSL_shader(GL.GL_VERTEX_SHADER, [[
#version 330

layout(location = 0) in vec2 position;

//smooth out vec2 theTexcoord;

void main() 
{
	gl_Position = (vec4(position.xy, 0, 1) * 2.0) - 1.0;
	//theTexcoord = position;
}

]])

film_grain_fragment_shader = GLSL_shader(GL.GL_FRAGMENT_SHADER, [[
#version 330
uniform int time;

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint hash( uint x ) {
    x += ( x << 10u );
    x ^= ( x >>  6u );
    x += ( x <<  3u );
    x ^= ( x >> 11u );
    x += ( x << 15u );
    return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint hash( uvec2 v ) { return hash( v.x ^ hash(v.y)                         ); }
uint hash( uvec3 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z)             ); }
uint hash( uvec4 v ) { return hash( v.x ^ hash(v.y) ^ hash(v.z) ^ hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float floatConstruct( uint m ) {
    const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
    const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

    m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
    m |= ieeeOne;                          // Add fractional part to 1.0

    float  f = uintBitsToFloat( m );       // Range [1:2]
    return f - 1.0;                        // Range [0:1]
}



// Pseudo-random value in half-open range [0:1].
float random( float x ) { return floatConstruct(hash(floatBitsToUint(x))); }
float random( vec2  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec3  v ) { return floatConstruct(hash(floatBitsToUint(v))); }
float random( vec4  v ) { return floatConstruct(hash(floatBitsToUint(v))); }

out vec4 outputColor;

void main() 
{
	vec3  inputs = vec3( gl_FragCoord.xy, time ); // Spatial and temporal inputs
    float rand   = random( inputs );              // Random per-pixel value
    vec3  luma   = vec3( rand );                  // Expand to RGB
	
    outputColor = vec4(luma, 0.1);
}

]])

my_vertex_shader = GLSL_shader(GL.GL_VERTEX_SHADER, [[
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

]])
 
my_fragment_shader = GLSL_shader(GL.GL_FRAGMENT_SHADER, [[
#version 330
smooth in vec4 theColor;
 in vec2 theTexcoord;

out vec4 outputColor;

uniform sampler2D basic_texture;

void main() 
{
    outputColor = theColor * texture(basic_texture, theTexcoord);
}

]])

film_grain_program = GLSL_program()
film_grain_program:attach(film_grain_vertex_shader)
film_grain_program:attach(film_grain_fragment_shader)
film_grain_program:use()
time_uniform = GL.glGetUniformLocation(film_grain_program.id, "time")

my_shader_program = GLSL_program()
my_shader_program:attach(my_vertex_shader)
my_shader_program:attach(my_fragment_shader)
my_shader_program:use()

projection_matrix_uniform = GL.glGetUniformLocation(my_shader_program.id, "projection_matrix")
basic_texture_uniform = GL.glGetUniformLocation(my_shader_program.id, "basic_texture")

GL.glUniform1i(basic_texture_uniform, 0)

local my_timer = timer()


world_camera = create_entity (archetyped(camera_archetype, {
	transform = {
		pos = vec2(),
		rotation = 0
	},

	camera = {
		screen_rect = rect_xywh(0, 0, config_table.resolution_w, config_table.resolution_h),
		ortho = rect_ltrb(0, 0, config_table.resolution_w, config_table.resolution_h),
		
		drawing_callback = function (subject, renderer, visible_area, drawn_transform, target_transform, mask)
			my_shader_program:use()
			my_atlas:bind()
			renderer:generate_triangles(visible_area, drawn_transform, mask)
			
			GL.glUniformMatrix4fv(
			projection_matrix_uniform, 
			1, 
			GL.GL_FALSE, 
			orthographic_projection(visible_area.x, visible_area.r, visible_area.b, visible_area.y, 0, 1):data()
			)
			
			renderer:default_render(visible_area)
			
			--GL.glDisable(GL.GL_TEXTURE_2D)
			renderer:draw_debug_info(drawn_transform)
			--GL.glEnable(GL.GL_TEXTURE_2D)
			
			renderer:clear_triangles()
			
			film_grain_program:use()
			GL.glUniform1i(time_uniform, my_timer:get_milliseconds())
			
			GL.glBegin(GL.GL_QUADS)	
				GL.glVertexAttrib2f(0,1,1);
				GL.glVertexAttrib2f(0,1,0);
				GL.glVertexAttrib2f(0,0,0);
				GL.glVertexAttrib2f(0,0,1);
			GL.glEnd()
			
		end
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
