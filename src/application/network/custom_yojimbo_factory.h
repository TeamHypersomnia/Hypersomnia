#pragma once

class GameMessageFactory : public yojimbo::MessageFactory {

public: 
	using id_t = net_messages::id_t;

	GameMessageFactory(yojimbo::Allocator & allocator): MessageFactory(allocator, id_t::max_index_v) {}

	yojimbo::Message * CreateMessageInternal(const int type) {
		yojimbo::Allocator & allocator = GetAllocator();
		(void) allocator;

		if (static_cast<id_t::index_type>(type) >= id_t::max_index_v) {
			return nullptr;
		}

		id_t id;
		id.set_index(static_cast<id_t::index_type>(type));

		return id.dispatch(
			[&](auto* e) -> yojimbo::Message* {
				using E = remove_cptr<decltype(e)>;

				const auto message = YOJIMBO_NEW(allocator, E);

				if (!message) {
					return nullptr;
				}

				SetMessageType(message, type);

				return message;
			}
		);
	}
};
