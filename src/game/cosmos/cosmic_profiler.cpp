#include "augs/templates/introspect.h"
#include "game/cosmos/cosmic_profiler.h"

/* So that we don't have to include generated/introspectors with the header */

cosmic_profiler::cosmic_profiler() {
	setup_names_of_measurements();
}
