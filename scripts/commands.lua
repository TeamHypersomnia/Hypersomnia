print("Calling commands.lua...")
--visibility_system.draw_discontinuities = 1
target_seek_steering.force_color = rgba(255, 255, 255, 255)
forward_seek_steering.force_color = rgba(0, 255, 255, 255)
containment_steering.force_color = rgba(255, 0, 0, 255)
visibility_system.draw_cast_rays = 0

visibility_system.epsilon_ray_distance_variation = 0.01
--print(player.body.transform.current.pos.x)
--ai_system.draw_triangle_edges = 1
--ai_system.draw_cast_rays = 0
----ai_system.draw_visibility = 0
----
--render_system.visibility_expansion = 1
--ai_system.draw_memorised_walls = 0



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

