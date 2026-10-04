#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "augs/math/transform.h"
#include "augs/misc/randomization_declaration.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"
#include "game/assets/ids/asset_ids.h"

struct throw_trajectory_flags;

/*
	Previews where the grenades and the melee weapons held by the viewed character would fly if thrown right now,
	in place of their laser - see throw_trajectory_flags.

	The throw is simulated in a small physics world made of copies of the colliders around the thrower:
	static ones, and dynamic obstacles like crates - where they are and as they move now, pushed by the throw like the real ones.
	Grenades fly through characters, and thrown melee weapons stop at them, as they stand now.

	A trajectory is simulated at most once per step.
	That of an explosive only when what it is launched with changes -
	the item, where it starts from, how fast and how it is damped -
	since the static colliders never move, unless there are dynamic obstacles around.

	Thrown explosives are launched just like the logic launches them - see calc_explosive_throw_launch -
	and fly until their fuse runs out. Thrown melee weapons hop off the floor just like in item_system,
	and are shown only for as long as they hurt - see melee_system::advance_thrown_melee_logic.

	The silhouettes:
	- PATH along the real trajectory,
	- BLOCKED along the straight line from where it first hits something to where it would stop if nothing was in the way,
	- END where it really stops.

	The silhouettes are a diameter plus a gap apart along the trajectory - see game_drawing_settings::throw_trajectory_gap -
	where the radius is half the longer side of the item as it lies on the ground.
*/

enum class throw_silhouette_type : uint8_t {
	PATH,
	BLOCKED,
	END
};

/*
	Interpolated between the trajectories of the previous and the current step, just like entities are -
	see interpolation_system - so that they move smoothly at any frame rate.
	Silhouettes are matched between the steps by their type and index - how many spacings along the trajectory they are -
	and a silhouette that was not there in the previous step is not interpolated.
*/

struct throw_silhouette {
	static constexpr uint32_t LAST_INDEX = static_cast<uint32_t>(-1);

	transformr previous;
	transformr current;
	throw_silhouette_type type = throw_silhouette_type::PATH;
	uint32_t index = 0;

	transformr get_interpolated(const real32 interpolation_ratio) const {
		return previous.interp(current, interpolation_ratio);
	}
};

struct throw_trajectory_input {
	entity_id item;
	transformr start;
	real32 speed = 0.f;
	real32 angular_velocity = 0.f;
	real32 angular_impulse = 0.f;
	real32 fuse_delay_ms = 0.f;
	rng_seed_type nontemporal_seed = 0;
	uint8_t num_falls = 0;
	uint8_t floor_hits_when_thrown = 0;
	real32 linear_damping = 0.f;
	real32 angular_damping = 0.f;
	uint32_t max_steps = 0;
	real32 min_speed = 0.f;
	real32 silhouette_gap = 0.f;
	bool explosive = false;
	bool faces_flight_direction = false;

	bool operator==(const throw_trajectory_input& b) const {
		return
			item == b.item
			&& start == b.start
			&& speed == b.speed
			&& angular_velocity == b.angular_velocity
			&& angular_impulse == b.angular_impulse
			&& fuse_delay_ms == b.fuse_delay_ms
			&& nontemporal_seed == b.nontemporal_seed
			&& num_falls == b.num_falls
			&& floor_hits_when_thrown == b.floor_hits_when_thrown
			&& linear_damping == b.linear_damping
			&& angular_damping == b.angular_damping
			&& max_steps == b.max_steps
			&& min_speed == b.min_speed
			&& silhouette_gap == b.silhouette_gap
			&& explosive == b.explosive
			&& faces_flight_direction == b.faces_flight_direction
		;
	}
};

/*
	What a throw ends at: where it comes to rest or explodes, or a character it hits - see throw_trajectory_system.
*/

enum class throw_end_type : uint8_t {
	REST,
	ENEMY,
	FRIEND
};

struct throw_trajectory {
	throw_trajectory_input input;
	throw_end_type end_type = throw_end_type::REST;
	bool has_dynamic_obstacles = false;

	assets::image_id image;
	vec2i size;

	std::vector<throw_silhouette> silhouettes;
};

class throw_trajectory_system {
	std::vector<throw_trajectory> trajectories;

public:
	void advance(const_entity_handle viewed_character, const throw_trajectory_flags& flags, real32 silhouette_gap);

	const auto& get_trajectories() const {
		return trajectories;
	}

	void clear() {
		trajectories.clear();
	}

	void reserve_caches_for_entities(const std::size_t) const {}
};
