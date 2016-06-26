#pragma once
#include "game/entity_handle_declaration.h"

class cosmos;

template <class white_box, class substance_definition_base, class incremental_resubstantializator>
class substantialized_component : public white_box {
	cosmos* cosmos_ptr = nullptr;
	friend class basic_entity_handle<false>;
protected:
	entity_id owner;

	cosmos& get_cosmos() {
		return *cosmos_ptr;
	}

	const cosmos& get_cosmos() const {
		return *cosmos_ptr;
	}

	entity_handle get_entity() {
		return get_cosmos()[owner];
	}

	const_entity_handle get_entity() const {
		return get_cosmos()[owner];
	}

	bool has_synchronizable_substance() const {
		return get_entity().has<components::substance>();
	}

	void full_resubstantialization() {
		if (has_synchronizable_substance()) {
			auto self = get_entity();
			
			auto substance = self.get<components::substance>();
			self.remove<components::substance>();
			self += substance;
		}
	}
	
	class substance_definition : public substance_definition_base {
		bool activated = false;
	};

	substance_definition substance;
	incremental_resubstantializator resubstantializator;
public:
	class definition : public white_box, public substance_definition {

	};

	substantialized_component& operator=(const substantialized_component&);
	substantialized_component(const substantialized_component&);

	substantialized_component(const definition& = definition());

	void set_from_definition(const definition& = definition());
	definition get_definition() const;

	void set_activated(bool flag) {
		substance.activated = flag;
	}

	bool is_activated() const {
		return substance.activated;
	}
};