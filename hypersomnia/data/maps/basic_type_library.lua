local dynamic_object_archetype = {
	physics = {
		body_type = Box2D.b2_dynamicBody,
		
		body_info = {
			shape_type = physics_info.POLYGON,
			filter = filter_objects,
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
			filter = filter_objects,
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
	}
}