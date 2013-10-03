print("Executing commands.lua...")

my_effect = create_particle_effect(wood_effect)

msg = create(particle_burst_message, {
	pos = vec2(400, 300), 
	rotation = 180,
	set_effect = my_effect,
	subject = nil
})

world:post_message(msg)






