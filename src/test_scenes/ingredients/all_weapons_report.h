#pragma once
#include "game/enums/gun_action_type.h"

struct json_spell_entry {
	// GEN INTROSPECTOR struct json_spell_entry
	std::string id;
	std::string display_name;
	std::string pic_filename;
	std::string notes;
	std::string incantation;
	rgba associated_color = white;

	float cooldown = 0;
	float mana_required = 0;
	float price = 0;
	float kill_award = 0;
	// END GEN INTROSPECTOR
};

struct json_explosive_entry {
	// GEN INTROSPECTOR struct json_explosive_entry
	std::string id;
	std::string display_name;
	std::string pic_filename;
	std::string notes;

	float fuse_delay = 0;
	float base_damage = 0;

	float price = 0;
	float kill_award = 0;
	float weight = 0;
	// END GEN INTROSPECTOR
};

struct json_melee_entry {
	// GEN INTROSPECTOR struct json_melee_entry
	std::string id;
	std::string display_name;
	std::string pic_filename;

	float fast_swings_per_second = 0;
	float strong_swings_per_second = 0;

	float fast_damage = 0;
	float strong_damage = 0;

	float fast_hs_damage = 0;
	float strong_hs_damage = 0;

	float fast_stamina_required = 0;
	float strong_stamina_required = 0;

	float throw_damage = 0;
	float throw_hs_damage = 0;

	float price = 0;
	float kill_award = 0;
	float weight = 0;
	// END GEN INTROSPECTOR
};

struct json_firearm_entry {
	// GEN INTROSPECTOR struct json_firearm_entry
	std::string id;
	std::string display_name;
	std::string pic_filename;

	int pellets_per_shot = 1;
	float damage = 0;
	float headshot_damage = 0;
	float shots_per_second = 0;
	float min_bullet_speed = 0;
	float max_bullet_speed = 0;
	float price = 0;
	float kill_award = 0;
	float weight = 0;

	float reload_time = 0;
	float slow_reload_time = 0;
	float chambering_time = 0;

	gun_action_type action_type = gun_action_type::INVALID;

	bool has_magazine = false;

	bool draw_magazine_under = true;
	std::string magazine_id;
	float magazine_x = 0;
	float magazine_y = 0;
	float magazine_rotation = 0;
	// END GEN INTROSPECTOR
};
