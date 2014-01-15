blank_body = (create_entity {
	physics = {
		body_type = Box2D.b2_staticBody,
		
		body_info = {
			shape_type = physics_info.RECT,
			rect_size = vec2(10, 10),
			filter = filter_nothing
		}
	},
	
	transform = {
		pos = vec2(0, 0),
		rotation = 0
	}
}).physics.body