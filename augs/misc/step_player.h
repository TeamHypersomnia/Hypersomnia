#pragma once
#include <fstream>
#include <vector>

namespace augs {
	template <typename entry_internal_type>
	class step_player {
	public:

	private:
		struct entry_type {
			entry_internal_type internal_data;
			unsigned step_occurred = 0xdeadbeef;
		};

		unsigned player_current_step = 0;
		unsigned next_entry_to_be_replayed = 0;

		std::vector<entry_type> recording;

		std::string live_saving_filename;

	public:
		void rewind() {
			player_current_step = 0;
			next_entry_to_be_replayed = 0;
		}

		void seek(const size_t step = 0) {

		}

		bool load_recording(const std::string& filename) {
			rewind();
			recording.clear();

			std::ifstream source(filename, std::ios::in | std::ios::binary);

			while (source.peek() != EOF) {
				entry_type entry;

				augs::read_object(source, entry.internal_data);
				augs::read_object(source, entry.step_occurred);

				recording.emplace_back(entry);
			}

			return is_recording_available();
		}

		bool is_recording_available() const {
			return recording.size() > 0;
		}

		void append_step_to_recording(const entry_internal_type& currently_processed_entry) {
			if (!currently_processed_entry.empty()) {
				entry_type new_entry;

				new_entry.internal_data = currently_processed_entry;
				new_entry.step_occurred = player_current_step;

				recording.emplace_back(std::move(new_entry));
			}

			next_entry_to_be_replayed = recording.size();
			++player_current_step;
		}

		void append_step_to_live_file(const entry_internal_type& currently_processed_entry) {
			if (!currently_processed_entry.empty()) {
				entry_type new_entry;

				new_entry.internal_data = currently_processed_entry;
				new_entry.step_occurred = player_current_step;

				std::ofstream recording_file(live_saving_filename, std::ios::out | std::ios::binary | std::ios::app);

				augs::write_object(recording_file, new_entry.internal_data);
				augs::write_object(recording_file, new_entry.step_occurred);
			}

			++player_current_step;
		}

		void set_live_filename(const std::string live_saving_filename) {
			this->live_saving_filename = live_saving_filename;
		};

		bool replay_next_step(entry_internal_type& currently_processed_entry) {
			if (next_entry_to_be_replayed >= recording.size()) {
				return false;
			}

			auto& next = recording[next_entry_to_be_replayed];
			
			if (next.step_occurred == player_current_step) {
				currently_processed_entry = next.internal_data;
				++next_entry_to_be_replayed;
			}
			else {
				currently_processed_entry = entry_internal_type();
			}

			++player_current_step;
			return true;
		}
	};
}
