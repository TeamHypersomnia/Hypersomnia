#pragma once
#include "augs/misc/constant_size_vector.h"
#include "game/modes/mode_entropy.h"
#include "augs/misc/serialization_buffers.h"
#include "application/network/server_step_entropy.h"

#define LOG_NET_SERIALIZATION !IS_PRODUCTION_BUILD

template <class... Args>
void NSR_LOG(Args&&... args) {
#if LOG_NET_SERIALIZATION
	LOG(std::forward<Args>(args)...);
#else
	((void)args, ...);
#endif
}

#if LOG_NET_SERIALIZATION
#define NSR_LOG_NVPS LOG_NVPS
#else
#define NSR_LOG_NVPS NSR_LOG
#endif

constexpr std::size_t chosen_packet_size_v = 1024;

constexpr std::size_t total_header_bytes_v = 
	yojimbo::ConservativeMessageHeaderBits
	+ yojimbo::ConservativePacketHeaderBits 
	+ yojimbo::ConservativeMessageHeaderBits 
;

/* 
	Make the max message size conservative enough 
	so that a single message does never cause a packet to split. 
*/

constexpr std::size_t max_message_size_v = ((chosen_packet_size_v - total_header_bytes_v) / 4) * 4;

using message_bytes_type = augs::constant_size_vector<std::byte, max_message_size_v>;

struct server_vars;

struct preserialized_message : public yojimbo::Message {
	message_bytes_type bytes;

	template <typename Stream>
	bool Serialize(Stream& stream);

	YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

struct only_block_message : public yojimbo::BlockMessage {
	/* The client will never send blocks */

	static constexpr bool server_to_client = true;
	static constexpr bool client_to_server = false;

	template <typename Stream>
	bool Serialize(Stream& stream) {
		(void)stream;
		return true;
	}

	YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
};

template <bool C>
struct initial_arena_state_payload;

namespace net_messages {
	struct initial_arena_state : only_block_message {
		bool read_payload(
			augs::serialization_buffers&,
			initial_arena_state_payload<false>
		);

		const std::vector<std::byte>* write_payload(
			augs::serialization_buffers&,
			initial_arena_state_payload<true>
		);
	};

	//struct initial_steps_correction : only_block_message {};

	struct new_server_vars : preserialized_message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		bool write_payload(const server_vars&);
		bool read_payload(server_vars&);
	};

	struct server_step_entropy : preserialized_message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		bool write_payload(const server_step_entropy_for_client&);
		bool read_payload(server_step_entropy_for_client&);
	};

	struct client_entropy : preserialized_message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		bool write_payload(const total_client_entropy&);
		bool read_payload(total_client_entropy&);
	};

	struct client_welcome : public yojimbo::Message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		requested_client_settings payload;

		template <typename Stream>
		bool Serialize(Stream& stream);

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();

		bool write_payload(const requested_client_settings&);
		bool read_payload(requested_client_settings&);
	};

	using all_t = type_list<
		initial_arena_state*,
		//initial_steps_correction*,
		new_server_vars*,
		server_step_entropy*,
		client_welcome*,
		client_entropy*
	>;
	
	using id_t = type_in_list_id<all_t>;
}
