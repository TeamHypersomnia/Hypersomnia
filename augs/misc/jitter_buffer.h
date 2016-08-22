#pragma once
#include <vector>

namespace augs {
	template<class command>
	class jitter_buffer {
		size_t lower_limit = 3;
		unsigned steps_extrapolated = 0;

		bool locked = false;
		std::vector<command> buffer;
	public:
		void acquire_new_command(const command& c) {
			buffer.push_back(c);
		}

		void acquire_new_command(command&& c) {
			buffer.emplace_back(c);
		}
		
		template <class Iter>
		void acquire_new_commands(Iter first, Iter last) {
			buffer.insert(buffer.end(), first, last);
		}

		std::vector<command> unpack_commands_once() {
			std::vector<command> next_commands;

			if (buffer.size() >= lower_limit + steps_extrapolated)
				locked = false;

			if (!locked) {
				if (buffer.empty()) {
					locked = true;
				}
				else {
					next_commands.assign(buffer.begin(), buffer.begin() + steps_extrapolated + 1);
					buffer.        erase(buffer.begin(), buffer.begin() + steps_extrapolated + 1);
				}
			}

			bool unpacked_successfully = next_commands.size() > 0;

			if (unpacked_successfully)
				steps_extrapolated = 0;
			else
				++steps_extrapolated;

			return std::move(next_commands);
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