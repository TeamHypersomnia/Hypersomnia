local dynamic_object_archetype = {
	physics = {
		body_type = Box2D.b2_dynamicBody,
		
		body_info = {
			shape_type = physics_info.POLYGON,
			filter = filters.OBJECT,
			density = 1.0,
			friction = 0.1,
			
			linear_damping = 1,
			angular_damping = 1
		}
	}
}

local static_object_archetype = {
	physics = {
		body_type = Box2D.b2_staticBody,
		
		body_info = {
			shape_type = physics_info.POLYGON,
			filter = filters.STATIC_OBJECT,
			density = 1.0,
			friction = 0.1
		}
	}
}

return {
	default_type = {
		render_layer = "OBJECTS",
		texture = "crate.jpg",
		entity_archetype = static_object_archetype
	},
	
	wall_wood = {
		render_layer = "OBJECTS",
		texture = "wall2.jpg",
		entity_archetype = static_object_archetype
	},
	
	crate = {
		render_layer = "OBJECTS",
		texture = "crate.jpg",
		entity_archetype = dynamic_object_archetype
	},
	
	ground = {
		render_layer = "GROUND"
	},
	
	static = {
		render_layer = "OBJECTS",
		entity_archetype = static_object_archetype
	},	
	
	static_snow = {
		render_layer = "OBJECTS",
		texture = "wall2.jpg",
		entity_archetype = static_object_archetype
	},
	
	ground_snow = {
		render_layer = "GROUND",
		texture = "snow3.jpg"
	},
	
	dynamic = {
		render_layer = "OBJECTS",
		entity_archetype = dynamic_object_archetype
	}
}