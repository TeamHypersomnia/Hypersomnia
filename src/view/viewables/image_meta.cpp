#include "view/viewables/image_meta.h"
#include "augs/templates/introspection_utils/introspective_equal.h"

bool image_extra_loadables::operator==(const image_extra_loadables& b) const {
	return augs::introspective_equal(*this, b);
}
