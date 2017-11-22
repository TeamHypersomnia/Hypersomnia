#pragma once
#include <cstdint>
#include "augs/window_framework/event.h"

typedef uint32_t xcb_keysym_t; 
augs::event::keys::key translate_keysym(const xcb_keysym_t);
