#pragma once
#include "augs/readwrite/memory_stream.h"
#include "augs/misc/serialization_buffers.h"
#include "augs/misc/compress.h"
#include "augs/misc/readable_bytesize.h"
#include "augs/templates/logically_empty.h"
#include "application/network/net_serialization_helpers.h"

template <bool C>
struct initial_arena_state_payload {
	maybe_const_ref_t<C, cosmos_solvable_significant> signi;
	maybe_const_ref_t<C, online_mode_and_rules> mode;
	maybe_const_ref_t<C, uint32_t> client_id;
};

using ref_net_stream = augs::basic_ref_memory_stream<message_bytes_type>;
using cref_net_stream = augs::basic_ref_memory_stream<const message_bytes_type>;

template <class T, class P>
bool unsafe_read_message(
	T& msg,
	P& payload
) {
	auto s = cref_net_stream(msg.bytes);
	augs::read_bytes(s, payload);

	return true;
}

template <class T, class P>
bool unsafe_write_message(
	T& msg,
	const P& payload
) {
	auto s = ref_net_stream(msg.bytes);
	augs::write_bytes(s, payload);

	return true;
}

constexpr std::size_t max_server_step_size_v = 
	max_message_size_v 
	- yojimbo::ConservativeMessageHeaderBits / 8
;

namespace net_messages {
	template <class Stream>
	bool serialize(Stream& s, ::prestep_client_context& c) {
		serialize_int(s, c.num_entropies_accepted, 0, 255);
		return true;
	}

	template <class Stream>
	bool serialize(Stream&, mode_restart_command&) {
		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, mode_commands::team_choice& c) {
		return serialize_enum(s, c);
	}

	template <class Stream>
	bool serialize(Stream& s, mode_commands::item_purchase& c) {
		return serialize_trivial_as_bytes(s, c);
	}

	template <class Stream>
	bool serialize(Stream& s, mode_commands::spell_purchase& c) {
		return serialize_trivial_as_bytes(s, c);
	}

	template <class Stream>
	bool serialize(Stream& s, mode_commands::special_purchase& c) {
		return serialize_enum(s, c);
	}

	template <class Stream, class... Args>
	bool serialize(Stream& s, std::variant<Args...>& i) {
		static constexpr auto max_i = sizeof...(Args);
		static_assert(std::numeric_limits<uint8_t>::max() >= max_i);

		uint8_t type_id;

		if (Stream::IsWriting) {
			type_id = static_cast<uint8_t>(i.index());
		}

		serialize_int(s, type_id, 0, sizeof...(Args) - 1);

		type_in_list_id<type_list<Args*...>> idx;
		idx.set_index(type_id);

		const bool result = idx.dispatch(
			[&](auto* dummy) {
				using T = std::remove_pointer_t<decltype(dummy)>;

				if (Stream::IsReading) {
					i.template emplace<T>();
				}

				if constexpr(!is_monostate_v<T>) {
					serialize(s, std::get<T>(i));
				}

				return true;
			}
		);

		return result;
	}

	template <class Stream>
	bool serialize(Stream& s, ::mode_player_id& i) {
		static const auto max_val = mode_player_id::machine_admin();
		serialize_int(s, i.value, 0, max_val.value);
		serialize_align(s);
		return true;
	}

	template <class Stream>
	bool serialize_stdstring(Stream& s, std::string& e, const int mini, const int maxi) {
		auto length = static_cast<int>(e.length());

		serialize_int(s, length, mini, maxi);

		if (Stream::IsReading) {
			e.resize(length);
		}

		serialize_bytes(s, (uint8_t*)e.data(), length);
		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, wielding_setup& p) {
		return serialize_trivial_as_bytes(s, p);
	}

	template <class Stream>
	bool serialize(Stream& s, add_player_input& p) {
		return 
			serialize(s, p.id)
			&& serialize_stdstring(s, p.name, min_nickname_length_v, max_nickname_length_v)
			&& serialize_enum(s, p.faction)
		;
	}

	template <class Stream>
	bool serialize(Stream& s, total_mode_player_entropy& p) {
		auto& m = p.mode;
		auto& c = p.cosmic;

		bool has_mode_command = logically_set(m);

		bool has_cast_spell = logically_set(c.cast_spell);
		bool has_wield = logically_set(c.wield);
		bool has_intents = logically_set(c.intents);
		bool has_motions = logically_set(c.motions);
		bool has_transfer = logically_set(c.transfer);

		serialize_bool(s, has_mode_command);

		serialize_bool(s, has_cast_spell);
		serialize_bool(s, has_wield);
		serialize_bool(s, has_intents);
		serialize_bool(s, has_motions);
		serialize_bool(s, has_transfer);

		serialize_align(s);

		if (has_mode_command) {
			if (!serialize(s, m)) {
				return false;
			}
		}

		if (has_cast_spell) {
			if (!serialize_trivial_as_bytes(s, c.cast_spell)) {
				return false;
			}
		}

		if (has_wield) {
			if (!serialize(s, c.wield)) {
				return false;
			}
		}

		if (has_intents) {
			auto& ints = c.intents;
			auto num_intents = static_cast<uint8_t>(ints.size());

			/*
				6 bits for the number of intents

				1 bit for whether it was pressed or released
				currently 5 bits for the intent id

				= ~6 bits per intent
			*/

			serialize_int(s, num_intents, 1, 64);

			if (Stream::IsReading) {
				ints.resize(num_intents);
			}

			static_assert(int(game_intent_type::COUNT) < 128);

			for (auto& i : ints) {
				auto pressed = i.change == intent_change::PRESSED;
				auto intent = int(i.intent);

				serialize_bool(s, pressed);
				serialize_int(s, intent, 0, int(game_intent_type::COUNT) - 1);

				i.change = pressed ? intent_change::PRESSED : intent_change::RELEASED;
				i.intent = static_cast<game_intent_type>(intent);
			}

			serialize_align(s);
		}

		if (has_motions) {
			static_assert(int(game_motion_type::COUNT) == 1);

			/* 
				TODO BANDWIDTH: 
				use shorts and let the server multiply these by the sensitivity. 
			*/

			auto& mot = c.motions[game_motion_type::MOVE_CROSSHAIR];

			serialize_float(s, mot.x);
			serialize_float(s, mot.y);
		}

		if (has_transfer) {
			if (!serialize_trivial_as_bytes(s, c.transfer)) {
				return false;
			}
		}

		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, ::networked_server_step_entropy& total_networked) {
		auto& i = total_networked.payload;
		auto& g = i.general;

#if !CONTEXTS_SEPARATE
		if (!serialize(s, total_networked.context)) {
			return false;
		}
#endif

		auto& state_hash = total_networked.meta.state_hash;
		bool has_state_hash = logically_set(state_hash);

		serialize_bool(s, has_state_hash);

		bool has_players = logically_set(i.players);
		bool has_added_player = logically_set(g.added_player);
		bool has_removed_player = logically_set(g.removed_player);
		bool has_special_command = logically_set(g.special_command);

		serialize_bool(s, has_players);
		serialize_bool(s, has_added_player);
		serialize_bool(s, has_removed_player);
		serialize_bool(s, has_special_command);

		serialize_bool(s, total_networked.meta.reinference_required);

		serialize_align(s);

		if (has_state_hash) {
			if (state_hash == std::nullopt) {
				state_hash.emplace();
			}

			serialize_uint32(s, *state_hash);
		}
		else {
			state_hash = std::nullopt;
		}

		if (has_players) {
			auto& p = i.players;
			auto cnt = static_cast<int>(p.size());

			serialize_int(s, cnt, 1, max_incoming_connections_v + 1);

			if (Stream::IsReading) {
				p.resize(cnt);
			}

			for (auto& pp : p) {
				if (!serialize(s, pp.player_id)) {
					return false;
				}

				if (!serialize(s, pp.total)) {
					return false;
				}
			}
		}

		if (has_added_player) {
			if (!serialize(s, g.added_player)) {
				return false;
			}
		}

		if (has_removed_player) {
			if (!serialize(s, g.removed_player)) {
				return false;
			}
		}

		if (has_special_command) {
			if (!serialize(s, g.special_command)) {
				return false;
			}
		}

		return true;
	}

	template <class B, class T>
	bool safe_write(B& bytes, T& payload) {
		auto s = yojimbo::WriteStream(yojimbo::GetDefaultAllocator(), (uint8_t*)bytes.data(), bytes.size());

		if (!serialize(s, payload)) {
			return false;
		}

		s.Flush();
		bytes.resize(s.GetBytesProcessed());

		return true;
	}

	template <class B, class T>
	bool safe_read(const B& bytes, T& payload) {
		auto s = yojimbo::ReadStream(yojimbo::GetDefaultAllocator(), (const uint8_t*)bytes.data(), bytes.size());
		return serialize(s, payload);
	}

#if CONTEXTS_SEPARATE
	inline bool prestep_client_context::write_payload(const ::prestep_client_context& input) {
		payload = input;
		return true;
	}

	inline bool prestep_client_context::read_payload(::prestep_client_context& output) {
		output = payload;
		return true;
	}
#endif

	inline bool server_step_entropy::read_payload(::networked_server_step_entropy& output) {
		return safe_read(bytes, output);
	}

	inline bool server_step_entropy::write_payload(::networked_server_step_entropy& input) {
		bytes.resize(max_server_step_size_v);
		return safe_write(bytes, input);
	}

	inline bool client_entropy::read_payload(
		total_client_entropy& output
	) {
		return safe_read(bytes, output);
	}

	inline bool client_entropy::write_payload(
		total_client_entropy& input
	) {
		bytes.resize(max_message_size_v);
		return safe_write(bytes, input);
	}

	inline bool client_welcome::read_payload(
		decltype(client_welcome::payload)& output
	) {
		output = std::move(payload);
		return true;
	}

	inline bool client_welcome::write_payload(
		const decltype(client_welcome::payload)& input
	) {
		payload = input;
		return true;
	}

	inline bool special_client_request::read_payload(
		decltype(special_client_request::payload)& output
	) {
		output = std::move(payload);
		return true;
	}

	inline bool special_client_request::write_payload(
		const decltype(special_client_request::payload)& input
	) {
		payload = input;
		return true;
	}

	inline bool new_server_vars::read_payload(
		server_vars& output
	) {
		// TODO SECURITY: don't blindly trust the server!!!
		// TODO BANDWIDTH: optimize vars i/o

		return unsafe_read_message(*this, output);
	}

	inline bool new_server_vars::write_payload(
		const server_vars& input
	) {
		return unsafe_write_message(*this, input);
	}

	inline bool initial_arena_state::read_payload(
		augs::serialization_buffers& buffers,
		const initial_arena_state_payload<false> in
	) {
		const auto data = reinterpret_cast<const std::byte*>(GetBlockData());
		const auto size = static_cast<std::size_t>(GetBlockSize());

		NSR_LOG("RECEIVING INITIAL STATE");

		NSR_LOG("Compressed stream size: %x", size);

		if (size < sizeof(uint32_t)) {
			return false;
		}

		const auto uncompressed_size = *reinterpret_cast<const uint32_t*>(data);
	
		NSR_LOG("Uncompressed size: %x", uncompressed_size);

		/*
			TODO: validate uncompressed_size with some predefined max solvable size.
		*/

		if (uncompressed_size > 100 * 1024 * 1024) {
			return false;
		}

		auto& uncompressed_buf = buffers.serialization;
		uncompressed_buf.resize(uncompressed_size);

		try {
			augs::decompress(
				data + sizeof(uint32_t),
				size - sizeof(uint32_t),
				uncompressed_buf
			);

			NSR_LOG("Successfully uncompressed the initial state.");
		}
		catch (const augs::decompression_error& err) {
			LOG("Failed to decompress the initial state. Server might be malicious.");
			LOG(err.what());

			return false;
		}

		auto s = augs::cref_memory_stream(uncompressed_buf);

		augs::read_bytes(s, in.signi);
		augs::read_bytes(s, in.mode);
		augs::read_bytes(s, in.client_id);

		NSR_LOG_NVPS(in.client_id);

		return true;
	}

	inline const std::vector<std::byte>* initial_arena_state::write_payload(
		augs::serialization_buffers& buffers,
		const initial_arena_state_payload<true> in
	) {
		auto write_all_to = [&in](auto& s) {
			augs::write_bytes(s, in.signi);
			augs::write_bytes(s, in.mode);
			augs::write_bytes(s, in.client_id);
		};

		NSR_LOG("SENDING INITIAL STATE");

		{
			NSR_LOG("STAGE: ESTIMATION");

			augs::byte_counter_stream s;
			write_all_to(s);
			buffers.serialization.reserve(s.size());

			NSR_LOG("Reserved size: %x", s.size());

			{
				auto s = buffers.make_serialization_stream();
				write_all_to(s);
			}

			NSR_LOG("Result stream length: %x", buffers.serialization.size());
		}

		auto& c = buffers.compressed;

		{
			NSR_LOG("STAGE: COMPRESSION");

			c.clear();

			{
				auto s = augs::ref_memory_stream(c);
				const auto uncompressed_size = static_cast<uint32_t>(buffers.serialization.size());
				augs::write_bytes(s, uncompressed_size);

				NSR_LOG("Uncompressed size: %x", uncompressed_size);
			}

			augs::compress(buffers.compression_state, buffers.serialization, c);

			NSR_LOG("Compressed stream size: %x", c.size());
		}

		return std::addressof(c);
	}
}
