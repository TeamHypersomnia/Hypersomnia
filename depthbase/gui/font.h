#pragma once
#include <unordered_map>
#include <memory>
#include "../io/texture.h"

struct FT_LibraryRec_;
typedef struct FT_LibraryRec_  *FT_Library;
struct FT_Glyph_Metrics_;
typedef FT_Glyph_Metrics_ FT_Glyph_Metrics;

namespace db {
	namespace graphics {
		namespace gui {
			namespace text {
				struct drafter;
				struct printer;
			}

			class font_in {
				std::unique_ptr<FT_Library> library;
				friend struct font_file;
				bool inited;
			public:
				font_in();
				~font_in();

				bool init(), deinit();
			};

			struct font_file {
				struct glyph {
					io::image img;
					int adv, bear_x, bear_y;
					math::rect_wh size;

					std::vector<std::pair<unsigned, int> > kerning;
					
					glyph();
					glyph(int adv, int bear_x, int bear_y, rect_wh size);
					glyph(const FT_Glyph_Metrics&);
				private:
					friend struct font_file;
					unsigned int index, unicode;
				};
				
				std::vector<glyph> glyphs;
				std::unordered_map<unsigned, unsigned> unicode;

				font_file();
				
				typedef std::vector<std::pair<wchar_t, wchar_t> > charset;
				void create(std::pair<wchar_t, wchar_t> range);
				void create(const charset& ranges);
				void create(const std::wstring& characters);
				 
				bool open(font_in&, const char* filename, unsigned pt, std::pair<wchar_t, wchar_t> range);
				bool open(font_in&, const char* filename, unsigned pt, const charset& ranges);

				bool open(font_in&, const char* filename, unsigned _pt, const std::wstring& characters);
				
				glyph* get_glyphs(), *get_glyph(unsigned unicode);
				unsigned get_count() const, get_pt() const, get_height() const;

				void free_images(), destroy();
			private:
//				font_file(const font_file&) = delete;
				charset to_charset(const std::wstring&);
				friend struct text::drafter;
				friend struct text::printer;
				friend struct font;
				int ascender, descender;
				unsigned pt;
			};
			
			struct font {
				struct glyph {
					font_file::glyph* info;
					io::input::texture tex;
					glyph();
				};
				font *bold, *italics, *bi, *regular;

				font(); ~font();
				void build(font_file*), add_to_atlas(io::input::atlas&), destroy();

				glyph* get_glyph(unsigned unicode);
				font_file* get_font();

				void set_styles(font* bold, font* italics, font* bi);

				bool can_be_bolded();
				bool can_be_italicsed();
				bool is_bolded();
				bool is_italicsed();

				/* shortcut to get_parent()->get_height()*/
				unsigned get_height();
				
				font_file* get_parent();
				
				font* get_bold(bool flag);
				font* get_italics(bool flag);

			private:
				friend struct text::drafter;
				friend struct text::printer;
				glyph *glyphs;
				font_file* parent;
			};
		}
	}
}