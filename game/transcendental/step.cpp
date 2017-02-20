#include "step.h"
#include "game/systems_stateless/item_system.h"
#include "cosmos.h"

#include "game/transcendental/types_specification/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

template<bool C>
basic_cosmic_step<C>::basic_cosmic_step(cosmos_ref cosm) 
	: cosm(cosm)
{
}

template<bool C>
typename basic_cosmic_step<C>::cosmos_ref basic_cosmic_step<C>::get_cosmos() const {
	return cosm;
}

template<bool C>
basic_cosmic_step<C>::operator basic_cosmic_step<true>() const {
	return{ cosm };
}

template<bool C>
basic_logic_step<C>::basic_logic_step(
	cosmos_ref cosm,
	const cosmic_entropy& entropy,
	data_living_one_step_ref transient
) :
	basic_cosmic_step(cosm),
	entropy(entropy),
	transient(transient)
{
}

template<bool C>
augs::delta basic_logic_step<C>::get_delta() const {
	return cosm.get_fixed_delta();
}

template<bool C>
basic_logic_step<C>::operator basic_logic_step<true>() const {
	return { cosm, entropy, transient };
}

template class basic_cosmic_step<false>;
template class basic_cosmic_step<true>;
template class basic_logic_step<false>;
template class basic_logic_step<true>;