#include <Box2D/Box2D.h>

#include "augs/templates/algorithm_templates.h"
#include "game/assets/all_logical_assets.h"
#include "view/viewables/all_viewables_defs.h"

#include "augs/templates/introspect.h"

const all_viewables_defs all_viewables_defs::empty {};

void all_viewables_defs::clear() {
	augs::introspect([](auto, auto& c){ c.clear(); }, *this);
}