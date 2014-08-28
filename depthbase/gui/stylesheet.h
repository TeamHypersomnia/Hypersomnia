#pragma once
#include "stroke.h"
#include "rect.h"

namespace augs {
	namespace graphics {
		namespace gui {
			struct stylesheet {

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
						if(attr.active) { 
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
					attribute<pixel_32> color;
					attribute<augs::texture_baker::texture*> background_image;
					attribute<solid_stroke> border;

					style();
					style(const attribute<pixel_32>& color, 
						const attribute<augs::texture_baker::texture*>& background_image, 
						const attribute<solid_stroke>& border);

					operator material() const;

				} released, hovered, pushed, focused;

				stylesheet(const style& released = style(), 
						   const style& hovered = style(), 
						   const style& pushed = style(),
						   const style& focused = style());

				rect::appearance current_appearance;

				void update_appearance(rect::event);
				style get_style() const;

			private:
				bool focus_flag;
			};
		}
	}
}