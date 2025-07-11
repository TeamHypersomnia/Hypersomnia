#include <cstddef>
template <class F>
void multi_arena_synchronizer::for_each_with_progress(F callback) const {
	/* 
		For now there's no parallel downloads,
		so all maps before the current have 100%, 0% after, and non-0 for the current.
	*/

	for (std::size_t i = 0; i < input.size(); ++i) {
		float progress = 0.0f;

		if (i < current_map) {
			progress = 1.0f;
		}
		else if (i == current_map) {
			progress = session().get_total_percent_complete(get_current_file_percent_complete());
		}

		callback(input[i].name, progress);
	}
}
