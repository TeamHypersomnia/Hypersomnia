#pragma once
#include "message.h"

namespace messages {
	struct shot_message : public message {
		shot_message() : message() { send_to_scripts = true;  }
	};
}