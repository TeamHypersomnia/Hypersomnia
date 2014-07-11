components.weapon = inherits_from()

components.weapon.states = create_enum {
	"READY",
	"SWINGING",
	"SHOOTING_INTERVAL",
	"SWINGING_INTERVAL"
} 

components.weapon.triggers = create_enum {
	"NONE",
	"SHOOT",
	"MELEE"
} 

function components.weapon:set_state(state)
	self.time_ms = 0
	self.state = components.weapon.states[state]
end

function components.weapon:passed(interval)
	return self.time_ms > self[interval]
end

function components.weapon:constructor(init_table)
	-- 0 - none
	-- 1 - shoot/melee
	-- 2 - melee
	
	self.trigger = components.weapon.triggers.NONE
	
	--self.current_rounds = 0
	--self.is_automatic = false
	--
	--self.bullets_once = 0
	--self.spread_degrees = 0
	--self.bullet_damage = minmax(0, 0)
	--self.bullet_speed = minmax(0, 0)
	--
	--self.shooting_interval_ms = 0
	--self.max_bullet_distance = 0
	--
	--self.bullet_distance_offset = vec2(0, 0)
	
	--self.bullet_entity = {}
	--
	--self.current_rounds = 0
	--self.shake_radius = 0
	--self.shake_spread_degrees = 0
	--self.is_automatic = false
	
	-- melee weapon
	--self.swing_duration = 0
	--self.swing_radius = 0
	--self.swing_angle = 0
	--self.swing_angular_offset = 0
	--self.swing_interval_ms = 0
	--self.query_vertices = 0
		
	-- internals 
	self.current_swing_direction = false
	self:set_state("READY")
	
	rewrite(self, init_table)
end