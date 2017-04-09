#pragma once
#include <vector>
#include "augs/templates/memcpy_safety.h"
#include "augs/templates/get_index_type_for_size_of.h"

#include "augs/ensure.h"
#include "augs/misc/streams.h"

namespace augs {
	typedef unsigned short delta_offset_type;

	template <class delta_offset_type>
	std::vector<delta_offset_type> run_length_encoding(const std::vector<char>& bit_data) {
		ensure(bit_data.size() < std::numeric_limits<delta_offset_type>::max());

		std::vector<delta_offset_type> output;

		char previous_value = 0;
		delta_offset_type current_vec_pos = 0;

		for (delta_offset_type i = 0; i < bit_data.size(); ++i) {
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
					const delta_offset_type next_offset = i - current_vec_pos;
					
					ensure(output.size() > 0);

					output.push_back(next_offset);
					current_vec_pos += next_offset;
				}
				previous_value = bit_data[i];
			}
		}

		if (bit_data.back()) {
			output.push_back(static_cast<delta_offset_type>(bit_data.size()) - current_vec_pos);
		}

		return output;
	}

	typedef char delta_unit;

	template <class delta_offset_type>
	struct object_delta {
		std::vector<delta_unit> changed_bytes;
		std::vector<delta_offset_type> changed_offsets;
	};

	template <class T>
	auto delta_encode(
		const T& base_object, 
		const T& encoded_object
	) {
		static_assert(is_memcpy_safe_v<T>, "Attempt to encode a type that is not trivially copyable");

		static constexpr auto length_bytes = sizeof(T);
		static_assert(0 == length_bytes % sizeof(delta_unit), "Type size must be divisble by the size of delta unit");

		constexpr auto length = length_bytes / sizeof(delta_unit);

		typedef get_index_type_for_size_of_t<T> delta_offset_type;
		object_delta<delta_offset_type> result;

		const delta_unit* const base_object_ptr = reinterpret_cast<const delta_unit*>(std::addressof(base_object));
		const delta_unit* const encoded_object_ptr = reinterpret_cast<const delta_unit*>(std::addressof(encoded_object));

		thread_local std::vector<char> diff_flags;
		diff_flags.resize(length);

		auto* const diff_flags_ptr = diff_flags.data();

		for (size_t i = 0; i < length; ++i) {
			if (base_object_ptr[i] == encoded_object_ptr[i]) {
				diff_flags_ptr[i] = 0;
			}
			else {
				diff_flags_ptr[i] = 1;
				result.changed_bytes.push_back(encoded_object_ptr[i]);
			}
		}

		result.changed_offsets = run_length_encoding<delta_offset_type>(diff_flags);

		return result;
	};

	template <class T>
	void delta_decode(
		T& decoded, 
		const object_delta<get_index_type_for_size_of_t<T>>& delta
	) {
		static_assert(is_memcpy_safe_v<T>, "Attempt to decode a type that is not trivially copyable");

		static constexpr auto length_bytes = sizeof(T);
		static_assert(0 == length_bytes % sizeof(delta_unit), "Type size must be divisble by the size of delta unit");

		delta_unit* ptr = reinterpret_cast<delta_unit*>(std::addressof(decoded));

		constexpr auto length = length_bytes / sizeof(delta_unit);

		const delta_unit * const original_location = ptr;

		const auto& changed_offsets = delta.changed_offsets;
		const auto& changed_bytes = delta.changed_bytes;

		size_t previous_vector_pos = 0;

		for (size_t i = 0; i < changed_offsets.size(); i += 2) {
			ptr += changed_offsets[i];
			std::copy(changed_bytes.begin() + previous_vector_pos, changed_bytes.begin() + previous_vector_pos + changed_offsets[i + 1], ptr);
			previous_vector_pos += changed_offsets[i + 1];
			ptr += changed_offsets[i + 1];
		}

		ensure(ptr <= original_location + length);
	};

	template <class T>
	bool write_delta(
		const T& base,
		const T& enco,
		stream& out,
		const bool write_changed_bit = false
	) {
		const auto dt = augs::delta_encode(base, enco);
		const bool has_changed = dt.changed_bytes.size() > 0;

		if (write_changed_bit) {
			augs::write(out, has_changed);
		}

		if (has_changed) {
			augs::write(out, dt.changed_bytes, unsigned short());
			augs::write(out, dt.changed_offsets, unsigned short());
		}

		return has_changed;
	}

	template <class T>
	void read_delta(
		T& deco,
		stream& in,
		const bool read_changed_bit = false
	) {
		decltype(delta_encode(deco, deco)) dt;

		bool has_changed = true;

		if (read_changed_bit) {
			augs::read(in, has_changed);
		}

		if (has_changed) {
			augs::read(in, dt.changed_bytes, unsigned short());
			augs::read(in, dt.changed_offsets, unsigned short());

			augs::delta_decode(deco, dt);
		}
	}
}