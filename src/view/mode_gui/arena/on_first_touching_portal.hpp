#pragma once

template <class E, class F>
void on_first_touching_portal(
	const E& viewed_character,
	F callback
) {
	if (viewed_character.dead()) {
		return;
	}

	if (const auto rigid_body = viewed_character.template find<components::rigid_body>()) {
		for (auto ce = rigid_body.get_contact_list(); ce != nullptr; ce = ce->next) {
			auto ct = ce->contact;

			if (ct == nullptr) {
				continue;
			}

			if (!ct->IsTouching()) {
				continue;
			}

			if (ce->other) {
				const auto contacted_entity = viewed_character.get_cosmos()[ce->other->GetUserData()];

				if (contacted_entity.dead()) {
					continue;
				}

				if (const auto portal = contacted_entity.template find<components::portal>()) {
					(void)portal;

					callback(contacted_entity);
					return;
				}
			}
		}
	}
}

