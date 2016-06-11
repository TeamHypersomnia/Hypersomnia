#include "entity_system/storage_for_message_queues.h"
#include "entity_system/storage_for_systems.h"

namespace messages {
	struct intent_message;
	struct damage_message;
	struct destroy_message;
	struct animation_message;
	struct movement_response;
	struct collision_message;
	struct create_particle_effect;
	struct gunshot_response;
	struct raw_window_input_message;
	struct unmapped_intent_message;
	struct crosshair_intent_message;
	struct trigger_hit_confirmation_message;
	struct trigger_hit_request_message;
	struct new_entity_message;
	struct camera_render_request_message;
	struct item_slot_transfer_request;
	struct gui_item_transfer_intent;
	struct rebuild_physics_message;
	struct physics_operation;
	struct melee_swing_response;
	struct health_event;
}

struct input_system;
struct steering_system;
struct movement_system;
struct animation_system;
struct crosshair_system;
struct rotation_copying_system;
struct physics_system;
struct visibility_system;
struct pathfinding_system;
struct gun_system;
struct particles_system;
struct render_system;
struct camera_system;
struct position_copying_system;
struct damage_system;
struct destroy_system;
struct behaviour_tree_system;
struct car_system;
struct driver_system;
struct trigger_detector_system;
struct item_system;
struct force_joint_system;
struct intent_contextualization_system;
struct gui_system;
struct trace_system;
struct melee_system;
struct sentience_system;

typedef storage_for_systems <
	input_system,
	steering_system,
	movement_system,
	animation_system,
	crosshair_system,
	rotation_copying_system,
	physics_system,
	visibility_system,
	pathfinding_system,
	gun_system,
	particles_system,
	render_system,
	camera_system,
	position_copying_system,
	damage_system,
	destroy_system,
	behaviour_tree_system,
	car_system,
	driver_system,
	trigger_detector_system,
	item_system,
	force_joint_system,
	intent_contextualization_system,
	gui_system,
	trace_system,
	melee_system,
	sentience_system,
> storage_for_all_systems;

typedef augs::storage_for_message_queues <
	messages::intent_message,
	messages::damage_message,
	messages::destroy_message,
	messages::animation_message,
	messages::movement_response,
	messages::collision_message,
	messages::create_particle_effect,
	messages::gunshot_response,
	messages::raw_window_input_message,
	messages::unmapped_intent_message,
	messages::crosshair_intent_message,
	messages::trigger_hit_confirmation_message,
	messages::trigger_hit_request_message,
	messages::new_entity_message,
	messages::camera_render_request_message,
	messages::item_slot_transfer_request,
	messages::gui_item_transfer_intent,
	messages::rebuild_physics_message,
	messages::physics_operation,
	messages::melee_swing_response,
	messages::health_event,
> storage_for_all_message_queues;