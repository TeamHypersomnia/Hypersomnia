#pragma once
#include "3rdparty/yojimbo/yojimbo.h"
#undef write_bytes
#undef read_bytes

#include "augs/misc/constant_size_vector.h"
#include "game/modes/mode_entropy.h"
#include "augs/misc/serialization_buffers.h"
#include "application/network/server_step_entropy.h"
#include "application/network/special_client_request.h"
#include "application/network/rcon_command.h"
#include "application/setups/server/chat_structs.h"
#include "application/setups/server/net_statistics_update.h"
#include "view/mode_gui/arena/arena_player_meta.h"
#include "game/common_state/entity_flavours.h"
#include "application/setups/server/public_settings_update.h"
#include "game/modes/session_id.h"

#define LOG_NET_SERIALIZATION !IS_PRODUCTION_BUILD

struct cosmos_solvable_significant;

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

#define YOJIMBO_MESSAGE_BOILERPLATE() YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS(); \
	using yojimbo::Message::Acquire; \
	using yojimbo::Message::Release;

constexpr std::size_t max_message_size_v = ((chosen_packet_size_v - total_header_bytes_v) / 4) * 4;

using message_bytes_type = augs::constant_size_vector<std::byte, max_message_size_v>;

struct server_vars;
struct server_solvable_vars;

struct preserialized_message : public yojimbo::Message {
	message_bytes_type bytes;

	template <typename Stream>
	bool Serialize(Stream& stream);

	YOJIMBO_MESSAGE_BOILERPLATE();
};

struct only_block_message : public yojimbo::BlockMessage {
	static constexpr bool server_to_client = true;
	static constexpr bool client_to_server = false;

	template <typename Stream>
	bool Serialize(Stream& stream) {
		(void)stream;
		return true;
	}

	YOJIMBO_MESSAGE_BOILERPLATE();
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

	struct public_settings_update : public yojimbo::Message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		::public_settings_update payload;

		template <typename Stream>
		bool Serialize(Stream& stream);

		bool write_payload(const ::public_settings_update&);
		bool read_payload(::public_settings_update&);

		YOJIMBO_MESSAGE_BOILERPLATE();
	};

	struct special_client_request : public yojimbo::Message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		template <typename Stream>
		bool Serialize(Stream& stream);

		::special_client_request payload;

		bool write_payload(const ::special_client_request&);
		bool read_payload(::special_client_request&);

		YOJIMBO_MESSAGE_BOILERPLATE();
	};

	struct new_server_solvable_vars : preserialized_message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		bool write_payload(const server_solvable_vars&);
		bool read_payload(server_solvable_vars&);
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
			const cosmos_solvable_significant& initial_signi,
			initial_arena_state_payload<false>
		);

		template <class F>
		bool write_payload(
			F block_allocator,
			augs::serialization_buffers&,
			const cosmos_solvable_significant& initial_signi,
			const all_entity_flavours& all_flavours,
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

		bool write_payload(const ::prestep_client_context&);
		bool read_payload(::prestep_client_context&);

		YOJIMBO_MESSAGE_BOILERPLATE();
	};
#endif

	struct server_step_entropy : preserialized_message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		bool write_payload(::networked_server_step_entropy&);
		bool read_payload(::networked_server_step_entropy&);
	};

	struct client_entropy : preserialized_message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		bool write_payload(total_client_entropy&);
		bool read_payload(total_client_entropy&);
	};

	struct rcon_command : public yojimbo::Message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		template <typename Stream>
		bool Serialize(Stream& stream);

		::rcon_command_variant payload;

		bool write_payload(const ::rcon_command_variant&);
		bool read_payload(::rcon_command_variant&);

		YOJIMBO_MESSAGE_BOILERPLATE();
	};

	struct client_requested_chat : public yojimbo::Message {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;

		template <typename Stream>
		bool Serialize(Stream& stream);

		::client_requested_chat payload;

		bool write_payload(const ::client_requested_chat&);
		bool read_payload(::client_requested_chat&);

		YOJIMBO_MESSAGE_BOILERPLATE();
	};

	struct server_broadcasted_chat : public yojimbo::Message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		template <typename Stream>
		bool Serialize(Stream& stream);

		::server_broadcasted_chat payload;

		bool write_payload(const ::server_broadcasted_chat&);
		bool read_payload(::server_broadcasted_chat&);

		YOJIMBO_MESSAGE_BOILERPLATE();
	};

	struct net_statistics_update : public yojimbo::Message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		template <typename Stream>
		bool Serialize(Stream& stream);

		::net_statistics_update payload;

		bool write_payload(const ::net_statistics_update&);
		bool read_payload(::net_statistics_update&);

		YOJIMBO_MESSAGE_BOILERPLATE();
	};

	struct player_avatar_exchange : only_block_message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = true;

		bool read_payload(
			session_id_type& session_id,
			arena_player_avatar_payload&
		);

		template <class F>
		bool write_payload(
			F block_allocator,
			const session_id_type& session_id,
			const arena_player_avatar_payload&
		);
	};

	using all_t = type_list<
		client_welcome*,
		public_settings_update*, 
		new_server_vars*,
		new_server_solvable_vars*,
		initial_arena_state*,
		//initial_steps_correction*,
#if CONTEXTS_SEPARATE
		prestep_client_context*,
#endif
		server_step_entropy*,
		client_entropy*,
		special_client_request*,

		rcon_command*,
		client_requested_chat*,
		server_broadcasted_chat*,
		net_statistics_update*,
		player_avatar_exchange*
	>;
	
	using id_t = type_in_list_id<all_t>;
}
