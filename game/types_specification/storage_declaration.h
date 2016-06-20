#pragma once
#include "components_instantiation.h"
#include "entity_system/storage_for_components_and_aggregates.h"

typedef typename put_all_components_into<augs::storage_for_components_and_aggregates>::type
storage_for_all_components_and_aggregates;

template <bool is_const>
using basic_aggregate_handle = storage_for_all_components_and_aggregates::basic_aggregate_handle<is_const>;