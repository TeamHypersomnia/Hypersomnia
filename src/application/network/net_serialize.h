#pragma once
#include "augs/log.h"
#include "augs/readwrite/byte_readwrite_traits.h"
#include "augs/readwrite/byte_readwrite.h"
#include "application/setups/server/public_settings_update.h"
#include "augs/readwrite/to_bytes.h"
#include "game/modes/mode_commands/match_command.h"
#include "augs/window_framework/mouse_rel_bound.h"
#include "application/setups/server/request_arena_file_download.h"
#include "application/network/download_progress_message.h"

namespace sanitization {
	bool arena_name_safe(const std::string& untrusted_map_name);
}

namespace net_messages {
	template <class Stream, class T>
	bool unsafe_serialize(Stream& s, T& c) {
		std::vector<std::byte> bytes;

		if (Stream::IsWriting) {
			bytes = augs::to_bytes(c);
		}

		auto length = static_cast<int>(bytes.size());

		/* 
			To properly get a bound on its maximum size,
			this type needs to be trivially copyable.
		*/

		static_assert(std::is_trivially_copyable_v<T>);
		serialize_int(s, length, 0, sizeof(T));

		if (Stream::IsReading) {
			bytes.resize(length);
		}

		serialize_bytes(s, (uint8_t*)bytes.data(), length);

		try {
			augs::from_bytes(bytes, c);
		}
		catch (...) {
			return false;
		}

		return true;
	}

	template <class Stream, class V>
	bool serialize_trivial_as_bytes(Stream& s, V& v) {
		static_assert(augs::is_byte_readwrite_appropriate_v<augs::memory_stream, V>);
		serialize_bytes(s, (uint8_t*)&v, sizeof(V));
		return true;
	}

	template <class Stream, class E>
	bool serialize_enum(Stream& s, E& e) {
		auto ee = static_cast<int>(e);
		serialize_int(s, ee, 0, static_cast<int>(E::COUNT));
		e = static_cast<E>(ee);
		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, ::mode_player_id& i) {
		static const auto max_val = mode_player_id::machine_admin();
		serialize_int(s, i.value, 0, max_val.value);
		serialize_align(s);
		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, ::session_id_type& i) {
		serialize_uint32(s, i.value);
		return true;
	}

	template <class Stream, std::size_t count>
	bool serialize_fixed_byte_array(Stream& stream, std::array<uint8_t, count>& byte_array) {
		serialize_bytes(stream, byte_array.data(), count);

		return true;
	}

	template <class Stream, unsigned buffer_size>
	bool serialize_fixed_size_str(Stream& stream, augs::constant_size_string<buffer_size>& str) {
		int length = 0;
		if ( Stream::IsWriting )
		{
			length = str.size();
		}

		serialize_int( stream, length, 0, buffer_size );

		str.resize_no_init(length);
		const auto s = str.data();

		serialize_bytes(stream, reinterpret_cast<uint8_t*>(s), length);

		return true;
	}

	template <class Stream>
	bool serialize_vector_uint8_t(Stream& s, std::vector<uint8_t>& e, const int mini, const int maxi) {
		auto length = static_cast<int>(e.size());

		serialize_int(s, length, mini, maxi);

		if (Stream::IsReading) {
			e.resize(length);
		}

		serialize_bytes(s, (uint8_t*)e.data(), length);
		return true;
	}

	template <class Stream, uint32_t Max>
	bool serialize_fixed_size_vector_uint16_t(Stream& s, augs::constant_size_vector<uint16_t, Max>& e) {
		auto length = static_cast<int>(e.size());

		serialize_int(s, length, 0, Max);

		if (Stream::IsReading) {
			e.resize(length);
		}

		serialize_bytes(s, (uint8_t*)e.data(), length * sizeof(uint16_t));
		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, ::file_download_payload& c) {
		serialize_int(s, c.num_file_bytes, 1, max_direct_download_file_size_v);

		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, ::file_chunks_request_payload& c) {
		return serialize_fixed_size_vector_uint16_t(s, c.requests);
	}

	template <class Stream>
	bool serialize(Stream& s, ::file_download_link_payload& c) {
		return serialize_fixed_size_str(s, c.file_address);
	}

	template <class Stream>
	bool serialize(Stream& s, ::client_requested_chat& c) {
		if (!serialize_enum(s, c.target)) {
			LOG("FAILED TO SERIALIZE ENUM! %x", int(c.target));
			return false;
		}

			
		if (!serialize_fixed_size_str(s, c.message)) {
			LOG("FAILED TO SERIALIZE MSG! %x", int(c.target));
			return false;
		}

		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, ::server_broadcasted_chat& c) {
		if (!serialize(s, c.author)) {
			return false;
		}

		if (!serialize_enum(s, c.target)) {
			return false;
		}

		if (!serialize_enum(s, c.recipient_effect)) {
			return false;
		}

		serialize_align(s);

		return serialize_fixed_size_str(s, c.message);
	}

	template <class Stream>
	bool serialize(Stream& s, ::net_statistics_update& c) {
		return unsafe_serialize(s, c);
	}

	template <class Stream>
	bool serialize(Stream& s, rcon_commands::special& c) {
		return serialize_enum(s, c);
	}

	template <class Stream>
	bool serialize(Stream& s, server_vars& c) {
		if (!unsafe_serialize(s, c)) {
			return false;
		}

		return sanitization::arena_name_safe(c.arena);
	}

	template <class Stream>
	bool serialize(Stream& s, server_runtime_info& c) {
		if (!unsafe_serialize(s, c)) {
			return false;
		}

		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, server_public_vars& c) {
		if (!unsafe_serialize(s, c)) {
			return false;
		}

		return sanitization::arena_name_safe(c.arena);
	}

	template <class Stream>
	bool serialize(Stream& s, ::prestep_client_context& c) {
		serialize_int(s, c.num_entropies_accepted, 0, 255);
		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, ::public_client_settings& p) {
		auto& settings = p.character_input;

		serialize_float(s, settings.crosshair_sensitivity.x);
		serialize_float(s, settings.crosshair_sensitivity.y);

		serialize_bool(s, settings.keep_movement_forces_relative_to_crosshair);

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

	template <class Stream>
	bool serialize(Stream& s, match_command& c) {
		return serialize_enum(s, c);
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
					if (!serialize(s, std::get<T>(i))) {
						return false;
					}
				}

				return true;
			}
		);

		return result;
	}

	template <typename Stream>
	bool serialize(Stream& stream, ::special_client_request& payload) {
		if (!serialize_enum(stream, payload)) {
			return false;
		}

		return true;
	}

	template <typename Stream>
	bool serialize(Stream& stream, ::requested_client_settings& payload) {
		{
			if (!serialize_fixed_size_str(stream, payload.chosen_nickname)) {
				return false;
			}

			if (!serialize_fixed_size_str(stream, payload.rcon_password)) {
				return false;
			}
		}

		if (!serialize(stream, payload.public_settings)) {
			return false;
		}

		serialize_uint32(stream, payload.net.jitter.buffer_at_least_steps);
		serialize_uint32(stream, payload.net.jitter.buffer_at_least_ms);
		serialize_int(stream, payload.net.jitter.max_commands_to_squash_at_once, 0, 255);
		serialize_bool(stream, payload.suppress_webhooks);

		return true;
	}

	template <typename Stream>
	bool serialize(Stream& stream, ::request_arena_file_download& payload) {
		if (!serialize_fixed_byte_array(stream, payload.requested_file_hash)) {
			return false;
		}

		serialize_int(stream, payload.num_chunks_to_presend, 0, std::numeric_limits<uint16_t>::max());

		return true;
	}

	template <typename Stream>
	bool serialize(Stream& stream, ::download_progress_message& payload) {
		serialize_int(stream, payload.progress, 0, 255);

		return true;
	}

	template <typename Stream>
	bool serialize(Stream& stream, ::public_settings_update& payload) {
		if (!serialize(stream, payload.subject_id)) {
			return false;
		}

		if (!serialize(stream, payload.new_settings)) {
			return false;
		}

		return true;
	}

	template <class Stream>
	bool serialize(Stream& s, total_mode_player_entropy& payload) {
		auto& m = payload.mode;
		auto& c = payload.cosmic;

		auto get_motion = [&]() -> auto& {
			return c.motions[game_motion_type::MOVE_CROSSHAIR];
		};

		bool has_mode_command = logically_set(m);

		bool has_cast_spell = logically_set(c.cast_spell);
		bool has_wield = logically_set(c.wield);
		bool has_intents = logically_set(c.intents);
		bool has_motions = logically_set(c.motions);
		bool has_transfer = logically_set(c.transfer);

		auto one_byte_pred = [&](const auto& coord) {
			return coord >= -7 && coord <= 8;
		};

		auto two_byte_pred = [&](const auto& coord) {
			return coord >= -127 && coord <= 128;
		};

		bool motion_writable_in_one_byte = has_motions && one_byte_pred(get_motion().x) && one_byte_pred(get_motion().y);
		bool motion_writable_in_two_bytes = has_motions && two_byte_pred(get_motion().x) && two_byte_pred(get_motion().y);

		serialize_bool(s, has_mode_command);
		serialize_bool(s, has_cast_spell);
		serialize_bool(s, has_wield);
		serialize_bool(s, has_intents);
		serialize_bool(s, has_motions);
		serialize_bool(s, has_transfer);

		serialize_bool(s, motion_writable_in_one_byte);
		serialize_bool(s, motion_writable_in_two_bytes);

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

			const auto pos_before = s.GetBytesProcessed();

			int min_bound = 0;
			int max_bound = 0;

			if (motion_writable_in_one_byte) {
				min_bound = -7;
				max_bound = 8;
			}
			else if (motion_writable_in_two_bytes) {
				min_bound = -127;
				max_bound = 128;
			}
			else {
				min_bound = mouse_rel_min_v;
				max_bound = mouse_rel_max_v;
			}

			const auto offset = -min_bound;

			const auto min_io_bound = 0;
			const auto max_io_bound = max_bound + offset;

			if (Stream::IsReading) {
				auto& mot = get_motion();

				serialize_int(s, mot.x, min_io_bound, max_io_bound);
				serialize_int(s, mot.y, min_io_bound, max_io_bound);

				mot.x -= offset;
				mot.y -= offset;
			}
			else {
				auto mot = get_motion();

				mot.x += offset;
				mot.y += offset;

				serialize_int(s, mot.x, min_io_bound, max_io_bound);
				serialize_int(s, mot.y, min_io_bound, max_io_bound);
			}

			{
				const auto pos_after = s.GetBytesProcessed();

				if (motion_writable_in_one_byte) {
					ensure_eq(1, pos_after - pos_before);
				}
				else if (motion_writable_in_two_bytes) {
					ensure_eq(2, pos_after - pos_before);
				}
				else {
					ensure_eq(3, pos_after - pos_before);
				}
			}
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

		bool has_players = logically_set(i.players);
		bool has_added_player = logically_set(g.added_player);
		bool has_removed_player = logically_set(g.removed_player);
		bool has_special_command = logically_set(g.special_command);

		serialize_bool(s, has_state_hash);
		serialize_bool(s, has_players);
		serialize_bool(s, has_added_player);
		serialize_bool(s, has_removed_player);
		serialize_bool(s, has_special_command);

		serialize_bool(s, total_networked.meta.reinference_necessary);

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

			serialize_int(s, cnt, 1, max_mode_players_v);

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
}
