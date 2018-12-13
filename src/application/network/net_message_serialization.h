#pragma once

template <typename Stream>
bool preserialized_message::Serialize(Stream& stream) {
	int length = 0;

	if ( Stream::IsWriting ) {
		length = (int) bytes.size();
	}

	serialize_int( stream, length, 0, max_message_size_v );
	serialize_bytes( stream, (uint8_t*)bytes.data(), length );

	return true;
}

namespace net_messages {
	template <typename Stream>
	bool client_welcome::Serialize(Stream& stream) {
		serialize_string(stream, payload.chosen_nickname, requested_client_settings::buf_len);

		serialize_float(stream, payload.public_settings.mouse_sensitivity.x);
		serialize_float(stream, payload.public_settings.mouse_sensitivity.y);

		serialize_uint32(stream, payload.net.jitter.buffer_ms);
		serialize_uint32(stream, payload.net.jitter.merge_commands_when_above_ms);

		return true;
	}
}
