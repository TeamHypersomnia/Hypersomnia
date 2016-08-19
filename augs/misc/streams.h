#pragma once
#include <vector>
#include "augs/ensure.h"

namespace augs {
	class output_stream_reserver;

	class stream_position {
	protected:
		size_t read_pos = 0;
		size_t write_pos = 0;
	public:

		size_t get_write_pos() const {
			return write_pos;
		}

		size_t get_read_pos() const {
			return read_pos;
		}

		size_t get_unread_bytes() const {
			ensure(read_pos <= write_pos);
			return write_pos - read_pos;
		}

		void reset_write_pos() {
			write_pos = 0;
		}

		void reset_read_pos() {
			read_pos = 0;
		}

		size_t size() const {
			return write_pos;
		}
	};

	class stream : public stream_position {
	public:
		std::vector<char> buf;

		bool operator==(const stream&) const;

		char* data();
		const char* data() const;

		char& operator[](size_t);
		const char& operator[](size_t) const;

		std::string to_string() const;

		size_t capacity() const;

		void read(char* data, size_t bytes);
		void write(const char* data, size_t bytes);
		void write(const augs::stream&);
		void reserve(size_t);
		void reserve(const output_stream_reserver&);
	};

	class output_stream_reserver : public stream_position {
	public:
		stream make_stream();

		void write(const char* data, size_t bytes);
	};
}