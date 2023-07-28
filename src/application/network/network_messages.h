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
#include "application/setups/server/server_vars.h"
#include "view/mode_gui/arena/arena_player_meta.h"
#include "game/common_state/entity_flavours.h"
#include "application/setups/server/public_settings_update.h"
#include "application/setups/server/request_arena_file_download.h"
#include "game/modes/session_id.h"
#include "application/network/net_serialize.h"
#include "application/network/download_progress_message.h"

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

struct server_vars;
struct server_public_vars;

template <class B>
struct basic_preserialized_message : public yojimbo::Message {
	static_assert(is_constant_size_vector_v<B>);

	B bytes;

	template <typename Stream>
	bool Serialize(Stream& stream) {
		int length = 0;

		if (Stream::IsWriting) {
			length = static_cast<int>(bytes.size());
		}

		serialize_int(stream, length, 0, bytes.max_size());

		if (Stream::IsReading) {
			bytes.resize(length);
		}

		serialize_bytes(stream, (uint8_t*)bytes.data(), length);

		return true;
	}

	YOJIMBO_MESSAGE_BOILERPLATE();
};

template <class T>
struct preserialized_message_type_for {
	/* 
		To properly get a bound on its maximum size,
		this type needs to be trivially copyable.
	*/
	static_assert(std::is_trivially_copyable_v<T>);
	static constexpr std::size_t max_size_v = sizeof(T);

	using buffer_type = augs::constant_size_vector<std::byte, max_size_v>;
	using type = basic_preserialized_message<buffer_type>;
};

template <class T>
using preserialized_message_type_for_t = typename preserialized_message_type_for<T>::type;

template <class P>
struct net_message_with_payload : yojimbo::Message {
	P payload;

	template <typename Stream>
	bool Serialize(Stream& stream) {
		if (!net_messages::serialize(stream, payload)) {
			return false;
		}

		return true;
	}

	inline bool read_payload(
		P& output
	) {
		output = std::move(payload);
		return true;
	}

	inline bool write_payload(
		const P& input
	) {
		payload = input;
		return true;
	}

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
struct full_arena_snapshot_payload;

namespace net_messages {
	struct client_welcome : net_message_with_payload<requested_client_settings> {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;
	};

	struct public_settings_update : net_message_with_payload<::public_settings_update> {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;
	};

	//struct initial_steps_correction : only_block_message {};

#if CONTEXTS_SEPARATE
	struct prestep_client_context : net_message_with_payload<::prestep_client_context> {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;
	};
#endif

	struct server_step_entropy : net_message_with_payload<networked_server_step_entropy> {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;
	};

	struct client_entropy : net_message_with_payload<total_client_entropy> {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;
	};

	struct new_server_runtime_info : only_block_message {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		bool read_payload(
			server_runtime_info&
		);

		template <class F>
		bool write_payload(
			F block_allocator,
			const server_runtime_info&
		);
	};

	struct new_server_public_vars : preserialized_message_type_for_t<server_public_vars> {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		bool write_payload(const server_public_vars&);
		bool read_payload(server_public_vars&);
	};

	struct new_server_vars : preserialized_message_type_for_t<server_vars> {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;

		bool write_payload(const server_vars&);
		bool read_payload(server_vars&);
	};

	struct special_client_request : net_message_with_payload<::special_client_request> {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;
	};

	struct rcon_command : net_message_with_payload<::rcon_command_variant> {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;
	};

	struct client_requested_chat : net_message_with_payload<::client_requested_chat> {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;
	};

	struct server_broadcasted_chat : net_message_with_payload<::server_broadcasted_chat> {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;
	};

	struct net_statistics_update : net_message_with_payload<::net_statistics_update> {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;
	};

	struct full_arena_snapshot : only_block_message {
		bool read_payload(
			augs::serialization_buffers&,
			const cosmos_solvable_significant& clean_round_state,
			full_arena_snapshot_payload<false>
		);

		template <class F>
		bool write_payload(
			F block_allocator,
			augs::serialization_buffers&,
			const cosmos_solvable_significant& clean_round_state,
			const all_entity_flavours& all_flavours,
			full_arena_snapshot_payload<true>
		);
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

	struct request_arena_file_download : net_message_with_payload<::request_arena_file_download> {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;
	};

	struct file_download_link : net_message_with_payload<::file_download_link_payload> {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;
	};

	struct file_download : net_message_with_payload<::file_download_payload> {
		static constexpr bool server_to_client = true;
		static constexpr bool client_to_server = false;
	};

	struct file_chunks_request : net_message_with_payload<::file_chunks_request_payload> {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;
	};

	struct download_progress_message : net_message_with_payload<::download_progress_message> {
		static constexpr bool server_to_client = false;
		static constexpr bool client_to_server = true;
	};

	using all_t = type_list<
		client_welcome*,
		public_settings_update*, 
		new_server_vars*,
		new_server_public_vars*,
		new_server_runtime_info*,
		full_arena_snapshot*,
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
		player_avatar_exchange*,
		request_arena_file_download*,
		file_download*,
		file_download_link*,
		download_progress_message*,
		file_chunks_request*
	>;
	
	using id_t = type_in_list_id<all_t>;
}
