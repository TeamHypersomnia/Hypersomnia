#pragma once
#define RAPIDJSON_HAS_STDSTRING 1

#include "augs/string/string_to_enum.h"
#include "augs/log.h"

#include "augs/filesystem/path.h"
#include "augs/filesystem/file.h"
#include "augs/readwrite/json_readwrite_errors.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"

#include "augs/graphics/rgba.h"
#include "augs/templates/introspect.h"

#include "augs/readwrite/custom_json_representations.h"
#include "augs/readwrite/json_traits.h"
#include "augs/pad_bytes.h"
#include "augs/templates/traits/is_maybe.h"
#include "augs/string/to_forward_slashes.h"

namespace augs {
	template <class T, class F>
	std::optional<T> json_find(F& from, const std::string& label) {
		if (from.HasMember(label) && from[label].template Is<T>()) {
			return from[label].template Get<T>();
		}

		return std::nullopt;
	}

	template <class F, class T>
	void general_read_json_value(const F& from, T& out) {
		if constexpr(has_custom_to_json_value_v<T>) {
			from_json_value(from, out);
		}
		else if constexpr(is_constant_size_string_v<T> || std::is_same_v<T, std::string> || std::is_same_v<T, augs::path_type>) {
			if (from.IsString()) {
				out = from.GetString();

				if constexpr(std::is_same_v<T, path_type>) {
					out.make_preferred();
				}
			}
		}
		else if constexpr(is_constant_size_string_v<T> || std::is_same_v<T, std::string>) {
			if (from.IsString()) {
				out = from.GetString();
			}
		}
		else if constexpr(std::is_same_v<T, bool>) {
			if (from.IsBool()) {
				out = from.GetBool();
			}
			else if (from.IsUint()) {
				out = static_cast<bool>(from.GetUint());
			}
		}
		else if constexpr(std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, unsigned char>) {
			if (from.IsUint()) {
				out = static_cast<T>(from.GetUint());
			}
			else if (from.IsUint64()) {
				out = static_cast<T>(from.GetUint64());
			}
		}
		else if constexpr(std::is_arithmetic_v<T>) {
			if (from.template Is<T>()) {
				out = from.template Get<T>();
			}
			else {
				/* 
					Allow e.g. int -> float conversion for writers that round full numbers to int 
					(will be useful for manually crafted jsons too).

					This will also convert floats to ints if decimals are mistakenly added to what is meant to be an integer.
				*/

				if (from.IsNumber()) {
					out = static_cast<T>(from.GetDouble());
				}
			}
		}
		else if constexpr(std::is_enum_v<T>) {
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

	template <class T, class F>
	T general_read_json_value(const F& from) {
		T out;
		general_read_json_value(from, out);
		return out;
	}

	template <class F, class T>
	void read_json(const F& from, T& out) {
		if constexpr(representable_as_json_value_v<T>) {
			general_read_json_value(from, out);
		}
		else if constexpr(is_variant_v<T>) {
			if (from.IsObject() && from.HasMember("type") && from["type"].IsString()) {
				std::string variant_type = from["type"].GetString();

				if (variant_type == "std_monostate") {
					return;
				}

				if (from.HasMember("fields")) {
					for_each_type_in_list<T>(
						[variant_type, &from, &out](const auto& resolved){
							if constexpr(!is_monostate_v<decltype(resolved)>) {
								const auto this_type_name = get_type_name_strip_namespace(resolved);

								if (this_type_name == variant_type) {
									using O = remove_cref<decltype(resolved)>;
									O object{};
									read_json(from["fields"], object);
									out.template emplace<O>(std::move(object));
								}
							}
						}
					);
				}
			}
		}
		else if constexpr(is_enum_array_v<T>) {
			if (from.IsObject()) {
				augs::for_each_enum_except_bounds([&](typename T::enum_type e) {
					const auto key = augs::enum_to_string(e);

					if (from.HasMember(key)) {
						read_json(from[key], out[e]);
					}
				});
			}
			else {
				throw json_deserialization_error(
					"Not an enum array! %x", 
					get_type_name_strip_namespace<T>()
				);
			}
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
					const auto max_size = out.max_size();

					for (auto& it : from.GetObject()) {
						if (out.size() >= max_size) {
							throw json_deserialization_error(
								"Too many elements in a container!%x\nMax size: %x", 
								max_size
							);
						}

						typename Container::key_type key;
						typename Container::mapped_type mapped;

						bool read = true;

						if constexpr(std::is_enum_v<typename Container::key_type>) {
							if (
								const auto* enumized = mapped_or_nullptr(
									get_string_to_enum_map<typename Container::key_type>(), 
									it.name.GetString()
								)
							) {
								key = *enumized;
							}
							else {
								read = false;
							}
						}
						else if constexpr(std::is_arithmetic_v<typename Container::key_type>) {
							auto is = std::istringstream(it.name.GetString());
							is >> key;
						}
						else {
							key = it.name.GetString();
						}

						if (read) {
							read_json(it.value, mapped);

							out.emplace(std::move(key), std::move(mapped));
						}
					}
				}
			}
			else {
				if (from.IsArray()) {
					const auto max_size = out.max_size();
					const auto& from_array = from.GetArray();

					if constexpr(!is_std_array_v<Container>) {
						ensure(out.size() == 0);
					}

					if (from_array.Size() > max_size) {
						throw json_deserialization_error(
							"Too many elements in a container (%x)!\nElements passed: %x\nMax size: %x", 
							get_type_name_strip_namespace<Container>(),
							from_array.Size(),
							max_size
						);
					}

					std::size_t idx = 0;

					for (auto& it : from_array) {
						typename Container::value_type val;

						read_json(it, val);

						if constexpr(can_emplace_back_v<Container>) {
							out.emplace_back(std::move(val));
						}
						else if constexpr(is_std_array_v<Container>) {
							if (idx < out.size()) {
								out[idx++] = std::move(val);
							}
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

						if constexpr(is_optional_v<Field>) {
							if (from.HasMember(label)) {
								field.reset();
								field.emplace();

								read_json(from[label], *field);
							}
						}
						else if constexpr(!is_padding_field_v<Field> && !json_ignore_v<Field>) {
							if constexpr(is_maybe_v<Field>) {
								if (from.HasMember(label)) {
									read_json(from[label], field.value);
									field.is_enabled = true;
								}
								else {
									const auto OFF_label = std::string("OFF_") + label;

									if (from.HasMember(OFF_label)) {
										read_json(from[OFF_label], field.value);
										field.is_enabled = false;
									}
								}
							}
							else {
								if constexpr(json_serialize_in_parent_v<Field>) {
									read_json(from, field);
								}
								else {
									if (from.HasMember(label)) {
										read_json(from[label], field);
									}
								}
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

	template <unsigned parse_flags>
	inline rapidjson::Document json_document_from(const std::string& json) {
		rapidjson::Document document;

		if (document.Parse<parse_flags>(json.c_str()).HasParseError()) {
			throw json_deserialization_error(
				"Couldn't parse JSON: %x\nOffset: %x", 
				GetParseError_En(document.GetParseError()),
				document.GetErrorOffset()
			);
		}

		return document;
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

	inline rapidjson::Document json_document_from(const char* const json, const std::size_t length) {
		rapidjson::Document document;

		if (document.Parse(json, length).HasParseError()) {
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

	template <class T>
	void from_json_file(const path_type& path, T& into) {
		from_json_string(file_to_string(path), into);
	}

	template <class F, class T>
	void general_write_json_value(F& to, const T& from) {
		if constexpr(has_custom_to_json_value_v<T>) {
			to_json_value(to, from);
		}
		else if constexpr(is_constant_size_string_v<T>) {
			to.String(std::string(from));
		}
		else if constexpr(std::is_same_v<T, std::string>) {
			to.String(from);
		}
		else if constexpr(std::is_same_v<T, bool>) {
			to.Bool(from);
		}
		else if constexpr(std::is_same_v<T, path_type>) {
			to.String(::to_forward_slashes(from.string()));
		}
		else if constexpr(std::is_same_v<T, float>) {
			to.Double(from);
		}
		else if constexpr(std::is_same_v<T, double>) {
			to.Double(from);
		}
		else if constexpr(std::is_same_v<T, int> || std::is_same_v<T, short>) {
			to.Int(from);
		}
		else if constexpr(std::is_same_v<T, uint64_t>) {
			to.Uint64(from);
		}
		else if constexpr(std::is_unsigned_v<T>) {
			to.Uint(from);
		}
		else if constexpr(std::is_enum_v<T>) {
			to.String(enum_to_string(from));
		}
		else {
			static_assert(always_false_v<T>, "Non-exhaustive general_write_json_value");
		}
	}

	template <class T>
	decltype(auto) json_stringize(const T& val) {
		if constexpr(std::is_enum_v<T>) {
			return enum_to_string(val);
		}
		else if constexpr(std::is_arithmetic_v<T>) {
			return std::to_string(val);
		}
		else {
			return val;
		}
	}

	template <class F, class T>
	void write_json(F& to, const T& from) {
		if constexpr(representable_as_json_value_v<T>) {
			general_write_json_value(to, from);
		}
		else if constexpr(is_variant_v<T>) {
			to.StartObject();

			std::visit(
				[&](const auto& resolved) mutable {
					const auto variant_type_label = "type";
					const auto variant_fields_label = "fields";

					if constexpr(is_monostate_v<decltype(resolved)>) {
						to.Key(variant_type_label);
						to.String("std_monostate");
					}
					else {
						const auto this_type_name = get_type_name_strip_namespace(resolved);

						to.Key(variant_type_label);
						to.String(this_type_name);

						to.Key(variant_fields_label);
						write_json(to, resolved);
					}
				}, 
				from
			);

			to.EndObject();
		}
		else if constexpr(is_enum_array_v<T>) {
			to.StartObject();

			augs::for_each_enum_except_bounds([&](typename T::enum_type e) {
				to.Key(json_stringize(e));
				write_json(to, from[e]);
			});

			to.EndObject();
		}
		else if constexpr(is_container_v<T>) {
			using Container = T;

			if constexpr(is_associative_v<Container>) {
				static_assert(key_representable_as_string_v<Container>);

				to.StartObject();

				for (const auto& it : from) {
					to.Key(json_stringize(it.first));
					write_json(to, it.second);
				}

				to.EndObject();
			}
			else {
				to.StartArray();

				for (const auto& it : from) {
					write_json(to, it);
				}

				to.EndArray();
			}
		}
		else {
			to.StartObject();

			introspect(
				[&to](const auto& label, const auto& field) {
					using Field = remove_cref<decltype(field)>;
					if constexpr(!is_padding_field_v<Field> && !json_ignore_v<Field>) {
						if constexpr(is_optional_v<Field>) {
							if (field.has_value()) {
								to.Key(label);
								write_json(to, *field);
							}
						}
						else if constexpr(is_maybe_v<Field>) {
							if (field.is_enabled) {
								to.Key(label);
								write_json(to, field.value);
							}
							else {
								if constexpr(Field::serialize_disabled) {
									to.Key(std::string("OFF_") + label);
									write_json(to, field.value);
								}
							}
						}
						else {
							to.Key(label);
							write_json(to, field);
						}
					}
				},
				from
			);

			to.EndObject();
		}
	}

	template <class T>
	std::string to_json_string_nopretty(const T& from) {
		rapidjson::StringBuffer s;
		rapidjson::Writer<rapidjson::StringBuffer> writer(s);

		write_json(writer, from);

		return s.GetString();
	}

	template <class T>
	std::string to_json_string(const T& from) {
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

		write_json(writer, from);

		return s.GetString();
	}

	template <class T>
	void save_as_json(const T& from, const path_type& path) {
		return save_as_text(path, to_json_string(from));
	}

	template <class F, class T>
	void write_json_diff(F& to, const T& from, const T& reference_object, const bool write_object_delimiters = false) {
		if constexpr(representable_as_json_value_v<T>) {
			general_write_json_value(to, from);
		}
		else if constexpr(is_variant_v<T>) {
			static_assert(always_false_v<T>, "Not implemented");
		}
		else if constexpr(is_enum_array_v<T>) {
			if (write_object_delimiters) {
				to.StartObject();
			}

			augs::for_each_enum_except_bounds([&](typename T::enum_type e) {
				const auto defaults = reference_object[e];
				const auto& it = from[e];

				if (!(defaults == it)) {
					to.Key(json_stringize(e));
					write_json_diff(to, it, defaults, true);
				}
			});

			if (write_object_delimiters) {
				to.EndObject();
			}
		}
		else if constexpr(is_container_v<T>) {
			using Container = T;

			if constexpr(is_associative_v<Container>) {
				static_assert(key_representable_as_string_v<Container>);

				if (write_object_delimiters) {
					to.StartObject();
				}

				const auto defaults = typename Container::mapped_type();

				for (const auto& it : from) {
					to.Key(json_stringize(it.first));
					write_json_diff(to, it.second, defaults, true);
				}

				if (write_object_delimiters) {
					to.EndObject();
				}
			}
			else {
				if (write_object_delimiters) {
					to.StartArray();
				}

				const auto defaults = typename Container::value_type();

				for (const auto& it : from) {
					write_json_diff(to, it, defaults, true);
				}

				if (write_object_delimiters) {
					to.EndArray();
				}
			}
		}
		else {
			if (write_object_delimiters) {
				to.StartObject();
			}

			introspect(
				[&to](const auto& label, const auto& field, const auto& reference_field) {
					using Field = remove_cref<decltype(field)>;
					if constexpr(!is_padding_field_v<Field> && !json_ignore_v<Field>) {
						if constexpr(is_optional_v<Field>) {
							if (field == reference_field) {
								return;
							}

							if (field) {
								auto ref = reference_field.has_value() ? *reference_field : typename Field::value_type();

								to.Key(label);
								write_json_diff(to, *field, ref, true);
							}
						}
						else if constexpr(is_maybe_v<Field>) {
							if (field == reference_field) {
								return;
							}

							if (field.is_enabled) {
								to.Key(label);
								write_json_diff(to, field.value, reference_field.value, true);
							}
							else {
								if constexpr(Field::serialize_disabled) {
									to.Key(std::string("OFF_") + label);
									write_json_diff(to, field.value, reference_field.value, true);
								}
							}
						}
						else {
							if constexpr(json_serialize_in_parent_v<Field>) {
								write_json_diff(to, field, reference_field, false);
							}
							else {
								if (field == reference_field) {
									return;
								}

								to.Key(label);
								write_json_diff(to, field, reference_field, true);
							}
						}
					}
				},
				from,
				reference_object
			);

			if (write_object_delimiters) {
				to.EndObject();
			}
		}
	}

	template <class T>
	std::string to_json_diff_string(const T& from, const T& reference_object) {
		rapidjson::StringBuffer s;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

		write_json_diff(writer, from, reference_object, true);

		return s.GetString();
	}

	template <class T>
	void save_as_json_diff(
		const path_type& path,
		const T& from,
		const T& reference_object,
		std::string preamble = ""
	) {
		save_as_text(path, preamble + to_json_diff_string(from, reference_object));
	}
}

