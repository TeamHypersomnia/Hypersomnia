#include "cosmos.h"

template<class OutputStream>
void cosmos::delta_encode(cosmos& base, OutputStream& out) const {

}

template<class InputStream>
void cosmos::delta_decode(InputStream& in, bool resubstantiate_partially) {

}

template void cosmos::delta_encode<augs::stream>(cosmos&, augs::stream&) const;
template void cosmos::delta_decode<augs::stream>(augs::stream&, bool);
