#pragma once
#include <vector>
#include "augs/ensure.h"
#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/templates/exception_templates.h"

namespace augs {
	class output_stream_reserver;

	struct stream_read_error : error_with_typesafe_sprintf {
		using error_with_typesafe_sprintf::error_with_typesafe_sprintf;
	};

	class stream_position {
	protected:
		std::size_t read_pos = 0;
		std::size_t write_pos = 0;
	public:

		std::size_t get_write_pos() const {
			return write_pos;
		}

		std::size_t get_read_pos() const {
			return read_pos;
		}

		std::size_t get_unread_bytes() const {
			ensure(read_pos <= write_pos);
			return write_pos - read_pos;
		}

		void set_write_pos(const std::size_t new_pos) {
			write_pos = new_pos;
		}

		void set_read_pos(const std::size_t new_pos) {
			read_pos = new_pos;
		}

		void reset_write_pos() {
			write_pos = 0;
		}

		void reset_read_pos() {
			read_pos = 0;
		}

		std::size_t size() const {
			return write_pos;
		}
	};

	class stream : public stream_position {
		std::vector<std::byte> buf;

		template <class T>
		friend std::vector<std::byte> to_bytes(const T& object);
	public:

		std::size_t mismatch(const stream&) const;

		bool operator==(const stream&) const;
		bool operator!=(const stream&) const;

		std::byte* data();
		const std::byte* data() const;

		std::byte& operator[](const std::size_t);
		const std::byte& operator[](const std::size_t) const;

		std::size_t capacity() const;

		template<class T>
		T peek() const {
			return *reinterpret_cast<const T*>(buf.data() + read_pos);
		}

		template <class Archive>
		void write_with_properties(Archive& ar) const {
			augs::write_bytes(ar, buf);
			augs::write_bytes(ar, write_pos);
			augs::write_bytes(ar, read_pos);
		}

		template <class Archive>
		void read_with_properties(Archive& ar) {
			augs::read_bytes(ar, buf);
			augs::read_bytes(ar, write_pos);
			augs::read_bytes(ar, read_pos);
		}

		void read(std::byte* const data, const std::size_t bytes);
		void write(const std::byte* const data, const std::size_t bytes);
		void write(const augs::stream&);
		void reserve(const std::size_t);
		void reserve(const output_stream_reserver&);
	};

	class output_stream_reserver : public stream_position {
	public:
		stream make_stream();

		void write(const std::byte* const data, const std::size_t bytes);
	};

	template <class T>
	std::vector<std::byte> to_bytes(const T& object) {
		stream s;
		augs::write_bytes(s, object);
		return std::move(s.buf);
	}
}

#if READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	template<class A>
	void write_stream_with_properties(A& ar, const augs::stream& storage) {
		storage.write_with_properties(ar);
	}

	template <class A>
	void read_stream_with_properties(A& ar, augs::stream& storage) {
		storage.read_with_properties(ar);
	}

	template <class A>
	void write_stream_with_size(A& ar, const augs::stream& storage) {
		ensure(storage.get_read_pos() == 0);
		augs::write_bytes(ar, storage.size());
		
		detail::write_raw_bytes(ar, storage.data(), storage.size());
	}

	template <class A>
	void read_stream_with_size(A& ar, augs::stream& storage) {
		std::size_t s;

		augs::read_bytes(ar, s);
		
		storage.reserve(s);
		storage.set_write_pos(s);
		
		detail::read_raw_bytes(ar, storage.data(), storage.size());
	}
}