#pragma once
#include <vector>
#include "augs/ensure.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/exception_templates.h"

namespace augs {
	class byte_counter_stream;

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

	class memory_stream;

	class byte_counter_stream : public stream_position {
	public:
		memory_stream create_reserved_stream();

		void write(const std::byte* const data, const std::size_t bytes);
	};

	template <class derived>
	class memory_stream_mixin : public stream_position {
		auto& buffer() {
			return static_cast<derived*>(this)->buffer;
		}

		const auto& buffer() const {
			return static_cast<const derived*>(this)->buffer;
		}

	public:
		using stream_position::size;

		void read(std::byte* data, const std::size_t bytes) {
			if (read_pos + bytes > size()) {
				throw stream_read_error(
					"Failed to read bytes: %x-%x (size: %x)", 
					read_pos, 
					read_pos + bytes, 
					size()
				);
			}

			std::memcpy(data, buffer().data() + read_pos, bytes);
			read_pos += bytes;
		}

		std::size_t mismatch(const memory_stream_mixin& b) const {
			return std::mismatch(data(), data() + size(), b.data()).first - data();
		}

		bool operator==(const memory_stream_mixin& b) const {
			return std::equal(data(), data() + size(), b.data());
		}

		bool operator!=(const memory_stream_mixin& b) const {
			return !operator==(b);
		}

		std::byte* data() {
			return buffer().data();
		}

		const std::byte* data() const {
			return buffer().data();
		}

		std::byte& operator[](const size_t idx) {
			return data()[idx];
		}

		const std::byte& operator[](const size_t idx) const {
			return data()[idx];
		}

		size_t capacity() const {
			return buffer().size();
		}

		void write(const std::byte* const data, const size_t bytes) {
			if (write_pos + bytes > capacity()) {
				reserve((write_pos + bytes) * 2);
			}

			std::memcpy(buffer().data() + write_pos, data, bytes);
			write_pos += bytes;
		}

		void write(const memory_stream_mixin& s) {
			write(s.data(), s.size());
		}

		void reserve(const std::size_t bytes) {
			buffer().resize(bytes);
		}

		void reserve(const byte_counter_stream& r) {
			reserve(r.size());
		}

		template<class T>
		T peek() const {
			return *reinterpret_cast<const T*>(buffer().data() + read_pos);
		}

		template <class Archive>
		void write_with_properties(Archive& ar) const {
			augs::write_bytes(ar, buffer());
			augs::write_bytes(ar, write_pos);
			augs::write_bytes(ar, read_pos);
		}

		template <class Archive>
		void read_with_properties(Archive& ar) {
			augs::read_bytes(ar, buffer());
			augs::read_bytes(ar, write_pos);
			augs::read_bytes(ar, read_pos);
		}
	};

	class memory_stream : public memory_stream_mixin<memory_stream> {
		using base = memory_stream_mixin<memory_stream>;
		friend base;

		std::vector<std::byte> buffer;

	public:
		memory_stream() = default;

		operator std::vector<std::byte>&&() && {
			buffer.resize(get_write_pos());
			return std::move(buffer);
		}

		memory_stream(memory_stream&&) = default;
		memory_stream(const memory_stream&) = default;

		memory_stream& operator=(const memory_stream&) = default;
		memory_stream& operator=(memory_stream&&) = default;

		memory_stream(const std::vector<std::byte>& new_buffer);
		memory_stream(std::vector<std::byte>&& new_buffer);
		memory_stream& operator=(const std::vector<std::byte>& new_buffer);
		memory_stream& operator=(std::vector<std::byte>&& new_buffer);
	};

	template <class B>
	class basic_ref_memory_stream : public memory_stream_mixin<basic_ref_memory_stream<B>> {
		using base = memory_stream_mixin<basic_ref_memory_stream<B>>;
		friend base;

		B& buffer;
	public:
		using base::set_write_pos;
		using base::get_write_pos;

		basic_ref_memory_stream(B& b) : buffer(b) {
			set_write_pos(b.size());
		}

		~basic_ref_memory_stream() {
			if constexpr(!is_const_ref_v<B>) {
				buffer.resize(get_write_pos());
			}
		}

		basic_ref_memory_stream(basic_ref_memory_stream&&) = delete;
		basic_ref_memory_stream& operator=(basic_ref_memory_stream&&) = delete;
	};

	template <class T>
	void to_bytes(std::vector<std::byte>& buffer, const T& object) {
		auto s = ref_memory_stream(buffer);
		augs::write_bytes(s, object);
	}

	template <class T>
	void from_bytes(const std::vector<std::byte>& bytes, T& object) {
		auto s = cref_memory_stream(bytes);
		augs::read_bytes(s, object);
	}

	template <class T>
	auto to_bytes(const T& object) {
		std::vector<std::byte> s;
		to_bytes(s, object);
		return s;
	}

	struct trivial_type_marker {};

	template <class T>
	auto from_bytes(const std::vector<std::byte>& bytes) {
		static_assert(
			!std::is_same_v<T, trivial_type_marker>,
			"Use the other overload that takes destination as argument."
		);

		T object;
		from_bytes(bytes, object);
		return object;
	}

	void from_bytes(const std::vector<std::byte>& bytes, trivial_type_marker& object);
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
		ensure_eq(storage.get_read_pos(), 0);
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