#include "image.h"
#include "augs/log.h"
#include "augs/build_settings/setting_enable_polygonization.h"

#define OFFSET_COUNT 8

namespace augs {

	struct posrgba
	{
		vec2u pos;
		rgba col;
	};


	std::vector<vec2i> image::get_polygonized() const {
		std::vector<vec2i> vertices;
#if ENABLE_POLYGONIZATION
		std::vector<bool> pixelFields;
		pixelFields.resize(size.area(), false);

		for (unsigned i = 0 ; i < size.y; ++i) {
			for (unsigned j = 0; j < size.x; ++j) {
				const rgba p = pixel({ j, i });
				if (p == red)
				{
					vertices.push_back(vec2i(j, i));
					break;
				}
			}
			if (vertices.size() != 0)
				break;
		}
		if (vertices.size() == 0)
		{
			LOG("SMH WRONG FAM");
		}
		pixelFields[vertices.back().y * size.x + vertices.back().x] = true;
		vec2i field;
		field = vertices[0];
		vec2i offsets[OFFSET_COUNT] = { vec2i(1,0),vec2i(0,1),vec2i(-1,0),vec2i(0,-1),vec2i(1,-1),vec2i(1,1),vec2i(-1,1),vec2i(-1,-1) }; //CLOCKWISE
		bool quit = false;
		do
		{
			for (int i = 0;i < OFFSET_COUNT;++i)
			{
				posrgba current;
				current.pos = field + offsets[i];
				if (current.pos.x >= size.x || current.pos.x < 0 || current.pos.y >= size.y
					|| current.pos.y < 0)
					continue;
				if (vertices.size() > 1 && current.pos == vertices[0])
				{
					quit = true;
					break;
				}
				if (pixelFields[current.pos.y * size.x + current.pos.x])
					continue;
				current.col = pixel(current.pos);
				if (current.col == black)
				{
					field = current.pos;
					pixelFields[current.pos.y * size.x + current.pos.x] = true;
					break;
				}	
				else if (current.col == red)
				{
					vertices.push_back(current.pos);
					field = current.pos;
					pixelFields[current.pos.y * size.x + current.pos.x] = true;
					break;
				}
				pixelFields[current.pos.y * size.x + current.pos.x] = true;
			}	
		}while (!quit);
#else
		vertices = { vec2i(0, 0), vec2i(size.x, 0), vec2i(size.x, size.y), vec2i(0, size.y) };
#endif
		return vertices;	
	}	

}