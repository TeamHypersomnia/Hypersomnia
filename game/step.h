#pragma once
#include "game/types_specification/all_messages_declaration.h"
#include "misc/delta.h"
#include "misc/machine_entropy.h"
#include "game/entity_handle_declaration.h"

class cosmos;

class basic_viewing_step {
public:
	basic_viewing_step(const cosmos&, augs::variable_delta, augs::renderer& renderer);

	const cosmos& cosm;
	augs::variable_delta delta;
	augs::renderer& renderer;

	augs::variable_delta get_delta() const;
};

class viewing_step : public basic_viewing_step {
public:
	viewing_step(basic_viewing_step basic_step, const_entity_handle camera);

	const_entity_handle camera;
};

class fixed_step {
	friend class cosmos;
	fixed_step(cosmos&, augs::machine_entropy);

public:
	storage_for_all_message_queues messages;
	cosmos& cosm;
	augs::machine_entropy entropy;

	augs::fixed_delta get_delta() const;
};