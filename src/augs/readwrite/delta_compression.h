#pragma once
#include <vector>
#include "augs/templates/triviality_traits.h"
#include "augs/templates/get_index_type_for_size_of.h"
#include "augs/templates/introspect_declaration.h"

#include "augs/ensure.h"
#include "augs/readwrite/memory_stream.h"

namespace augs {
	template <class offset_type>
	auto run_length_encoding(const std::vector<char>& bit_data) {
		ensure(bit_data.size() < std::numeric_limits<offset_type>::max());

		std::vector<offset_type> output;

		char previous_value = 0;
		offset_type current_vec_pos = 0;

		for (offset_type i = 0; i < bit_data.size(); ++i) {
			if (previous_value != bit_data[i]) {
				if (bit_data[i] != 0) {
					if (!output.size()) {
						output.push_back(i);
						current_vec_pos += i;
					}
					else {
						output.push_back(i - current_vec_pos);
						current_vec_pos += i - current_vec_pos;
					}
				}
				else {
					const offset_type next_offset = i - current_vec_pos;
					
					ensure(output.size() > 0);

					output.push_back(next_offset);
					current_vec_pos += next_offset;
				}
				previous_value = bit_data[i];
			}
		}

		if (bit_data.back()) {
			output.push_back(static_cast<offset_type>(bit_data.size()) - current_vec_pos);
		}

		return output;
	}

	using delta_unit = std::byte;

	template <class T, class = void>
	class object_delta;

	template <class T>
	class object_delta<T, std::enable_if_t<std::is_trivially_copyable_v<T>>> {
		using offset_type = get_index_type_for_size_of_t<T>;
		static constexpr std::size_t length_bytes = sizeof(T);
		static_assert(0 == length_bytes % sizeof(delta_unit), "Type size must be divisble by the size of delta unit");

		std::vector<delta_unit> changed_bytes;
		std::vector<offset_type> changed_offsets;
	
	public:
		offset_type get_first_divergence_pos() const {
			return changed_offsets.at(0);
		}

		const auto& get_changed_offsets() const {
			return changed_offsets;
		}

		const auto& get_changed_bytes() const {
			return changed_bytes;
		}

		bool has_changed() const {
			return changed_bytes.size() > 0;
		}

		bool write(
			memory_stream& out,
			const bool write_changed_bit = false
		) const {
			const bool changed = has_changed();

			if (write_changed_bit) {
				augs::write_bytes(out, changed);
			}

			if (changed) {
				augs::write_container_bytes(out, changed_bytes, offset_type());
				augs::write_container_bytes(out, changed_offsets, offset_type());
			}

			return changed;
		}

		object_delta(
			memory_stream& in,
			const bool read_changed_bit = false
		) {
			bool changed = true;

			if (read_changed_bit) {
				augs::read_bytes(in, changed);
			}

			if (changed) {
				augs::read_container_bytes(in, changed_bytes, offset_type());
				augs::read_container_bytes(in, changed_offsets, offset_type());
			}
		}

		object_delta(
			const T& base_object, 
			const T& encoded_object
		) {
			const delta_unit* const base_object_ptr = reinterpret_cast<const delta_unit*>(std::addressof(base_object));
			const delta_unit* const encoded_object_ptr = reinterpret_cast<const delta_unit*>(std::addressof(encoded_object));

			thread_local std::vector<char> diff_flags;
			diff_flags.resize(length_bytes);

			auto* const diff_flags_ptr = diff_flags.data();

			for (std::size_t i = 0; i < length_bytes; ++i) {
				if (base_object_ptr[i] == encoded_object_ptr[i]) {
					diff_flags_ptr[i] = 0;
				}
				else {
					diff_flags_ptr[i] = 1;
					changed_bytes.push_back(encoded_object_ptr[i]);
				}
			}

			changed_offsets = run_length_encoding<offset_type>(diff_flags);
		}

		void decode_into(T& decoded) const {
			delta_unit* ptr = reinterpret_cast<delta_unit*>(std::addressof(decoded));

			constexpr auto length = length_bytes / sizeof(delta_unit);

			const delta_unit * const original_location = ptr;

			std::size_t previous_vector_pos = 0;

			for (std::size_t i = 0; i < changed_offsets.size(); i += 2) {
				ptr += changed_offsets[i];
				std::copy(changed_bytes.begin() + previous_vector_pos, changed_bytes.begin() + previous_vector_pos + changed_offsets[i + 1], ptr);
				previous_vector_pos += changed_offsets[i + 1];
				ptr += changed_offsets[i + 1];
			}

			ensure(ptr <= original_location + length);
		}
	};


	template <class T>
	class object_delta<T, std::enable_if_t<!std::is_trivially_copyable_v<T>>> {
		std::vector<std::byte> new_content;
	public:
		std::size_t get_first_divergence_pos() const {
			ensure(false && "not implemented");
			return 0xdeadbeef;
		}

		bool has_changed() const {
			return new_content.size() > 0;
		}

		bool write(
			memory_stream& out,
			const bool write_changed_bit = false
		) const {
			const bool changed = has_changed();

			if (write_changed_bit) {
				augs::write_bytes(out, changed);
			}

			if (changed) {
				augs::write_bytes(out, new_content);
			}

			return changed;
		}

		object_delta(
			memory_stream& in,
			const bool read_changed_bit = false
		) {
			bool changed = true;

			if (read_changed_bit) {
				augs::read_bytes(in, changed);
			}

			if (changed) {
				augs::read_bytes(in, new_content);
			}
		}

		object_delta(
			const T& base_object,
			const T& encoded_object
		) {
			if (!introspective_equal(base_object, encoded_object)) {
				new_content = to_bytes(encoded_object);
			}
		}

		void decode_into(T& decoded) {
			if (new_content.size() > 0) {
				from_bytes(std::move(new_content), decoded);
			}
		}
	};

	/* shortcuts */

	template <class T>
	bool write_delta(
		const T& base,
		const T& enco,
		memory_stream& out,
		const bool write_changed_bit = false
	) {
		const auto dt = object_delta<T>(base, enco);
		return dt.write(out, write_changed_bit);
	}

	template <class T>
	void read_delta(
		T& into,
		memory_stream& in,
		const bool read_changed_bit = false
	) {
		auto dt = object_delta<T>(in, read_changed_bit);
		dt.decode_into(into);
	}
}