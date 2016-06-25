#pragma once

class cosmos;

template <class white_box, class black_box, class black_box_detail>
class black_box_component : public white_box {
	cosmos* cosmos_ptr = nullptr;
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
	
	black_box black;
	black_box_detail black_detail;
public:
	class definition : public white_box, public black_box {

	};

	black_box_component& operator=(const black_box_component&);
	black_box_component(const black_box_component&);

	black_box_component(const definition& = definition());

	void set_from_definition(const definition& = definition());
	colliders_definition get_definition() const;
};