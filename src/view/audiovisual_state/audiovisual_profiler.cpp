#include "augs/templates/introspect.h"
#include "view/audiovisual_state/audiovisual_profiler.h"


/* So that we don't have to include generated/introspectors with the header */

audiovisual_profiler::audiovisual_profiler() {
	setup_names_of_measurements();
}