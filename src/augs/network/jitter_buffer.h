#pragma once
#include <vector>

namespace augs {
	template<class command>
	class jitter_buffer {
		size_t lower_limit = 3;
		unsigned steps_extrapolated = 0;

		bool initial_filling = true;
	public:
		std::vector<command> buffer;
		
		void acquire_new_command(const command& c) {
			buffer.push_back(c);

			if (initial_filling) {
				if (buffer.size() >= lower_limit) {
					initial_filling = false;
				}
			}
		}

		void acquire_new_command(command&& c) {
			buffer.emplace_back(c);

			if (initial_filling) {
				if (buffer.size() >= lower_limit) {
					initial_filling = false;
				}
			}
		}
		
		template <class Iter>
		void acquire_new_commands(Iter first, Iter last) {
			buffer.insert(buffer.end(), first, last);

			if (initial_filling) {
				if (buffer.size() >= lower_limit) {
					initial_filling = false;
				}
			}
		}

		std::vector<command> unpack_commands_once() {
			std::vector<command> next_commands;

			if (!initial_filling) {
				const auto steps_to_unpack = steps_extrapolated + 1;

				if (buffer.size() >= steps_to_unpack) {
					next_commands.assign(buffer.begin(), buffer.begin() + steps_to_unpack);
					buffer		  .erase(buffer.begin(), buffer.begin() + steps_to_unpack);
				}

				const bool unpacked_successfully = next_commands.size() > 0;

				if (unpacked_successfully) {
					steps_extrapolated = 0;
				}
				else {
					++steps_extrapolated;
				}
			}

			return next_commands;
		}

		bool unpack_next_command(command& next_command) {
			if (!initial_filling) {
				if (buffer.size() > 0) {
					next_command = std::move(buffer.front());
					buffer.erase(buffer.begin());

					return true;
				}
			}

			return false;
		}

		void allow_to_refill() {
			initial_filling = true;
		}

		bool is_still_refilling() const {
			return initial_filling;
		}

		size_t get_available_command_count() const {
			return buffer.size();
		}

		size_t get_lower_limit() const {
			return lower_limit;
		}

		void set_lower_limit(size_t new_lower_limit) {
			lower_limit = new_lower_limit;
		}

		unsigned get_steps_extrapolated() const {
			return steps_extrapolated;
		}
	};


}