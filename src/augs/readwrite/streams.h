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
	public:
		std::size_t mismatch(const stream&) const;

		bool operator==(const stream&) const;
		bool operator!=(const stream&) const;

		std::byte* data();
		const std::byte* data() const;

		std::byte& operator[](const std::size_t);
		const std::byte& operator[](const std::size_t) const;

		std::string to_string() const;

		std::size_t capacity() const;

		template<class T>
		T peek() const {
			return *reinterpret_cast<const T*>(buf.data() + read_pos);
		}

		std::string format_as_uchars() const;

		template <class Archive>
		void write_with_properties(Archive& ar) const {
			augs::write(ar, buf);
			augs::write(ar, has_read_failed);
			augs::write(ar, write_pos);
			augs::write(ar, read_pos);
		}

		template <class Archive>
		void read_with_properties(Archive& ar) {
			augs::read(ar, buf);
			augs::read(ar, has_read_failed);
			augs::read(ar, write_pos);
			augs::read(ar, read_pos);
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
}

#if READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

namespace augs {
	template <class...>
	void read_object(augs::stream& ar, augs::stream& storage) {
		static_assert(false, "Reading a stream from a stream is ill-formed.");
	}

	void write_object(augs::stream& ar, const augs::stream& storage);

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
		write(ar, storage.size());
		write_n(ar, storage.data(), storage.size());
	}

	template <class A>
	void read_stream_with_size(A& ar, augs::stream& storage) {
		std::size_t s;

		read(ar, s);
		
		storage.reserve(s);
		storage.set_write_pos(s);
		
		read_n(ar, storage.data(), storage.size());
	}
}