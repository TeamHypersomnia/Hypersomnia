#include <algorithm>
#include <cstring>
#include <vector>
#include <iterator>
#include <cwctype>
#include <tuple>
#include <type_traits>
#include <optional>
#include <utility>
#include <variant>

#include "augs/string/string_templates.h"
#include "augs/misc/scope_guard.h"
#include "augs/ensure.h"
#include "augs/gui/formatted_string.h"

namespace augs {
	namespace gui {
		namespace text {
			namespace {
				enum class tag_type {
					COLOR,

					COUNT,
					INVALID = COUNT,
				};

				enum class tag_position {
					START,
					MIDDLE, // asterix inside [list]
					END,

					COUNT,
					INVALID = COUNT,
				};

				struct tag_data {
					tag_type type = tag_type::INVALID;
					tag_position position = tag_position::INVALID;
				};

				using input_iterator_t = std::string::const_iterator;

				std::tuple<tag_data, input_iterator_t> parse_tag(const input_iterator_t first, const input_iterator_t last) {
					tag_data result;

					auto current_token = first;

					if(current_token != last && *current_token == '/') {
						result.position = tag_position::END;
						++current_token;
					} else {
						result.position = tag_position::START;
					}

					const auto tag_name_token_end = std::find_if_not(current_token, last, std::iswalpha);

					struct name_type_corresp {
						std::string_view name;
						tag_type type;
					};

					// constexpr // msvc 14.1: some implementation from C++17 is missing
					thread_local const name_type_corresp corresp_table[] = {
						{"color", tag_type::COLOR},
					};

					static_assert(std::size(corresp_table) == std::size_t(tag_type::COUNT));

					for(const auto corresp_entry : corresp_table) {
						if(std::equal(
							current_token, tag_name_token_end,
							corresp_entry.name.begin(), corresp_entry.name.end(),
							[](const auto left, const auto right) {
								return std::tolower(left) == right;
							}
						)) {
							result.type = corresp_entry.type;
							return std::make_tuple(result, tag_name_token_end);
						}
					}

					return std::make_tuple(tag_data{}, first);
				}

				std::tuple<rgba_channel, input_iterator_t> parse_color_component(const input_iterator_t first, const input_iterator_t last) {
					// hh (h - hex digit)

					rgba_channel result_channel = 0;

					auto current_token = first;
					for(auto i = 0; i != 2; ++i) {
						result_channel <<= 4;

						if(current_token == last) {
							return { 0, first };
						}

						const auto digit = *current_token++;

						if(std::iswdigit(digit)) {
							result_channel += rgba_channel(digit - '0');
							continue;
						}

						const auto lower_case_digit = std::tolower(digit);

						if(lower_case_digit < 'a' || lower_case_digit > 'f') {
							return { 0, first };
						}

						result_channel += rgba_channel(10 + (lower_case_digit - 'a'));
					}

					return { result_channel, current_token };
				}

				std::tuple<rgba, input_iterator_t> parse_color(const input_iterator_t first, const input_iterator_t last) {
					const auto alpha_token_end = std::find_if_not(first, last, std::iswalpha);
					if(first != alpha_token_end) {
						// named color

						struct name_color_corresp {
							std::string_view name;
							rgba color;
						};

						// constexpr
						thread_local const name_color_corresp corresp_table[] = {
							// standard CSS colors
							{"maroon", maroon},
							{"red", red},
							{"orange", orange},
							{"yellow", yellow},
							{"olive", olive},
							{"purple", purple},
							{"fuchsia", fuchsia},
							{"white", white},
							{"lime", lime},
							{"green", green},
							{"navy", navy},
							{"blue", blue},
							{"aqua", aqua},
							{"teal", teal},
							{"black", black},
							{"silver", silver},
							{"gray", gray},
							{"gray4", gray4},
							{"lightgreen", lightgreen},
							{"lightred", lightred},
							{"lightyellow", lightyellow},

							// custom colors
							{"vsyellow", vsyellow},
							{"vsblue", vsblue},
							{"vscyan", vscyan},
							{"cyan", cyan},
							{"vsgreen", vsgreen},
							{"vsdarkgray", vsdarkgray},
							{"vslightgray", vslightgray},
							{"violet", violet},
							{"pink", pink},
							{"ltblue", ltblue},
							{"turquoise", turquoise},
						};

						const auto corresp_entry_iter = std::find_if(
							std::begin(corresp_table),
							std::end(corresp_table),
							[=](const auto& corresp_entry) {
								return std::equal(
									first, alpha_token_end,
									corresp_entry.name.begin(), corresp_entry.name.end(),
									[](const auto left, const auto right) {
										return std::tolower(left) == right;
									}
								);
							}
						);

						if (corresp_entry_iter == std::end(corresp_table)) {
							return { rgba(), first };
						}

						return { corresp_entry_iter->color, alpha_token_end };
					}


					// #rrggbb

					auto current_token = first;

					// #
					if (current_token == last || *current_token != '#') {
						return { rgba(), first };
					}

					++current_token;

					rgba result_color;

					for (
						rgba_channel* channel = std::addressof(result_color.r); 
						channel <= std::addressof(result_color.b);
					   	++channel
					) {
						decltype(current_token) component_token_end;
						std::tie(*channel, component_token_end) = parse_color_component(current_token, last);

						if (component_token_end == current_token) {
							return { rgba(), first };
						}

						current_token = component_token_end;
					};

					return { result_color, current_token };
				}

				struct format_alteration {
					std::optional<decltype(style::font)> font;
					std::optional<decltype(style::color)> color;

					template <typename F>
					static void foreach_member(F f) {
						f(&format_alteration::font, &style::font);
						f(&format_alteration::color, &style::color);
					};

					void apply_to(style& st) const {
						foreach_member([&](const auto member, const auto style_member) {
							if (this->*member == std::nullopt) {
								return;
							}

							st.*style_member = *(this->*member);
						});
					}
				};
			} // anonymous namespace

			formatted_string from_bbcode(
				const std::string& input_str_,
				const style default_style
			) {
				formatted_string result;
				result.reserve(input_str_.size());

				auto current_style = default_style;

				struct tag_entry {
					tag_data tag;
					format_alteration alteration;
				};

				thread_local std::vector<tag_entry> tag_stack;
				tag_stack.clear();

				const auto append_with_current_style = [&](const auto first, const auto last) {
					std::transform(first, last, std::back_inserter(result), [&](const char character) -> formatted_char {
						return { current_style, character };
					});
				};

				for (auto current_token = input_str_.begin(), input_end = input_str_.end(); current_token != input_str_.end(); ) {
					{
						// parse normal text
						const auto text_end = std::find(current_token, input_end, '[');
						append_with_current_style(current_token, text_end);

						current_token = text_end;
					}

					if (current_token == input_end) {
						break;
					}

					// parse whole tag

					auto rollback_to_raw_parse_guard = augs::scope_guard([&, saved_token = current_token]() {
						current_token = saved_token;
						const auto brace_token_end = std::next(current_token);
						append_with_current_style(current_token, brace_token_end);
						current_token = brace_token_end;
					});

					++current_token; // skip '['

					using token_type = decltype(current_token);

					// parse openning/closing tag name
					tag_data current_tag;
					{
						token_type tag_name_token_end;
						std::tie(current_tag, tag_name_token_end) = parse_tag(current_token, input_end);

						if (tag_name_token_end == current_token) {
							continue;
						}

						current_token = tag_name_token_end;
					}

					// in the end of successful tag parsing we will need to apply gathered request

					struct add_entry_action {
						tag_entry entry;
					};

					struct remove_entry_action {
						decltype(tag_stack.crend()) entry;
					};

					std::variant<
						std::monostate,
						add_entry_action,
						remove_entry_action
					> final_action_variant;

					switch(current_tag.position) {
						case tag_position::START: {
							switch(current_tag.type) {
								case tag_type::COLOR: {
									// =color

									// =
									if(current_token == input_end || *current_token != '=') {
										continue;
									}

									++current_token;

									// color
									rgba color;
									{
										token_type color_token_end;
										std::tie(color, color_token_end) = parse_color(current_token, input_end);
										if(color_token_end == current_token) {
											continue;
										}

										current_token = color_token_end;
									}

									add_entry_action add_entry_action;
									add_entry_action.entry.tag = current_tag;
									add_entry_action.entry.alteration.color = color;
									final_action_variant = add_entry_action;
									break;
								}

								default: {
									ensure(false); // not implemented
									break;
								}
							}
							break;
						}

						case tag_position::MIDDLE: {
							ensure(false); // not implemented
							break;
						}

						case tag_position::END: {
							const auto matching_stack_entry = std::find_if(
								tag_stack.crbegin(), tag_stack.crend(),
								[&] (const auto& entry) {
									return entry.tag.type == current_tag.type;
								}
							);

							if(matching_stack_entry == tag_stack.crend()) {
								continue;
							}

							remove_entry_action remove_entry_action;
							remove_entry_action.entry = matching_stack_entry;
							final_action_variant = remove_entry_action;
							break;
						}

						default: {
							ensure(false);
							break;
						}
					}

					// ]
					if(current_token == input_end || *current_token != ']') {
						continue;
					}
					++current_token;

					// tag parse success

					std::visit([&] (const auto& variant) {
						using variant_t = remove_cref<decltype(variant)>;

						if constexpr(std::is_same_v<variant_t, add_entry_action>) {
							const auto& entry = variant.entry;
							entry.alteration.apply_to(current_style);
							tag_stack.push_back(entry);
						}
						else if constexpr(std::is_same_v<variant_t, remove_entry_action>) {
							const auto& entry_to_remove = variant.entry;

							format_alteration rollback_alteration;
							format_alteration::foreach_member([&](const auto member, const auto style_member) {
								if(entry_to_remove->alteration.*member == std::nullopt) {
									return; // our entry doesn't affect this field
								}

								const auto overriding_entry = std::find_if(
									tag_stack.crbegin(), entry_to_remove,
									[&] (const auto& entry) {
										return entry.alteration.*member != std::nullopt;
									}
								);

								if(overriding_entry != entry_to_remove) {
									return; // field was overridden by later entries
								}

								const auto overridden_entry = std::find_if(
									std::next(entry_to_remove), tag_stack.crend(),
									[&] (const auto& entry) {
										return entry.alteration.*member != std::nullopt;
									}
								);

								if(overridden_entry != tag_stack.crend()) {
									rollback_alteration.*member = overridden_entry->alteration.*member;
									return;
								}

								rollback_alteration.*member = default_style.*style_member;
							});

							rollback_alteration.apply_to(current_style);
							tag_stack.erase(std::next(entry_to_remove).base()); // revert reverse iterator
						} else {
							static_assert(std::is_same_v<variant_t, std::monostate>);
						}
					}, final_action_variant);

					rollback_to_raw_parse_guard.release();
				}

				return result;
			}
		}
	}
}
