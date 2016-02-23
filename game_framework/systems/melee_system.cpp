//
//auto begin_swinging_routine = [&]() {
//	messages::animation_response_message msg;
//	msg.response = gun.current_swing_direction ? messages::animation_response_message::SWING_CW : messages::animation_response_message::SWING_CCW;
//	msg.preserve_state_if_animation_changes = false;
//	msg.change_animation = true;
//	msg.change_speed = true;
//	msg.speed_factor = 1.f;
//	msg.subject = it;
//	msg.action = messages::animation_message::START;
//	msg.animation_priority = 1;
//
//	parent_world.post_message(msg);
//
//	gun.current_swing_direction = !gun.current_swing_direction;
//};