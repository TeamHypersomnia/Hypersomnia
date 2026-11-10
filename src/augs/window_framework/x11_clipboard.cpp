#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <xcb/xcb.h>
#include "augs/log.h"
#include "augs/window_framework/x11_clipboard.h"

namespace augs {
	namespace x11_clipboard {
		constexpr auto paste_timeout = std::chrono::milliseconds(1000);
		constexpr auto clipboard_manager_timeout = std::chrono::milliseconds(1000);
		constexpr std::uint32_t max_property_words = 1u << 24;
		constexpr std::size_t request_header_bytes = 64;
		constexpr std::size_t xcb_event_size_v = 32;

		template <class T>
		struct freed {
			void operator()(T* const p) const {
				std::free(p);
			}
		};

		template <class T>
		using xcb_reply_ptr = std::unique_ptr<T, freed<T>>;

		struct clipboard_atoms {
			xcb_atom_t clipboard = XCB_ATOM_NONE;
			xcb_atom_t utf8_string = XCB_ATOM_NONE;
			xcb_atom_t text = XCB_ATOM_NONE;
			xcb_atom_t targets = XCB_ATOM_NONE;
			xcb_atom_t incr = XCB_ATOM_NONE;
			xcb_atom_t transfer_property = XCB_ATOM_NONE;
			xcb_atom_t clipboard_manager = XCB_ATOM_NONE;
			xcb_atom_t save_targets = XCB_ATOM_NONE;
			xcb_atom_t wake = XCB_ATOM_NONE;
		};

		class x11_selection_owner {
			xcb_connection_t* connection = nullptr;
			xcb_window_t window = XCB_WINDOW_NONE;
			clipboard_atoms atoms;

			std::mutex paste_lk;
			std::mutex state_lk;
			std::condition_variable state_changed;

			std::string owned_text;
			bool owns_clipboard = false;

			bool receiving_incrementally = false;
			std::string received_bytes;
			std::optional<std::string> pasted;
			bool clipboard_manager_answered = false;

			std::atomic<bool> quitting = false;
			std::thread event_thread;

			xcb_atom_t intern(const char* const name) {
				const auto cookie = xcb_intern_atom(connection, 0, static_cast<std::uint16_t>(std::strlen(name)), name);
				const auto reply = xcb_reply_ptr<xcb_intern_atom_reply_t>(xcb_intern_atom_reply(connection, cookie, nullptr));

				return reply != nullptr ? reply->atom : XCB_ATOM_NONE;
			}

			void flush() {
				xcb_flush(connection);
			}

			template <class Event>
			void send_to(const xcb_window_t destination, const Event& event) {
				static_assert(sizeof(Event) <= xcb_event_size_v);

				auto whole_event = std::array<char, xcb_event_size_v>();
				std::memcpy(whole_event.data(), std::addressof(event), sizeof(Event));

				xcb_send_event(connection, 0, destination, XCB_EVENT_MASK_NO_EVENT, whole_event.data());
				flush();
			}

			bool fits_in_one_request(const std::string& text) const {
				const auto max_request_bytes = static_cast<std::size_t>(xcb_get_maximum_request_length(connection)) * 4;
				return text.size() + request_header_bytes <= max_request_bytes;
			}

			xcb_atom_t answer(const xcb_selection_request_event_t& request, const xcb_atom_t property) {
				if (request.selection != atoms.clipboard) {
					return XCB_ATOM_NONE;
				}

				const auto text = [&]() {
					std::scoped_lock lk(state_lk);
					return owned_text;
				}();

				if (request.target == atoms.targets) {
					const auto offered = std::array<xcb_atom_t, 4> { atoms.targets, atoms.utf8_string, atoms.text, XCB_ATOM_STRING };

					xcb_change_property(
						connection,
						XCB_PROP_MODE_REPLACE,
						request.requestor,
						property,
						XCB_ATOM_ATOM,
						32,
						static_cast<std::uint32_t>(offered.size()),
						offered.data()
					);

					return property;
				}

				const bool text_target =
					request.target == atoms.utf8_string ||
					request.target == atoms.text ||
					request.target == XCB_ATOM_STRING
				;

				if (!text_target) {
					return XCB_ATOM_NONE;
				}

				if (!fits_in_one_request(text)) {
					LOG("X11 clipboard: %x bytes are too many to paste in one request.", text.size());
					return XCB_ATOM_NONE;
				}

				const auto type = request.target == atoms.text ? atoms.utf8_string : request.target;

				xcb_change_property(
					connection,
					XCB_PROP_MODE_REPLACE,
					request.requestor,
					property,
					type,
					8,
					static_cast<std::uint32_t>(text.size()),
					text.data()
				);

				return property;
			}

			void on_selection_request(const xcb_selection_request_event_t& request) {
				const auto property = request.property == XCB_ATOM_NONE ? request.target : request.property;

				auto notify = xcb_selection_notify_event_t();
				notify.response_type = XCB_SELECTION_NOTIFY;
				notify.time = request.time;
				notify.requestor = request.requestor;
				notify.selection = request.selection;
				notify.target = request.target;
				notify.property = answer(request, property);

				send_to(request.requestor, notify);
			}

			void finish_paste(std::string text) {
				{
					std::scoped_lock lk(state_lk);
					receiving_incrementally = false;
					pasted = std::move(text);
				}

				state_changed.notify_all();
			}

			std::optional<std::string> take_transfer_property(bool& incremental) {
				const auto cookie = xcb_get_property(connection, 1, window, atoms.transfer_property, XCB_GET_PROPERTY_TYPE_ANY, 0, max_property_words);
				const auto reply = xcb_reply_ptr<xcb_get_property_reply_t>(xcb_get_property_reply(connection, cookie, nullptr));

				if (reply == nullptr) {
					return std::nullopt;
				}

				incremental = reply->type == atoms.incr;

				const auto bytes = static_cast<const char*>(xcb_get_property_value(reply.get()));
				const auto length = static_cast<std::size_t>(xcb_get_property_value_length(reply.get()));

				return std::string(bytes, length);
			}

			void on_selection_notify(const xcb_selection_notify_event_t& notify) {
				if (notify.selection == atoms.clipboard_manager) {
					{
						std::scoped_lock lk(state_lk);
						clipboard_manager_answered = true;
					}

					state_changed.notify_all();
					return;
				}

				if (notify.selection != atoms.clipboard) {
					return;
				}

				if (notify.property == XCB_ATOM_NONE) {
					const bool utf8_refused = notify.target == atoms.utf8_string;

					if (utf8_refused) {
						request_paste(XCB_ATOM_STRING);
						return;
					}

					finish_paste({});
					return;
				}

				bool incremental = false;
				const auto value = take_transfer_property(incremental);

				if (!value.has_value()) {
					finish_paste({});
					return;
				}

				if (incremental) {
					std::scoped_lock lk(state_lk);
					receiving_incrementally = true;
					received_bytes.clear();
					return;
				}

				finish_paste(*value);
			}

			void on_property_notify(const xcb_property_notify_event_t& change) {
				const bool new_chunk =
					change.window == window &&
					change.atom == atoms.transfer_property &&
					change.state == XCB_PROPERTY_NEW_VALUE
				;

				if (!new_chunk) {
					return;
				}

				{
					std::scoped_lock lk(state_lk);

					if (!receiving_incrementally) {
						return;
					}
				}

				bool incremental = false;
				const auto chunk = take_transfer_property(incremental);

				if (!chunk.has_value() || chunk->empty()) {
					finish_paste([&]() {
						std::scoped_lock lk(state_lk);
						return received_bytes;
					}());

					return;
				}

				std::scoped_lock lk(state_lk);
				received_bytes += *chunk;
			}

			void on_selection_clear(const xcb_selection_clear_event_t& clear) {
				if (clear.selection == atoms.clipboard) {
					std::scoped_lock lk(state_lk);
					owns_clipboard = false;
				}
			}

			void handle_events() {
				while (const auto raw_event = xcb_reply_ptr<xcb_generic_event_t>(xcb_wait_for_event(connection))) {
					const auto type = raw_event->response_type & ~0x80;
					const auto* const event = raw_event.get();

					switch (type) {
						case XCB_SELECTION_REQUEST:
							on_selection_request(*reinterpret_cast<const xcb_selection_request_event_t*>(event));
							break;
						case XCB_SELECTION_NOTIFY:
							on_selection_notify(*reinterpret_cast<const xcb_selection_notify_event_t*>(event));
							break;
						case XCB_PROPERTY_NOTIFY:
							on_property_notify(*reinterpret_cast<const xcb_property_notify_event_t*>(event));
							break;
						case XCB_SELECTION_CLEAR:
							on_selection_clear(*reinterpret_cast<const xcb_selection_clear_event_t*>(event));
							break;
						default:
							break;
					}

					if (quitting.load()) {
						return;
					}
				}
			}

			void request_paste(const xcb_atom_t target) {
				xcb_convert_selection(connection, window, atoms.clipboard, target, atoms.transfer_property, XCB_CURRENT_TIME);
				flush();
			}

			void hand_over_to_clipboard_manager() {
				const auto owner_cookie = xcb_get_selection_owner(connection, atoms.clipboard_manager);
				const auto owner = xcb_reply_ptr<xcb_get_selection_owner_reply_t>(xcb_get_selection_owner_reply(connection, owner_cookie, nullptr));

				if (owner == nullptr || owner->owner == XCB_WINDOW_NONE) {
					return;
				}

				const auto save_all_targets = XCB_ATOM_NONE;

				xcb_convert_selection(connection, window, atoms.clipboard_manager, atoms.save_targets, save_all_targets, XCB_CURRENT_TIME);
				flush();

				std::unique_lock lk(state_lk);
				state_changed.wait_for(lk, clipboard_manager_timeout, [this]() { return clipboard_manager_answered; });
			}

			void wake_event_thread() {
				auto wake = xcb_client_message_event_t();
				wake.response_type = XCB_CLIENT_MESSAGE;
				wake.format = 32;
				wake.window = window;
				wake.type = atoms.wake;

				send_to(window, wake);
			}

		public:
			x11_selection_owner() {
				connection = xcb_connect(nullptr, nullptr);

				if (xcb_connection_has_error(connection)) {
					LOG("X11 clipboard: could not connect to the X server - copying and pasting work only inside the game.");
					xcb_disconnect(connection);
					connection = nullptr;
					return;
				}

				const auto screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;

				window = xcb_generate_id(connection);

				const std::uint32_t event_mask = XCB_EVENT_MASK_PROPERTY_CHANGE;

				xcb_create_window(
					connection,
					XCB_COPY_FROM_PARENT,
					window,
					screen->root,
					0, 0, 1, 1,
					0,
					XCB_WINDOW_CLASS_INPUT_ONLY,
					XCB_COPY_FROM_PARENT,
					XCB_CW_EVENT_MASK,
					&event_mask
				);

				atoms.clipboard = intern("CLIPBOARD");
				atoms.utf8_string = intern("UTF8_STRING");
				atoms.text = intern("TEXT");
				atoms.targets = intern("TARGETS");
				atoms.incr = intern("INCR");
				atoms.transfer_property = intern("AUGS_CLIPBOARD_TRANSFER");
				atoms.clipboard_manager = intern("CLIPBOARD_MANAGER");
				atoms.save_targets = intern("SAVE_TARGETS");
				atoms.wake = intern("AUGS_CLIPBOARD_WAKE");

				flush();

				event_thread = std::thread([this]() { handle_events(); });
			}

			~x11_selection_owner() {
				if (connection == nullptr) {
					return;
				}

				const bool owned = [&]() {
					std::scoped_lock lk(state_lk);
					return owns_clipboard;
				}();

				if (owned) {
					hand_over_to_clipboard_manager();
				}

				quitting = true;
				wake_event_thread();

				if (event_thread.joinable()) {
					event_thread.join();
				}

				xcb_destroy_window(connection, window);
				xcb_disconnect(connection);
			}

			x11_selection_owner(const x11_selection_owner&) = delete;
			x11_selection_owner& operator=(const x11_selection_owner&) = delete;

			void set_text(const std::string& text) {
				if (connection == nullptr) {
					std::scoped_lock lk(state_lk);
					owned_text = text;
					return;
				}

				{
					std::scoped_lock lk(state_lk);
					owned_text = text;
					owns_clipboard = true;
				}

				xcb_set_selection_owner(connection, window, atoms.clipboard, XCB_CURRENT_TIME);

				const auto owner_cookie = xcb_get_selection_owner(connection, atoms.clipboard);
				const auto owner = xcb_reply_ptr<xcb_get_selection_owner_reply_t>(xcb_get_selection_owner_reply(connection, owner_cookie, nullptr));

				if (owner == nullptr || owner->owner != window) {
					LOG("X11 clipboard: could not take the clipboard.");

					std::scoped_lock lk(state_lk);
					owns_clipboard = false;
				}
			}

			std::string get_text() {
				if (connection == nullptr) {
					std::scoped_lock lk(state_lk);
					return owned_text;
				}

				std::scoped_lock one_paste_at_a_time(paste_lk);

				{
					std::scoped_lock lk(state_lk);

					if (owns_clipboard) {
						return owned_text;
					}

					pasted = std::nullopt;
					receiving_incrementally = false;
				}

				request_paste(atoms.utf8_string);

				std::unique_lock lk(state_lk);

				if (!state_changed.wait_for(lk, paste_timeout, [this]() { return pasted.has_value(); })) {
					LOG("X11 clipboard: the clipboard's owner did not answer in time.");
					receiving_incrementally = false;
					return {};
				}

				return *pasted;
			}
		};

		static x11_selection_owner& get_owner() {
			static x11_selection_owner owner;
			return owner;
		}

		void set_text(const std::string& text) {
			get_owner().set_text(text);
		}

		std::string get_text() {
			return get_owner().get_text();
		}
	}
}
