#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "network/network_interface.h"
#include "misc/map_wrapper.h"

struct connection_attempt_result {};
struct send_priority {};
struct send_reliability {};

namespace bindings {
	luabind::scope _network_binding() {
		using namespace network;

		return
			(


			luabind::class_<send_priority>("send_priority")
			.enum_("send_priority")[
				luabind::value("IMMEDIATE_PRIORITY", PacketPriority::IMMEDIATE_PRIORITY),
					luabind::value("HIGH_PRIORITY", PacketPriority::HIGH_PRIORITY),
				luabind::value("MEDIUM_PRIORITY", PacketPriority::MEDIUM_PRIORITY),
				luabind::value("LOW_PRIORITY", PacketPriority::LOW_PRIORITY)
			],

			luabind::class_<send_reliability>("send_reliability")
			.enum_("send_reliability")[
				luabind::value("CONNECTION_ATTEMPT_STARTED", RakNet::CONNECTION_ATTEMPT_STARTED),
					luabind::value("INVALID_PARAMETER", RakNet::INVALID_PARAMETER),
					luabind::value("CANNOT_RESOLVE_DOMAIN_NAME", RakNet::CANNOT_RESOLVE_DOMAIN_NAME),
					luabind::value("ALREADY_CONNECTED_TO_ENDPOINT", RakNet::ALREADY_CONNECTED_TO_ENDPOINT),
					luabind::value("CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS", RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS),
					luabind::value("SECURITY_INITIALIZATION_FAILED", RakNet::SECURITY_INITIALIZATION_FAILED)
			],

			luabind::class_<connection_attempt_result>("connection_attempt_result")
			.enum_("connection_attempt_result")[
				luabind::value("CONNECTION_ATTEMPT_STARTED", RakNet::CONNECTION_ATTEMPT_STARTED),
					luabind::value("INVALID_PARAMETER", RakNet::INVALID_PARAMETER),
					luabind::value("CANNOT_RESOLVE_DOMAIN_NAME", RakNet::CANNOT_RESOLVE_DOMAIN_NAME),
					luabind::value("ALREADY_CONNECTED_TO_ENDPOINT", RakNet::ALREADY_CONNECTED_TO_ENDPOINT),
					luabind::value("CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS", RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS),
					luabind::value("SECURITY_INITIALIZATION_FAILED", RakNet::SECURITY_INITIALIZATION_FAILED)
			],

			luabind::class_<RakNet::BitStream>("BitStream")
			.def(luabind::constructor<>())
			.def("IgnoreBytes", &RakNet::BitStream::IgnoreBytes)
			.def("WriteInt", &RakNet::BitStream::Write<int>)
			.def("WriteUint", &RakNet::BitStream::Write<unsigned>)
			.def("WriteFloat", &RakNet::BitStream::Write<float>)
			.def("WriteDouble", &RakNet::BitStream::Write<double>)
			.def("WriteVec2", &RakNet::BitStream::Write<vec2<>>)
			.def("WriteByte", &RakNet::BitStream::Write<unsigned char>)

			.def("ReadInt", &RakNet::BitStream::Read<int>)
			.def("ReadUint", &RakNet::BitStream::Read<unsigned>)
			.def("ReadFloat", &RakNet::BitStream::Read<float>)
			.def("ReadDouble", &RakNet::BitStream::Read<double>)
			.def("ReadVec2", &RakNet::BitStream::Read<vec2<>>)
			.def("ReadByte", &RakNet::BitStream::Read<unsigned char>)

			,



			luabind::class_<RakNet::RakNetGUID>("RakNetGUID")
			.def(luabind::constructor<>()),

			map_wrapper<RakNet::RakNetGUID, luabind::object>::bind("guid_to_object_map"),

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
			.def("send", &network_interface::send)
	

			);
	}
}