#pragma once
#include "appearing_text.h"

struct creators_screen {
	using formatted_string = augs::gui::text::formatted_string;
	using style = augs::gui::text::style;

	struct entry {
		appearing_text task;
		std::vector<appearing_text> personae;

		void set_task(const formatted_string f) {
			task.target_text[0] = f;
			task.population_interval = 100.f;
			task.should_disappear = false;
		}

		void add_person(const formatted_string f) {
			appearing_text p;

			p.target_text[0] = f;
			p.population_interval = 100.f;
			p.should_disappear = false;

			personae.push_back(p);
		}

		vec2i get_personae_bbox() const {
			vec2i result;

			for (const auto& p : personae) {
				const auto p_bbox = get_text_bbox(p.get_total_target_text(), 0u);

				result.x = std::max(p_bbox.x, result.x);
				result.y += get_text_bbox(format(L"\n.", p.get_total_target_text()[0]), p_bbox.y).y;
			}

			return result;
		}
	};

	int personae_width = 0;
	int tasks_width = 0;
	int column_height = 0;

	std::vector<entry> entries;
	appearing_text afterword;

	void add_entry(entry e) {
		e.task.target_pos.set(0.f, column_height);

		const auto t_bbox = get_text_bbox(e.task.get_total_target_text(), 0u);

		tasks_width = std::max(t_bbox.x, tasks_width);

		for (auto& p : e.personae) {
			p.target_pos.set(0.f, column_height);

			const auto p_bbox = get_text_bbox(p.get_total_target_text(), 0u);

			personae_width = std::max(p_bbox.x, personae_width);
			column_height += get_text_bbox(format(L"\n.", p.get_total_target_text()[0]), p_bbox.y).y / 2;
		}

		column_height += 15;

		entries.push_back(e);
	}

	void center_all(const vec2i screen_size) {
		for (auto& e : entries) {
			e.task.target_pos.y += screen_size.y / 2.f - column_height / 2.f;
			e.task.target_pos.x = screen_size.x / 2.f - tasks_width - 60.f;

			for (auto& p : e.personae) {
				p.target_pos.y += screen_size.y / 2.f - column_height / 2.f;
				p.target_pos.x = screen_size.x / 2.f + 60.f;
			}
		}
	}

	void setup(const style person_style, const style task_style, const vec2i screen_size) {
		{
			entry c;

			c.set_task(format(L"Founder & Programmer", task_style));
			c.add_person(format(L"Patryk B. Czachurski", person_style));

			add_entry(c);
		}

		{
			entry c;

			c.set_task(format(L"Linux port", task_style));
			c.add_person(format(L"Adam Piekarczyk", person_style));

			add_entry(c);
		}

		{
			entry c;

			c.set_task(format(L"Pixel art", task_style));
			c.add_person(format(L"Michal Kawczynski", person_style));
			c.add_person(format(L"Patryk B. Czachurski", person_style));

			add_entry(c);
		}

		{
			entry c;

			c.set_task(format(L"Occasional helping hands", task_style));
			c.add_person(format(L"DaTa-", person_style));
			c.add_person(format(L"Bartosz P. Grzelak", person_style));

			add_entry(c);
		}

		center_all(screen_size);

		afterword.target_text[0] = format(L"\
What stands before your eyes is an outcome of a man'person_style burning passion,\n\
a digital inamorata, chef d'oeuvre of a single coder, masterful musicians and a champion at pixel art.\n\n", person_style);

		afterword.target_text[1] = format(L"\
Its history of making recounts profound hopes,\n\
disillusions, crises and divine moments of joy.\n\
Cherish your ambitions for the immaterial.\n\
In the end you will either conquer that which you dreamed of,\n\
or tell a beautiful story of a man devastated by struggle.\n", person_style)
+ format(L"    ~Founder of the Hypersomnia Universe", task_style);

		afterword.target_pos.set(screen_size.x / 2 - get_text_bbox(afterword.get_total_target_text(), 0u).x / 2, screen_size.y / 2 + column_height + 50);

		afterword.should_disappear = false;
		afterword.population_interval = 50.f;
		afterword.population_variation = 0.7f;
	}

	void push_into(augs::action_list& into) {
		for (auto& e : entries) {
			augs::action_list entry_acts;

			augs::action_list task_acts;
			augs::action_list personae_acts;

			e.task.push_actions(task_acts);

			for (auto& p : e.personae) {
				augs::action_list person_acts;

				p.push_actions(person_acts);

				personae_acts.push_non_blocking(act(new augs::list_action(std::move(person_acts))));
			}

			entry_acts.push_non_blocking(act(new augs::list_action(std::move(task_acts))));
			entry_acts.push_non_blocking(act(new augs::list_action(std::move(personae_acts))));

			into.push_blocking(act(new augs::list_action(std::move(entry_acts))));
		}

		afterword.push_actions(into);

		augs::action_list disappearance_acts;

		for (auto& e : entries) {
			e.task.push_disappearance(disappearance_acts, false);

			for (auto& p : e.personae) {
				p.push_disappearance(disappearance_acts, false);
			}
		}

		into.push_blocking(act(new augs::list_action(std::move(disappearance_acts))));

		afterword.push_disappearance(into);
	}

	void draw(augs::renderer& renderer) {
		for (auto& e : entries) {
			e.task.draw(renderer.get_triangle_buffer());

			for (auto& p : e.personae) {
				p.draw(renderer.get_triangle_buffer());
			}
		}

		afterword.draw(renderer.get_triangle_buffer());
	}
};