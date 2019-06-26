#pragma once
#include "augs/readwrite/byte_readwrite_traits.h"
#include "application/setups/server/public_settings_update.h"

namespace net_messages {
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

	template <class Stream, unsigned buffer_size>
	bool serialize_fixed_size_str(Stream& stream, augs::constant_size_string<buffer_size>& str) {
		const auto s = str.data();

		int length = 0;
		if ( Stream::IsWriting )
		{
			length = str.size();
		}

		serialize_int( stream, length, 0, buffer_size - 1 );
		serialize_bytes( stream, (uint8_t*)s, length );

		if ( Stream::IsReading ) {
			s[length] = '\0';
		}

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
		return 
			serialize(s, c.author)
			&& serialize_enum(s, c.target)
			&& serialize_fixed_size_str(s, c.message)
		;
	}

	template <class Stream>
	bool serialize(Stream& s, ::net_statistics_update& c) {
		return serialize_vector_uint8_t(s, c.ping_values, 0, max_mode_players_v);
	}

	template <class Stream>
	bool serialize(Stream& s, rcon_commands::special& c) {
		return serialize_enum(s, c);
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

}
