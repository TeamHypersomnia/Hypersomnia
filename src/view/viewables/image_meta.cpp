#include "view/viewables/image_meta.h"
#include "augs/templates/introspection_utils/introspective_equal.h"

bool image_meta::operator==(const image_meta& b) const {
	return augs::introspective_equal(*this, b);
}
