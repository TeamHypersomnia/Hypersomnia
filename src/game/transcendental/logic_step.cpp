#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/will_soon_be_deleted.h"

template<bool C>
basic_logic_step<C>::basic_logic_step(
	cosmos_ref cosm,
	const logic_step_input input,
	data_living_one_step_ref transient
) :
	cosm(cosm),
	input(input),
	transient(transient)
{
}

template<bool C>
augs::delta basic_logic_step<C>::get_delta() const {
	return cosm.get_fixed_delta();
}

template<bool C>
typename basic_logic_step<C>::cosmos_ref basic_logic_step<C>::get_cosmos() const {
	return cosm;
}

template<bool C>
basic_logic_step<C>::operator const_logic_step() const {
	return { cosm, input, transient };
}

template<bool C>
bool basic_logic_step<C>::any_deletion_occured() const {
	return transient.messages.template get_queue<messages::will_soon_be_deleted>().size() > 0;
}

template struct basic_logic_step<false>;
template struct basic_logic_step<true>;
