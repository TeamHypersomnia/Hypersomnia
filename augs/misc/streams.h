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

		void set_write_pos(const size_t new_pos) {
			write_pos = new_pos;
		}

		void set_read_pos(const size_t new_pos) {
			read_pos = new_pos;
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
		bool has_read_failed = false;

		bool failed() const {
			return has_read_failed;
		}

		bool operator==(const stream&) const;
		bool operator!=(const stream&) const;

		char* data();
		const char* data() const;

		char& operator[](const size_t);
		const char& operator[](const size_t) const;

		std::string to_string() const;

		size_t capacity() const;

		template<class T>
		T peek() const {
			return *reinterpret_cast<const T*>(buf.data() + read_pos);
		}

		std::string format_as_uchars() const;

		template <class Archive>
		void write_with_properties(Archive& ar) const {
			augs::write_object(ar, buf);
			augs::write_object(ar, has_read_failed);
			augs::write_object(ar, write_pos);
			augs::write_object(ar, read_pos);
		}

		template <class Archive>
		void read_with_properties(Archive& ar) {
			augs::read_object(ar, buf);
			augs::read_object(ar, has_read_failed);
			augs::read_object(ar, write_pos);
			augs::read_object(ar, read_pos);
		}

		void read(char* const data, const size_t bytes);
		void write(const char* const data, const size_t bytes);
		void write(const augs::stream&);
		void reserve(const size_t);
		void reserve(const output_stream_reserver&);
	};

	class output_stream_reserver : public stream_position {
	public:
		stream make_stream();

		void write(const char* const data, const size_t bytes);
	};
}

namespace augs {
	template<class... Args>
	void write_object(augs::stream& ar, const augs::stream& storage) {
		ar.write(storage);
	}

	template<class A, class... Args>
	void write_stream_with_properties(A& ar, const augs::stream& storage, Args... args) {
		storage.write_with_properties(ar);
	}

	template<class A, class... Args>
	auto read_stream_with_properties(A& ar, augs::stream& storage, Args... args) {
		return storage.read_with_properties(ar);
	}

	template<class A, class... Args>
	void write_sized_stream(A& ar, const augs::stream& storage, Args... args) {
		ensure(storage.get_read_pos() == 0);
		write_object(ar, storage.buf);
	}

	template<class A, class... Args>
	auto read_sized_stream(A& ar, augs::stream& storage, Args... args) {
		auto result = read_object(ar, storage.buf);
		storage.set_write_pos(storage.buf.size());

		return result;
	}
}