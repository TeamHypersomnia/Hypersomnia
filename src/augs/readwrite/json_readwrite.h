#pragma once
#define RAPIDJSON_HAS_STDSTRING 1

#include "augs/filesystem/path.h"
#include "augs/filesystem/file.h"
#include "augs/readwrite/json_readwrite_errors.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "augs/graphics/rgba.h"
#include "augs/templates/introspect.h"

#include "augs/readwrite/custom_json_representations.h"
#include "augs/readwrite/json_traits.h"
#include "augs/pad_bytes.h"

namespace augs {
	template <class F, class T>
	void general_read_json_value(const F& from, T& out) {
		if constexpr(has_custom_to_json_value_v<T>) {
			from_json_value(from, out);
		}
		else if constexpr(is_constant_size_string_v<T>) {
			if (from.IsString()) {
				out = from.GetString();
			}
		}
		else if constexpr(std::is_same_v<T, std::string> || std::is_arithmetic_v<T>) {
			if (from.template Is<T>()) {
				out = from.template Get<T>();
			}
		}
		else if constexpr(std::is_enum_v<T>) {
			static_assert(has_enum_to_string_v<T>, "This enum does not provide a an enum_to_string overload.");

			if (from.IsString()) {
				const auto stringized_enum = from.GetString();

				if (
					const auto* enumized = mapped_or_nullptr(
						get_string_to_enum_map<T>(), 
						stringized_enum
					)
				) {
					out = *enumized;
				}
				else {
					LOG(
						"Failed to read \"%x\" out %x enum. Check if such option exists, or if spelling is correct.",
						stringized_enum,
						get_type_name<T>()
					);
				}
			}
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive general_read_json_value");
		}
	}

	template <class F, class T>
	void read_json(const F& from, T& out) {
		if constexpr(representable_as_json_value_v<T>) {
			general_read_json_value(from, out);
		}
		else if constexpr(is_container_v<T>) {
			using Container = T;

			if constexpr(can_clear_v<Container>) {
				out.clear();
			}
			else {
				out = {};
			}

			if constexpr(is_associative_v<Container>) {
				static_assert(key_representable_as_string_v<Container>);

				if (from.IsObject()) {
					for (auto& it : from.GetObject()) {
						typename Container::key_type key;
						typename Container::mapped_type mapped;

						key = it.name.GetString();
						read_json(it.value, mapped);

						out.emplace(std::move(key), std::move(mapped));
					}
				}

			}
			else {
				if (from.IsArray()) {
					for (auto& it : from.GetArray()) {
						typename Container::value_type val;

						read_json(it, val);

						if constexpr(can_emplace_back_v<Container>) {
							out.emplace_back(std::move(val));
						}
						else {
							out.emplace(std::move(val));
						}
					}
				}
			}
		}
		else {
			if (from.IsObject()) {
				introspect(
					[&from](const auto& label, auto& field) {
						using Field = remove_cref<decltype(field)>;

						if constexpr(!is_padding_field_v<Field>) {
							if (from.HasMember(label)) {
								read_json(from[label], field);
							}
						}
					},
					out
				);
			}
		}
	}

	template <class T, class F>
	T from_json(const F& from) {
		T out;
		read_json(from, out);
		return out;
	}

	template <class T, class F>
	T from_json_subobject(const F& from, const std::string& subobject_name) {
		T out;

		if (from.IsObject() && from.HasMember(subobject_name.c_str()) && from[subobject_name.c_str()].IsObject()) {
			read_json(from[subobject_name], out);
		}

		return out;
	}

	inline rapidjson::Document json_document_from(const std::string& json) {
		rapidjson::Document document;

		if (document.Parse(json.c_str()).HasParseError()) {
			throw json_deserialization_error(
				"Couldn't parse JSON: %x\nOffset: %x", 
				GetParseError_En(document.GetParseError()),
				document.GetErrorOffset()
			);
		}

		return document;
	}

	inline rapidjson::Document json_document_from(const augs::path_type& path) {
		return json_document_from(file_to_string(path));
	}

	template <class T>
	void from_json_string(const std::string& json, T& out) {
		read_json(json_document_from(json), out);
	}

	template <class T>
	T from_json_string(const std::string& json) {
		T out;
		from_json_string(json, out);
		return out;
	}

	template <class T>
	T from_json_file(const path_type& path) {
		return from_json_string<T>(file_to_string(path));
	}
}

