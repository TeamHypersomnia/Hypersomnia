#include "view/viewables/atlas_distributions.h"
#include "view/viewables/image_in_atlas.h"

#include "view/viewables/images_in_atlas_map.h"
#include "view/viewables/image_definition.h"

avatar_atlas_output create_avatar_atlas(avatar_atlas_input in) {
	thread_local atlas_input_subjects atlas_subjects;
	thread_local std::vector<int> indices_of_existing;
	indices_of_existing.clear();
	atlas_subjects.clear();

	for (int i = 0; i < static_cast<int>(in.subjects.size()); ++i) {
		auto& meta = in.subjects[i];
		auto& avatar = meta.avatar;

		if (avatar.image_bytes.size() > 0) {
			atlas_subjects.loaded_images.push_back(std::move(avatar.image_bytes));
			indices_of_existing.push_back(i);
		}
	}

	thread_local baked_atlas baked;
	baked.clear();

	atlas_profiler performance;

	bake_fresh_atlas(
		{
			atlas_subjects,
			in.max_atlas_size,
			1
		},
		{
			in.atlas_image_output,
			in.fallback_output,
			baked,
			performance
		}
	);

	avatar_atlas_output out;
	out.atlas_size = baked.atlas_image_size;

	ensure_eq(indices_of_existing.size(), baked.loaded_images.size());

	for (int i = 0; i < static_cast<int>(indices_of_existing.size()); ++i) {
		const auto& baked_image = baked.loaded_images[i];
		const auto& idx_in_entries = indices_of_existing[i];

		out.atlas_entries[idx_in_entries] = baked_image;
	}

	return out;
}