#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "renderable_mixin.h"



// explicit instantiation
template class renderable_mixin<false, basic_entity_handle<false>>;
template class renderable_mixin<true, basic_entity_handle<true>>;
template class basic_renderable_mixin<false, basic_entity_handle<false>>;
template class basic_renderable_mixin<true, basic_entity_handle<true>>;
