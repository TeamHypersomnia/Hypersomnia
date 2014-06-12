#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "network/network_interface.h"
#include "misc/map_wrapper.h"

struct connection_attempt_result {};
struct send_priority {};
struct send_reliability {};
struct network_message {};

struct UnsignedChar {
	unsigned char c;

	UnsignedChar() : c(0) {}
	UnsignedChar(int d) : c(d) {}
	UnsignedChar(const unsigned char& c) : c(c) {}
	UnsignedChar& operator=(const unsigned char& d) { c = d; }

	void set(int d) {
		c = d;
	}

	operator unsigned char() {
		return c;
	}
};

void Write(RakNet::BitStream& bs, const std::string& str) {
	bs.Write(str.c_str());
}

namespace bindings {
	luabind::scope _network_binding() {
		using namespace network;

		return
			(

			
			luabind::class_<UnsignedChar>("UnsignedChar")
			.def(luabind::constructor<>())
			.def(luabind::constructor<const UnsignedChar&>())
			.def(luabind::constructor<int>())
			.def("set", &UnsignedChar::set)
			,

			luabind::class_<network_message>("network_message")
			.enum_("network_message")[
				luabind::value("ID_CONNECTION_REQUEST_ACCEPTED", ID_CONNECTION_REQUEST_ACCEPTED),
				luabind::value("ID_CONNECTION_ATTEMPT_FAILED", ID_CONNECTION_ATTEMPT_FAILED),
				luabind::value("ID_ALREADY_CONNECTED", ID_ALREADY_CONNECTED),
				luabind::value("ID_NEW_INCOMING_CONNECTION", ID_NEW_INCOMING_CONNECTION),
				luabind::value("ID_NO_FREE_INCOMING_CONNECTIONS", ID_NO_FREE_INCOMING_CONNECTIONS),
				luabind::value("ID_DISCONNECTION_NOTIFICATION", ID_DISCONNECTION_NOTIFICATION),
				luabind::value("ID_CONNECTION_LOST", ID_CONNECTION_LOST),
				luabind::value("ID_CONNECTION_BANNED", ID_CONNECTION_BANNED),
				luabind::value("ID_INVALID_PASSWORD", ID_INVALID_PASSWORD),
				luabind::value("ID_INCOMPATIBLE_PROTOCOL_VERSION", ID_INCOMPATIBLE_PROTOCOL_VERSION),
				luabind::value("ID_IP_RECENTLY_CONNECTED", ID_IP_RECENTLY_CONNECTED),
				luabind::value("ID_TIMESTAMP", ID_TIMESTAMP),
				luabind::value("ID_UNCONNECTED_PONG", ID_UNCONNECTED_PONG),
				luabind::value("ID_ADVERTISE_SYSTEM", ID_ADVERTISE_SYSTEM),
				luabind::value("ID_DOWNLOAD_PROGRESS", ID_DOWNLOAD_PROGRESS),
				luabind::value("ID_USER_PACKET_ENUM", ID_USER_PACKET_ENUM)
			],

			luabind::class_<send_priority>("send_priority")
			.enum_("send_priority")[
				luabind::value("IMMEDIATE_PRIORITY", PacketPriority::IMMEDIATE_PRIORITY),
					luabind::value("HIGH_PRIORITY", PacketPriority::HIGH_PRIORITY),
				luabind::value("MEDIUM_PRIORITY", PacketPriority::MEDIUM_PRIORITY),
				luabind::value("LOW_PRIORITY", PacketPriority::LOW_PRIORITY)
			],




			luabind::class_<send_reliability>("send_reliability")
			.enum_("send_reliability")[
				luabind::value("UNRELIABLE", PacketReliability::UNRELIABLE),
				luabind::value("UNRELIABLE_SEQUENCED", PacketReliability::UNRELIABLE_SEQUENCED),
				luabind::value("RELIABLE", PacketReliability::RELIABLE),
				luabind::value("RELIABLE_ORDERED", PacketReliability::RELIABLE_ORDERED),
				luabind::value("RELIABLE_SEQUENCED", PacketReliability::RELIABLE_SEQUENCED),
				luabind::value("UNRELIABLE_WITH_ACK_RECEIPT", PacketReliability::UNRELIABLE_WITH_ACK_RECEIPT),
				luabind::value("RELIABLE_WITH_ACK_RECEIPT", PacketReliability::RELIABLE_WITH_ACK_RECEIPT),
				luabind::value("RELIABLE_ORDERED_WITH_ACK_RECEIPT", PacketReliability::RELIABLE_ORDERED_WITH_ACK_RECEIPT)
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

			luabind::class_<RakNet::RakString>("RakString")
			.def(luabind::constructor<>())
			.def("C_String", &RakNet::RakString::C_String),


			/* little helper */
			luabind::def("WriteCString", Write),

			luabind::class_<RakNet::BitStream>("BitStream")
			.def(luabind::constructor<>())
			.def("IgnoreBytes", &RakNet::BitStream::IgnoreBytes)
			.def("WriteInt", &RakNet::BitStream::Write<int>)
			.def("WriteUint", &RakNet::BitStream::Write<unsigned>)
			.def("WriteFloat", &RakNet::BitStream::Write<float>)
			.def("WriteDouble", &RakNet::BitStream::Write<double>)
			.def("WriteVec2", &RakNet::BitStream::Write<vec2<>>)
			.def("WriteByte", &RakNet::BitStream::Write<UnsignedChar>)

			.def("ReadInt", &RakNet::BitStream::Read<int>)
			.def("ReadUint", &RakNet::BitStream::Read<unsigned>)
			.def("ReadFloat", &RakNet::BitStream::Read<float>)
			.def("ReadDouble", &RakNet::BitStream::Read<double>)
			.def("ReadVec2", &RakNet::BitStream::Read<vec2<>>)
			.def("ReadByte", &RakNet::BitStream::Read<UnsignedChar>)
			.def("ReadRakString", &RakNet::BitStream::Read<RakNet::RakString>)

			,



			luabind::class_<RakNet::RakNetGUID>("RakNetGUID")
			.def(luabind::constructor<>()),

			map_wrapper<RakNet::RakNetGUID, luabind::object>::bind("guid_to_object_map"),

			luabind::class_<network_interface::packet>("network_packet")
			.def(luabind::constructor<>())
			.def("byte", &network_interface::packet::byte)
			.def("length", &network_interface::packet::length)
			.def("guid", &network_interface::packet::guid)
			.def("get_bitstream", &network_interface::packet::get_bitstream)
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