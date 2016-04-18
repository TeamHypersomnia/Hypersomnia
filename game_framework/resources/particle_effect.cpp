#include "particle_effect.h"

namespace resources {
	void emission::apply_modifier(particle_effect_modifier m) {
		for (auto& p : particle_templates) {
			p.face.color *= m.colorize;
		}
	}
}