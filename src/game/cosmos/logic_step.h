#pragma once
#include "augs/misc/timing/delta.h"
#include "game/cosmos/cosmic_entropy.h"
#include "game/messages/will_soon_be_deleted.h"
#include "game/messages/queue_deletion.h"
#include "game/stateless_systems/deletion_system.h"
#include "augs/misc/randomization_declaration.h"
#include "game/cosmos/solvers/solve_structs.h"

#define LOG_DELETIONS 0

struct data_living_one_step;
struct cosmos_common_significant;

template <bool C>
struct basic_logic_step_input {
	using cosmos_ref = maybe_const_ref_t<C, cosmos>;

	cosmos_ref cosm;
	const cosmic_entropy& entropy;
	const solve_settings& settings;

	operator const_logic_step_input() const {
		return { cosm, entropy, settings };
	}
};

template <bool is_const>
class basic_logic_step {
	/* 
		Message queues are never const in the logic step. 
		The is_const && !is_const expression is to make it template argument-dependent.

		For example, the bomb mode is given const_logic_step in its post-solve,
		and it may still want to post some audiovisual messages,
		it just won't modify the cosmos at that occassion.
	*/

	using data_living_one_step_ref = maybe_const_ref_t<
		is_const && !is_const, 
		data_living_one_step
	>;

	const basic_logic_step_input<is_const> input;

public:
	data_living_one_step_ref transient;
	randomization& step_rng;
	maybe_const_ref_t<is_const, solve_result> result;

	basic_logic_step(
		const basic_logic_step_input<is_const> input,
		data_living_one_step_ref transient,
		randomization& step_rng,
		maybe_const_ref_t<is_const, solve_result> result
	) :
		input(input),
		transient(transient),
		step_rng(step_rng),
		result(result)
	{}

	auto& get_cosmos() const {
		return input.cosm;
	}

	const auto& get_entropy() const {
		return input.entropy;
	}

	const auto& get_settings() const {
		return input.settings;
	}

	const auto& get_logical_assets() const {
		return input.cosm.get_logical_assets();
	}

	auto get_delta() const {
		return get_cosmos().get_fixed_delta();
	}

	operator const_logic_step() const {
		return { input, transient, step_rng, result };
	}
	
	bool any_deletion_occured() const {
		return transient.messages.template get_queue<messages::will_soon_be_deleted>().size() > 0;
	}

	template <class T>
	auto& get_queue() const {
		return transient.messages.template get_queue<T>();
	}

	template <class T>
	void post_message(T&& msg) const {
		static_assert(!std::is_same_v<T, messages::queue_deletion>, "Use queue_deletion_of for better logging.");

		transient.messages.post(std::forward<T>(msg));
	}

	template <class T>
	void post_messages(const T& msgs) const {
		static_assert(!std::is_same_v<typename T::value_type, messages::queue_deletion>, "Use queue_deletion_of for better logging.");

		transient.messages.post(msgs);
	}

	template <class T>
	void post_message_if(const std::optional<T>& msg) const {
		static_assert(!std::is_same_v<T, messages::queue_deletion>, "Use queue_deletion_of for better logging.");

		if (msg.has_value()) {
			transient.messages.post(*msg);
		}
	}

	template <class T>
	void queue_deletion_of(T&& t, const char* const due_to) const {
		(void)due_to;
#if LOG_DELETIONS
		if constexpr(std::is_same_v<T, entity_id>) {
			LOG("Queuing deletion of %x (Due to %x): ", std::forward<T>(t), get_cosmos()[due_to]);
		}
		else {
			LOG("Queuing deletion of %x (Due to %x): ", std::forward<T>(t), due_to);
		}
#endif

		transient.messages.post(messages::queue_deletion(std::forward<T>(t)));
	}

	template <bool C = !is_const, class = std::enable_if_t<C>>
	void perform_deletions() const {
		deletion_system().reverse_perform_deletions(*this);
	}

	template <bool C = !is_const, class = std::enable_if_t<C>>
	void flush_create_entity_requests() const {
		creation_system().flush_create_entity_requests(*this);
	}
};