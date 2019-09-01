#pragma once
#include <vector>
#include "augs/ensure.h"
#include "augs/readwrite/memory_stream_declaration.h"
#include "augs/readwrite/byte_readwrite_declaration.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/resize_no_init.h"
#include "augs/readwrite/stream_read_error.h"

namespace augs {
	class byte_counter_stream;

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

	class byte_counter_stream : public stream_position {
	public:
		memory_stream create_reserved_stream();

		void write(const std::byte* const data, const std::size_t bytes) {
			write_pos += bytes;
			(void)data;
		}
	};

	template <class derived>
	class memory_stream_mixin : public stream_position {
		auto& get_buffer() {
			return static_cast<derived*>(this)->buffer;
		}

		const auto& get_buffer() const {
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

			std::memcpy(data, get_buffer().data() + read_pos, bytes);
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
			return get_buffer().data();
		}

		const std::byte* data() const {
			return get_buffer().data();
		}

		std::byte& operator[](const size_t idx) {
			return data()[idx];
		}

		const std::byte& operator[](const size_t idx) const {
			return data()[idx];
		}

		size_t capacity() const {
			return get_buffer().size();
		}

		void write(const std::byte* const data, const std::size_t bytes) {
			const auto new_write_pos = write_pos + bytes;

			if (new_write_pos > capacity()) {
				reserve(new_write_pos * 2);
			}

			std::memcpy(get_buffer().data() + write_pos, data, bytes);
			write_pos = new_write_pos;
		}

		void write(const memory_stream_mixin& s) {
			write(s.data(), s.size());
		}

		void reserve(const std::size_t bytes) {
			resize_no_init(get_buffer(), bytes);
		}

		void reserve(const byte_counter_stream& r) {
			reserve(r.size());
		}

		template<class T>
		T peek() const {
			return *reinterpret_cast<const T*>(get_buffer().data() + read_pos);
		}

		template <class Archive>
		void write_with_properties(Archive& ar) const {
			augs::write_bytes(ar, get_buffer());
			augs::write_bytes(ar, write_pos);
			augs::write_bytes(ar, read_pos);
		}

		template <class Archive>
		void read_with_properties(Archive& ar) {
			augs::read_bytes(ar, get_buffer());
			augs::read_bytes(ar, write_pos);
			augs::read_bytes(ar, read_pos);
		}
	};

	template <class B>
	class basic_memory_stream : public memory_stream_mixin<basic_memory_stream<B>> {
		using base = memory_stream_mixin<memory_stream>;
		friend base;

		B buffer;

	public:
		using base::set_write_pos;
		using base::get_write_pos;
		using base::set_read_pos;
		using base::get_read_pos;

		basic_memory_stream() = default;

		B&& extract() && {
			resize_no_init(buffer, get_write_pos());
			return std::move(buffer);
		}

		operator B&&() && {
			resize_no_init(buffer, get_write_pos());
			return std::move(buffer);
		}

		basic_memory_stream(basic_memory_stream&&) = default;
		basic_memory_stream(const basic_memory_stream&) = default;

		basic_memory_stream& operator=(const basic_memory_stream&) = default;
		basic_memory_stream& operator=(basic_memory_stream&&) = default;

		basic_memory_stream(const B& new_buffer)
			: buffer(new_buffer)
		{
			set_write_pos(buffer.size());
		}

		basic_memory_stream(B&& new_buffer)
			: buffer(std::move(new_buffer))
		{
			set_write_pos(buffer.size());
		}

		basic_memory_stream& operator=(const B& new_buffer) {
			buffer = new_buffer;
			set_write_pos(buffer.size());
			set_read_pos(0);
			return *this;
		}
		basic_memory_stream& operator=(B&& new_buffer) {
			buffer = std::move(new_buffer);
			set_write_pos(buffer.size());
			set_read_pos(0);
			return *this;
		}
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