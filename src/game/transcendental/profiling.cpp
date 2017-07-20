#include "profiling.h"
#include "augs/templates/introspect.h"
#include "generated/introspectors.h"

/* So that we don't have to include generated/introspectors with profiling.h */

session_profiler::session_profiler() {
	setup_names_of_measurements();
}

cosmic_profiler::cosmic_profiler() {
	setup_names_of_measurements();
}
