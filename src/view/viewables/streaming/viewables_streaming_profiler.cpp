#include "augs/templates/introspect.h"
#include "view/viewables/streaming/viewables_streaming_profiler.h"

/* So that we don't have to include generated/introspectors with the header */

viewables_streaming_profiler::viewables_streaming_profiler() {
	setup_names_of_measurements();
}
