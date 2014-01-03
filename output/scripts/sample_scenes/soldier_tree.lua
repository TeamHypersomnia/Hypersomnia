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
		if get_scripted(entity).current_weapon ~= bare_hands then
			return behaviour_node.SUCCESS
		end
		return behaviour_node.FAILURE
	end
}

escape_archetype = {
	on_enter = function(entity, current_task)
		current_task:interrupt_runner(behaviour_node.FAILURE)
		
		entity.pathfinding.favor_velocity_parallellness = true
		entity.pathfinding.custom_exploration_hint.enabled = true
		entity.pathfinding:start_exploring()
	end,
	
	on_update = function(entity)
		set_max_speed(entity, 3000)
		entity.pathfinding.custom_exploration_hint.origin = entity.transform.current.pos
		entity.pathfinding.custom_exploration_hint.target = entity.transform.current.pos - (get_scripted(entity).target_entities.last_seen.transform.current.pos - entity.transform.current.pos)
		render_system:push_line(debug_line(entity.pathfinding.custom_exploration_hint.origin, entity.pathfinding.custom_exploration_hint.target, rgba(255, 255, 255, 255)))
		return behaviour_node.RUNNING
	end,
	
	on_exit = function(entity, status)
		entity.pathfinding.custom_exploration_hint.enabled = false
		entity.pathfinding:clear_pathfinding_info()
	end
}

function gun_trigger(entity, flag)
	local pull_trigger = intent_message()
	pull_trigger.subject = entity
	pull_trigger.intent = intent_message.SHOOT
	pull_trigger.state_flag = flag
	world:post_message(pull_trigger)
end

npc_legs_behaviour_tree = create_behaviour_tree {
	decorators = {
		delay_chase = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 400
		}
	},
	
	nodes = {
		legs = {
			node_type = behaviour_node.SELECTOR,
			default_return = behaviour_node.SUCCESS,
			skip_to_running_child = 0
		},
		
		player_seen_before = archetyped(player_seen_before_archetype, {}),
		
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
						local dist_len = dist:length_sq()
						if dist_len < nearest_gun.distance then
							nearest_gun = { item = v.item, distance = dist_len } 
						end
					end
					
					set_max_speed(entity, 3000)
			
					get_scripted(entity):pursue_target(nearest_gun.item)
					if get_scripted(entity):pick_up_weapon(nearest_gun.item) then
--print("picked up dropped..")					

						get_scripted(entity):stop_pursuit()
						return behaviour_node.SUCCESS 
					end
					
					return behaviour_node.RUNNING
				end
				return behaviour_node.FAILURE
			end,
			
			on_exit = function(entity, code)
				get_scripted(entity):stop_pursuit()
			end
		},
		
		player_visible = {
			node_type = behaviour_node.SELECTOR,
			on_update = function(entity) 
				if not player.body:exists() then return behaviour_node.FAILURE end
				
				if get_scripted(entity).is_seen then 
					entity.lookat.target:set(player.body:get())
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
		
		stand_still = {
			on_update = function(entity)
				if player.body:exists() and entity.gun.current_rounds > 0 then
					set_max_speed(entity, 1000)
					get_scripted(entity):pursue_target(player.body:get())	
					get_scripted(entity).steering_behaviours.wandering.weight_multiplier = 1.0
					return behaviour_node.RUNNING
				end
				return behaviour_node.FAILURE
			end,
			
			on_exit = function(entity, code)
				get_scripted(entity).steering_behaviours.wandering.weight_multiplier = 0.2 
				get_scripted(entity):stop_pursuit()
			end
		},
		
		chase_him = {
			on_update = function(entity)
				if not player.body:exists() then return behaviour_node.FAILURE end
				
				if get_scripted(entity).current_weapon ~= bare_hands then
					local npc_info = get_scripted(entity)
					
					set_max_speed(entity, 3000)
					--npc_behaviour_tree.delay_chase.maximum_running_time_ms = 0
					
					get_scripted(entity):pursue_target(player.body:get())
				
					return behaviour_node.RUNNING
				end
				return behaviour_node.FAILURE
			end,
			
			on_exit = function(entity, status)
				get_scripted(entity):stop_pursuit()
			end
		},
		
		escape = escape_archetype,
		
		anything_in_hands = archetyped(anything_in_hands_archetype, { node_type = behaviour_node.SEQUENCER }),
		
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
		
		go_to_last_seen = {
			on_enter = function(entity, current_task)		
				current_task:interrupt_runner(behaviour_node.FAILURE)
				local npc_info = get_scripted(entity)
				
				local temporary_copy = vec2(npc_info.target_entities.last_seen.transform.current.pos.x, npc_info.target_entities.last_seen.transform.current.pos.y)
				local target_point_pushed_away = physics_system:push_away_from_walls(temporary_copy, 100, 10, create(b2Filter, filter_pathfinding_visibility), entity)
				npc_info.target_entities.last_seen.transform.current.pos = target_point_pushed_away
				
				render_system:push_non_cleared_line(debug_line(temporary_copy, target_point_pushed_away, rgba(0, 255, 0, 255)))
				
				entity.pathfinding:start_pathfinding(target_point_pushed_away)
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
			on_enter = function(entity, current_task)
				current_task:interrupt_runner(behaviour_node.FAILURE)
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
				entity.pathfinding:clear_pathfinding_info()
			end
		},
		
		try_to_evade = archetyped(escape_archetype, {}),
		
		walk_around = {
			default_return = behaviour_node.RUNNING,
			
			on_enter = function(entity, current_task)
				current_task:interrupt_runner(behaviour_node.FAILURE)
				--npc_behaviour_tree.delay_chase.maximum_running_time_ms = 400
				set_max_speed(entity, 700)
				entity.pathfinding:clear_pathfinding_info()
				--entity.pathfinding:start_exploring()
				entity.pathfinding.favor_velocity_parallellness = true
				get_scripted(entity).steering_behaviours.wandering.weight_multiplier = 0.0 
				--get_scripted(entity).steering_behaviours.forward_seeking.enabled = true 
			end,
			
			on_exit = function(entity, status)
				entity.pathfinding:clear_pathfinding_info()
			end
		},
		
		try_melee_range = {
			on_update = function(entity) 
				if anything_in_hands_archetype.on_update(entity) == behaviour_node.SUCCESS 
					and get_scripted(entity).is_seen 
				then
					return behaviour_node.SUCCESS
				end
				
				return behaviour_node.FAILURE
			end
		}
	},
	
	connections = {
		legs = {
			"player_seen_before", "player_visible", "is_alert", "walk_around"
		},
		
		player_seen_before = {
			"get_better_weapon"
		},
		
		player_visible = {
			"stand_still", "chase_him", "escape"
		},
		
		is_alert = {
			"anything_in_hands", "try_to_evade" 	
		},
		
		anything_in_hands = {
			"go_to_last_seen", "follow_hint"
		}
	},
	
	root = "legs"
}


npc_hands_behaviour_tree = create_behaviour_tree {
	decorators = {},
	
	nodes = {
		hands = {
			node_type = behaviour_node.SEQUENCER,
			default_return = behaviour_node.SUCCESS
		},
		
		player_visible = {
			on_update = function(entity) 
				if get_scripted(entity).is_seen then 
					return behaviour_node.SUCCESS
				end
				return behaviour_node.FAILURE
			end
		},
		
		anything_in_hands = archetyped(anything_in_hands_archetype, { node_type = behaviour_node.SELECTOR, skip_to_running_child = 0 }),
		
		melee_range = {
			on_update = function(entity) 
				if player.body:exists() then 
					if (player.body:get().transform.current.pos - entity.transform.current.pos):length() < 100 then
						entity.gun.is_melee = true
						gun_trigger(entity, true)
						return behaviour_node.RUNNING
					end
				end
				return behaviour_node.FAILURE
			end,
			
			on_exit = function(entity) 
				gun_trigger(entity, false)
			end
			
		},
		
		try_to_shoot = {
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
	},
	
	connections = {
		hands = {
			"player_visible", "anything_in_hands"
		},
		
		anything_in_hands = {
			"melee_range", "try_to_shoot"
		}
	},
	
	root = "hands"
}

npc_alertness = create_behaviour_tree {
	decorators = {
		temporary_alertness = {
			decorator_type = behaviour_timer_decorator,
			maximum_running_time_ms = 5000
		}
	},
	
	nodes = {
		behave = {
			default_return = behaviour_node.SUCCESS
		},
	
		limit_alertness = {
			decorator_chain = "temporary_alertness",
			
			on_update = function(entity)
				if get_scripted(entity).is_alert and not get_scripted(entity).is_seen then 
					return behaviour_node.RUNNING 
				end
				return behaviour_node.FAILURE
			end,
			
			on_exit = function(entity, code)
				if code == behaviour_node.SUCCESS then get_scripted(entity).is_alert = false end
			end
		}
	},
	
	connections = {
		behave = {
			"limit_alertness"
		}
	},
	
	root = "behave"
}

