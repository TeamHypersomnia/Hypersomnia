#include "cosmic_step.h"

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

template class basic_cosmic_step<false>;
template class basic_cosmic_step<true>;