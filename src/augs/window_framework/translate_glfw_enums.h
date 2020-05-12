#pragma once
#include <cstdint>
#include "augs/window_framework/event.h"

augs::event::keys::key translate_glfw_key(int);
augs::event::keys::key translate_glfw_mouse_key(int);
