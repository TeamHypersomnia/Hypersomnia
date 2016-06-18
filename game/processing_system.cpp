#pragma once
#include "processing_system.h"
#include "entity.h"
#include <algorithm>

#include "graphics/renderer.h"

#include "world.h"

namespace augs {
	processing_system::processing_system(cosmos& parent_cosmos) : parent_cosmos(parent_cosmos), parent_overworld(parent_cosmos.parent_overworld) {}

	void processing_system::add(entity_id e) {
		targets.push_back(e);
	}

	void processing_system::remove(entity_id e) {
		targets.erase(std::remove(targets.begin(), targets.end(), e), targets.end());
	}

	type_hash_vector processing_system::get_needed_components() const {
		return type_hash_vector();
	}

	void processing_system::clear() {
		targets.clear();
	}

	deterministic_timestamp processing_system::get_current_timestamp() const {
		return parent_cosmos.get_current_timestamp();
	}

	double processing_system::delta_seconds() const {
		return parent_overworld.delta_seconds();
	}

	double processing_system::frame_timestamp_seconds() const {
		return parent_overworld.frame_timestamp_seconds();
	}

	int processing_system::randval(int min, int max) {
		return ::randval(min, max, parent_overworld.get_current_generator());
	}

	unsigned processing_system::randval(unsigned min, unsigned max) {
		return ::randval(min, max, parent_overworld.get_current_generator());
	}

	float processing_system::randval(float min, float max) {
		return ::randval(min, max, parent_overworld.get_current_generator());
	}

	float processing_system::randval(float minmax) {
		return randval(-minmax, minmax);
	}

	unsigned processing_system::randval(std::pair<unsigned, unsigned> p) {
		return randval(p.first, p.second);
	}

	float processing_system::randval(std::pair<float, float> p) {
		return randval(p.first, p.second);
	}

	double processing_system::delta_milliseconds() const {
		return parent_overworld.delta_milliseconds();
	}

	double processing_system::fixed_delta_milliseconds() const {
		return parent_overworld.fixed_delta_milliseconds();
	}
	
	double processing_system::view_interpolation_ratio() const {
		return parent_overworld.view_interpolation_ratio();
	}

	augs::renderer& processing_system::get_renderer() {
		return augs::renderer::get_current();
	}

	bool processing_system::passed(augs::deterministic_timeout& t) const {
		return (parent_cosmos.current_step_number - t.step_recorded) * delta_milliseconds() >= t.timeout_ms;
	}

	bool processing_system::unset_or_passed(augs::deterministic_timeout& t) const {
		return !t.was_set || passed(t);
	}

	bool processing_system::was_set_and_passed(augs::deterministic_timeout& t) const {
		return t.was_set && passed(t);
	}

	bool processing_system::check_timeout_and_reset(augs::deterministic_timeout& t) {
		if (unset_or_passed(t)) {
			reset(t);
			return true;
		}

		return false;
	}

	float processing_system::get_milliseconds_left(augs::deterministic_timeout& t) const {
		if (!t.was_set) 
			return 0.f;

		return std::max(0.0, t.timeout_ms - (parent_cosmos.current_step_number - t.step_recorded) * delta_milliseconds());
	}

	float processing_system::get_percentage_left(augs::deterministic_timeout& t) const {
		return get_milliseconds_left(t) / t.timeout_ms;
	}

	void processing_system::reset(augs::deterministic_timeout& t) {
		t.step_recorded = parent_cosmos.current_step_number;
		t.was_set = true;
	}
}