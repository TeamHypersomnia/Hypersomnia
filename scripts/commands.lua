print("Calling commands.lua...")
physics_system.timestep_multiplier = 0.005

ai_system.draw_triangle_edges = 0
ai_system.draw_cast_rays = 1
ai_system.draw_memorised_walls = 0




--for i = 1, 100 do
--	msg = create(particle_burst_message, {
--		pos = vec2(i*40-300, i*40-300), 
--		rotation = 180,
--		set_effect = cached_wood_effect,
--		subject = nil
--	})
--
--	world:post_message(msg)
--end

