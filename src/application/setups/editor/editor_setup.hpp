#pragma once
#include "application/setups/editor/editor_setup.h"

template <class F>
decltype(auto) editor_setup::on_resource_if(const editor_resource_id& id, F&& callback) {
	if (id.is_official) {
		return official_resources.dispatch_if(id, std::forward<F>(callback));
	}

	return project.resources.dispatch_if(id, std::forward<F>(callback));
}

template <class F>
decltype(auto) editor_setup::on_resource_if(const editor_resource_id& id, F&& callback) const {
	if (id.is_official) {
		return official_resources.dispatch_if(id, std::forward<F>(callback));
	}

	return project.resources.dispatch_if(id, std::forward<F>(callback));
}