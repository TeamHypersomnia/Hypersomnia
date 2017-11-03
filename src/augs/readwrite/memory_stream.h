#pragma once
#include <vector>
#include "augs/ensure.h"
#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/templates/exception_templates.h"

namespace augs {
	class memory_stream_reserver;

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

		std::size_t size() const {
			return write_pos;
		}
	};

	class memory_stream : public stream_position {
		std::vector<std::byte> buffer;

	public:
		memory_stream() = default;
		
		memory_stream(memory_stream&&) = default;
		memory_stream(const memory_stream&) = default;

		memory_stream& operator=(const memory_stream&) = default;
		memory_stream& operator=(memory_stream&&) = default;

		memory_stream(std::vector<std::byte>&& new_buffer);
		memory_stream& operator=(std::vector<std::byte>&& new_buffer);

		std::size_t mismatch(const memory_stream&) const;

		bool operator==(const memory_stream&) const;
		bool operator!=(const memory_stream&) const;

		std::byte* data();
		const std::byte* data() const;

		std::byte& operator[](const std::size_t);
		const std::byte& operator[](const std::size_t) const;

		std::size_t capacity() const;

		template<class T>
		T peek() const {
			return *reinterpret_cast<const T*>(buffer.data() + read_pos);
		}

		template <class Archive>
		void write_with_properties(Archive& ar) const {
			augs::write_bytes(ar, buffer);
			augs::write_bytes(ar, write_pos);
			augs::write_bytes(ar, read_pos);
		}

		template <class Archive>
		void read_with_properties(Archive& ar) {
			augs::read_bytes(ar, buffer);
			augs::read_bytes(ar, write_pos);
			augs::read_bytes(ar, read_pos);
		}

		void read(std::byte* const data, const std::size_t bytes);
		void write(const std::byte* const data, const std::size_t bytes);
		void write(const augs::memory_stream&);
		void reserve(const std::size_t);
		void reserve(const memory_stream_reserver&);

		operator std::vector<std::byte>&&() && {
			return std::move(buffer);
		}
	};

	class memory_stream_reserver : public stream_position {
	public:
		memory_stream create_reserved_stream();

		void write(const std::byte* const data, const std::size_t bytes);
	};

	template <class T>
	std::vector<std::byte> to_bytes(const T& object) {
		memory_stream s;
		augs::write_bytes(s, object);
		return std::move(s);
	}
}

namespace augs {
	template<class A>
	void write_stream_with_properties(A& ar, const augs::memory_stream& storage) {
		storage.write_with_properties(ar);
	}

	template <class A>
	void read_stream_with_properties(A& ar, augs::memory_stream& storage) {
		storage.read_with_properties(ar);
	}

	template <class A>
	void write_stream_with_size(A& ar, const augs::memory_stream& storage) {
		ensure(storage.get_read_pos() == 0);
		augs::write_bytes(ar, storage.size());
		
		detail::write_raw_bytes(ar, storage.data(), storage.size());
	}

	template <class A>
	void read_stream_with_size(A& ar, augs::memory_stream& storage) {
		std::size_t s;

		augs::read_bytes(ar, s);
		
		storage.reserve(s);
		storage.set_write_pos(s);
		
		detail::read_raw_bytes(ar, storage.data(), storage.size());
	}
}