#pragma once
#include "augs/misc/timing/delta.h"
#include "game/transcendental/cosmic_entropy.h"
#include "game/messages/will_soon_be_deleted.h"
#include "game/stateless_systems/destroy_system.h"

struct data_living_one_step;
struct cosmos_common_significant;

template <bool C>
struct basic_logic_step_input {
	using cosmos_ref = maybe_const_ref_t<C, cosmos>;

	cosmos_ref cosm;
	const cosmic_entropy& entropy;

	operator const_logic_step_input() const {
		return { cosm, entropy };
	}
};

template <bool is_const>
class basic_logic_step {
	using data_living_one_step_ref = maybe_const_ref_t<is_const, data_living_one_step>;

	const basic_logic_step_input<is_const> input;

public:
	data_living_one_step_ref transient;

	basic_logic_step(
		const basic_logic_step_input<is_const> input,
		data_living_one_step_ref transient
	) :
		input(input),
		transient(transient)
	{}

	auto& get_cosmos() const {
		return input.cosm;
	}

	const auto& get_entropy() const {
		return input.entropy;
	}

	const auto& get_logical_assets() const {
		return input.cosm.get_logical_assets();
	}

	auto get_delta() const {
		return get_cosmos().get_fixed_delta();
	}

	operator const_logic_step() const {
		return { input, transient };
	}
	
	bool any_deletion_occured() const {
		return transient.messages.template get_queue<messages::will_soon_be_deleted>().size() > 0;
	}

	template <class T>
	auto& get_queue() const {
		return transient.messages.template get_queue<T>();
	}

	template <class T, bool C = !is_const, class = std::enable_if_t<C>>
	void post_message(const T& msg) const {
		transient.messages.post(msg);
	}

	template <class T, bool C = !is_const, class = std::enable_if_t<C>>
	void post_messages(const T& msgs) const {
		transient.messages.post(msgs);
	}

	template <class T, bool C = !is_const, class = std::enable_if_t<C>>
	void post_message_if(const std::optional<T>& msg) const {
		if (msg.has_value()) {
			transient.messages.post(*msg);
		}
	}

	template <bool C = !is_const, class = std::enable_if_t<C>>
	void perform_deletions() const {
		destroy_system().reverse_perform_deletions(*this);
	}
};