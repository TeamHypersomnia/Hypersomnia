player_seen_before_archetype = {
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
}

npc_behaviour_tree = create_behaviour_tree {

	decorators = {
		temporarily_follow_hint = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 5000
		},
	
		timed_exploration = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 1000
		},
		
		delay_chase = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 400
		}
	},
	
	nodes = {
		behave = {
			node_type = behaviour_node.SELECTOR,
			default_return = behaviour_node.SUCCESS
		},
			
		player_seen_before_first = player_seen_before_archetype,
		
		can_get_better_weapon = {
			on_update = function(entity) 
				-- take only guns with ammo
				local condition = function(weapon) return not weapon.is_melee and weapon.current_rounds > 0 end
				
				-- if we're armed, we can't get better weapon
				if condition(entity.gun) then return behaviour_node.FAILURE end
				
				-- if we only have bare hands
				local npc_info = get_scripted(entity)
				if entity.gun.is_melee and npc_info.current_weapon == bare_hands then 
					-- take anything
					condition = function(weapon) return true end
				end
				
				for k, v in ipairs(global_item_table) do
					
				end
			end
		},
		
			
		player_seen_before = player_seen_before_archetype,
		
		
		player_visible = {
			node_type = behaviour_node.SEQUENCER,
			on_update = function(entity) 
				local p1 = entity.transform.current.pos
				--print(debug.traceback())
				local p2 = player.body.transform.current.pos
				
				ray_output = physics_system:ray_cast(p1, p2, create(b2Filter, filter_pathfinding_visibility), entity)
				
				if not ray_output.hit then
					local npc_info = get_scripted(entity)
					npc_info.target_entities.last_seen.transform.current.pos = player.body.transform.current.pos
					
					return behaviour_node.SUCCESS 
				end
				
				return behaviour_node.FAILURE
			end
		},
		
		delay = { decorator_chain = "delay_chase",
			on_enter = function(entity) end,
			on_exit = function(entity, status) end
		},
		
		chase_him = {
			on_enter = function(entity)
				entity.lookat.target:set(player.body)
				entity.lookat.look_mode = lookat_component.POSITION
				
				entity.pathfinding:clear_pathfinding_info()
				local npc_info = get_scripted(entity)
				
				set_max_speed(entity, 3000)
				npc_info.steering_behaviours.wandering.weight_multiplier = 0.2
				npc_behaviour_tree.delay_chase.maximum_running_time_ms = 0
				
				npc_info.was_seen = true
				npc_info.steering_behaviours.pursuit.target_from:set(player.body)
				npc_info.steering_behaviours.pursuit.enabled = true
				npc_info.steering_behaviours.obstacle_avoidance.enabled = false
				npc_info.steering_behaviours.sensor_avoidance.target_from:set(player.body)
			end,
			
			on_update = function(entity)
				local pull_trigger = intent_message()
				pull_trigger.subject = entity
				pull_trigger.intent = intent_message.SHOOT
				pull_trigger.state_flag = true
				world:post_message(pull_trigger)
				return behaviour_node.RUNNING
			end,
			
			on_exit = function(entity, status)
				local pull_trigger = intent_message()
				
				pull_trigger.subject = entity
				pull_trigger.intent = intent_message.SHOOT
				pull_trigger.state_flag = false
				world:post_message(pull_trigger)
				
				entity.lookat.target:set(entity)
				entity.lookat.look_mode = lookat_component.VELOCITY
				
				local npc_info = get_scripted(entity)
				npc_info.steering_behaviours.pursuit.enabled = false
				npc_info.steering_behaviours.obstacle_avoidance.enabled = true
				npc_info.steering_behaviours.sensor_avoidance.target_from:set(npc_info.target_entities.navigation)
				
				local player_vel = player.body.physics.body:GetLinearVelocity()
				entity.pathfinding.custom_exploration_hint.origin = player.body.transform.current.pos
				entity.pathfinding.custom_exploration_hint.target = player.body.transform.current.pos + (vec2(player_vel.x, player_vel.y) * 50)
			end
		},
		
	
		
		begin_pathfinding_to_last_seen = {
			default_return = behaviour_node.SUCCESS,
			
			on_enter = function(entity)
				local npc_info = get_scripted(entity)
				
				local temporary_copy = vec2(npc_info.target_entities.last_seen.transform.current.pos.x, npc_info.target_entities.last_seen.transform.current.pos.y)
				local target_point_pushed_away = physics_system:push_away_from_walls(temporary_copy, 100, 10, create(b2Filter, filter_pathfinding_visibility), entity)
				npc_info.target_entities.last_seen.transform.current.pos = target_point_pushed_away
				
				render_system:push_non_cleared_line(debug_line(temporary_copy, target_point_pushed_away, rgba(0, 255, 0, 255)))
				
				entity.pathfinding:start_pathfinding(target_point_pushed_away)
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
				npc_behaviour_tree.delay_chase.maximum_running_time_ms = 400
				set_max_speed(entity, 700)
				entity.pathfinding:start_exploring()
				entity.pathfinding.favor_velocity_parallellness = true
				get_scripted(entity).steering_behaviours.wandering.weight_multiplier = 1.0 
			end,
			
			on_exit = function(entity, status)
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
			 "delay", "chase_him"
		},
		
		player_seen_before = {
			"begin_pathfinding_to_last_seen", "go_to_last_seen", "follow_hint"
		}
	},
	
	root = "behave"
}
