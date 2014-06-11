#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "network/network_interface.h"

struct connection_attempt_result {

};

namespace bindings {
	luabind::scope _network_binding() {
		using namespace network;

		return
			(

			luabind::class_<connection_attempt_result>("connection_attempt_result")
			.enum_("connection_attempt_result")[
				luabind::value("CONNECTION_ATTEMPT_STARTED", RakNet::CONNECTION_ATTEMPT_STARTED),
					luabind::value("INVALID_PARAMETER", RakNet::INVALID_PARAMETER),
					luabind::value("CANNOT_RESOLVE_DOMAIN_NAME", RakNet::CANNOT_RESOLVE_DOMAIN_NAME),
					luabind::value("ALREADY_CONNECTED_TO_ENDPOINT", RakNet::ALREADY_CONNECTED_TO_ENDPOINT),
					luabind::value("CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS", RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS),
					luabind::value("SECURITY_INITIALIZATION_FAILED", RakNet::SECURITY_INITIALIZATION_FAILED)
			],

			luabind::class_<network_interface::packet>("network_packet")
			.def(luabind::constructor<>())
			.def("byte", &network_interface::packet::byte)
			.def("length", &network_interface::packet::length)
			.def("guid", &network_interface::packet::guid)
			,

			luabind::class_<network_interface>("network_interface")
			.def(luabind::constructor<>())
			.def("listen", &network_interface::listen)
			.def("connect", &network_interface::connect)
			.def("receive", &network_interface::receive)
	

			);
	}
}