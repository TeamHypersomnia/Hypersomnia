#pragma once
#include "stdafx.h"
#include "bindings.h"

#include <sstream>

#include "network/network_interface.h"
#include "misc/map_wrapper.h"
#include "misc/vector_wrapper.h"

#include "network/reliable_channel.h"
#include "network/bitstream_wrapper.h"

struct connection_attempt_result {};
struct send_priority {};
struct send_reliability {};
struct network_message {};

struct receive_result {};

std::string auto_string_indent(std::string in) {
	size_t i = 0;
	int level = 0;

	while (i < in.length()) {
		switch(in[i]) {
		case '{': ++level; break;
		case '}': --level; break;
		case '\n': {
			bool end_found = false;
			if (i + 1 < in.length())
				end_found = in[i + 1] == '}';

			in.insert(i+1, (level-end_found)*2, ' '); 
		}
			break;
		default: break;
		}

		++i;
	}

	return in;
}


namespace bindings {
	luabind::scope _network_binding() {
		using namespace network;

		return
			(
			luabind::def("auto_string_indent", auto_string_indent),

			luabind::class_<network_message>("network_event")
			.enum_("network_event")[
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

			luabind::class_<RakNet::RakNetGUID>("RakNetGUID")
			.def(luabind::constructor<>()),

			luabind::class_<bitstream>("BitStream")
			.def(luabind::constructor<>())
			.def_readwrite("content", &bitstream::content)
			.def_readwrite("read_report", &bitstream::read_report)
			.def("assign", &bitstream::operator=)
			.def("name_property", &bitstream::name_property)
			.def("IgnoreBytes", &bitstream::IgnoreBytes)
			.def("ReadRakString", &bitstream::Read<RakNet::RakString>)
			.def("size", &bitstream::GetNumberOfBitsUsed)
			.def("Reset", &bitstream::Reset)
			.def("ResetReadPointer", &bitstream::ResetReadPointer)
			.def("SetReadOffset", &bitstream::SetReadOffset)
			.def("GetReadOffset", &bitstream::GetReadOffset)
			.def("GetNumberOfUnreadBits", &bitstream::GetNumberOfUnreadBits)

			.def("WriteBitstream", &bitstream::WriteBitstream)
			.def("WriteBit", &bitstream::WritePOD<bool>)
			.def("WriteInt", &bitstream::WritePOD<int>)
			.def("WriteByte", &bitstream::WritePOD<unsigned char>)
			.def("WriteUint", &bitstream::WritePOD<unsigned>)
			.def("WriteUshort", &bitstream::WritePOD<unsigned short>)
			.def("WriteFloat", &bitstream::WritePOD<float>)
			.def("WriteDouble", &bitstream::WritePOD<double>)
			.def("Writeb2Vec2", &bitstream::WriteVec<b2Vec2>)
			.def("WriteVec2", &bitstream::WriteVec<vec2<>>)

			.def("ReadRakNetGUID", &bitstream::ReadGuid)
			.def("WriteRakNetGUID", &bitstream::WriteGuid)
			
			.def("ReadBit", &bitstream::ReadPOD<bool>)
			.def("ReadInt", &bitstream::ReadPOD<int>)
			.def("ReadByte", &bitstream::ReadPOD<unsigned char>)
			.def("ReadUint", &bitstream::ReadPOD<unsigned>)
			.def("ReadUshort", &bitstream::ReadPOD<unsigned short>)
			.def("ReadFloat", &bitstream::ReadPOD<float>)
			.def("ReadDouble", &bitstream::ReadPOD<double>)
			.def("Readb2Vec2", &bitstream::ReadVec<b2Vec2>) 
			.def("ReadVec2", &bitstream::ReadVec<vec2<>>)
			,

			luabind::class_<receive_result>("receive_result")
			.enum_("receive_result")[
				luabind::value("RELIABLE_RECEIVED", reliable_receiver::RELIABLE_RECEIVED),
				luabind::value("ONLY_UNRELIABLE_RECEIVED", reliable_receiver::ONLY_UNRELIABLE_RECEIVED),
				luabind::value("NOTHING_RECEIVED", reliable_receiver::NOTHING_RECEIVED)
			],

			luabind::class_<reliable_sender::message>("net_channel_message")
			.def(luabind::constructor<>())
			.def_readwrite("flag_for_deletion", &reliable_sender::message::flag_for_deletion)
			.def_readwrite("script", &reliable_sender::message::script)
			.def_readwrite("output_bitstream", &reliable_sender::message::output_bitstream),
			
			misc::vector_wrapper<reliable_sender::message>::bind_vector("net_channel_message_vector"),

			luabind::class_<reliable_sender>("reliable_sender")
			.def(luabind::constructor<>())
			.def("post_message", &reliable_sender::post_message)
			.def("read_ack", &reliable_sender::read_ack)
			.def("write_data", &reliable_sender::write_data)
			.def_readwrite("reliable_buf", &reliable_sender::reliable_buf)
			.def_readwrite("unreliable_buf", &reliable_sender::unreliable_buf)
			.def_readwrite("sequence", &reliable_sender::sequence)
			.def_readwrite("ack_sequence", &reliable_sender::ack_sequence)
			, 

			luabind::class_<reliable_receiver>("reliable_receiver")
			.def(luabind::constructor<>())
			.def_readwrite("last_sequence", &reliable_receiver::last_sequence)
			.def("read_sequence", &reliable_receiver::read_sequence)
			.def("write_ack", &reliable_receiver::write_ack),

			luabind::class_<reliable_channel>("reliable_channel")
			.def(luabind::constructor<>())
			.def_readwrite("receiver", &reliable_channel::receiver)
			.def_readwrite("sender", &reliable_channel::sender)
			.def("disable_starting_byte", &reliable_channel::disable_starting_byte)
			.def("enable_starting_byte", &reliable_channel::enable_starting_byte)
			.def("send", &reliable_channel::send)
			.def("recv", &reliable_channel::recv),

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
			.def("enable_lag", &network_interface::enable_lag)
			.def("close_connection", &network_interface::close_connection)
			.def("shutdown", &network_interface::shutdown)
			.def("send", &network_interface::send)
	

			);
	}
}