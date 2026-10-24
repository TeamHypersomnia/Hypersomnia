#pragma once
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include "augs/readwrite/json_readwrite.h"

/*
	What read_json passes over without a word: a key the type doesn't have, a value of another type
	("port": "8412" - text where a number goes), an enum value that doesn't exist. Read the same way as read_json,
	so whatever it would take is never called a problem here.

	Each problem: "<path>: <what>", e.g. "server.server_nmae: no such setting".

	known: the same place in a JSON that documents every key (default_config.json) - a key it has isn't unknown,
	even if this build has no field for it (another platform's setting, one read by hand like "instance_vars").
*/

namespace augs {
	namespace detail {
		template <class T>
		void collect_json_labels(std::vector<std::string>& labels) {
			const auto object = std::make_unique<T>();

			introspect(
				[&labels](const auto& label, const auto& field) {
					using Field = remove_cref<decltype(field)>;

					if constexpr(is_padding_field_v<Field> || json_ignore_v<Field>) {
						return;
					}
					else if constexpr(is_maybe_v<Field>) {
						labels.push_back(label);
						labels.push_back(std::string("OFF_") + label);
					}
					else if constexpr(!is_optional_v<Field> && json_serialize_in_parent_v<Field>) {
						collect_json_labels<Field>(labels);
					}
					else {
						labels.push_back(label);
					}
				},
				*object
			);
		}

		inline std::string json_child_path(const std::string& path, const std::string& key) {
			return path.empty() ? key : path + "." + key;
		}

		inline std::string json_value_preview(const rapidjson::Value& from) {
			if (from.IsString()) {
				return std::string("\"") + from.GetString() + "\"";
			}

			if (from.IsBool()) {
				return from.GetBool() ? "true" : "false";
			}

			if (from.IsNumber()) {
				return typesafe_sprintf("%x", from.GetDouble());
			}

			if (from.IsArray()) {
				return "a list";
			}

			if (from.IsObject()) {
				return "an object";
			}

			return "null";
		}
	}

	/*
		check_unknown_keys = false: a json_serialize_in_parent field - its parent's keys are its neighbours, not unknown.
	*/

	template <class T>
	void find_json_problems(const rapidjson::Value& from, T&, const rapidjson::Value* known, const std::string& path, std::vector<std::string>& out, bool check_unknown_keys = true);

	namespace detail {
		inline const rapidjson::Value* known_child(const rapidjson::Value* known, const std::string& key) {
			if (known != nullptr && known->IsObject() && known->HasMember(key.c_str())) {
				return &(*known)[key.c_str()];
			}

			return nullptr;
		}
	}

	template <class T>
	void find_json_value_problems(const rapidjson::Value& from, const std::string& path, std::vector<std::string>& out) {
		auto expected = [&](const std::string& what) {
			out.push_back(typesafe_sprintf("%x: expected %x, got %x", path, what, detail::json_value_preview(from)));
		};

		if constexpr(has_custom_to_json_value_v<T>) {
			/* Its own representation - whatever from_json_value takes. */
		}
		else if constexpr(is_constant_size_string_v<T> || std::is_same_v<T, std::string> || std::is_same_v<T, augs::path_type>) {
			if (!from.IsString()) {
				expected("text");
			}
		}
		else if constexpr(std::is_same_v<T, bool>) {
			if (!from.IsBool() && !from.IsUint()) {
				expected("true or false");
			}
		}
		else if constexpr(std::is_arithmetic_v<T>) {
			if (!from.IsNumber()) {
				expected("a number");
			}
			else if constexpr(std::is_integral_v<T>) {
				const auto value = from.GetDouble();

				if (value < static_cast<double>(std::numeric_limits<T>::lowest()) || value > static_cast<double>(std::numeric_limits<T>::max())) {
					out.push_back(typesafe_sprintf("%x: %x is out of range", path, detail::json_value_preview(from)));
				}
			}
		}
		else if constexpr(std::is_enum_v<T>) {
			if (!from.IsString() || mapped_or_nullptr(get_string_to_enum_map<T>(), from.GetString()) == nullptr) {
				std::string options;

				for (const auto& entry : get_string_to_enum_map<T>()) {
					options += (options.empty() ? "" : ", ") + entry.first;
				}

				out.push_back(typesafe_sprintf("%x: %x is not one of: %x", path, detail::json_value_preview(from), options));
			}
		}
	}

	template <class T>
	void find_json_problems(const rapidjson::Value& from, T& object, const rapidjson::Value* known, const std::string& path, std::vector<std::string>& out, const bool check_unknown_keys) {
		if constexpr(representable_as_json_value_v<T>) {
			find_json_value_problems<T>(from, path, out);
		}
		else if constexpr(is_variant_v<T>) {
			/* Its "type" picks what "fields" mean - not checked. */
		}
		else if constexpr(is_enum_array_v<T>) {
			if (!from.IsObject()) {
				return;
			}

			for (const auto& it : from.GetObject()) {
				const auto key = std::string(it.name.GetString());

				if (mapped_or_nullptr(get_string_to_enum_map<typename T::enum_type>(), key) == nullptr) {
					out.push_back(typesafe_sprintf("%x: no such setting", detail::json_child_path(path, key)));
				}
				else {
					typename T::value_type element{};
					find_json_problems(it.value, element, detail::known_child(known, key), detail::json_child_path(path, key), out);
				}
			}
		}
		else if constexpr(is_container_v<T>) {
			if constexpr(is_associative_v<T>) {
				if (!from.IsObject()) {
					return;
				}

				for (const auto& it : from.GetObject()) {
					const auto key = std::string(it.name.GetString());

					if constexpr(std::is_enum_v<typename T::key_type>) {
						if (mapped_or_nullptr(get_string_to_enum_map<typename T::key_type>(), key) == nullptr) {
							out.push_back(typesafe_sprintf("%x: no such key", detail::json_child_path(path, key)));
							continue;
						}
					}

					typename T::mapped_type mapped{};
					find_json_problems(it.value, mapped, detail::known_child(known, key), detail::json_child_path(path, key), out);
				}
			}
			else {
				typename T::value_type element{};

				if (from.IsArray()) {
					std::size_t i = 0;

					for (const auto& it : from.GetArray()) {
						find_json_problems(it, element, nullptr, typesafe_sprintf("%x[%x]", path, i++), out);
					}
				}
				else if constexpr(!is_std_array_v<T>) {
					/* A single value for a list of one - as read_json takes it. */
					find_json_problems(from, element, known, path, out);
				}
			}
		}
		else {
			if (!from.IsObject()) {
				out.push_back(typesafe_sprintf("%x: expected an object, got %x", path, detail::json_value_preview(from)));
				return;
			}

			if (check_unknown_keys) {
				std::vector<std::string> labels;
				detail::collect_json_labels<T>(labels);

				for (const auto& it : from.GetObject()) {
					const auto key = std::string(it.name.GetString());

					if (std::find(labels.begin(), labels.end(), key) == labels.end() && detail::known_child(known, key) == nullptr) {
						out.push_back(typesafe_sprintf("%x: no such setting", detail::json_child_path(path, key)));
					}
				}
			}

			introspect(
				[&from, known, &path, &out](const auto& label, auto& field) {
					using Field = remove_cref<decltype(field)>;

					if constexpr(is_padding_field_v<Field> || json_ignore_v<Field>) {
						return;
					}
					else if constexpr(is_optional_v<Field>) {
						if (from.HasMember(label)) {
							typename Field::value_type value{};
							find_json_problems(from[label], value, detail::known_child(known, label), detail::json_child_path(path, label), out);
						}
					}
					else if constexpr(is_maybe_v<Field>) {
						const auto OFF_label = std::string("OFF_") + label;

						if (from.HasMember(label)) {
							find_json_problems(from[label], field.value, detail::known_child(known, label), detail::json_child_path(path, label), out);
						}
						else if (from.HasMember(OFF_label)) {
							find_json_problems(from[OFF_label], field.value, detail::known_child(known, OFF_label), detail::json_child_path(path, OFF_label), out);
						}
					}
					else if constexpr(json_serialize_in_parent_v<Field>) {
						find_json_problems(from, field, known, path, out, false);
					}
					else if (from.HasMember(label)) {
						find_json_problems(from[label], field, detail::known_child(known, label), detail::json_child_path(path, label), out);
					}
				},
				object
			);
		}
	}

	/*
		The problems of a whole JSON text read into T - its parse error if it's no JSON at all.
	*/

	template <class T, unsigned Flags = rapidjson::kParseDefaultFlags>
	std::vector<std::string> find_json_problems(const std::string& json, const std::string& known_json = "") {
		std::vector<std::string> out;

		rapidjson::Document doc;
		doc.Parse<Flags>(json.c_str());

		if (doc.HasParseError()) {
			out.push_back(typesafe_sprintf("not valid JSON: %x (at %x)", rapidjson::GetParseError_En(doc.GetParseError()), doc.GetErrorOffset()));
			return out;
		}

		rapidjson::Document known;
		known.Parse<Flags>(known_json.c_str());

		const auto object = std::make_unique<T>();
		find_json_problems(doc, *object, known.HasParseError() ? nullptr : &known, "", out);
		return out;
	}
}
