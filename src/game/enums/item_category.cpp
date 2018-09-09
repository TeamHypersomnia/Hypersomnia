#include "item_category.h"

bool is_clothing(const item_category_flagset& category) {
 return
	 category.test(item_category::TORSO_ARMOR)
	 || category.test(item_category::BACK_WEARABLE);
}