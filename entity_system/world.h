#pragma once
#include <unordered_map>
#include <vector>

#include "entity.h"
#include "type_registry.h"

#include "utility/sorted_vector.h"
#include "utility/timer.h"

namespace augmentations {
	namespace entity_system {
		class entity;

		class processing_system;
		class world {
			friend class entity_ptr;

			struct message_queue {
				virtual void validate_delayed_messages() = 0;
				virtual void clear() = 0;
				virtual void clear_delayed() = 0;
				virtual bool empty() = 0;
				virtual void purify(entity* invalidated_subject) = 0;
				virtual ~message_queue() {}
			};

			template <typename message>
			struct templated_message_queue : public message_queue {
				struct delayed_message {
					util::timer was_sent;
					float post_after_ms;

					message msg;
				};

				std::vector<delayed_message> delayed_messages;
				std::vector<message> messages;

				void validate_delayed_messages() override {
					delayed_messages.erase(std::remove_if(delayed_messages.begin(), delayed_messages.end(), [this](const delayed_message& m){
						if (m.was_sent.get<std::chrono::milliseconds>() >= m.post_after_ms) {
							messages.push_back(m.msg);
							return true;
						}

						return false;
					}), delayed_messages.end());
				}

				void clear() override { messages.clear(); }
				void clear_delayed() override { delayed_messages.clear(); }
				bool empty() override { return messages.empty();  }
				void purify(entity* invalidated_subject) override {
					messages.erase(std::remove_if(messages.begin(), messages.end(), [invalidated_subject](const message& m){ return m.subject == invalidated_subject; }), messages.end());
					delayed_messages.erase(std::remove_if(delayed_messages.begin(), delayed_messages.end(), [invalidated_subject](const delayed_message& m){ return m.msg.subject == invalidated_subject; }), delayed_messages.end());
				}

				~templated_message_queue() override {}
			};

			boost::object_pool<entity> entities;
			
			std::unordered_map<size_t, std::unique_ptr<message_queue>> input_queue;
			std::unordered_map<size_t, boost::pool<>> size_to_container;
			std::unordered_map<entity*, util::sorted_vector<entity_ptr*>> registered_entity_watchers;


			std::vector<processing_system*> all_systems;
			std::unordered_map<size_t, processing_system*> hash_to_system;

			void register_entity_watcher(entity_ptr&);
			void unregister_entity_watcher(entity_ptr&);
		public:
			boost::pool<>& get_container_for_size(size_t size);
			boost::pool<>& get_container_for_type(type_hash hash);
			boost::pool<>& get_container_for_type(const base_type& type);
			
			type_registry component_library;

			template <typename T>
			void register_message_queue() {
				for (int i = 0; i < 1; ++i)
					input_queue.emplace(std::make_pair(typeid(T).hash_code(),
						std::unique_ptr<message_queue>(static_cast <message_queue*>(new templated_message_queue<T>()))));
			}

			template <typename T>
			std::vector<T>& get_message_queue() {
				return (static_cast<templated_message_queue<T>*>(input_queue.at(typeid(T).hash_code()).get()))->messages;
			}

			template <typename T>
			void post_message(const T& message_object) {
				return (static_cast<templated_message_queue<T>*>(input_queue.at(typeid(T).hash_code()).get()))->messages.push_back(message_object);
			}
			
			template <typename T>
			void post_delayed_message(const T& message_object, float delay_ms) {
				templated_message_queue<T>::delayed_message msg;
				msg.msg = message_object;
				msg.post_after_ms = delay_ms;
				msg.was_sent.reset();

				return (static_cast<templated_message_queue<T>*>(input_queue.at(typeid(T).hash_code()).get()))->delayed_messages.push_back(msg);
			}

			std::vector<processing_system*>& get_all_systems() {
				return all_systems;
			}

			void purify_queues(entity* invalidated_subject);
			void validate_delayed_messages();

			template <class T>
			void register_system(T* new_system) {
				bool such_a_system_found = false;
				for (auto it : all_systems) {
					if (it == new_system) {
						such_a_system_found = true;
						break;
					}
				}

				if (!such_a_system_found)
					all_systems.push_back(new_system);
				/*
				here we register systems' signatures so we can ensure that whenever we add a component it is already registered
				of course entities must be created AFTER the systems are specified and added
				*/
				new_system->components_signature
					= signature_matcher_bitset(component_library.register_types(new_system->get_needed_components()));

				/* register to enable by-type system retrieval */
				hash_to_system[typeid(T).hash_code()] = new_system;
			}
			
			template<class T>
			T& get_system() {
				const auto& info = typeid(T);
				auto it = hash_to_system.find(info.hash_code());
				if (it == hash_to_system.end()) 
					throw std::runtime_error((std::string(info.name()) + std::string(" not found in entity_system::world.")).c_str());
				
				return *static_cast<T*>(hash_to_system.at(info.hash_code()));
			}

			world();
			~world();

			entity& create_entity_named(std::string name);
			entity& create_entity();
			void delete_entity(entity&, entity* redirect_pointers = nullptr);
			
			void delete_all_entities(bool clear_systems_manually);

			void flush_message_queues();
			void flush_delayed_message_queues();
		};
	}
}