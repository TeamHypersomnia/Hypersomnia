#include "logic_step.h"
#include "game/transcendental/cosmos.h"

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

template class basic_logic_step<false>;
template class basic_logic_step<true>;