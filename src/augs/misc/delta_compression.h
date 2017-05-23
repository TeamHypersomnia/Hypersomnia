#pragma once
#include <vector>
#include "augs/templates/memcpy_safety.h"
#include "augs/templates/get_index_type_for_size_of.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

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

	typedef char delta_unit;

	template <class T, class = void>
	class object_delta;

	template <class T>
	class object_delta<T, std::enable_if_t<is_memcpy_safe_v<T>>> {
		using offset_type = get_index_type_for_size_of_t<T>;
		static constexpr std::size_t length_bytes = sizeof T;
		static_assert(0 == length_bytes % sizeof delta_unit, "Type size must be divisble by the size of delta unit");

		std::vector<delta_unit> changed_bytes;
		std::vector<offset_type> changed_offsets;
	
	public:
		bool write(
			stream& out,
			const bool write_changed_bit = false
		) const {
			const bool has_changed = changed_bytes.size() > 0;

			if (write_changed_bit) {
				augs::write(out, has_changed);
			}

			if (has_changed) {
				augs::write(out, changed_bytes, offset_type());
				augs::write(out, changed_offsets, offset_type());
			}

			return has_changed;
		}

		void read(
			stream& in,
			const bool read_changed_bit = false
		) {
			bool has_changed = true;

			if (read_changed_bit) {
				augs::read(in, has_changed);
			}

			if (has_changed) {
				augs::read(in, changed_bytes, offset_type());
				augs::read(in, changed_offsets, offset_type());
			}
		}

		static auto encode(
			const T& base_object, 
			const T& encoded_object
		) {
			object_delta result;

			const delta_unit* const base_object_ptr = reinterpret_cast<const delta_unit*>(std::addressof(base_object));
			const delta_unit* const encoded_object_ptr = reinterpret_cast<const delta_unit*>(std::addressof(encoded_object));

			thread_local std::vector<char> diff_flags;
			diff_flags.resize(length);

			auto* const diff_flags_ptr = diff_flags.data();

			for (std::size_t i = 0; i < length; ++i) {
				if (base_object_ptr[i] == encoded_object_ptr[i]) {
					diff_flags_ptr[i] = 0;
				}
				else {
					diff_flags_ptr[i] = 1;
					result.changed_bytes.push_back(encoded_object_ptr[i]);
				}
			}

			result.changed_offsets = run_length_encoding<offset_type>(diff_flags);

			return result;
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
	class object_delta<T, std::enable_if_t<!is_memcpy_safe_v<T>>> {
		augs::stream new_content;
	public:
		bool write(
			stream& out,
			const bool write_changed_bit = false
		) const {
			const bool has_changed = new_content.size() > 0;

			if (write_changed_bit) {
				augs::write(out, has_changed);
			}

			if (has_changed) {
				augs::write(out, new_content);
			}

			return has_changed;
		}

		void read(
			stream& in,
			const bool read_changed_bit = false
		) {
			bool has_changed = true;

			if (read_changed_bit) {
				augs::read(in, has_changed);
			}

			if (has_changed) {
				augs::read(in, new_content);
			}
		}

		static auto encode(
			const T& base_object,
			const T& encoded_object
		) {
			augs::stream base_content;

			augs::write(base_content, base_object);
			augs::write(new_content, encoded_object);

			if (base_content == new_content) {
				new_content.set_write_pos(0u);
			}
		}

		void decode_into(T& decoded) const {
			if (new_content.size() > 0) {
				augs::read(new_content, decoded);
			}
		}
	};

	/* shortcuts */

	template <class T>
	bool write_delta(
		const T& base,
		const T& enco,
		stream& out,
		const bool write_changed_bit = false
	) {
		const auto dt = object_delta<T>::encode(base, enco);
		return dt.write(out, write_changed_bit);
	}

	template <class T>
	void read_delta(
		T& decode_target,
		stream& in,
		const bool read_changed_bit = false
	) {
		object_delta<T> dt;
		dt.read(in, read_changed_bit);
		dt.decode_into(decode_target);
	}
}