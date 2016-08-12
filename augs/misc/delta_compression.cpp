#include "delta_compression.h"
#include "augs/ensure.h"

namespace augs {
	std::vector<delta_offset_type> run_length_encoding(const std::vector<bool>& bit_data) {
		std::vector<delta_offset_type> output;

		bool previous_value = false;
		size_t current_vec_pos = 0;

		for (size_t i = 0; i < bit_data.size(); ++i) {
			if (previous_value != bit_data[i]) {
				if (bit_data[i]) {
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
					size_t next_offset = i - current_vec_pos;
					
					ensure(output.size() > 0);
					ensure(next_offset <= std::numeric_limits<delta_offset_type>::max());

					output.push_back(static_cast<delta_offset_type>(next_offset));
					current_vec_pos += next_offset;
				}
				previous_value = bit_data[i];
			}
		}

		if (bit_data.back())
			output.push_back(bit_data.size() - current_vec_pos);

		return std::move(output);
	}


	object_delta delta_encode(const char* base_object, const char* encoded_object, const size_t length) {
		object_delta result;

		std::vector<bool> byte_mask;
		byte_mask.reserve(length);

		for (size_t i = 0; i < length; ++i) {
			if (base_object[i] == encoded_object[i])
				byte_mask.push_back(false);
			else {
				byte_mask.push_back(true);
				result.changed_bytes.push_back(encoded_object[i]);
			}
		}

		result.changed_offsets = run_length_encoding(byte_mask);

		return std::move(result);
	};

	void delta_decode(char* ptr, const size_t length, const object_delta& delta) {
		const char * const original_location = ptr;

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
}