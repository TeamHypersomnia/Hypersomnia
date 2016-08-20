#include "viewing_session.h"

std::wstring viewing_session::summary() const {
	return fps_profiler.summary() + triangles.summary();
}
