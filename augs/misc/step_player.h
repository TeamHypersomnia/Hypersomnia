#pragma once

#include <fstream>

#include "templates.h"

namespace augs {
	template <typename entry_internal_type>
	class step_player {
	public:
		enum class player_state {
			DISABLED,
			RECORDING,
			REPLAYING
		};

	private:
		struct entry_type {
			entry_internal_type internal_data;
			int step_occurred;
		};

		int player_position = 0;
		int next_entry_to_be_replayed = 0;

		player_state current_player_state = player_state::DISABLED;

		std::vector<entry_type> loaded_recording;

		std::string live_saving_filename;

	public:
		void record(std::string live_saving_filename) {
			stop();

			this->live_saving_filename = live_saving_filename;
			current_player_state = player_state::RECORDING;
		};

		void replay() {
			stop();
			current_player_state = player_state::REPLAYING;
		}

		void stop() {
			current_player_state = player_state::DISABLED;
			player_position = 0;
			next_entry_to_be_replayed = 0;
		}

		void load_recording(std::string filename) {
			stop();
			loaded_recording.clear();

			std::ifstream source(filename, std::ios::in | std::ios::binary);

			while (source.peek() != EOF) {
				entry_type entry;

				deserialize(source, entry.step_occurred);
				deserialize(source, entry.internal_data);

				loaded_recording.emplace_back(entry);
			}
		}

		bool is_recording_available() const {
			return loaded_recording.size() > 0;
		}

		void biserialize(entry_internal_type& currently_processed_entry) {
			if (current_player_state == player_state::RECORDING) {
				if (!currently_processed_entry.empty()) {
					entry_type new_entry;

					new_entry.internal_data = currently_processed_entry;
					new_entry.step_occurred = player_position;

					std::ofstream recording_file(live_saving_filename, std::ios::out | std::ios::binary | std::ios::app);

					serialize(recording_file, new_entry.step_occurred);
					serialize(recording_file, new_entry.internal_data);
				}
			}
			else if (current_player_state == player_state::REPLAYING) {
				if (next_entry_to_be_replayed >= loaded_recording.size()) {
					stop();
					return;
				}

				auto& next = loaded_recording[next_entry_to_be_replayed];
				
				if (next.step_occurred == player_position) {
					currently_processed_entry = next.internal_data;
					++next_entry_to_be_replayed;
				}
				else {
					currently_processed_entry = entry_internal_type();
				}
			}

			if(current_player_state != player_state::DISABLED)
				++player_position;
		}

		player_state get_state() const {
			return current_player_state;
		}

		bool is_replaying() const {
			return current_player_state == player_state::REPLAYING;
		}
	};
}
