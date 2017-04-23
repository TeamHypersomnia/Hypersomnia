#pragma once
#include "augs/window_framework/event.h"

augs::window::event::message translate_enum(unsigned int);
augs::window::event::keys::key translate_key(const unsigned int lParam, unsigned int);