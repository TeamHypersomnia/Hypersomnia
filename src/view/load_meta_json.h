#pragma once
#include "augs/filesystem/path.h"
#include "augs/readwrite/json_readwrite.h"

inline auto get_meta_json_path(augs::path_type resolved) {
	return resolved.replace_extension(".meta.json");
}

template <class T>
void load_meta_json_if_exists(T& meta, const augs::path_type& resolved) {
	try {
		meta = augs::from_json_file<T>(get_meta_json_path(resolved));
	}
	catch (const augs::file_open_error& err) {
		/* Do not intervene. */
	}
}


template <class T>
void save_meta_json(const T& meta, augs::path_type resolved) {
	augs::save_as_json(meta, get_meta_json_path(resolved));
}
