#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/input_component.h"
#include "../systems/input_system.h"

#include "window_framework/event.h"

namespace bindings {
	luabind::scope _input_component() {
		struct dummy_mouse {};
		struct dummy_keys {};

		using namespace window::event;
		using namespace mouse;

		return (
			luabind::class_<input_system::context>("input_context")
			.def(luabind::constructor<>())
			.def("set_intent", &input_system::context::set_intent)
			.def_readwrite("enabled", &input_system::context::enabled),

	

			luabind::class_<input>("input_component")
			.def(luabind::constructor<>())
			.def("add", &input::add),

			luabind::class_<dummy_mouse>("mouse")
				.enum_("constants")
				[
					luabind::value("ltripleclick", ltripleclick),
					luabind::value("raw_motion", raw_motion),
					luabind::value("motion", motion),
					luabind::value("wheel", wheel),
					luabind::value("ldoubleclick", ldoubleclick),
					luabind::value("mdoubleclick", mdoubleclick),
					luabind::value("rdoubleclick", rdoubleclick),
					luabind::value("ldown", ldown),
					luabind::value("lup", lup),
					luabind::value("mdown", mdown),
					luabind::value("mup", mup),
					luabind::value("rdown", rdown),
					luabind::value("rup", rup)
				],
				
				luabind::class_<dummy_keys>("keys")
				.enum_("constants")
				[
					luabind::value("LMOUSE", keys::LMOUSE		),
					luabind::value("RMOUSE", keys::RMOUSE		),
					luabind::value("MMOUSE", keys::MMOUSE		),
					luabind::value("CANCEL", keys::CANCEL		),
					luabind::value("BACKSPACE", keys::BACKSPACE	),
					luabind::value("TAB", keys::TAB		),
					luabind::value("CLEAR", keys::CLEAR		),
					luabind::value("ENTER", keys::ENTER		),
					luabind::value("SHIFT", keys::SHIFT		),
					luabind::value("CTRL", keys::CTRL		),
					luabind::value("ALT", keys::ALT		),
					luabind::value("PAUSE", keys::PAUSE		),
					luabind::value("CAPSLOCK", keys::CAPSLOCK	),
					luabind::value("ESC", keys::ESC		),
					luabind::value("SPACE", keys::SPACE		),
					luabind::value("PAGEUP", keys::PAGEUP		),
					luabind::value("PAGEDOWN", keys::PAGEDOWN	),
					luabind::value("END", keys::END		),
					luabind::value("HOME", keys::HOME		),
					luabind::value("LEFT", keys::LEFT		),
					luabind::value("UP", keys::UP			),
					luabind::value("RIGHT", keys::RIGHT		),
					luabind::value("DOWN", keys::DOWN		),
					luabind::value("SELECT", keys::SELECT		),
					luabind::value("PRINT", keys::PRINT		),
					luabind::value("EXECUTE", keys::EXECUTE	),
					luabind::value("PRINTSCREEN", keys::PRINTSCREEN),
					luabind::value("INSERT", keys::INSERT		),
					luabind::value("DEL", keys::DEL		),
					luabind::value("HELP", keys::HELP		),
					luabind::value("LWIN", keys::LWIN		),
					luabind::value("RWIN", keys::RWIN		),
					luabind::value("APPS", keys::APPS		),
					luabind::value("SLEEP", keys::SLEEP		),
					luabind::value("NUMPAD0", keys::NUMPAD0	),
					luabind::value("NUMPAD1", keys::NUMPAD1	),
					luabind::value("NUMPAD2", keys::NUMPAD2	),
					luabind::value("NUMPAD3", keys::NUMPAD3	),
					luabind::value("NUMPAD4", keys::NUMPAD4	),
					luabind::value("NUMPAD5", keys::NUMPAD5	),
					luabind::value("NUMPAD6", keys::NUMPAD6	),
					luabind::value("NUMPAD7", keys::NUMPAD7	),
					luabind::value("NUMPAD8", keys::NUMPAD8	),
					luabind::value("NUMPAD9", keys::NUMPAD9	),
					luabind::value("MULTIPLY", keys::MULTIPLY	),
					luabind::value("ADD", keys::ADD		),
					luabind::value("SEPARATOR", keys::SEPARATOR	),
					luabind::value("SUBTRACT", keys::SUBTRACT	),
					luabind::value("DECIMAL", keys::DECIMAL	),
					luabind::value("DIVIDE", keys::DIVIDE		),
					luabind::value("F1", keys::F1			),
					luabind::value("F2", keys::F2			),
					luabind::value("F3", keys::F3			),
					luabind::value("F4", keys::F4			),
					luabind::value("F5", keys::F5			),
					luabind::value("F6", keys::F6			),
					luabind::value("F7", keys::F7			),
					luabind::value("F8", keys::F8			),
					luabind::value("F9", keys::F9			),
					luabind::value("F10", keys::F10		),
					luabind::value("F11", keys::F11		),
					luabind::value("F12", keys::F12		),
					luabind::value("F13", keys::F13		),
					luabind::value("F14", keys::F14		),
					luabind::value("F15", keys::F15		),
					luabind::value("F16", keys::F16		),
					luabind::value("F17", keys::F17		),
					luabind::value("F18", keys::F18		),
					luabind::value("F19", keys::F19		),
					luabind::value("F20", keys::F20		),
					luabind::value("F21", keys::F21		),
					luabind::value("F22", keys::F22		),
					luabind::value("F23", keys::F23		),
					luabind::value("F24", keys::F24		),
					luabind::value("A", keys::A			),
					luabind::value("B", keys::B			),
					luabind::value("C", keys::C			),
					luabind::value("D", keys::D			),
					luabind::value("E", keys::E			),
					luabind::value("F", keys::F			),
					luabind::value("G", keys::G			),
					luabind::value("H", keys::H			),
					luabind::value("I", keys::I			),
					luabind::value("J", keys::J			),
					luabind::value("K", keys::K			),
					luabind::value("L", keys::L			),
					luabind::value("M", keys::M			),
					luabind::value("N", keys::N			),
					luabind::value("O", keys::O			),
					luabind::value("P", keys::P			),
					luabind::value("Q", keys::Q			),
					luabind::value("R", keys::R			),
					luabind::value("S", keys::S			),
					luabind::value("T", keys::T			),
					luabind::value("U", keys::U			),
					luabind::value("V", keys::V			),
					luabind::value("W", keys::W			),
					luabind::value("X", keys::X			),
					luabind::value("Y", keys::Y			),
					luabind::value("Z", keys::Z			),
					luabind::value("_0", keys::_0			),
					luabind::value("_1", keys::_1			),
					luabind::value("_2", keys::_2			),
					luabind::value("_3", keys::_3			),
					luabind::value("_4", keys::_4			),
					luabind::value("_5", keys::_5			),
					luabind::value("_6", keys::_6			),
					luabind::value("_7", keys::_7			),
					luabind::value("_8", keys::_8			),
					luabind::value("_9", keys::_9			),
					luabind::value("NUMLOCK", keys::NUMLOCK	),
					luabind::value("SCROLL", keys::SCROLL		),
					luabind::value("LSHIFT", keys::LSHIFT		),
					luabind::value("RSHIFT", keys::RSHIFT		),
					luabind::value("LCTRL", keys::LCTRL		),
					luabind::value("RCTRL", keys::RCTRL		),
					luabind::value("LALT", keys::LALT		),
					luabind::value("RALT", keys::RALT		),
					luabind::value("DASH", keys::DASH		)
				]
			);
	}
}