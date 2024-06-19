#pragma once
#include "application/intercosm.h"

#include "game/modes/arena_mode.h"
#include "game/modes/test_mode.h"
#include "application/setups/editor/resources/editor_resource_pools.h"
#include "application/setups/editor/editor_official_resource_map.h"

struct packaged_official_content {
	intercosm built_content;

	/* 
		For now has to be mutable because of how dereferencing resources works. 
		We don't know whether we're getting a project resource or an official resource,
		but we might want to sometimes modify the former.
	*/

	mutable editor_resource_pools resources;
	editor_official_resource_map resource_map;

	packaged_official_content();

private:
	template <class R, class F>
	void for_each_resource(F callback);

	void create_official_prefabs();
};

