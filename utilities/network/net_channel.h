#pragma once
#include <vector>
#include <unordered_map>
#include <memory>

#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakNetTypes.h"

namespace augs {
	namespace network {
		struct reliable_channel_sender {
			struct message {
				luabind::object script;
				bool flag_for_deletion = false;

				RakNet::BitStream* output_bitstream = nullptr;
			};
			
			std::vector<message> reliable_buf;

			std::unordered_map<unsigned, unsigned> sequence_to_reliable_range;

			unsigned short sequence = 0u;
			unsigned short ack_sequence = 0u;

			void post_message(message&);
			bool write_data(RakNet::BitStream& output);
			void read_ack(RakNet::BitStream& input);
		};

		struct reliable_channel_receiver {
			unsigned short last_sequence = 0u;

			bool read_sequence(RakNet::BitStream& input);
			void write_ack(RakNet::BitStream& input);
		};
	}
}