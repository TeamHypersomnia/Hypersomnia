npc_behaviour_tree = create_behaviour_tree {

	decorators = {
		temporarily_follow_hint = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 5000
		},
	
		timed_exploration = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 1000
		}
	},
	
	nodes = {
		behave = {
			node_type = behaviour_node.SELECTOR,
			default_return = behaviour_node.SUCCESS
		},
		
		player_visible = {
			on_update = function(entity) 
				local p1 = entity.transform.current.pos
				local p2 = player.body.transform.current.pos
				
				
				ray_output = physics_system:ray_cast(p1, p2, create(b2Filter, filter_pathfinding_visibility), entity)
				
				if not ray_output.hit then
					local npc_info = get_scripted(entity)
					npc_info.was_seen = true
					npc_info.target_entities.last_seen.transform.current.pos = player.body.transform.current.pos
					
					return behaviour_node.SUCCESS 
				end
				
				return behaviour_node.FAILURE
			end
		},
		
		chase_him = {
			on_enter = function(entity)
				entity.pathfinding:clear_pathfinding_info()
				local npc_info = get_scripted(entity)
				npc_info.steering_behaviours.pursuit.target_from:set(player.body)
				npc_info.steering_behaviours.pursuit.enabled = true
				npc_info.steering_behaviours.obstacle_avoidance.enabled = false
				npc_info.steering_behaviours.sensor_avoidance.target_from:set(player.body)
			end,
			
			on_exit = function(entity, status)
				local npc_info = get_scripted(entity)
				npc_info.steering_behaviours.pursuit.enabled = false
				npc_info.steering_behaviours.obstacle_avoidance.enabled = true
				npc_info.steering_behaviours.sensor_avoidance.target_from:set(npc_info.target_entities.navigation)
				
				local player_vel = player.body.physics.body:GetLinearVelocity()
				entity.pathfinding.custom_exploration_hint.origin = player.body.transform.current.pos
				entity.pathfinding.custom_exploration_hint.target = player.body.transform.current.pos + (vec2(player_vel.x, player_vel.y) * 50)
			end
		},
		
		player_seen_before = {
			node_type = behaviour_node.SEQUENCER,
			on_update = function(entity)
				local self = get_scripted(entity)
				
				if self.was_seen then
					return behaviour_node.SUCCESS 
				else
					return behaviour_node.FAILURE
				end
				
				return behaviour_node.FAILURE
			end
		},
		
		
		begin_pathfinding_to_last_seen = {
			default_return = behaviour_node.SUCCESS,
			
			on_enter = function(entity)
				local npc_info = get_scripted(entity)
				entity.pathfinding:start_pathfinding(npc_info.target_entities.last_seen.transform.current.pos)
			end,
			
			on_exit = function(entity, status)
				--entity.pathfinding:clear_pathfinding_info()
			end
		},
		
		go_to_last_seen = {
			on_enter = function(entity)
			--	local npc_info = get_scripted(entity)
			--	npc_info.steering_behaviours.pursuit.target_from:set(npc_info.last_seen)
			--	npc_info.steering_behaviours.pursuit.enabled = true
			end,
			
			on_update = function(entity)
				
				render_system:push_line(debug_line(entity.transform.current.pos, get_scripted(entity).target_entities.last_seen.transform.current.pos, rgba(255, 0, 0, 255)))
				render_system:push_line(debug_line(entity.pathfinding.custom_exploration_hint.origin, entity.pathfinding.custom_exploration_hint.target, rgba(255, 0, 0, 255)))
				
				if entity.pathfinding:is_still_pathfinding() then return behaviour_node.RUNNING end
				return behaviour_node.SUCCESS 
			end
			,
			
			on_exit = function(entity, status)
				entity.pathfinding:clear_pathfinding_info()
			end
		},
		
		follow_hint = {
			decorator_chain = "temporarily_follow_hint",
			
			on_enter = function(entity)
				entity.pathfinding.favor_velocity_parallellness = true
				entity.pathfinding.custom_exploration_hint.enabled = true
				entity.pathfinding:start_exploring()
			end,
			
			on_update = function(entity)
				render_system:push_line(debug_line(entity.pathfinding.custom_exploration_hint.origin, entity.pathfinding.custom_exploration_hint.target, rgba(255, 0, 255, 255)))
				return behaviour_node.RUNNING
			end,
			
			on_exit = function(entity, status)
				entity.pathfinding.custom_exploration_hint.enabled = false
				get_scripted(entity).was_seen = false
			end
		},
		
		--explore = {
		--	decorator_chain = "timed_exploration",
		--	
		--	on_enter = function(entity)
		--		entity.pathfinding.favor_velocity_parallellness = false
		--		entity.pathfinding.custom_exploration_hint.enabled = false
		--	end,
		--	
		--	on_exit = function(entity, status)
		--		--entity.pathfinding:clear_pathfinding_info()
		--		get_scripted(entity).was_seen = false
		--	end
		--	--,
		--	--
		--	--on_update = function(entity)
		--	--	if entity.pathfinding:is_still_pathfinding() then return behaviour_node.RUNNING end
		--	--	return behaviour_node.SUCCESS 
		--	--end
		--},
		
		idle = {
			default_return = behaviour_node.SUCCESS
		},
		
		walk_around = {
			default_return = behaviour_node.RUNNING,
			
			on_enter = function(entity)
				set_max_speed(entity, 700)
				entity.pathfinding:start_exploring()
				entity.pathfinding.favor_velocity_parallellness = true
				get_scripted(entity).steering_behaviours.wandering.weight_multiplier = 1.0 
			end,
			
			on_exit = function(entity, status)
				set_max_speed(entity, 3000)
				get_scripted(entity).steering_behaviours.wandering.weight_multiplier = 0.2
			end
		}
	},
	
	connections = {
		behave = {
			"player_visible", "player_seen_before", "idle"
		},
		
		idle = {
			"walk_around"
		},
		
		player_visible = {
			"chase_him"
		},
		
		player_seen_before = {
			"begin_pathfinding_to_last_seen", "go_to_last_seen", "follow_hint"
		}
	},
	
	root = "behave"
}
