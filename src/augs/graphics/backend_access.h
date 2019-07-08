#pragma once

namespace augs {
	namespace graphics {
		class backend_access {
			backend_access() {}
			friend class renderer_backend;
		};
	}
}
