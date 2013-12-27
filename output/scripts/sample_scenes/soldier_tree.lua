player_seen_before_archetype = {
	node_type = behaviour_node.SELECTOR,
	on_update = function(entity)
		if get_scripted(entity).was_seen then
			return behaviour_node.SUCCESS 
		end
		return behaviour_node.FAILURE
	end
}

anything_in_hands_archetype = {
	--on_enter = function(entity) end,
	--on_exit = function(entity, c) end,
	on_update = function(entity)
		if(get_scripted(entity).current_weapon ~= bare_hands) then
			return behaviour_node.SUCCESS
		end
		return behaviour_node.FAILURE
	end
}

function gun_trigger(entity, flag)
	local pull_trigger = intent_message()
	pull_trigger.subject = entity
	pull_trigger.intent = intent_message.SHOOT
	pull_trigger.state_flag = flag
	world:post_message(pull_trigger)
end

npc_behaviour_tree = create_behaviour_tree {
	decorators = {
		temporarily_follow_hint = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 2000
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
		
		player_seen_before_first = archetyped(player_seen_before_archetype, {}),
		
		get_better_weapon = {
			on_update = function(entity) 
				-- take only guns with ammo
				local condition = function(weapon) return weapon.current_rounds > 0 end
				
				-- if we're armed, we can't get better weapon
				if condition(entity.gun) then return behaviour_node.FAILURE end
				
				-- if we only have bare hands
				local npc_info = get_scripted(entity)
				if entity.gun.is_melee and npc_info.current_weapon == bare_hands then 
					-- take anything
					condition = function(weapon) return true end
				end
				
				local visible_guns = {}
				
				for k, v in ipairs(global_item_table) do
					if condition(get_scripted(v).item_data.weapon_info) and not physics_system:ray_cast(entity.transform.current.pos, v.transform.current.pos, create(b2Filter, filter_pathfinding_visibility), entity).hit then
						table.insert(visible_guns, { item = v, distance = (entity.transform.current.pos - v.transform.current.pos):length_sq() })
					end
				end
				
				if #visible_guns > 0 then
					local nearest_gun = visible_guns[1]
					
					for k, v in ipairs(visible_guns) do
						local dist = (v.item.transform.current.pos - entity.transform.current.pos)
						if dist:length_sq() < nearest_gun.distance then
							nearest_gun = { item = v, distance = dist } 
						end
					end
					
					
					set_max_speed(entity, 3000)
					get_scripted(entity):pursue_target(nearest_gun.item)
					if get_scripted(entity):pick_up_weapon(nearest_gun.item) then
--print("picked up dropped..")					

				get_scripted(entity):stop_pursuit()
return 
						
					behaviour_node.SUCCESS end
					
					return behaviour_node.RUNNING
				end
				
				--print("no gun visible..")
				get_scripted(entity):stop_pursuit()
				return behaviour_node.FAILURE
			end,
		},
		
		player_visible = {
			node_type = behaviour_node.SEQUENCER,
			on_update = function(entity) 
				if get_scripted(entity).is_seen then 
					entity.lookat.target:set(player.body)
					entity.lookat.look_mode = lookat_component.POSITION
					return behaviour_node.SUCCESS
				end
					entity.lookat.target:set(entity)
					entity.lookat.look_mode = lookat_component.VELOCITY
					return behaviour_node.FAILURE
			end
		},
		
		delay = { decorator_chain = "delay_chase",
			on_enter = function(entity) end,
			on_exit = function(entity, status) end
		},
		
		anything_in_hands_first = archetyped(anything_in_hands_archetype, { node_type = behaviour_node.SELECTOR }),
		
		melee = {
	on_enter = function(entity) end,
	on_exit = function(entity, c) end,
		skip_to_running_child = 0,
			node_type = behaviour_node.SEQUENCER,
			default_return = behaviour_node.SUCCESS
		},
		
		melee_range = {
			on_update = function(entity) 
				if (player.body.transform.current.pos - entity.transform.current.pos):length() < 100 then
					entity.gun.is_melee = true
					gun_trigger(entity, true)
					return behaviour_node.RUNNING
				end
				
				return behaviour_node.SUCCESS
			end,
			
			on_exit = function(entity) 
				gun_trigger(entity, false)
			end
			
		},
		
		has_gun_and_ammo = {
			on_update = function(entity) 
				if entity.gun.current_rounds > 0 then
					entity.gun.is_melee = false
					gun_trigger(entity, true)
					return behaviour_node.RUNNING
				end
				
				return behaviour_node.FAILURE
			end,
			
			on_exit = function(entity) 
				gun_trigger(entity, false)
			end
		},
		
		chase_him = {
			on_enter = function(entity)
				
			end,
			
			on_update = function(entity)
				entity.pathfinding:clear_pathfinding_info()
				local npc_info = get_scripted(entity)
				
				set_max_speed(entity, 3000)
				npc_info.steering_behaviours.wandering.weight_multiplier = 0.2
				npc_behaviour_tree.delay_chase.maximum_running_time_ms = 0
				
				get_scripted(entity):pursue_target(player.body)
				return behaviour_node.RUNNING
			end,
			
			on_exit = function(entity, status)
				get_scripted(entity):stop_pursuit()
			end
		},
		
		is_alert = {
			node_type = behaviour_node.SELECTOR,
			on_update = function(entity)
				if get_scripted(entity).is_alert then
					set_max_speed(entity, 3000)
					return behaviour_node.SUCCESS 
				end
				return behaviour_node.FAILURE
			end
		},
		
		anything_in_hands_second = archetyped(anything_in_hands_archetype, { node_type = behaviour_node.SEQUENCER }),
		
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
				local npc_info = get_scripted(entity)
			
				entity.pathfinding.custom_exploration_hint.origin = npc_info.target_entities.last_seen.transform.current.pos
				entity.pathfinding.custom_exploration_hint.target = npc_info.target_entities.last_seen.transform.current.pos + (npc_info.last_seen_velocity * 50)
				
				render_system:push_line(debug_line(entity.transform.current.pos, get_scripted(entity).target_entities.last_seen.transform.current.pos, rgba(255, 0, 0, 255)))
				render_system:push_line(debug_line(entity.pathfinding.custom_exploration_hint.origin, entity.pathfinding.custom_exploration_hint.target, rgba(255, 0, 255, 255)))
				
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
				local npc_info = get_scripted(entity)
			
				--entity.pathfinding.custom_exploration_hint.origin = npc_info.target_entities.last_seen.transform.current.pos
				--entity.pathfinding.custom_exploration_hint.target = npc_info.target_entities.last_seen.transform.current.pos + (npc_info.last_seen_velocity * 50)
				
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
				get_scripted(entity).is_alert = false
			end
		},
		
		escape_and_evade = {
			decorator_chain = "temporarily_follow_hint",
			
			on_enter = function(entity)
				set_max_speed(entity, 3000)
				entity.pathfinding.favor_velocity_parallellness = true
				entity.pathfinding.custom_exploration_hint.enabled = true
				entity.pathfinding:start_exploring()
			end,
			
			on_update = function(entity)
				entity.pathfinding.custom_exploration_hint.origin = entity.transform.current.pos
				entity.pathfinding.custom_exploration_hint.target = entity.transform.current.pos - (get_scripted(entity).target_entities.last_seen.transform.current.pos - entity.transform.current.pos)
				render_system:push_line(debug_line(entity.pathfinding.custom_exploration_hint.origin, entity.pathfinding.custom_exploration_hint.target, rgba(255, 255, 255, 255)))
				return behaviour_node.RUNNING
			end,
			
			on_exit = function(entity, status)
				entity.pathfinding.custom_exploration_hint.enabled = false
				get_scripted(entity).is_alert = false
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
			"player_seen_before_first", "player_visible", "is_alert", "idle"
		},
		
		player_seen_before_first = {
			"get_better_weapon"
		},
		
		player_visible = {
			"anything_in_hands_first"
		},
		
		anything_in_hands_first = {
			"has_gun_and_ammo", "melee"
		},
		
		melee = {
			"melee_range", "chase_him"
		},
		
		idle = {
			"walk_around"
		},
		
		is_alert = {
			"anything_in_hands_second", "escape_and_evade" 	
		},
		
		anything_in_hands_second = {
			"begin_pathfinding_to_last_seen", "go_to_last_seen", "follow_hint"
		}
	},
	
	root = "behave"
}
