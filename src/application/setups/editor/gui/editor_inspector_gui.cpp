#include "augs/templates/enum_introspect.h"
#include "augs/templates/history.hpp"
#include "application/setups/editor/editor_setup.hpp"
#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_enum_combo.h"
#include "augs/string/string_templates.h"

#include "application/setups/editor/resources/editor_sound_resource.h"
#include "application/setups/editor/resources/editor_sprite_resource.h"
#include "application/setups/editor/resources/editor_light_resource.h"
#include "application/setups/editor/resources/editor_prefab_resource.h"

#include "application/setups/editor/nodes/editor_sound_node.h"
#include "application/setups/editor/nodes/editor_sprite_node.h"
#include "application/setups/editor/nodes/editor_light_node.h"
#include "application/setups/editor/nodes/editor_prefab_node.h"

#include "application/setups/editor/gui/editor_inspector_gui.h"
#include "application/setups/editor/detail/simple_two_tabs.h"

#include "application/setups/editor/detail/maybe_different_colors.h"
#include "application/setups/editor/resources/resource_traits.h"
#include "application/setups/editor/detail/make_command_from_selections.h"
#include "test_scenes/test_scene_flavours.h"
#include "application/setups/debugger/property_debugger/widgets/non_standard_shape_widget.h"
#include "augs/misc/from_concave_polygon.h"

#include "application/setups/editor/defaults/editor_game_mode_defaults.h"
#include "application/setups/editor/defaults/editor_resource_defaults.h"
#include "application/setups/editor/defaults/editor_node_defaults.h"
#include "application/setups/editor/defaults/editor_project_defaults.h"
#include "application/setups/editor/gui/id_widget_handler.h"

struct default_widget_handler {
	template <class... Args>
	default_widget_handler(Args&&...) {}

	template <class T>
	static constexpr bool handles = always_false_v<T>;

	template <class T>
	bool handle(std::string&, const std::string&, const T&) {
		static_assert(handles<T>);
		return false;
	}
};

#define EDIT_ATTENUATIONS 1

#define PROPERTY(label, MEMBER) \
edit_property(result, label, special_handler, insp.MEMBER);

#define MULTIPROPERTY(label, MEMBER) \
{\
	bool values_different = false;\
\
	for (auto& ee : es) {\
		if (!(ee.after.MEMBER == insp.MEMBER)) {\
			values_different = true;\
		}\
	}\
\
	auto cols = maybe_different_value_cols({}, values_different);\
\
	last_result = edit_property(result, label, special_handler, insp.MEMBER);\
\
	if (last_result) {\
		for (auto& ee : es) {\
			ee.after.MEMBER = insp.MEMBER;\
		}\
	}\
}

#define EDIT_FUNCTION template <class T> std::string perform_editable_gui
#define SINGLE_EDIT_FUNCTION std::string perform_editable_gui_single

bool force_show_extra_ammo = false;

#define EQUIPMENT_PROPERTIES(field) \
	MULTIPROPERTY("Firearm", field.firearm);\
	MULTIPROPERTY("Melee", field.melee);\
	MULTIPROPERTY("Explosive", field.explosive);\
	if (insp.field.explosive.is_set()) {\
		auto ind = scoped_indent();\
		MULTIPROPERTY("Explosives count", field.num_explosives)\
	}\
\
	MULTIPROPERTY("Backpack", field.backpack);\
	MULTIPROPERTY("Electric armor", field.electric_armor);\
	MULTIPROPERTY("All spells", field.all_spells);\
	if (force_show_extra_ammo || insp.field.firearm.is_set()) {\
	MULTIPROPERTY("Extra ammo pieces", field.extra_ammo_pieces);\
	}\
	if (auto scope = augs::imgui::scoped_tree_node_ex("Advanced")) {\
		MULTIPROPERTY("Haste (secs)", field.haste_time);\
	}

#define EQUIPMENT_PROPERTY(label, field) \
if (auto scope = augs::imgui::scoped_tree_node_ex(label)) {\
	EQUIPMENT_PROPERTIES(field);\
}

#define FACTION_EQUIPMENT_PROPERTY(label, field) \
if (auto scope = augs::imgui::scoped_tree_node_ex(label)) {\
	ImGui::Separator();\
	text_color("Metropolis", yellow);\
	{ auto scope = scoped_id("##Metropolis"); EQUIPMENT_PROPERTIES(field.metropolis); }\
	text_color("Resistance", yellow);\
	{ auto scope = scoped_id("##Resistance"); EQUIPMENT_PROPERTIES(field.resistance); }\
	ImGui::Separator();\
	/*\
		We'll only show the two relevant factions always, unless we have a three-way-conflict\
		text_color("Atlantis", yellow);\
		EQUIPMENT_PROPERTIES(field.atlantis);\
	*/\
}

#define THEME_PROPERTY(label, field) \
	PROPERTY(label, field.id);\
	/* TODO: warmup_theme needs to be a sound_effect input for these to have any effect at all! */\
	if (false && insp.field.id.is_set()) {\
		auto here_id = scoped_id(label);\
		auto indent = scoped_indent();\
		auto indent2 = scoped_indent();\
\
		PROPERTY("Gain", field.gain);\
		PROPERTY("Pitch", field.pitch);\
	}\

#define SOUND_EFFECT_LEAN_MULTIPROPERTY(label, field) \
	MULTIPROPERTY(label, field.id);\
	if (insp.field.id.is_set()) {\
		auto here_id = scoped_id(label);\
		auto indent = scoped_indent();\
		auto indent2 = scoped_indent();\
\
		MULTIPROPERTY("Gain", field.gain);\
		MULTIPROPERTY("Pitch", field.pitch);\
	}

#define PARTICLE_EFFECT_MODIFIER_MULTIPROPERTY(label, field) \
	MULTIPROPERTY("Color", field.color);\
	MULTIPROPERTY("Scale amounts", field.scale_amounts);\
	MULTIPROPERTY("Scale lifetimes", field.scale_lifetimes);

#define PARTICLE_EFFECT_MULTIPROPERTY(label, field) \
	MULTIPROPERTY(label, field.id);\
	if (insp.field.id.is_set()) {\
		auto here_id = scoped_id(label);\
		auto indent = scoped_indent();\
		auto indent2 = scoped_indent();\
		PARTICLE_EFFECT_MODIFIER_MULTIPROPERTY(label, field);\
	}

#define PARTICLE_EFFECT_MODIFIER_MULTIPROPERTY_NOCOLOR(label, field) \
	MULTIPROPERTY("Scale amounts", field.scale_amounts);\
	MULTIPROPERTY("Scale lifetimes", field.scale_lifetimes);

#define PARTICLE_EFFECT_MULTIPROPERTY_NOCOLOR(label, field) \
	MULTIPROPERTY(label, field.id);\
	if (insp.field.id.is_set()) {\
		auto here_id = scoped_id(label);\
		auto indent = scoped_indent();\
		auto indent2 = scoped_indent();\
		PARTICLE_EFFECT_MODIFIER_MULTIPROPERTY_NOCOLOR(label, field);\
	}

#define MULTIPROPERTY_POSITION(field) \
	MULTIPROPERTY("Position X", pos.x); \
	MULTIPROPERTY("Position Y", pos.y);

void editor_tweaked_widget_tracker::reset() {
	last_tweaked.reset();
}

bool editor_tweaked_widget_tracker::changed(const std::size_t current_command_index) const {
	const auto current_session = tweak_session { ImGui::GetCurrentContext()->LastActiveId, current_command_index };
	return last_tweaked != current_session;
}

void editor_tweaked_widget_tracker::update(const std::size_t current_command_index) {
	const auto current_session = tweak_session { ImGui::GetCurrentContext()->LastActiveId, current_command_index };
	last_tweaked = current_session;
}

std::string get_hex_representation(const unsigned char*, size_t length);

std::string as_hex(const rgba& col) {
	return to_uppercase(get_hex_representation(std::addressof(col.r), 4));
}

template <class T, class H>
bool edit_property(
	std::string& result,
	const std::string& label,
	H special_handler,
	T& property
) {
	using namespace augs::imgui;

	if constexpr(H::template handles<T>) {
		if (special_handler.handle(result, label, property)) {
			return true;
		}
	}
	else if constexpr(std::is_same_v<T, rgba>) {
		if (label.size() > 0 && label[0] == '#') {
			if (color_picker(label, property)) {
				result = typesafe_sprintf("Set Color to %x in %x", as_hex(property));
				return true;
			}
		}
		else {
			if (color_edit(label, property)) {
				result = typesafe_sprintf("Set %x to %x in %x", label, as_hex(property));
				return true;
			}
		}
	}
	else if constexpr(std::is_same_v<T, bool>) {
		if (checkbox(label, property)) {
			if (property) {
				result = typesafe_sprintf("Enabled %x in %x", label);
				return true;
			}
			else {
				result = typesafe_sprintf("Disabled %x in %x", label);
				return true;
			}
		}
	}
	else if constexpr(std::is_same_v<rgba_channel, T>) {
		if (slider(label, property, rgba_channel(0), rgba_channel(255))) { 
			result = typesafe_sprintf("Set %x to %x in %x", label, property);
			return true;
		}
	}
	else if constexpr(std::is_arithmetic_v<T>) {
		if constexpr(std::is_same_v<float, T>) {
			if (label == "Opacity" || label == "Constant" || label == "Linear" || label == "Quadratic") {
				if (slider(label, property, 0.0f, 1.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Radius") {
				if (slider(label, property, 0.0f, 2000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Haste (secs)") {
				if (slider(label, property, 0.0f, 600.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Animation speed factor" || label == "Positional vibration" || label == "Intensity vibration" || label == "Light size mult") {
				if (slider(label, property, 0.0f, 5.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Walking speed") {
				if (slider(label, property, 0.0f, 2.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Effect speed" || label == "Character airborne increase speed") {
				if (slider(label, property, 0.0f, 10.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Highlight size mult") {
				if (slider(label, property, 0.0f, 3.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}


			if (label == "Gain" || label == "Pitch" || label == "Min pitch" || label == "Max pitch") {
				if (slider(label, property, 0.0f, 10.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Collision sound sensitivity") {
				if (slider(label, property, 0.0f, 3.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Penetrability") {
				if (slider(label, property, 0.0f, 4.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("0 - impenetrable.\n1 - default value.\nThe higher the penetrability,\nthe easier it is to wallbang.\nA value of 2 means that the bullets\nwill travel twice as far while penetrating.");

				return false;
			}

			if (label == "Max ricochet angle") {
				if (slider(label, property, 0.0f, 180.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Scale lifetimes") {
				if (slider(label, property, 0.0f, 100.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("How long until particles disappear in this effect.");

				return false;
			}

			if (label == "Scale amounts") {
				if (slider(label, property, 0.0f, 100.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("Scales the amount of particles in this effect.");

				return false;
			}

			if (label == "Minimum interval (ms)") {
				if (slider(label, property, 0.0f, 2000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("Minimum amount of time that must elapse\nbetween playing two consecutive collision sounds.");

				return false;
			}

			if (label == "Unmute after (ms)") {
				if (slider(label, property, 0.0f, 2000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("The amount of time surface must not be touched\nin order for collision sounds to be played again,\nafter having been played 'Mute after playing times' quickly.");

				return false;
			}

			if (begins_with(label, "Linear impulse") || label == "Character force" || label == "Object force") {
				if (slider(label, property, 0.0f, 20000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("How strongly to push.");

				return false;
			}

			if (ends_with(label, "shake strength")) {
				if (slider(label, property, 0.0f, 3.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (ends_with(label, "shake duration (ms)")) {
				if (slider(label, property, 0.0f, 3000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (ends_with(label, "highlight time (ms)")) {
				if (slider(label, property, 0.0f, 3000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (begins_with(label, "Angular impulse") || label == "Object torque") {
				if (slider(label, property, 0.0f, 2000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("How strongly to rotate.");

				return false;
			}

			if (label == "Enter time (ms)" || label == "Travel time (ms)") {
				if (slider(label, property, 0.0f, 5000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("How much time you have to stay inside the portal\nin order to appear on the other side.");

				return false;
			}

			if (label == "Exit airborne duration (ms)" || label == "Character target airborne (ms)") {
				if (slider(label, property, 0.0f, 3000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("How long the character is \"airborne\" after exiting.\nWhen \"airborne\", the character moves very fast, the price being\nyou have less control over the movement direction.");

				return false;
			}

			if (label == "Exit cooldown (ms)") {
				if (slider(label, property, 0.0f, 2000.0f)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}
		}
		else if constexpr(std::is_same_v<uint32_t, T>) {
			if (label == "Respawn after (ms)" || label == "Spawn protection (ms)") {
				if (slider(label, property, 0u, 10000u)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label.find("Fish") != std::string::npos || label.find("austics") != std::string::npos) {
				if (label.find("count") != std::string::npos) {
					if (slider(label, property, 0u, 50u)) { 
						result = typesafe_sprintf("Set %x to %x in %x", label, property);
						return true;
					}

					return false;
				}
			}

			if (label.find("Extra ammo pieces") != std::string::npos || label == "Explosives count") {
				if (slider(label, property, 0u, 10u)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}

			if (label == "Mute after playing times") {
				if (slider(label, property, 1u, 20u)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				tooltip_on_hover("Will be played at most this many times in a quick succession.\nThen the sound will stop playing altogether until surfaces stops colliding.\n\nExplanation:\nNote that when the player starts hugging a wall,\nthe collision sound will normally be played once or twice and then go silent.\nThis is to prevent spamming the sounds constantly\nwhen you want to walk along walls while touching them.\n\nPicking a value:\nA short wooden 'knock' sound is fine to be repeated even 3-4 times\nas the player grazes the wooden surface with a rifle.\n\nHowever, a very iconic and lengthy knife clanging sound,\nor a sturdy bag collapsing sound\nshouldn't be played too often even if happens twice in a split second.\nSetting the parameter to 1 is appropriate for such sounds.");

				return false;
			}
		}
		else if constexpr(std::is_same_v<uint16_t, T>) {
			if (label.find("Num particles") != std::string::npos) {
				if (slider(label, property, uint16_t(0u), MAX_WANDERING_PIXELS)) { 
					result = typesafe_sprintf("Set %x to %x in %x", label, property);
					return true;
				}

				return false;
			}
		}

		if (drag(label, property)) { 
			result = typesafe_sprintf("Set %x to %x in %x", label, property);
			return true;
		}
	}
	else if constexpr(std::is_enum_v<T>) {
		if constexpr(std::is_same_v<T, faction_type>) {
			if (enum_combo_constrained<
				faction_type, 
				faction_type::METROPOLIS,
				faction_type::RESISTANCE
			>(label, property)) { 
				result = typesafe_sprintf("Switched %x to %x in %x", label, property);
				return true;
			}
		}
		else if constexpr(std::is_same_v<T, impulse_type>) {
			if (enum_combo_constrained<
				impulse_type, 
				impulse_type::IMPULSE,
				impulse_type::ADD_VELOCITY,
				impulse_type::SET_VELOCITY
			>(label, property)) { 
				result = typesafe_sprintf("Switched %x to %x in %x", label, property);
				return true;
			}

			tooltip_on_hover("Impulse: the lighter the object, the further it will fly.\n\nAdd velocity: all objects will be sped up by exactly the same amount\nregardless of their mass.\n\nSet velocity: the same as Add velocity, \nbut the object's velocity will be replaced rather than added to.\nThis means that the velocity with which the object entered does not matter.");
		}
		else {
			if (enum_combo(label, property)) { 
				result = typesafe_sprintf("Switched %x to %x in %x", label, property);
				return true;
			}
		}
	}
	else if constexpr(is_one_of_v<T, vec2, vec2i, vec2u>) {
		if (drag_vec2(label, property)) { 
			result = typesafe_sprintf("Set %x to %x in %x", label, property);
			return true;
		}
	}
	else if constexpr(std::is_same_v<T, std::string>) {
		if (input_multiline_text(label, property, 3)) {
			result = "Edited %x";
			return true;
		}
	}
	else {
		static_assert(always_false_v<T>, "Non-exhaustive if constexpr");
	}

	return false;
}

static bool reset_button(
	const bool already_default,
	const bool multiple
) {
	using namespace augs::imgui;

	bool request_defaults = false;

	const auto reset_bgs = std::array<rgba, 3> {
		rgba(0, 0, 0, 0),
		rgba(80, 20, 20, 255),
		rgba(150, 40, 40, 255)
	};

	{
		auto cols = scoped_button_colors(reset_bgs);
		auto disabled = maybe_disabled_cols(already_default);

		if (ImGui::Button("Reset") && !already_default) {
			request_defaults = true;
		}
	}

	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
		if (already_default) {
			auto scope = scoped_tooltip();
			text_color("There's nothing to reset now.", green); 

			if (multiple) {
				text("All selected resources have default properties.");
			}
			else {
				text("All properties below are default.");
			}
		}
		else {
			auto scope = scoped_tooltip();

			if (multiple) {
				text_color("Some of the selected resources can be reset to default.\nClick to restore defaults to them all.", orange); text("Can safely be undone.");
			}
			else {
				text_color("Restore defaults to all properties below.", orange); text("Can safely be undone.");
			}
		}
	}

	return request_defaults;
}

EDIT_FUNCTION(editor_sprite_node_editable& insp, T& es, editor_sprite_resource& resource) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;

	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);

	MULTIPROPERTY("Flip horizontally", flip_horizontally);
	ImGui::SameLine();
	MULTIPROPERTY("Flip vertically", flip_vertically);

	MULTIPROPERTY("Color", color);
	MULTIPROPERTY("Neon color", neon_color);

	MULTIPROPERTY("Resize", size.is_enabled);

	if (insp.size.is_enabled) {
		ImGui::SameLine();
		MULTIPROPERTY("##Overridden size", size.value);

		if (last_result) {
			result = "Resized %x";
		}
	}

	if (resource.editable.color_wave_speed.is_enabled) {
		MULTIPROPERTY("Randomize color wave offset", randomize_color_wave_offset);
	}

	const bool is_animation = !resource.animation_frames.empty() || (resource.official_tag.has_value() && std::holds_alternative<test_dynamic_decorations>(resource.official_tag.value()));

	if (is_animation) {
		ImGui::Separator();

		text_color("Animation", yellow);

		MULTIPROPERTY("Animation speed factor", animation_speed_factor);
		MULTIPROPERTY("Override starting animation frame", starting_animation_frame.is_enabled);

		if (insp.starting_animation_frame.is_enabled) {
			auto indent = scoped_indent();

			MULTIPROPERTY("##Overridden frame", starting_animation_frame.value);
		}

		MULTIPROPERTY("Randomize starting animation frame", randomize_starting_animation_frame);

	}

	if (resource.editable.domain == editor_sprite_domain::PHYSICAL) {
		text_disabled("Tip: you can also set penetrability\nper-resource or even per-material.\nAll three will combine.\nModify it per-node as a last resort.");
		MULTIPROPERTY("Penetrability", penetrability);
	}

	return result;
}

EDIT_FUNCTION(editor_sound_node_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);

	/* 
		TODO: Sounds should have per-entity sound effect modifiers.
		Once it's done, edit gain and pitch here.
	*/

	return result;
}

EDIT_FUNCTION(editor_firearm_node_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);

	return result;
}

EDIT_FUNCTION(editor_ammunition_node_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);

	return result;
}

EDIT_FUNCTION(editor_tool_node_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);

	return result;
}


EDIT_FUNCTION(editor_melee_node_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);

	return result;
}

EDIT_FUNCTION(editor_wandering_pixels_node_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);
	MULTIPROPERTY("Size", size);

	ImGui::Separator();

	MULTIPROPERTY("Color", color);
	MULTIPROPERTY("Num particles", num_particles);
	MULTIPROPERTY("Force particles within bounds", force_particles_within_bounds);
	MULTIPROPERTY("Illuminate", illuminate);

	return result;
}

static bool has_letter(const point_marker_type type) {
	switch (type) {
		default:
			return false;
	}
}

static bool has_team(const point_marker_type type) {
	switch (type) {
		case point_marker_type::TEAM_SPAWN:
			return true;
		default:
			return false;
	}
}

static bool has_letter(const area_marker_type type) {
	switch (type) {
		case area_marker_type::BOMBSITE:
			return true;
		default:
			return false;
	}
}

static bool has_team(const area_marker_type type) {
	switch (type) {
		case area_marker_type::BUY_ZONE:
			return true;
		default:
			return false;
	}
}

EDIT_FUNCTION(editor_point_marker_node_editable& insp, T& es, const editor_point_marker_resource& resource) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);

	const auto type = resource.editable.type;

	if (has_team(type)) {
		MULTIPROPERTY("Faction", faction);
	}

	if (has_letter(type)) {
		MULTIPROPERTY("Letter", letter);
	}

	return result;
}

rgba editor_portal_info::get_icon_color() const {
	if (color_preset == editor_color_preset::CUSTOM) {
		return rings_effect.value.inner_color;
	}

	return get_editor_color_preset(color_preset).portal.inner_ring;
}

void editor_portal_info::apply(const editor_color_preset preset) {
	auto cols = get_editor_color_preset(preset).portal;

	rings_effect.value.inner_color = cols.inner_ring;
	rings_effect.value.outer_color = cols.outer_ring;
	light_color = cols.light;
	ambience_particles.color = cols.ambience_particles;
	begin_entering_particles.color = cols.begin_entering_particles;
	enter_particles.color = cols.enter_particles;
	exit_particles.color = cols.exit_particles;
}

EDIT_FUNCTION(editor_area_marker_node_editable& insp, T& es, const editor_area_marker_resource& resource, const id_widget_handler& special_handler) {
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);
	MULTIPROPERTY("Size", size);
	 
	const auto type = resource.editable.type;

	if (has_team(type)) {
		MULTIPROPERTY("Faction", faction);
	}

	if (has_letter(type)) {
		MULTIPROPERTY("Letter", letter);
	}

	const bool has_shape = ::is_portal_based(type);

	if (has_shape) {
		MULTIPROPERTY("Shape", shape);
		tooltip_on_hover("Note that rectangular effects for portals are not supported yet,\nbut the physical sensor itself will work just fine as a box.");
	}

	const bool is_hurt = type == area_marker_type::HAZARD;
	(void)is_hurt;

	if (::is_portal_based(type)) {
		ImGui::ColorButton("##Cbutton", insp.as_portal.get_icon_color().operator ImVec4());

		ImGui::SameLine();
		MULTIPROPERTY("Color preset", as_portal.color_preset);

		if (insp.as_portal.color_preset == editor_color_preset::CUSTOM) {
			MULTIPROPERTY("Inner ring color", as_portal.rings_effect.value.inner_color);
			MULTIPROPERTY("Outer ring color", as_portal.rings_effect.value.outer_color);

			MULTIPROPERTY("Light color", as_portal.light_color);

			if (insp.as_portal.ambience_particles.id.is_set()) {
				MULTIPROPERTY("Ambience (particles)", as_portal.ambience_particles.color);
			}

			if (insp.as_portal.begin_entering_particles.id.is_set()) {
				MULTIPROPERTY("Begin entering particles", as_portal.begin_entering_particles.color);
			}

			if (insp.as_portal.enter_particles.id.is_set()) {
				MULTIPROPERTY("Enter particles", as_portal.enter_particles.color);
			}

			if (insp.as_portal.exit_particles.id.is_set()) {
				MULTIPROPERTY("Exit particles", as_portal.exit_particles.color);
			}
		}

		ImGui::Separator();
		text_color("AS ENTRY", cyan);
		ImGui::Separator();

		MULTIPROPERTY("Trampoline-like", as_portal.trampoline_like);
		tooltip_on_hover("Convenience option to disable enter/travel delay and set target to itself.\nThis will make the portal work like a trampoline -\nObjects will not be teleported, they will simply be pushed with a force,\nin accordance with AS EXIT parameters.");

		const bool trampoline = insp.as_portal.trampoline_like;

		if (!trampoline) {
			MULTIPROPERTY("Portal exit", as_portal.portal_exit);
			tooltip_on_hover("Where to teleport entering objects.\nCan only be another portal.\nIf you leave it empty, the character will just stay invisible and won't teleport anywhere.\nIf you set it to itself, the portal will behave like a trampoline.");
		}

		MULTIPROPERTY("Ignore airborne characters", as_portal.ignore_airborne_characters);
		tooltip_on_hover("'Airborne' characters are ones that only just exited a portal or e.g. during dash.\nThis is useful for making portals that only react when you 'land'.\nA perfect usecase is the background portal on 'surf' maps\nthat teleports you back to the beginning if you happen to miss a trampoline.");


		MULTIPROPERTY("Ignore walking characters", as_portal.ignore_walking_characters);
		tooltip_on_hover("Characters walking silently will be unaffected by this portal.");

		if (!trampoline) {
			MULTIPROPERTY("Enter time (ms)", as_portal.enter_time_ms);
			tooltip_on_hover("The time it takes to enter the portal and disappear.");
			MULTIPROPERTY("Travel time (ms)", as_portal.travel_time_ms);
			tooltip_on_hover("The time it takes exit the portal after disappearing.");
		}

		MULTIPROPERTY("Disable all entry effects", as_portal.disable_all_entry_effects);

		tooltip_on_hover("Convenience option to disable all visuals, sounds and particles on entry.\nThis will make the portal 'seamless' - the subjects will instantly change positions.\n\nNecessary for 'undetectable' portals,\nwhen you want the player to not even notice that they just warped somewhere.\n This could even let you create some mind-bending creepy labrinyths.");

		if (!insp.as_portal.disable_all_entry_effects) {
			MULTIPROPERTY("Auto scale pitches", as_portal.auto_scale_pitches);

			tooltip_on_hover("When altering enter/travel time,\nautomatically scale pitch of entering/travelling sounds.\n\nE.g. if you set Enter time to 500ms,\nthe Begin entering sound will automatically have 2x the normal pitch.\nAnalogously with Travel time and Enter sound.");

			if (auto scope = augs::imgui::scoped_tree_node_ex("Visual effects")) {

				MULTIPROPERTY("Begin entering highlight time (ms)", as_portal.begin_entering_highlight_ms);
				tooltip_on_hover("Highlights any entered object in pure color.\nThe color is automatically set to Rings effect -> Inner color.");
				MULTIPROPERTY("Decrease opacity to", as_portal.decrease_opacity_to);

				tooltip_on_hover("As objects enter the portal, they will become \nless and less visible until they finally disappear.\n\nYou can control the target to which opacity is decreased.\nIf you want objects to become fully invisible before teleporting, set to 0.\nIf you want to disable this effect (to make the portal undetectible), set it to maximum: 255.");

				//MULTIPROPERTY("Light color", as_portal.light_color);
				MULTIPROPERTY("Light size mult", as_portal.light_size_mult);

				MULTIPROPERTY("Rings effect", as_portal.rings_effect.is_enabled);

				if (last_result) {
					if (insp.as_portal.rings_effect.is_enabled) {
						result = "Enabled Rings effect in %x";
					}
					else {
						result = "Disabled Rings effect in %x";
					}
				}

				if (insp.as_portal.rings_effect.is_enabled) {
					auto scope = scoped_indent();

					MULTIPROPERTY("Effect speed", as_portal.rings_effect.value.effect_speed);

					//MULTIPROPERTY("Inner color", as_portal.rings_effect.value.inner_color);
					//MULTIPROPERTY("Outer color", as_portal.rings_effect.value.outer_color);
				}

				PARTICLE_EFFECT_MULTIPROPERTY_NOCOLOR("Ambience (particles)", as_portal.ambience_particles);
				PARTICLE_EFFECT_MULTIPROPERTY_NOCOLOR("Begin entering particles", as_portal.begin_entering_particles);

				if (!trampoline) {
					PARTICLE_EFFECT_MULTIPROPERTY_NOCOLOR("Enter particles", as_portal.enter_particles);
					//MULTIPROPERTY("Enter shake strength", as_portal.enter_shake.strength);
					//tooltip_on_hover("Applies only to characters.\nHow strong to shake the character\nwhen they successfully enter the portal.");
					MULTIPROPERTY("Enter shake duration (ms)", as_portal.enter_shake.duration_ms);
					tooltip_on_hover("Applies only to characters.\nHow long to shake the character for\nwhen they successfully enter the portal.");
				}
			}

			if (auto scope = augs::imgui::scoped_tree_node_ex("Sound effects")) {
				SOUND_EFFECT_LEAN_MULTIPROPERTY("Ambience (sound)", as_portal.ambience_sound);

				if (insp.as_portal.ambience_sound.id.is_set()) {
					auto scope = scoped_indent();
					auto scope2 = scoped_indent();

					MULTIPROPERTY("Ambience sound distance mult", as_portal.ambience_sound_distance_mult);

					tooltip_on_hover("How far away the portal ambience will be heard.\n1.2 means it will only be heard within the 120%% of the portal radius\n(so basically only very close).");
				}

				SOUND_EFFECT_LEAN_MULTIPROPERTY("Begin entering sound", as_portal.begin_entering_sound);

				if (!trampoline) {
					SOUND_EFFECT_LEAN_MULTIPROPERTY("Enter sound", as_portal.enter_sound);
				}
			}
		}

		{
			MULTIPROPERTY("##hazard", as_portal.hazard.is_enabled);

			if (last_result) {
				if (insp.as_portal.hazard.is_enabled) {
					result = "Enabled hazard in %x";
				}
				else {
					result = "Disabled hazard in %x";
				}
			}

			ImGui::SameLine();

			auto disabled = maybe_disabled_only_cols(!insp.as_portal.hazard.is_enabled);

			auto scope = augs::imgui::scoped_tree_node_ex("Hazard");

			if (scope) {
				auto actually_disabled = maybe_disabled_cols(!insp.as_portal.hazard.is_enabled);

				MULTIPROPERTY("Damage", as_portal.hazard.value.damage);
			}
		}

		{
			MULTIPROPERTY("##ForceField", as_portal.force_field.is_enabled);

			if (last_result) {
				if (insp.as_portal.force_field.is_enabled) {
					result = "Enabled force field in %x";
				}
				else {
					result = "Disabled force field in %x";
				}
			}

			ImGui::SameLine();

			auto disabled = maybe_disabled_only_cols(!insp.as_portal.force_field.is_enabled);

			auto scope = augs::imgui::scoped_tree_node_ex("Force field");

			if (scope) {
				auto actually_disabled = maybe_disabled_cols(!insp.as_portal.force_field.is_enabled);

				MULTIPROPERTY("Direction", as_portal.force_field.value.direction);

				MULTIPROPERTY("Falloff", as_portal.force_field.value.falloff);
				tooltip_on_hover("None:\nthe force will have the same strength everywhere.\n\nLinear:\nthe force will be the strongest near the center,\nweakest on boundaries (or vice versa).\n\nQuadratic:\nsame as linear, but will drop/increase very quickly with distance.\n\nSqrt: \nSame as linear, but will drop/increase slowly with distance.");

				if (insp.as_portal.force_field.value.falloff != force_field_falloff::NONE) {
					auto ind = scoped_indent();
					MULTIPROPERTY("Stronger towards center", as_portal.force_field.value.stronger_towards_center);
					tooltip_on_hover("If ticked, forces will be stronger\nnear the the center instead of boundaries.\nUntick for the reverse.");

					MULTIPROPERTY("Falloff decreases airborne acceleration", as_portal.force_field.value.falloff_decreases_airborne_acceleration);
					tooltip_on_hover("Normally, falloff only influences the applied force.\nHowever, this option also lets you scale the speed\nwith which the character becomes 'airborne'.");
				}


				MULTIPROPERTY("Character force", as_portal.force_field.value.character_force);
				MULTIPROPERTY("Character target airborne (ms)", as_portal.force_field.value.character_target_airborne_ms);
				MULTIPROPERTY("Character airborne increase speed", as_portal.force_field.value.character_airborne_acceleration);
				MULTIPROPERTY("Fly even without sprint", as_portal.force_field.value.fly_even_without_sprint);
				tooltip_on_hover("Normally, you have to sprint to fully take advantage\nof the force field's physics. This spends stamina, although only 50%\nof the amount when sprinting on land.\nTick this to create a force field that doesn't require sprint.");

				MULTIPROPERTY("Object force", as_portal.force_field.value.object_force);
				MULTIPROPERTY("Object torque", as_portal.force_field.value.object_torque);
				MULTIPROPERTY("Mass invariant", as_portal.force_field.value.mass_invariant);
			}
		}

		ImGui::Separator();
		text_color("Reacts to", yellow);
		ImGui::Separator();

		MULTIPROPERTY("Characters", as_portal.reacts_to.characters);
		MULTIPROPERTY("Character weapons", as_portal.reacts_to.character_weapons);
		MULTIPROPERTY("Bullets", as_portal.reacts_to.bullets);
		MULTIPROPERTY("Flying explosives", as_portal.reacts_to.flying_explosives);
		MULTIPROPERTY("Flying melees", as_portal.reacts_to.flying_melees);
		MULTIPROPERTY("Lying items", as_portal.reacts_to.lying_items);
		MULTIPROPERTY("Shells", as_portal.reacts_to.shells);
		MULTIPROPERTY("Obstacles", as_portal.reacts_to.obstacles);

		ImGui::Separator();
		text_color("Reacts to factions", yellow);
		ImGui::Separator();
		
		MULTIPROPERTY("Resistance", as_portal.reacts_to_factions.resistance);
		MULTIPROPERTY("Metropolis", as_portal.reacts_to_factions.metropolis);
		MULTIPROPERTY("Atlantis", as_portal.reacts_to_factions.atlantis);

		ImGui::Separator();
		text_color("AS EXIT", orange);
		ImGui::Separator();

		MULTIPROPERTY("Exit position", as_portal.exit_position);
		tooltip_on_hover("Portal center:\nalways spawn objects exactly at this portal center.\n\nPortal center plus entering offset:\nexit position will be slightly offset to match\nexactly how far from the center did object enter the portal.\nNecessary to make the portals fully undetectable,\nas normally the player will notice their character has been shifted\nto match the portal exit position.\nPreferred setting for trampoline portals.\n\nPortal boundary:\nwill spawn objects on the edge of this portal\naccording to Exit direction.");

		MULTIPROPERTY("Exit direction", as_portal.exit_direction);

		MULTIPROPERTY("Disable all exit effects", as_portal.disable_all_exit_effects);
		tooltip_on_hover("Convenience option to disable all impulses, sounds and particles on exit.\n\nNecessary for 'undetectable' portals,\nwhen you want the player to not even notice that they just warped somewhere.\n This could even let you create some mind-bending creepy labrinyths.");

		if (!insp.as_portal.disable_all_exit_effects) {
			if (auto scope = augs::imgui::scoped_tree_node_ex("Exit effects")) {
				MULTIPROPERTY("Exit highlight time (ms)", as_portal.exit_highlight_ms);
				tooltip_on_hover("Highlights any exiting object in pure color.\nThe color is automatically set to Rings effect -> Inner color.");

				SOUND_EFFECT_LEAN_MULTIPROPERTY("Exit sound", as_portal.exit_sound);
				PARTICLE_EFFECT_MULTIPROPERTY_NOCOLOR("Exit particles", as_portal.exit_particles);
			}

			if (auto scope = augs::imgui::scoped_tree_node_ex("Character exit forces")) {
				text_disabled("Forces to apply when a character appears\nthrough this portal exit.");

				//MULTIPROPERTY("Exit shake strength", as_portal.exit_shake.strength);
				//tooltip_on_hover("Applies only to characters.\nHow strong to shake the character\nwhen they successfully exit the portal.");
				MULTIPROPERTY("Exit shake duration (ms)", as_portal.exit_shake.duration_ms);
				tooltip_on_hover("Applies only to characters.\nHow long to shake the character for\nwhen they successfully exit the portal.");

				MULTIPROPERTY("Exit airborne duration (ms)", as_portal.exit_impulses.character_exit_airborne_ms);

				MULTIPROPERTY("Linear impulse##CharLinear", as_portal.exit_impulses.character_exit_impulse.amount);

				{
					auto scope = scoped_indent();
					MULTIPROPERTY("Mode##CharLinear", as_portal.exit_impulses.character_exit_impulse.mode);
				}
			}

			if (auto scope = augs::imgui::scoped_tree_node_ex("Object exit forces")) {
				text_disabled("Forces to apply when any object appears\nthrough this portal exit.");

				MULTIPROPERTY("Linear impulse##ObjLinear", as_portal.exit_impulses.object_exit_impulse.amount);

				{
					auto scope = scoped_indent();
					MULTIPROPERTY("Mode##ObjLinear", as_portal.exit_impulses.object_exit_impulse.mode);
				}

				MULTIPROPERTY("Angular impulse##ObjAngular", as_portal.exit_impulses.object_exit_angular_impulse.amount);

				{
					auto scope = scoped_indent();
					MULTIPROPERTY("Mode##ObjAngular", as_portal.exit_impulses.object_exit_angular_impulse.mode);
				}
			}
		}

		if (auto scope = augs::imgui::scoped_tree_node_ex("Advanced")) {
			MULTIPROPERTY("Exit cooldown (ms)", as_portal.exit_cooldown_ms);
			tooltip_on_hover("Since the exit is a portal that can be entered too,\nwe need to ignore it for a split second,\nto prevent an infinite loop where an object perpetually teleports back and forth.");

		}

		MULTIPROPERTY("Context tip", as_portal.context_tip.is_enabled);

		if (last_result) {
			if (insp.as_portal.context_tip.is_enabled) {
				result = "Enabled Context tip in %x";
			}
			else {
				result = "Disabled Context tip in %x";
			}
		}

		{
			auto scope = scoped_id("CONTEXTTIP");

			if (insp.as_portal.context_tip.is_enabled) {
				auto ind = scoped_indent();

				MULTIPROPERTY("##ContextTip", as_portal.context_tip.value);
			}
		}
	}

	return result;
}

EDIT_FUNCTION(editor_explosive_node_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);

	return result;
}

EDIT_FUNCTION(editor_prefab_node_editable& insp, T& es, const editor_prefab_resource& resource, const id_widget_handler& special_handler) {
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);
	MULTIPROPERTY("Size", size);

	auto edit_as_aquarium = [&]() {
		MULTIPROPERTY("Sand 1", as_aquarium.sand_1);
		MULTIPROPERTY("Sand 2", as_aquarium.sand_2);
		MULTIPROPERTY("Sand edge", as_aquarium.sand_edge);

		MULTIPROPERTY("Dune small", as_aquarium.dune_small);
		MULTIPROPERTY("Dune big", as_aquarium.dune_big);

		MULTIPROPERTY("Wall", as_aquarium.wall);
		MULTIPROPERTY("Wall top corners", as_aquarium.wall_top_corners);
		MULTIPROPERTY("Wall bottom corners", as_aquarium.wall_bottom_corners);
		MULTIPROPERTY("Wall smooth end", as_aquarium.wall_smooth_end);

		MULTIPROPERTY("Wall top foreground", as_aquarium.wall_top_foreground);

		MULTIPROPERTY("Glass", as_aquarium.glass);
		MULTIPROPERTY("Glass start", as_aquarium.glass_start);

		MULTIPROPERTY("Sand lamp body", as_aquarium.sand_lamp_body);
		MULTIPROPERTY("Sand lamp light", as_aquarium.sand_lamp_light);

		MULTIPROPERTY("Wall lamp body", as_aquarium.wall_lamp_body);
		MULTIPROPERTY("Wall lamp light", as_aquarium.wall_lamp_light);

		MULTIPROPERTY("Flower 1", as_aquarium.flower_1);
		MULTIPROPERTY("Flower 2", as_aquarium.flower_2);
		MULTIPROPERTY("Coral", as_aquarium.coral);

		MULTIPROPERTY("Fish 1", as_aquarium.fish_1);
		MULTIPROPERTY("Fish 2", as_aquarium.fish_2);
		MULTIPROPERTY("Fish 3", as_aquarium.fish_3);
		MULTIPROPERTY("Fish 4", as_aquarium.fish_4);
		MULTIPROPERTY("Fish 5", as_aquarium.fish_5);
		MULTIPROPERTY("Fish 6", as_aquarium.fish_6);
		MULTIPROPERTY("Fish 1 count", as_aquarium.fish_1_count);
		MULTIPROPERTY("Fish 2 count", as_aquarium.fish_2_count);
		MULTIPROPERTY("Fish 3 count", as_aquarium.fish_3_count);
		MULTIPROPERTY("Fish 4 count", as_aquarium.fish_4_count);
		MULTIPROPERTY("Fish 5 count", as_aquarium.fish_5_count);
		MULTIPROPERTY("Fish 6 count", as_aquarium.fish_6_count);

		MULTIPROPERTY("Fish random seed", as_aquarium.fish_seed);

		MULTIPROPERTY("Bubbles", as_aquarium.bubbles);
		MULTIPROPERTY("Flower bubbles", as_aquarium.flower_bubbles);

		MULTIPROPERTY("Wandering pixels 1", as_aquarium.wandering_pixels_1);
		MULTIPROPERTY("Wandering pixels 2", as_aquarium.wandering_pixels_2);

		MULTIPROPERTY("Ambience left", as_aquarium.ambience_left);
		MULTIPROPERTY("Ambience right", as_aquarium.ambience_right);

		MULTIPROPERTY("Water overlay", as_aquarium.water_overlay);
		MULTIPROPERTY("Interior collider", as_aquarium.collider_interior);

		MULTIPROPERTY("Caustics", as_aquarium.caustics);

		MULTIPROPERTY("Caustics count", as_aquarium.caustics_count);
		MULTIPROPERTY("Caustics random seed", as_aquarium.caustics_seed);

		MULTIPROPERTY("Dim caustics count", as_aquarium.dim_caustics_count);
		MULTIPROPERTY("Dim caustics random seed", as_aquarium.dim_caustics_seed);

		ImGui::Separator();
		text_color("Colors", yellow);

		text_disabled("(Set alpha to 0 to disable the lamp completely.)");

		MULTIPROPERTY("Left bottom lamp color", as_aquarium.left_bottom_lamp_color);
		MULTIPROPERTY("Right top lamp color", as_aquarium.right_top_lamp_color);
		MULTIPROPERTY("Top light color", as_aquarium.top_lamp_color);
		MULTIPROPERTY("Sand lamp color", as_aquarium.sand_lamp_color);

		ImGui::Separator();
		text_color("Positioning", yellow);

		MULTIPROPERTY("Flip glass vertically", as_aquarium.flip_glass_vertically);
		MULTIPROPERTY("Glass start offset", as_aquarium.glass_start_offset);
		MULTIPROPERTY("Wall lamp offset", as_aquarium.wall_lamp_offset);
	};

	using P = editor_builtin_prefab_type;

	switch (resource.editable.type) {
		case P::AQUARIUM:
			edit_as_aquarium();
			break;

		default:
			break;
	}

	return result;
}

EDIT_FUNCTION(editor_light_node_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Color", color);
	MULTIPROPERTY("Positional vibration", positional_vibration);
	MULTIPROPERTY("Intensity vibration", intensity_vibration);

	ImGui::Separator();
	text("Falloff");

	MULTIPROPERTY("Radius", falloff.radius);
	MULTIPROPERTY("Strength", falloff.strength);

#if EDIT_ATTENUATIONS
	thread_local editor_tweaked_widget_tracker tracker;
	thread_local editor_light_falloff operated_falloff;

	auto prev_foff = insp.falloff;
	auto prev_woff = insp.wall_falloff;

	auto rebalance_coeffs = [&](
		auto& changed, 
		auto& first, 
		auto& second,
		const auto& first_orig, 
		const auto& second_orig,
		bool is_wall
	) {
		//LOG_NVPS(first_orig, second_orig);
		if (last_result) {
			if (tracker.changed(0)) {
				changed = std::clamp(changed, 0.0f, 1.0f);
				operated_falloff = is_wall ? prev_woff.value : prev_foff;
				tracker.update(0);
			}

			const auto total_orig = first_orig + second_orig;

			const auto first_shares = first_orig == second_orig ? 0.5f : first_orig / total_orig;
			const auto second_shares = first_orig == second_orig ? 0.5f : second_orig / total_orig;

			const auto new_total = 1 - changed;

			first = first_shares * new_total;
			second = second_shares * new_total;

			for (auto& e : es) {
				if (is_wall) {
					e.after.wall_falloff = insp.wall_falloff;
				}
				else {
					e.after.falloff = insp.falloff;
				}
			}
		}
	};

	if (auto scope = augs::imgui::scoped_tree_node_ex("Attenuations (advanced)")) {
		MULTIPROPERTY("Constant", falloff.constant);
		rebalance_coeffs(
			insp.falloff.constant,

			insp.falloff.linear,
			insp.falloff.quadratic,
			operated_falloff.linear,
			operated_falloff.quadratic,
			false
		);

		MULTIPROPERTY("Linear", falloff.linear);
		rebalance_coeffs(
			insp.falloff.linear,

			insp.falloff.constant,
			insp.falloff.quadratic,
			operated_falloff.constant,
			operated_falloff.quadratic,
			false
		);

		MULTIPROPERTY("Quadratic", falloff.quadratic);
		rebalance_coeffs(
			insp.falloff.quadratic,

			insp.falloff.constant,
			insp.falloff.linear,
			operated_falloff.constant,
			operated_falloff.linear,
			false
		);
	}
#endif

	MULTIPROPERTY("Custom falloff for walls", wall_falloff.is_enabled);

	if (last_result) {
		if (insp.wall_falloff.is_enabled) {
			result = "Enabled Custom wall falloff in %x";
		}
		else {
			result = "Disabled Custom wall falloff %x";
		}
	}

	{
		auto scope = scoped_id("WALLFALLOFS");

		auto disabled = maybe_disabled_only_cols(!insp.wall_falloff.is_enabled);

		if (insp.wall_falloff.is_enabled) {
			auto ind = scoped_indent();

			MULTIPROPERTY("Radius", wall_falloff.value.radius);
			MULTIPROPERTY("Strength", wall_falloff.value.strength);

#if EDIT_ATTENUATIONS
			if (auto scope = augs::imgui::scoped_tree_node_ex("Attenuations (advanced)")) {
				auto& edited_foff = insp.wall_falloff.value;

				MULTIPROPERTY("Constant", wall_falloff.value.constant);
				rebalance_coeffs(
					edited_foff.constant,

					edited_foff.linear,
					edited_foff.quadratic,
					operated_falloff.linear,
					operated_falloff.quadratic,
					true
				);

				MULTIPROPERTY("Linear", wall_falloff.value.linear);
				rebalance_coeffs(
					edited_foff.linear,

					edited_foff.constant,
					edited_foff.quadratic,
					operated_falloff.constant,
					operated_falloff.quadratic,
					true
				);

				MULTIPROPERTY("Quadratic", wall_falloff.value.quadratic);
				rebalance_coeffs(
					edited_foff.quadratic,

					edited_foff.constant,
					edited_foff.linear,
					operated_falloff.constant,
					operated_falloff.linear,
					true
				);
			}
#endif
		}
	}

	return result;
}

EDIT_FUNCTION(editor_particles_node_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY_POSITION(pos);
	MULTIPROPERTY("Rotation", rotation);

	MULTIPROPERTY("Color", color);
	MULTIPROPERTY("Scale amounts", scale_amounts);
	MULTIPROPERTY("Scale lifetimes", scale_lifetimes);

	return result;
}

EDIT_FUNCTION(editor_layer_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY("Active", active);

	if (ImGui::IsItemHovered()) {
		text_tooltip("If you untick this, nodes will not be added to the scene.\nYou won't be able to see them or interact with them.");
	}

	MULTIPROPERTY("Selectable on scene", selectable_on_scene);

	if (ImGui::IsItemHovered()) {
		text_tooltip("If you untick this, nodes will stop reacting to the mouse cursor.\nYou'll still see the nodes on the scene.\n\nThis way you can comfortably work on nodes only from layers of interest:\ne.g. disable - but still see - foregrounds while you tweak the objects beneath.");
	}

	MULTIPROPERTY("Opacity", opacity);
	MULTIPROPERTY("Tint", tint);

	return result;
}

EDIT_FUNCTION(
	editor_sprite_resource_editable& insp,
	T& es,
	const std::vector<editor_sprite_resource_editable>& defaults,
	const image_color_picker_widget& picker,
	const non_standard_shape_widget& shape_picker,
	const id_widget_handler& special_handler
) {
	using namespace augs::imgui;

	bool last_result = false;
	std::string result;

	MULTIPROPERTY("Domain", domain);

	//ImGui::Separator();

	MULTIPROPERTY("Color", color);
	MULTIPROPERTY("Neon color", neon_color);
	//ImGui::Separator();
	MULTIPROPERTY("Size", size);

	bool perform_size_reset = false;

	for (std::size_t i = 0; i < defaults.size(); ++i) {
		const auto original_size = defaults[i].size;
		const auto current_size = es[i].after.size;

		if (original_size != current_size) {
			perform_size_reset = true;
		}
	}

	if (perform_size_reset) {
		ImGui::SameLine();

		if (ImGui::Button("Reset")) {
			for (std::size_t i = 0; i < defaults.size(); ++i) {
				es[i].after.size = defaults[i].size;
			}

			return "Reset %x size to original";
		}
	}

	MULTIPROPERTY("Stretch when resized", stretch_when_resized);

	{
		MULTIPROPERTY("##NeonMap", neon_map.is_enabled);

		if (last_result) {
			if (insp.neon_map.is_enabled) {
				result = "Enabled neon map in %x";
			}
			else {
				result = "Disabled neon map in %x";
			}
		}

		ImGui::SameLine();

		auto disabled = maybe_disabled_only_cols(!insp.neon_map.is_enabled);

		auto scope = augs::imgui::scoped_tree_node_ex("Neon map");

		if (scope) {
			auto actually_disabled = maybe_disabled_cols(!insp.neon_map.is_enabled);

			MULTIPROPERTY("Color neon", neon_color);

			MULTIPROPERTY("Neon alpha vibration", neon_alpha_vibration.is_enabled);

			if (insp.neon_alpha_vibration.is_enabled) {
				auto indent = scoped_indent();
				MULTIPROPERTY("Lower", neon_alpha_vibration.value.lower);
				MULTIPROPERTY("Upper", neon_alpha_vibration.value.upper);
				//MULTIPROPERTY("Change per second", neon_alpha_vibration.value.change_per_second);
				MULTIPROPERTY("Vibrate diffuse too", vibrate_diffuse_too);
			}

			std::optional<std::size_t> removed_i;

			text("Click on the image to add neon light sources.");

			auto& v = insp.neon_map.value;

			{
				bool colors_different = false;

				for (auto& e : es) {
					if (!(v.light_colors == e.after.neon_map.value.light_colors)) {
						colors_different = true;
					}
				}

				{
					if (picker.handle_prologue("##LightColorPicker", v.light_colors)) {
						result = "Picked new light color in %x";

						for (auto& e : es) {
							e.after.neon_map.value.light_colors.push_back(v.light_colors.back());
						}
					}
				}

				auto cols = maybe_different_value_cols({}, colors_different);

				for (auto& col : v.light_colors) {
					const auto idx = index_in(v.light_colors, col);
					auto id_col = typesafe_sprintf("Light %x", idx + 1);
					auto id_but = typesafe_sprintf("-##Light%x", idx);

					if (ImGui::Button(id_but.c_str())) {
						removed_i = idx;
					}

					ImGui::SameLine();

					if (edit_property(result, id_col, special_handler, col)) {
						for (auto& e : es) {
							auto& lc = e.after.neon_map.value.light_colors;

							if (idx < lc.size()) {
								lc[idx] = col;
							}
						}
					}
				}

				if (removed_i) {
					for (auto& e : es) {
						auto& lc = e.after.neon_map.value.light_colors;

						if (*removed_i < lc.size()) {
							lc.erase(lc.begin() + *removed_i);
						}
					}

					result = typesafe_sprintf("Removed Light %x in %x", *removed_i);
				}
			}


#if 0
			if (ImGui::Button("+##AddNewLightColor")) {
				v.light_colors.emplace_back(white);
				result = "Added new light color in %x";
			}

			text("Add new light source");
#endif

			if (auto scope = augs::imgui::scoped_tree_node_ex("Advanced neon map parameters")) {
				MULTIPROPERTY("Standard deviation", neon_map.value.standard_deviation);
				MULTIPROPERTY("Radius", neon_map.value.radius.x);

				for (auto& e : es) {
					e.after.neon_map.value.radius.y = e.after.neon_map.value.radius.x;
				}

				MULTIPROPERTY("Amplification", neon_map.value.amplification);
			}
		}
	}

	MULTIPROPERTY("Color wave", color_wave_speed.is_enabled);
	{
		if (insp.color_wave_speed.is_enabled) {
			auto indent = scoped_indent();
			MULTIPROPERTY("Speed", color_wave_speed.value);
		}
	}

	MULTIPROPERTY("Rotate continuously", rotate_continuously_degrees_per_sec.is_enabled);
	{
		if (insp.rotate_continuously_degrees_per_sec.is_enabled) {
			auto indent = scoped_indent();
			MULTIPROPERTY("Degrees per sec", rotate_continuously_degrees_per_sec.value);
		}
	}

	if (insp.domain == editor_sprite_domain::PHYSICAL) {
		MULTIPROPERTY("Cover background neons", as_physical.cover_background_neons);
		MULTIPROPERTY("Illuminate like wall", as_physical.illuminate_like_wall);

		ImGui::Separator();
		text_color("Physics", yellow);
		ImGui::Separator();

		{
			if (auto tweak_type = shape_picker.handle("Custom collider shape", insp.as_physical.custom_shape)) {
				augs::fix_polygon_winding(insp.as_physical.custom_shape.source_polygon);
				augs::refresh_convex_partitioning(insp.as_physical.custom_shape);

				for (auto& e : es) {
					e.after.as_physical.custom_shape = insp.as_physical.custom_shape;
				}

				if (tweak_type == tweaker_type::CONTINUOUS) {
					result = "Edited shape vertices in %x";
				}
				else {
					result = "Altered physical shape of %x";
				}
			}

			ImGui::Separator();
		}

		MULTIPROPERTY("Material", as_physical.material);

		MULTIPROPERTY("Density", as_physical.density);
		MULTIPROPERTY("Friction", as_physical.friction);
		MULTIPROPERTY("Bounciness", as_physical.bounciness);
		MULTIPROPERTY("Penetrability", as_physical.penetrability);
		MULTIPROPERTY("Collision sound sensitivity", as_physical.collision_sound_sensitivity);

		MULTIPROPERTY("Immovable", as_physical.is_static);

		if (ImGui::IsItemHovered()) {
			text_tooltip("If enabled, will be permanently set in place.\nWon't move no matter what.\nUse it on layout-defining walls and objects.");
		}

		if (!insp.as_physical.is_static) {
			MULTIPROPERTY("Linear damping", as_physical.linear_damping);
			MULTIPROPERTY("Angular damping", as_physical.angular_damping);
		}

		MULTIPROPERTY("Walk-through", as_physical.is_walk_through);

		if (ImGui::IsItemHovered()) {
			text_tooltip("If enabled, won't collide with characters.\nImagine a smoke that blocks the line of sight,\nbut lets all physical bodies through.");
		}

		MULTIPROPERTY("See-through", as_physical.is_see_through);

		if (ImGui::IsItemHovered()) {
			text_tooltip("If enabled, lets the light through.\nEnemies will be visible behind this object.\nUse it on walls of glass.");
		}

		MULTIPROPERTY("Grenades fly over", as_physical.is_throw_through);

		if (ImGui::IsItemHovered()) {
			text_tooltip("If enabled, grenades will freely fly over this object.\nNOTE that rockets will fly over this object as well.");
		}

		MULTIPROPERTY("Knives fly over", as_physical.is_melee_throw_through);

		if (ImGui::IsItemHovered()) {
			text_tooltip("If enabled, knives and all thrown melees will freely fly over this object.");
		}

		MULTIPROPERTY("Bullets fly over", as_physical.is_shoot_through);

		if (ImGui::IsItemHovered()) {
			text_tooltip("If enabled, bullets and magic missiles will freely fly over this object.\nCharacters will still collide - except their weapons.\n\nNote that rockets are NOT considered bullets.\nFor these, you must tick Grenades fly over.");
		}
	}
	else {
		MULTIPROPERTY("Cover background neons", as_nonphysical.cover_background_neons);
		{
			const bool full_illumination_already = insp.domain == editor_sprite_domain::FOREGROUND && insp.as_nonphysical.full_illumination;
			auto disabled = maybe_disabled_only_cols(full_illumination_already);
			MULTIPROPERTY("Illuminate like wall", as_nonphysical.illuminate_like_wall);
		}

		if (insp.domain == editor_sprite_domain::FOREGROUND) {
			MULTIPROPERTY("Full illumination", as_nonphysical.full_illumination);
		}

		ImGui::Separator();

		MULTIPROPERTY("Custom footstep", as_nonphysical.custom_footstep.is_enabled);

		if (insp.as_nonphysical.custom_footstep.is_enabled) {
			auto indent = scoped_indent();

			MULTIPROPERTY("Walking speed", as_nonphysical.custom_footstep.value.walking_speed);
			SOUND_EFFECT_LEAN_MULTIPROPERTY("Footstep sound", as_nonphysical.custom_footstep.value.sound);
		}
	}

	ImGui::Separator();

	return result;
}

EDIT_FUNCTION(editor_sound_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	MULTIPROPERTY("Gain", gain);
	MULTIPROPERTY("Pitch", pitch);
	MULTIPROPERTY("Spatialize", spatialize);

	if (insp.spatialize) {
		const auto tooltip_content = "If left unticked, will be dependent on the context.\nE.g. if the sound is used as a footstep,\ndefault heard distance will be way different than\nif it's a gunshot sound or a shell collision sound.";

		auto do_tooltip = [&]() {
			if (ImGui::IsItemHovered()) {
				text_tooltip(tooltip_content);
			}
		};

		MULTIPROPERTY("Custom distance model", distance_model.is_enabled);
		do_tooltip();

		if (insp.distance_model.is_enabled) {
			MULTIPROPERTY("##DistanceModel", distance_model.value);

			if (last_result) {
				result = "Changed distance model in %x";
			}
		}

		MULTIPROPERTY("Custom max distance", max_distance.is_enabled);
		do_tooltip();

		if (insp.max_distance.is_enabled) {
			MULTIPROPERTY("##MaxDistance", max_distance.value);

			if (last_result) {
				result = "Changed max distance in %x";
			}
		}

		MULTIPROPERTY("Custom reference distance", reference_distance.is_enabled);
		do_tooltip();

		if (insp.reference_distance.is_enabled) {
			MULTIPROPERTY("##ReferenceDistance", reference_distance.value);

			if (last_result) {
				result = "Changed reference distance in %x";
			}
		}

		MULTIPROPERTY("Doppler intensity", doppler_intensity);
	}

	MULTIPROPERTY("Loop forever", loop);

	if (!insp.loop) {
		MULTIPROPERTY("Times played", play_times);
	}

	return result;
}

EDIT_FUNCTION(editor_prefab_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

EDIT_FUNCTION(editor_light_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

EDIT_FUNCTION(editor_particles_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

EDIT_FUNCTION(editor_material_resource_editable& insp, T& es, const id_widget_handler& special_handler) {
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	auto silence_opposite_collision_sound_tooltip = []() {
		if (ImGui::IsItemHovered()) {
			auto scope = scoped_tooltip();
			text("If ticked, the collision sound chosen by the other surface will be silenced.\n\nExample:\n");
			text("Normally, if e.g. a crate specifies a default \"woody\" collision\nand a fence specifies a metallic sound for Fence<->Wood collision,\nboth would be played whenever a crate hits a fence.\nBut you could want to silence the crate's woody effect to strongly emphasize the fence sound.");
		}
	};

	MULTIPROPERTY("Penetrability", penetrability);
	MULTIPROPERTY("Max ricochet angle", max_ricochet_angle);

	tooltip_on_hover("0 - never ricochets.\n180 - always ricochets (like a perfectly reflecting mirror).\nThe higher the angle, the easier it is to ricochet.");

	MULTIPROPERTY("Point-blank ricochets", point_blank_ricochets);

	tooltip_on_hover("Normally bullets never ricochet if fired very close to the surface.\nThis setting enables ricochets even at point-blank range.");

	ImGui::Separator();
	text_color("Default damage behavior", yellow);
	ImGui::Separator();

	text_disabled("Played when exposed to any damage,\ne.g. the surface is shot/knifed.");
	SOUND_EFFECT_LEAN_MULTIPROPERTY("Damage sound", damage_sound);

	PARTICLE_EFFECT_MULTIPROPERTY("Damage particles", damage_particles);

	if (auto scope = augs::imgui::scoped_tree_node_ex("Silence damager")) {
		MULTIPROPERTY("Silence damager impact sound", silence_damager_impact_sound);
		tooltip_on_hover("Tick to silence the default melee impact/bullet penetration sound.");
		MULTIPROPERTY("Silence damager destruction sound", silence_damager_destruction_sound);
		tooltip_on_hover("Tick to silence the default bullet destruction sound.");
	}

	ImGui::Separator();
	text_color("Default collision behavior", yellow);
	ImGui::Separator();

	text_disabled("Played by default, when colliding with anything.");

	MULTIPROPERTY("Sound", default_collision.sound);

	const auto sensitivity_tooltip = "High values:\nCollison sound volume will always be 100% even if you barely touch it.\nGood for e.g. bouncers/trampolines.\n\nLow values:\nVolume will scale more smoothly with how strongly you hit the surface.\nPerfect for e.g. bags of dirt.";

	const auto pitch_tooltip = "Collision sound varies depending on angle of impact.\nThe sound might be higher and sometimes lower.";

	if (insp.default_collision.sound.is_set()) {
		MULTIPROPERTY("Min pitch", default_collision.min_pitch);
		tooltip_on_hover(pitch_tooltip);
		MULTIPROPERTY("Max pitch", default_collision.max_pitch);
		tooltip_on_hover(pitch_tooltip);

		MULTIPROPERTY("Collision sound sensitivity", default_collision.collision_sound_sensitivity);

		tooltip_on_hover(sensitivity_tooltip);

		if (auto scope = augs::imgui::scoped_tree_node_ex("Cooldowns")) {
			MULTIPROPERTY("Minimum interval (ms)", default_collision.min_interval_ms);
			MULTIPROPERTY("Mute after playing times", default_collision.mute_after_playing_times);
			MULTIPROPERTY("Unmute after (ms)", default_collision.unmute_after_ms);
		}
	}

	PARTICLE_EFFECT_MULTIPROPERTY("Particles (optional)", default_collision.particles);

	MULTIPROPERTY("Silence opposite collision sound", default_collision.silence_opposite_collision_sound);
	silence_opposite_collision_sound_tooltip();

	ImGui::Separator();
	text_color("Specific collisions", yellow);
	ImGui::Separator();

	text_disabled("You can specify non-default sounds for\ncollisions with specific materials.");

	bool collisions_different = false;

	for (auto& e : es) {
		if (!(insp.specific_collisions == e.after.specific_collisions)) {
			collisions_different = true;
		}
	}

	if (ImGui::Button("Add specific collision")) {
		insp.specific_collisions.push_back({});
		result = "Added new specific collision in %x";

		for (auto& e : es) {
			auto& lc = e.after.specific_collisions;
			lc.push_back({});
		}
	}

	auto cols = maybe_different_value_cols({}, collisions_different);

	std::optional<std::size_t> removed_i;

	for (auto& coll : insp.specific_collisions) {
		const auto idx = index_in(insp.specific_collisions, coll);

		auto id_but = typesafe_sprintf("-##Coll%x", idx);
		const auto id_str = typesafe_sprintf("SpecificCollision%x", idx);
		auto this_scope = scoped_id(id_str.c_str());

		ImGui::Separator();

		if (ImGui::Button(id_but.c_str())) {
			removed_i = idx;
		}

		auto write_to_others = [&]() {
			for (auto& e : es) {
				auto& lc = e.after.specific_collisions;

				if (idx < lc.size()) {
					lc[idx] = coll;
				}
			}
		};

		if (edit_property(result, "Collider", special_handler, coll.collider)) write_to_others();

		if (coll.collider.is_set()) {
			if (edit_property(result, "Sound", special_handler, coll.sound)) write_to_others();

			if (coll.sound.is_set()) {
				if (edit_property(result, "Min pitch", special_handler, coll.min_pitch)) write_to_others();
				tooltip_on_hover(pitch_tooltip);
				if (edit_property(result, "Max pitch", special_handler, coll.max_pitch)) write_to_others();
				tooltip_on_hover(pitch_tooltip);
				if (edit_property(result, "Collision sound sensitivity", special_handler, coll.collision_sound_sensitivity)) write_to_others();
				tooltip_on_hover(sensitivity_tooltip);

				if (auto scope = augs::imgui::scoped_tree_node_ex("Cooldowns")) {
					if (edit_property(result, "Minimum interval (ms)", special_handler, coll.min_interval_ms)) write_to_others();
					if (edit_property(result, "Mute after playing times", special_handler, coll.mute_after_playing_times)) write_to_others();
					if (edit_property(result, "Unmute after (ms)", special_handler, coll.unmute_after_ms)) write_to_others();
				}
			}

			if (edit_property(result, "Silence opposite collision sound", special_handler, coll.silence_opposite_collision_sound)) write_to_others();
			silence_opposite_collision_sound_tooltip();
		}
	}

	if (removed_i) {
		for (auto& e : es) {
			auto& lc = e.after.specific_collisions;

			if (*removed_i < lc.size()) {
				lc.erase(lc.begin() + *removed_i);
			}
		}

		result = typesafe_sprintf("Removed Collision %x in %x", *removed_i);
	}

	return result;
}

EDIT_FUNCTION(editor_firearm_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

EDIT_FUNCTION(editor_ammunition_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

EDIT_FUNCTION(editor_tool_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

EDIT_FUNCTION(editor_melee_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

EDIT_FUNCTION(editor_wandering_pixels_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	if (auto scope = augs::imgui::scoped_tree_node_ex("Node defaults")) {
		// TODO: let this be edited
		/* if (const auto new_res = perform_editable_gui(e.node_defaults); !new_res.empty()) { */
		/* 	result = new_res; */
		/* } */
	}

	return result;
}

EDIT_FUNCTION(editor_point_marker_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

EDIT_FUNCTION(editor_area_marker_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

EDIT_FUNCTION(editor_explosive_resource_editable& insp, T& es) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	bool last_result = false;
	std::string result;

	(void)insp; (void)es; (void)last_result;

	return result;
}

SINGLE_EDIT_FUNCTION(editor_project_about& insp) {
	auto special_handler = default_widget_handler();
	using namespace augs::imgui;
	std::string result;

	if (input_text("Author", insp.author)) {
		result = "Edited Author in %x";
	}

	ImGui::Separator();

	if (input_multiline_text("Short description", insp.short_description, 3)) {
		result = "Edited Short description in %x";
	}

	ImGui::Separator();

	if (input_multiline_text("Full description", insp.full_description, 10)) {
		result = "Edited Full description in %x";
	}

	ImGui::Separator();

	if (input_multiline_text("Welcome message", insp.welcome_message, 6)) {
		result = "Edited Welcome message in %x";
	}

	return result;
}

SINGLE_EDIT_FUNCTION(editor_playtesting_settings& insp, const editor_playtesting_settings defaults, id_widget_handler special_handler) {
	using namespace augs::imgui;
	std::string result;

	(void)defaults;

	special_handler.allow_none = false;

	PROPERTY("Mode to test", mode);

	special_handler.allow_none = true;

	PROPERTY("Starting faction", starting_faction);

	PROPERTY("Skip warmup", skip_warmup);
	PROPERTY("Skip freeze time", skip_freeze_time);
	PROPERTY("Unlimited money", unlimited_money);
	PROPERTY("Bots (static)", bots);

	return result;
}

SINGLE_EDIT_FUNCTION(editor_arena_settings& insp, const editor_arena_settings defaults, id_widget_handler special_handler) {
	using namespace augs::imgui;
	std::string result;

	(void)defaults;

	special_handler.allow_none = false;

	PROPERTY("Default server mode", default_server_mode);

	if (ImGui::IsItemHovered()) {
		text_tooltip("When an actual server loads your arena,\nthis game mode will be chosen first.\n\nTo change the mode you're testing in the editor,\ngo to Playtesting tab.");
	}

	special_handler.allow_none = true;
	PROPERTY("Include disabled nodes", include_disabled_nodes);

	if (ImGui::IsItemHovered()) {
		text_tooltip("If ticked, all nodes disabled with the \"eye\" icon\nwill still appear in-game (both when playtesting and live).\n\nThey will remain hidden when editing.");
	}

	PROPERTY("Ambient light color", ambient_light_color);
	THEME_PROPERTY("Warmup theme", warmup_theme);

	return result;
}

EDIT_FUNCTION(
	editor_game_mode_resource_editable& insp,
	T& es,
	const std::vector<editor_game_mode_resource_editable>& defaults,
	const editor_game_mode_id type,
	id_widget_handler special_handler
) {
	using namespace augs::imgui;
	std::string result;
	bool last_result = false;

	if (auto scope = augs::imgui::scoped_tree_node_ex("Layers")) {
		if (auto scope = augs::imgui::scoped_tree_node_ex("Activate")) {
			bool progressions_different = false;

			for (auto& e : es) {
				if (!(insp.common.activate_layers == e.after.common.activate_layers)) {
					progressions_different = true;
				}
			}

			if (ImGui::Button("Add layer")) {
				insp.common.activate_layers.push_back({});
				result = "New entry in %x";

				for (auto& e : es) {
					auto& lc = e.after.common.activate_layers;
					lc.push_back({});
				}
			}

			auto cols = maybe_different_value_cols({}, progressions_different);

			std::optional<std::size_t> removed_i;

			for (auto& prog : insp.common.activate_layers) {
				const auto idx = index_in(insp.common.activate_layers, prog);

				auto id_but = typesafe_sprintf("-##Elem%x", idx);
				const auto id_str = typesafe_sprintf("Element%x", idx);
				auto this_scope = scoped_id(id_str.c_str());

				if (ImGui::Button(id_but.c_str())) {
					removed_i = idx;
				}

				ImGui::SameLine();

				auto write_to_others = [&]() {
					for (auto& e : es) {
						auto& lc = e.after.common.activate_layers;

						if (idx < lc.size()) {
							lc[idx] = prog;
						}
					}
				};

				special_handler.allow_none = false;
				auto label = typesafe_sprintf("Layer %x", idx);
				if (edit_property(result, label, special_handler, prog)) write_to_others();
				special_handler.allow_none = true;
			}

			if (removed_i) {
				for (auto& e : es) {
					auto& lc = e.after.common.activate_layers;

					if (*removed_i < lc.size()) {
						lc.erase(lc.begin() + *removed_i);
					}
				}

				result = typesafe_sprintf("Removed entry %x from %x", *removed_i);
			}
		}

		if (auto scope = augs::imgui::scoped_tree_node_ex("Deactivate")) {
			bool progressions_different = false;

			for (auto& e : es) {
				if (!(insp.common.deactivate_layers == e.after.common.deactivate_layers)) {
					progressions_different = true;
				}
			}

			if (ImGui::Button("Add layer")) {
				insp.common.deactivate_layers.push_back({});
				result = "New entry in %x";

				for (auto& e : es) {
					auto& lc = e.after.common.deactivate_layers;
					lc.push_back({});
				}
			}

			auto cols = maybe_different_value_cols({}, progressions_different);

			std::optional<std::size_t> removed_i;

			for (auto& prog : insp.common.deactivate_layers) {
				const auto idx = index_in(insp.common.deactivate_layers, prog);

				auto id_but = typesafe_sprintf("-##Elem%x", idx);
				const auto id_str = typesafe_sprintf("Element%x", idx);
				auto this_scope = scoped_id(id_str.c_str());

				if (ImGui::Button(id_but.c_str())) {
					removed_i = idx;
				}

				ImGui::SameLine();

				auto write_to_others = [&]() {
					for (auto& e : es) {
						auto& lc = e.after.common.deactivate_layers;

						if (idx < lc.size()) {
							lc[idx] = prog;
						}
					}
				};

				special_handler.allow_none = false;
				auto label = typesafe_sprintf("Layer %x", idx);
				if (edit_property(result, label, special_handler, prog)) write_to_others();
				special_handler.allow_none = true;
			}

			if (removed_i) {
				for (auto& e : es) {
					auto& lc = e.after.common.deactivate_layers;

					if (*removed_i < lc.size()) {
						lc.erase(lc.begin() + *removed_i);
					}
				}

				result = typesafe_sprintf("Removed entry %x from %x", *removed_i);
			}
		}
	}

	auto edit = [&]<typename I>(const I&) {
		if constexpr(std::is_same_v<I, editor_quick_test_mode>) {
			MULTIPROPERTY("Respawn time (ms)", quick_test.respawn_time_ms);

			ImGui::Separator();

			FACTION_EQUIPMENT_PROPERTY("Equipment", quick_test.equipment);
		}
		else if constexpr(std::is_same_v<I, editor_bomb_defusal_mode>) {
			MULTIPROPERTY("Max team score", bomb_defusal.max_team_score);

			MULTIPROPERTY("Warmup time", bomb_defusal.warmup_time);
			MULTIPROPERTY("Freeze time", bomb_defusal.freeze_time);
			MULTIPROPERTY("Buy time", bomb_defusal.buy_time);

			MULTIPROPERTY("Round time", bomb_defusal.round_time);
			MULTIPROPERTY("Round end time", bomb_defusal.round_end_time);

			MULTIPROPERTY("Respawn after (ms)", bomb_defusal.respawn_after_ms);

			tooltip_on_hover("This will turn this mode into a team deathmatch\nwhere you have to either plant/defuse or\nkill enemy players fast enough until the moment\nwhere nobody is left alive.");

			MULTIPROPERTY("Spawn protection (ms)", bomb_defusal.spawn_protection_ms);

			ImGui::Separator();

			FACTION_EQUIPMENT_PROPERTY("Warmup equipment", bomb_defusal.warmup_equipment);

			ImGui::Separator();

			FACTION_EQUIPMENT_PROPERTY("Round start equipment", bomb_defusal.round_start_equipment);
		}
		else if constexpr(std::is_same_v<I, editor_gun_game_mode>) {
			force_show_extra_ammo = true;
			FACTION_EQUIPMENT_PROPERTY("Basic equipment", gun_game.basic_equipment);
			force_show_extra_ammo = false;
			tooltip_on_hover("Basic equipment to give at every level except the final.\nNote the \"Firearm\" here will become a secondary weapon.");

			FACTION_EQUIPMENT_PROPERTY("Final equipment", gun_game.final_equipment);
			tooltip_on_hover("Complete equipment at the final level,\nafter all weapons from Weapon Progression have been cycled.\n\nNote that to beat the final level,\nyou have to kill with any of the items specified here.\n\nE.g. if Hunter is the penultimate weapon and you make a double-kill with it,\nyou will not advance two levels (thus winning),\nbut will still have to score a kill with the knife specified here.");

			MULTIPROPERTY("Can throw melee on final level", gun_game.can_throw_melee_on_final_level);
			tooltip_on_hover("You might end up without weapons if you throw your only knife.\n");

			ImGui::Separator();
			text_color("Weapon Progression", yellow);
			ImGui::Separator();

			{
				bool progressions_different = false;

				for (auto& e : es) {
					if (!(insp.gun_game.progression == e.after.gun_game.progression)) {
						progressions_different = true;
					}
				}

				if (ImGui::Button("Add weapon")) {
					insp.gun_game.progression.push_back({});
					result = "Added new weapon in %x";

					for (auto& e : es) {
						auto& lc = e.after.gun_game.progression;
						lc.push_back({});
					}
				}

				auto cols = maybe_different_value_cols({}, progressions_different);

				std::optional<std::size_t> removed_i;

				for (auto& prog : insp.gun_game.progression) {
					const auto idx = index_in(insp.gun_game.progression, prog);

					auto id_but = typesafe_sprintf("-##Prog%x", idx);
					const auto id_str = typesafe_sprintf("Progression%x", idx);
					auto this_scope = scoped_id(id_str.c_str());

					if (ImGui::Button(id_but.c_str())) {
						removed_i = idx;
					}

					ImGui::SameLine();

					auto write_to_others = [&]() {
						for (auto& e : es) {
							auto& lc = e.after.gun_game.progression;

							if (idx < lc.size()) {
								lc[idx] = prog;
							}
						}
					};

					special_handler.allow_none = false;
					auto label = typesafe_sprintf("Level %x", idx);
					if (edit_property(result, label, special_handler, prog)) write_to_others();
					special_handler.allow_none = true;
				}

				if (removed_i) {
					for (auto& e : es) {
						auto& lc = e.after.gun_game.progression;

						if (*removed_i < lc.size()) {
							lc.erase(lc.begin() + *removed_i);
						}
					}

					result = typesafe_sprintf("Removed Weapon %x in %x", *removed_i);
				}
			}

			ImGui::Separator();

			MULTIPROPERTY("Max team score", gun_game.max_team_score);

			MULTIPROPERTY("Warmup time", gun_game.warmup_time);
			MULTIPROPERTY("Freeze time", gun_game.freeze_time);

			MULTIPROPERTY("Round time", gun_game.round_time);
			MULTIPROPERTY("Round end time", gun_game.round_end_time);

			MULTIPROPERTY("Respawn after (ms)", gun_game.respawn_after_ms);
			MULTIPROPERTY("Spawn protection (ms)", gun_game.spawn_protection_ms);

			ImGui::Separator();

			FACTION_EQUIPMENT_PROPERTY("Warmup equipment", gun_game.warmup_equipment);

			ImGui::Separator();
		}
		else {
			static_assert(always_false_v<I>, "Non-exhaustive if constexpr");
		}
	};

	type.dispatch(edit);
	(void)defaults;

	return result;
}

void editor_inspector_gui::inspect(const inspected_variant inspected, bool wants_multiple) {
	auto different_found = [&]<typename T>(const T&) {
		return inspects_any_different_than<T>();
	};

	const bool different_type_found = std::visit(different_found, inspected);

	if (wants_multiple) {
		if (different_type_found) {
			return;
		}

		if (found_in(all_inspected, inspected)) {
			erase_element(all_inspected, inspected);
		}
		else {
			all_inspected.push_back(inspected);
			mark_last_inspected(inspected, false);
		}
	}
	else {
		const bool not_the_same = all_inspected != decltype(all_inspected) { inspected };

		if (not_the_same) {
			all_inspected = { inspected };
			mark_last_inspected(inspected);

			/*
				Commands will invoke "inspect" on undo/redo,
				even when executing them for the first time - directly by user interaction.

				This is why we need to prevent the tweaked widget from being reset if the inspected object does not change,
				because otherwise continuous commands wouldn't work.
			*/
			tweaked_widget.reset();
		}
	}
}

void editor_inspector_gui::perform(const editor_inspector_input in) {
	using namespace augs::imgui;

	(void)in;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	auto id_handler = id_widget_handler{ in.setup, editor_icon_info_in(in), true };
	const auto& official_map = in.setup.get_official_resource_map();

	auto get_object_category_name = [&]<typename T>(const T&) {
		if constexpr(std::is_same_v<T, editor_node_id>) {
			return "nodes";
		}
		else if constexpr(std::is_same_v<T, editor_resource_id>) {
			return "resources";
		}
		else if constexpr(std::is_same_v<T, editor_layer_id>) {
			return "layers";
		}
		else if constexpr(std::is_same_v<T, inspected_special>) {
			return "objects";
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive handler");
		}

		return "objects";
	};

	auto get_command_index = [&]() {
		return in.setup.get_last_command_index();
	};

	auto post_new_or_rewrite = [&]<typename T>(T&& cmd) {
		bool force_new = false;

		if (begins_with(cmd.describe(), "Added") || begins_with(cmd.describe(), "Picked") || begins_with(cmd.describe(), "Removed") || begins_with(cmd.describe(), "Altered")) {
			force_new = true;
		}

		if (force_new || tweaked_widget.changed(get_command_index())) {
			in.setup.post_new_command(std::forward<T>(cmd));
			tweaked_widget.update(get_command_index());
		}
		else {
			in.setup.rewrite_last_command(std::forward<T>(cmd));
		}
	};

	auto reveal_in_explorer_button = [this](const auto& path) {
		auto cursor = scoped_preserve_cursor();

		if (ImGui::Selectable("##RevealButton")) {
			reveal_in_explorer_once = path;
		}

		if (ImGui::IsItemHovered()) {
			text_tooltip("Reveal in explorer");
		}
	};

	auto resource_handler = [&]<typename R>(
		const R& resource, 
		const auto& resource_id, 
		std::optional<std::vector<inspected_variant>> override_inspector_state = std::nullopt
	) {
		auto name = resource.get_display_name();

		bool request_defaults = false;

		if (!resource_id.is_official) {
			const auto region_avail = ImGui::GetContentRegionAvail().x;

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, region_avail - ImGui::CalcTextSize("Reset").x - ImGui::GetStyle().FramePadding.x);
		}

		const bool include_resources_from_selected_nodes = (node_current_tab == inspected_node_tab_type::RESOURCE);

		if constexpr(is_pathed_resource_v<R>) {
			/* Preserve extension when displaying name */
			name = resource.external_file.path_in_project.filename().string();
			
			const auto& path_in_project = resource.external_file.path_in_project;

			const auto full_path = 
				resource_id.is_official 
				? OFFICIAL_CONTENT_PATH / path_in_project
				: in.setup.resolve_project_path(path_in_project)
			;

			reveal_in_explorer_button(full_path);

			auto resource_type_label = std::string(resource.get_type_name());

			if (override_inspector_state == std::nullopt) {
				const auto cnt = all_inspected.size();
				if (cnt > 1) {
					resource_type_label += typesafe_sprintf(" (%x sel.)", cnt);
				}
			}

			text_color(resource_type_label + ": ", yellow);

			ImGui::SameLine();
			text(name);
		}
		else {
			auto resource_type_label = std::string(resource.get_type_name());

			if (override_inspector_state == std::nullopt) {
				const auto cnt = all_inspected.size();
				if (cnt > 1) {
					resource_type_label += typesafe_sprintf(" (%x sel.)", cnt);
				}
			}

			text_color(resource_type_label + ": ", yellow);

			ImGui::SameLine();

			auto edited_resource_name = resource.unique_name;

			if (input_text<100>("##ResourceName", edited_resource_name, ImGuiInputTextFlags_EnterReturnsTrue)) {
				if (edited_resource_name != resource.unique_name && !edited_resource_name.empty()) {
					auto name_cmd = in.setup.make_command_from_selected_typed_resources<rename_resource_command, R>("Renamed ", include_resources_from_selected_nodes);
					name_cmd.after = edited_resource_name;

					if (name_cmd.size() == 1) {
						name_cmd.built_description = typesafe_sprintf("Renamed resource to %x", name_cmd.after);
					}

					if (!name_cmd.empty()) {
						in.setup.post_new_command(std::move(name_cmd)); 
					}
				}
			}
		}

		auto cmd = in.setup.make_command_from_selected_typed_resources<edit_resource_command<R>, R>("Edited ", include_resources_from_selected_nodes);
		num_last_resources_last_time = cmd.entries.size();

		std::vector<decltype(resource.editable)> hypothetical_defaults;
		hypothetical_defaults.reserve(cmd.entries.size());

		for (auto& e : cmd.entries) {
			const auto entry_resource = in.setup.find_resource(e.resource_id);
			ensure(entry_resource != nullptr);
			e.after = entry_resource->editable;

			hypothetical_defaults.push_back({});
			auto& new_default = hypothetical_defaults.back();

			if constexpr(std::is_same_v<R, editor_sprite_resource>) {
				if (auto ad_hoc = mapped_or_nullptr(in.ad_hoc_atlas, entry_resource->thumbnail_id)) {
					new_default.size = ad_hoc->get_original_size();
				}
			}

			if constexpr(std::is_same_v<R, editor_game_mode_resource>) {
				::setup_game_mode_defaults(new_default, official_map);
			}
			else {
				::setup_resource_defaults(new_default, official_map);
			}
		}

		if (!resource_id.is_official) {
			ImGui::NextColumn();

			{
				bool already_default = true;
				const bool multiple = cmd.entries.size() > 1;

				for (std::size_t i = 0; i < cmd.entries.size(); ++i) {
					if (!(hypothetical_defaults[i] == cmd.entries[i].after)) {
						already_default = false;
						break;
					}
				}

				if (reset_button(already_default, multiple)) {
					request_defaults = true;
				}
			}

			ImGui::Columns(1);
		}

		//game_image(icon, scaled_icon_size, white, vec2::zero, atlas_type);

		ImGui::Separator();

		auto id = scoped_id(name.c_str());

		if (resource_id.is_official) {
			text_color("Official resources cannot be edited.", yellow);
		}
		else {
			auto edited_copy = resource.editable;
			auto changed = std::string();

			if constexpr(std::is_same_v<R, editor_sprite_resource>) {
				/* 
					It's probably better to leave the widgets themselves active
					in case someone wants to copy the values.
				*/

				auto disabled = maybe_disabled_only_cols(resource_id.is_official);

				const auto project_dir = in.setup.get_unofficial_content_dir();

				const auto picker = image_color_picker_widget {
					resource.scene_asset_id,
					in.game_atlas,
					in.setup.get_viewable_defs().image_definitions,
					neon_map_picker_preview,
					project_dir,
					false
				};

				const auto shape_picker = non_standard_shape_widget {
					resource.scene_asset_id,
					in.game_atlas,
					true
				};

				changed = perform_editable_gui(edited_copy, cmd.entries, hypothetical_defaults, picker, shape_picker, id_handler);
			}
			else if constexpr(std::is_same_v<R, editor_material_resource>) {
				changed = perform_editable_gui(edited_copy, cmd.entries, id_handler);
			}
			else if constexpr(std::is_same_v<R, editor_game_mode_resource>) {
				changed = perform_editable_gui(edited_copy, cmd.entries, hypothetical_defaults, resource.type, id_handler);
			}
			else {
				auto disabled = maybe_disabled_only_cols(resource_id.is_official);
				changed = perform_editable_gui(edited_copy, cmd.entries);
			}

			if (request_defaults) {
				for (std::size_t i = 0; i < cmd.entries.size(); ++i) {
					cmd.entries[i].after = hypothetical_defaults[i];
				}

				changed = "Restored defaults to %x";
			}

			if (!changed.empty() && !cmd.empty() && !resource_id.is_official) {
				if (cmd.size() == 1) {
					cmd.built_description = typesafe_sprintf(changed, resource.get_display_name());
				}

				cmd.override_inspector_state = std::move(override_inspector_state);

				post_new_or_rewrite(std::move(cmd)); 
			}
		}
	};

	auto node_handler = [&]<typename N>(const N& node, const auto& node_id) {
		auto id = scoped_id(node.unique_name.c_str());

		const auto resource = in.setup.find_resource(node.resource_id);
		ensure(resource != nullptr);

		if (resource == nullptr) {
			return;
		}

		auto edited_copy = node.editable;
		auto changed = std::string();

		auto cmd = in.setup.make_command_from_selected_typed_nodes<edit_node_command<N>, N>("Edited ");

		bool same_resource = true;

		for (auto& e : cmd.entries) {
			const auto entry_node = in.setup.find_node(e.node_id);
			ensure(entry_node != nullptr);
			e.after = entry_node->editable;

			if (entry_node->resource_id != node.resource_id) {
				same_resource = false;
			}
		}

		text_color(std::string(node.get_type_name()) + ": ", yellow);
		ImGui::SameLine();

		{
			const auto displayed_resource_name = resource->get_display_name();

			using R = typename N::resource_type;
			thread_local resource_chooser<R> chooser;

			auto cols = maybe_different_value_cols({}, !same_resource);

			const bool allow_none = false;

			chooser.perform(
				"##ResourceSelector",
				displayed_resource_name,
				node.resource_id,
				in.setup,
				editor_icon_info_in(in),
				allow_none,
				[&](const editor_typed_resource_id<R>& chosen_id, const auto& chosen_name) {
					auto cmd = in.setup.make_command_from_selected_nodes<change_resource_command>("Changed resource of ");
					cmd.after = chosen_id.operator editor_resource_id();
					cmd.built_description = typesafe_sprintf("Changed resource to %x", chosen_name);

					if (!cmd.empty()) {
						in.setup.post_new_command(std::move(cmd)); 
					}
				}
			);
		}

		{
			auto edited_node_name = node.unique_name;

			if (input_text<100>("Name", edited_node_name, ImGuiInputTextFlags_EnterReturnsTrue)) {
				if (edited_node_name != node.unique_name && !edited_node_name.empty()) {
					auto name_cmd = in.setup.make_command_from_selected_nodes<rename_node_command>("Renamed ");
					name_cmd.after = edited_node_name;

					if (name_cmd.size() == 1) {
						name_cmd.built_description = typesafe_sprintf("Renamed node to %x", name_cmd.after);
					}

					if (!name_cmd.empty()) {
						in.setup.post_new_command(std::move(name_cmd)); 
					}
				}
			}
		}

		ImGui::Separator();
		(void)node_id;

		if constexpr(std::is_same_v<N, editor_sprite_node>) {
			auto original_size = resource->editable.size;
			
			if (resource->official_tag) {
				std::visit(
					[&](const auto& tag) {
						const auto& flavour = in.setup.get_built_official_content().world.get_flavour(to_entity_flavour_id(tag));
						if (const auto sprite = flavour.template find<invariants::sprite>()) {
							original_size = sprite->get_size();
						}
					},
					*resource->official_tag
				);
			}

			changed = perform_editable_gui(edited_copy, cmd.entries, *resource);
		}
		else if constexpr(std::is_same_v<N, editor_prefab_node>) {
			const auto resource = in.setup.find_resource(node.resource_id);
			ensure(resource != nullptr);

			changed = perform_editable_gui(edited_copy, cmd.entries, *resource, id_handler);
		}
		else {
			if constexpr(is_one_of_v<N, editor_point_marker_node, editor_area_marker_node>) {
				const auto resource = in.setup.find_resource(node.resource_id);
				ensure(resource != nullptr);

				if constexpr(std::is_same_v<N, editor_area_marker_node>) {
					changed = perform_editable_gui(edited_copy, cmd.entries, *resource, id_handler);
				}
				else {
					changed = perform_editable_gui(edited_copy, cmd.entries, *resource);
				}
			}
			else {
				changed = perform_editable_gui(edited_copy, cmd.entries);
			}
		}

		if (!changed.empty() && !cmd.empty()) {
			if (cmd.size() == 1) {
				cmd.built_description = typesafe_sprintf(changed, node.get_display_name());
			}

			post_new_or_rewrite(std::move(cmd)); 
		}

		(void)node;
	};

	auto layer_handler = [&](const editor_layer& layer, const editor_layer_id layer_id) {
		const auto insp_count = all_inspected.size();
		auto label = std::string("Layer");

		if (insp_count > 1) {
			label += typesafe_sprintf("s (%x sel.)", insp_count);
		}

		label += ":";

		text_color(label, yellow);

		{
			ImGui::SameLine();

			{
				auto edited_layer_name = layer.unique_name;

				if (input_text<100>("##NameInput", edited_layer_name, ImGuiInputTextFlags_EnterReturnsTrue)) {
					if (edited_layer_name != layer.unique_name && !edited_layer_name.empty()) {
						auto cmd = in.setup.make_command_from_selected_layers<rename_layer_command>("Renamed ");
						cmd.after = edited_layer_name;

						if (cmd.size() == 1) {
							cmd.built_description = typesafe_sprintf("Renamed layer to %x", cmd.after);
						}

						if (!cmd.empty()) {
							in.setup.post_new_command(std::move(cmd)); 
						}
					}
				}
			}

			{
				auto edited_copy = layer.editable;
				auto cmd = in.setup.make_command_from_selected_layers<edit_layer_command>("Edited ");

				for (auto& e : cmd.entries) {
					const auto layer = in.setup.find_layer(e.layer_id);
					ensure(layer != nullptr);
					e.after = layer->editable;
				}

				(void)layer_id;
				const auto changed = perform_editable_gui(edited_copy, cmd.entries);

				if (!changed.empty() && !cmd.empty()) {
					if (cmd.size() == 1) {
						cmd.built_description = typesafe_sprintf(changed, layer.get_display_name());
					}

					post_new_or_rewrite(std::move(cmd)); 
				}
			}
		}
	};

	auto project_settings_handler = [&]() {
		const auto& project = in.setup.get_project();

		reveal_in_explorer_button(in.setup.get_paths().project_json);

		text_color("Project settings: ", yellow);

		ImGui::SameLine();

		text(typesafe_sprintf("%x", in.setup.get_paths().arena_name));

		ImGui::Separator();

		simple_tabs(
			project_current_tab,

			std::make_pair("Arena", inspected_project_tab_type::ARENA),
			std::make_pair("Playtesting", inspected_project_tab_type::PLAYTESTING),
			std::make_pair("About", inspected_project_tab_type::ABOUT)
		);

		ImGui::Separator();

		auto do_reset_button_for_project_settings = [&](auto cmd, auto& defaults, auto& edited_copy, const auto label) {
			::setup_project_defaults(defaults, project.get_game_modes(), official_map);
			const bool already_default = defaults == edited_copy;

			if (reset_button(already_default, false)) {
				cmd.after = defaults;
				cmd.built_description = typesafe_sprintf("Reset %x settings to default", label);

				post_new_or_rewrite(std::move(cmd));

				return true;
			}

			return false;
		};

		if (project_current_tab == inspected_project_tab_type::ARENA) {
			edit_project_settings_command cmd;
			cmd.tab = inspected_project_tab_type::ARENA;

			auto edited_copy = project.settings;

			editor_arena_settings defaults;
			const bool posted = do_reset_button_for_project_settings(cmd, defaults, edited_copy, "Arena");
			const auto changed = perform_editable_gui_single(edited_copy, std::as_const(defaults), id_handler);

			if (!changed.empty() && !posted) {
				cmd.after = edited_copy;
				cmd.built_description = typesafe_sprintf(changed, "Arena");
				post_new_or_rewrite(std::move(cmd));
			}
		}
		else if (project_current_tab == inspected_project_tab_type::ABOUT) {
			edit_project_settings_command cmd;
			cmd.tab = inspected_project_tab_type::ABOUT;

			auto edited_copy = project.about;
			const auto changed = perform_editable_gui_single(edited_copy);

			if (!changed.empty()) {
				cmd.after = edited_copy;
				cmd.built_description = typesafe_sprintf(changed, "About");
				post_new_or_rewrite(std::move(cmd));
			}

#if DEBUG_DESYNCS
			if (ImGui::Button("Dump binary file")) {
				in.setup.rebuild_arena(false);
				in.setup.get_scene().save_as_bytes(intercosm_paths("/tmp", "dump"));
			}
#endif

			ImGui::Separator();

			text_color("Miniatures & screenshots", yellow);

			ImGui::Separator();

			if (!in.setup.is_generating_miniature()) {
				constexpr int standard_miniature_size = 400;

				text_disabled("Generate a small 400x400 miniature.\nFor previewing arena in catalogues.");
				if (ImGui::Button("Generate miniature")) {
					in.setup.request_arena_screenshot(in.setup.get_paths().miniature, standard_miniature_size, false);
				}

				if (ImGui::IsItemHovered()) {
					text_tooltip(std::string("Generate a small 400x400 miniature.\n\nWill be saved to ") + in.setup.get_paths().miniature.string());
				}

				text_disabled("Generate a full arena screenshot.\nMight be useful for content creators.");

				if (ImGui::Button("Full arena screenshot")) {
					in.setup.request_arena_screenshot(in.setup.get_paths().screenshot, max_full_screenshot_size, true);

					text_tooltip(typesafe_sprintf("Generate a full screenshot of at most %xx%x size.\n\nWill be saved to %x", max_full_screenshot_size, max_full_screenshot_size, in.setup.get_paths().miniature.string()));
				}

				auto scope = scoped_indent();
				ImGui::SliderInt("Max screenshot size", &max_full_screenshot_size, 100, 10000);
				text_disabled("Screenshots are taken with the current zoom,\nthen scaled down if required.");
			}

		}
		else if (project_current_tab == inspected_project_tab_type::PLAYTESTING) {
			edit_project_settings_command cmd;
			cmd.tab = inspected_project_tab_type::PLAYTESTING;

			auto edited_copy = project.playtesting;

			editor_playtesting_settings defaults;
			const bool posted = do_reset_button_for_project_settings(cmd, defaults, edited_copy, "Playtesting");
			const auto changed = perform_editable_gui_single(edited_copy, defaults, id_handler);

			if (!changed.empty() && !posted) {
				cmd.after = edited_copy;
				cmd.built_description = typesafe_sprintf(changed, "Playtesting");
				post_new_or_rewrite(std::move(cmd));
			}
		}
	};

	auto edit_properties = [&]<typename T>(const T& inspected_id) {
		if constexpr(std::is_same_v<T, editor_node_id>) {
			const auto insp_count = all_inspected.size();
			const auto node_label = insp_count == 1 ? std::string("Node") : typesafe_sprintf("Nodes (%x)", insp_count);

			const auto resource_label = [this]() -> std::string {
				if (node_current_tab == inspected_node_tab_type::NODE) {
					return "Resource";
				}

				const auto cnt = num_last_resources_last_time;
				return cnt == 1 ? std::string("Resource") : typesafe_sprintf("Resources (%x)", cnt);
			}();

			simple_two_tabs(
				node_current_tab,
				inspected_node_tab_type::NODE,
				inspected_node_tab_type::RESOURCE,
				node_label,
				resource_label,
				[](){}
			);

			ImGui::Separator();

			if (node_current_tab == inspected_node_tab_type::NODE) {
				in.setup.on_node(inspected_id, node_handler);
			}
			else {
				in.setup.on_node(inspected_id, [&](const auto& typed_node, const auto id) {
					(void)id;

					if (const auto resource = in.setup.find_resource(typed_node.resource_id)) {
						resource_handler(*resource, typed_node.resource_id, in.setup.get_all_inspected());
					}
				});
			}
		}
		else if constexpr(std::is_same_v<T, editor_resource_id>) {
			in.setup.on_resource(inspected_id, resource_handler);
		}
		else if constexpr(std::is_same_v<T, editor_layer_id>) {
			if (auto layer = in.setup.find_layer(inspected_id)) {
				layer_handler(*layer, inspected_id);
			}

		}
		else if constexpr(std::is_same_v<T, inspected_special>) {

			switch (inspected_id) {
				case inspected_special::PROJECT_SETTINGS:
					project_settings_handler();
					break;
				default:
					break;
			}
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive handler");
		}

		(void)inspected_id;
	};

	if (all_inspected.size() == 0) {

	}
	else if (all_inspected.size() == 1) {
		std::visit(edit_properties, all_inspected[0]);
	}
	else {
		auto can_multi_edit = [&]<typename T>(const T& typed_inspected) -> bool { 
			if constexpr(std::is_same_v<T, editor_layer_id>) {
				return inspects_only<T>();
			}
			else if constexpr(std::is_same_v<T, editor_node_id>) {
				const auto same_type = in.setup.on_node(
					typed_inspected,
					[&]<typename E>(const E&, editor_typed_node_id<E> typed) {
						for (auto& e : all_inspected) {
							if (std::get<editor_node_id>(e).type_id != typed.get_type_id()) {
								return false;
							}
						}

						return true;
					}
				);

				return same_type.has_value() && same_type.value();
			}
			else if constexpr(std::is_same_v<T, editor_resource_id>) {
				const auto same_type = in.setup.on_resource(
					typed_inspected,
					[&]<typename E>(const E&, editor_typed_resource_id<E> typed) {
						for (auto& e : all_inspected) {
							if (std::get<editor_resource_id>(e).type_id != typed.get_type_id()) {
								return false;
							}
						}

						return true;
					}
				);

				return same_type.has_value() && same_type.value();
			}
			else if constexpr(std::is_same_v<T, inspected_special>) {
				return false;
			}
			else {
				static_assert(always_false_v<T>, "Non-exhaustive");
			}
		};

		if (!found_in(all_inspected, last_inspected_any)) {
			last_inspected_any = all_inspected.front();
		}

		if (std::visit(can_multi_edit, last_inspected_any)) {
			std::visit(edit_properties, last_inspected_any);
		}
		else {
			text_color(typesafe_sprintf("Selected %x %x:", all_inspected.size(), std::visit(get_object_category_name, all_inspected[0])), yellow);

			ImGui::Separator();

			for (const auto& a : all_inspected) {
				const auto name = in.setup.get_name(a);

				if (name.size() > 0) {
					text(name);
				}
			}
		}
	}
}
