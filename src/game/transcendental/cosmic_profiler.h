#pragma once
#include "augs/misc/measurements.h"

enum class meter_type {
	LOGIC,
	RENDERING,
	CAMERA_QUERY,
	GUI,
	INTERPOLATION,
	VISIBILITY,
	PHYSICS,
	PARTICLES,
	AI,
	PATHFINDING,

	METER_COUNT
};

class cosmic_profiler {
public:
	augs::measurements complete_reinference = augs::measurements(L"Complete reinference", true, 1);

	augs::measurements raycasts = augs::measurements(L"Raycasts", false);
	augs::measurements entropy_length = augs::measurements(L"Entropy length", false);
	augs::measurements meters[(int)meter_type::METER_COUNT];

	augs::measurements total_load = augs::measurements(L"Total load", true, 1);
	augs::measurements reading_savefile = augs::measurements(L"Loading savefile", true, 1);
	augs::measurements deserialization_pass = augs::measurements(L"Deserialization pass", true, 1);

	augs::measurements total_save = augs::measurements(L"Total save", true, 1);
	augs::measurements size_calculation_pass = augs::measurements(L"Size calculation pass", true, 1);
	augs::measurements memory_allocation_pass = augs::measurements(L"Memory allocation pass", true, 1);
	augs::measurements serialization_pass = augs::measurements(L"Serialization pass", true, 1);
	augs::measurements writing_savefile = augs::measurements(L"Writing savefile", true, 1);

	augs::measurements delta_bytes = augs::measurements(L"Delta bytes", false, 1);

	augs::measurements duplication = augs::measurements(L"Duplication");

	augs::measurements delta_encoding = augs::measurements(L"Delta encoding");
	augs::measurements delta_decoding = augs::measurements(L"Delta decoding");

	void start(meter_type);
	void stop(meter_type);

	cosmic_profiler();

	void set_count_of_tracked_measurements(size_t);
	std::wstring sorted_summary(bool detailed) const;
};