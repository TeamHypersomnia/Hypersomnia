should_debug_draw = false

function pv(v)
	--print(v.x, v.y)
end

function debug_draw(p1, p2, r, g, b, a)
	--pv(p1)
	--pv(p2)
	--if should_debug_draw then render_system:push_non_cleared_line(debug_line(p1*50, p2*50, rgba(r,g,b,a))) end
	render_system:push_line(debug_line(p1*50, p2*50, rgba(r,g,b,a)))
end

function simple_integration(p, dt)
	local new_p = {}
	
	new_p.acc = p.acc
	new_p.vel = p.vel + p.acc * dt 
	new_p.pos = p.pos + new_p.vel * dt
	-- uncomment this if you want to use quadratic integration
	-- but with small timesteps even this is an overkill since Box2D itself uses traditional Euler
	-- and I found that for calculations to be accurate I either way must keep the timesteps very low at the beginning of the jump
	 --+ p.acc * dt * dt * 0.5
	
	return new_p
end

function point_below_segment(a, b, p)
	-- make sure a is to the right
	if a.x < b.x then a,b = b,a end
	
	return ((b.x - a.x)*(p.y - a.y) - (b.y - a.y)*(p.x - a.x)) < 0
end

function calc_air_resistance(vel, mult)
	return (vec2(vel):normalize() * (-1) * mult * vel:length_sq())
end


-- returns true or false
function can_point_be_reached_by_jump
(
gravity, -- vector (meters per seconds^2)
movement_force, -- vector (meters per seconds^2)
air_resistance_mult, -- scalar
queried_point, -- vector (meters)
starting_position, -- vector (meters)
starting_velocity, -- vector (meters per seconds)
jump_impulse, -- vector (meters per seconds)

jetpack_impulse,
jetpack_steps,

mass -- scalar (kilogrammes)
)

--	print "\n\nBEGIN\n\n"
----	
--pv(gravity) -- vector (meters per seconds^2)
--pv(movement_force) -- vector (meters per seconds^2)
--print(air_resistance_mult) -- scalar
--pv(queried_point) -- vector (meters)
--pv(starting_position) -- vector (meters)
--pv(starting_velocity) -- vector (meters per seconds)
--pv(jump_impulse) -- vector (meters per seconds)
--pv(jetpack_impulse)
--print(jetpack_steps)
--print(mass) -- scalar (kilogrammes)
	
	local my_point = {
		pos = starting_position,
		vel = starting_velocity + jump_impulse/mass
	}
	
	local direction_left = queried_point.x < starting_position.x
	
	if direction_left then movement_force.x = movement_force.x * -1 end
	
	local step = 1/60
	
	debug_draw(queried_point, starting_position, 0, 255, 255, 255)
	
	while true do
		if jetpack_steps > 0 then
			my_point.vel = my_point.vel + jetpack_impulse/mass
			jetpack_steps = jetpack_steps - 1
		end
		
		-- calculate resultant force
		my_point.acc = 
		-- air resistance (multiplier * squared length of the velocity * opposite normalized velocity)
		calc_air_resistance(my_point.vel, air_resistance_mult) 
		-- remaining forces
		+ movement_force
		
		my_point.acc = my_point.acc * (1/mass)
		my_point.acc = my_point.acc + gravity
		
		-- i have discarded any timestep optimizations at the moment as they are very context specific
		local new_p = simple_integration(my_point, step)
		
		debug_draw(my_point.pos, new_p.pos, 255, 0, 255, 255)
		debug_draw(new_p.pos, new_p.pos+vec2(0, -1), 255, 255, 0, 255)
		
		if (direction_left and new_p.pos.x < queried_point.x) or (not direction_left and new_p.pos.x > queried_point.x) then
			if point_below_segment(new_p.pos, my_point.pos, queried_point) then
				debug_draw(new_p.pos, my_point.pos, 255, 255, 255, 255)
				--print "can jump!!"
				return true
			else
				debug_draw(new_p.pos, my_point.pos, 255, 0, 0, 255)
				return false
			end
		else 
			my_point = new_p
		end
	end

	return false
end

function calc_max_jump_height(
gravity, -- vector (meters per seconds^2)
air_resistance_mult, -- scalar
jump_impulse, -- vector (meters per seconds)

jetpack_impulse,
jetpack_steps,

mass -- scalar (kilogrammes)
)
	local my_point = {
		pos = vec2(0, 0),
		vel = jump_impulse/mass
	}
	
	local step = 1/60
	
	while true do	
		if jetpack_steps > 0 then
			my_point.vel = my_point.vel + jetpack_impulse/mass
			jetpack_steps = jetpack_steps - 1
		end
		
		-- calculate resultant force
		my_point.acc = 
		-- air resistance (multiplier * squared length of the velocity * opposite normalized velocity)
		calc_air_resistance(my_point.vel, air_resistance_mult) 
		
		my_point.acc = my_point.acc * (1/mass)
		my_point.acc = my_point.acc + gravity
		
		local new_p = simple_integration(my_point, step)
		
		if new_p.pos.y >= my_point.pos.y then 
			return -my_point.pos.y
		else my_point = new_p end
	end
	
	return 0
end