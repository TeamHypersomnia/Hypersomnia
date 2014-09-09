melee_system = inherits_from (processing_system)

--function weapon_system:constructor()
--	processing_system.constructor(self)
--end

function melee_system:process_swinging()
	local msgs = self.owner_entity_system.messages["begin_swinging"]
	
	for i=1, #msgs do
		local msg = msgs[i]
		local target = msg.subject
		local weapon = target.weapon
		local entity = target.cpp_entity
		
		-- try to recover the owner entity if we're dealing with an item
		if target.item and target.item.wielder then
			entity = target.item.wielder.cpp_entity
		end
		
		local anim_msg = animate_message()
		
		if weapon.current_swing_direction then
			anim_msg.animation_type = animation_events.SWING_CW
			entity.movement.animation_message = animation_events.MOVE_CCW
		else
			anim_msg.animation_type = animation_events.SWING_CCW
			entity.movement.animation_message = animation_events.MOVE_CW
		end
		
		anim_msg.preserve_state_if_animation_changes = false
		anim_msg.change_animation = true
		anim_msg.change_speed = true
		anim_msg.speed_factor = 1
		anim_msg.subject = entity
		anim_msg.message_type = animate_message.START
		anim_msg.animation_priority = 10
		
		target.cpp_entity.owner_world:post_message(anim_msg)
		weapon.current_swing_direction = not weapon.current_swing_direction
	end
	
	msgs = self.owner_entity_system.messages["swing_hitcheck"]
	
	for i=1, #msgs do
	
	end
end