#pragma once
#include "../globals/detector_domains.h"

namespace components {
	struct trigger_detector {
		//bool continuous_checking = false;
		augs::entity_id entity_whose_body_is_sampled;
		detection_domain domain = detection_domain::TRIGGER_SWITCHING;

		bool use_physics_of_detector = true;
		
		bool continuous_detection_mode = false;
		bool continuous_detection_flag = false;
	};
}