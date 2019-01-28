#pragma once
#include "augs/misc/constant_size_vector.h"
#include "game/modes/mode_entropy.h"
#include "augs/misc/serialization_buffers.h"
#include "application/network/server_step_entropy.h"
#include "application/network/special_client_request.h"

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

constexpr std::size_t total_header_bytes_v = (
	yojimbo::ConservativeMessageHeaderBits
	+ yojimbo::ConservativePacketHeaderBits 
	+ yojimbo::ConservativeMessageHeaderBits
) / 8;

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

	using yojimbo::Message::Acquire;
	using yojimbo::Message::Release;
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

	using yojimbo::Message::Acquire;
	using yojimbo::Message::Release;
};

template <bool C>
struct initial_arena_state_payload;

namespace net_messages {
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

	struct special_client_request : public yojimbo::Message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		template <typename Stream>
		bool Serialize(Stream& stream);

		::special_client_request payload;

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();

		bool write_payload(const ::special_client_request&);
		bool read_payload(::special_client_request&);
	};

	struct new_server_vars : preserialized_message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		bool write_payload(const server_vars&);
		bool read_payload(server_vars&);
	};

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

#if CONTEXTS_SEPARATE
	struct prestep_client_context : yojimbo::Message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		::prestep_client_context payload;

		template <typename Stream>
		bool Serialize(Stream& stream);

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();

		bool write_payload(const ::prestep_client_context&);
		bool read_payload(::prestep_client_context&);
	};
#endif

	struct server_step_entropy : preserialized_message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		bool write_payload(::networked_server_step_entropy&);
		bool read_payload(::networked_server_step_entropy&);

		using yojimbo::Message::Acquire;
		using yojimbo::Message::Release;
	};

	struct client_entropy : preserialized_message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		bool write_payload(total_client_entropy&);
		bool read_payload(total_client_entropy&);
	};

	using all_t = type_list<
		client_welcome*,
		new_server_vars*,
		initial_arena_state*,
		//initial_steps_correction*,
#if CONTEXTS_SEPARATE
		prestep_client_context*,
#endif
		server_step_entropy*,
		client_entropy*,
		special_client_request*
	>;
	
	using id_t = type_in_list_id<all_t>;
}
