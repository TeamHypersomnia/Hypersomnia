#pragma once
#include "augs/window_framework/event.h"

augs::event::message translate_enum(UINT);
augs::event::keys::key translate_virtual_key(const UINT);
augs::event::keys::key translate_key_with_lparam(const LPARAM lParam, WPARAM);
