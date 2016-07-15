#pragma once
#include "stroke.h"
#include "appearance_detector.h"

namespace augs {
	namespace gui {
		struct stylesheet : appearance_detector {
			template <class T>
			struct attribute {
				bool active;
				T value;

				attribute(const T& value) : value(value), active(false) {}

				void set(const T& v) {
					value = v;
					active = true;
				}

				attribute& operator=(const T& v) {
					set(v);
					return *this;
				}

				attribute& operator=(const attribute& attr) {
					if (attr.active) {
						value = attr.value;
						active = true;
					}
					return *this;
				}

				operator T() const {
					return value;
				}
			};

			struct style {
				attribute<rgba> color;
				attribute<assets::texture_id> background_image;
				attribute<solid_stroke> border;

				style();
				style(const attribute<rgba>& color,
					const attribute<assets::texture_id>& background_image,
					const attribute<solid_stroke>& border);

				operator material() const;

			} released, hovered, pushed, focused;

			stylesheet(const style& released = style(),
				const style& hovered = style(),
				const style& pushed = style(),
				const style& focused = style());

			style get_style() const;
		};
	}
}