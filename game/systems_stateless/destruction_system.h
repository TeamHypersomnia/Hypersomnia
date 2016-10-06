#pragma once

class fixed_step;

class destruction_system {
public:
	void generate_damages_from_forceful_collisions(fixed_step&) const;
	void apply_damages_and_split_fixtures(fixed_step&) const;
};