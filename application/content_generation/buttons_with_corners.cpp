#include <experimental/filesystem>

#include "buttons_with_corners.h"
#include "augs/gui/button_corners.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

namespace fs = std::experimental::filesystem;

void regenerate_buttons_with_corners() {
	const auto buttons_with_corners_directory = "generated/buttons_with_corners/";

	augs::create_directories(buttons_with_corners_directory);

	const auto lines = augs::get_file_lines("buttons_with_corners_generator_input.cfg");
	size_t current_line = 0;

	while (current_line < lines.size()) {
		button_with_corners_metadata new_meta;

		const auto target_stem = lines[current_line];
		
		++current_line;

		{
			std::istringstream in(lines[current_line]);
			in >> new_meta.border_color;
		}

		++current_line;

		{
			std::istringstream in(lines[current_line]);
			in >> new_meta.inside_color;
		}
		
		++current_line;

		{
			std::istringstream in(lines[current_line]);
			in >> new_meta.lower_side >> new_meta.upper_side >> new_meta.inside_border_padding >> new_meta.make_lb_complement;
		}
		
		++current_line;
		++current_line;

		const auto button_with_corners_filename_template = buttons_with_corners_directory + target_stem + "_%x.png";
		const auto button_with_corners_meta_filename = buttons_with_corners_directory + target_stem + ".meta";

		augs::stream new_meta_stream;
		augs::write_object(new_meta_stream, new_meta);

		bool should_regenerate = false;

		for (size_t i = 0; i < static_cast<int>(button_corner_type::COUNT); ++i) {
			if (
				!augs::file_exists(
					typesafe_sprintf(
						button_with_corners_filename_template,
						get_filename_for(static_cast<button_corner_type>(i))
					)
				)
			) {
				should_regenerate = true;
				break;
			}
		}

		if (!should_regenerate) {
			if (!augs::file_exists(button_with_corners_meta_filename)) {
				should_regenerate = true;
			}
			else {
				augs::stream existent_meta_stream;
				augs::assign_file_contents_binary(button_with_corners_meta_filename, existent_meta_stream);

				const bool are_metas_identical = (new_meta_stream == existent_meta_stream);

				if (!are_metas_identical) {
					should_regenerate = true;
				}
			}
		}

		if (should_regenerate) {
			LOG("Regenerating button with corners: %x", target_stem);

			create_and_save_button_with_corners(
				button_with_corners_filename_template,
				new_meta
			);

			augs::create_binary_file(button_with_corners_meta_filename, new_meta_stream);
		}

		// skip parameters line
		++current_line;

		// skip separating newline
		++current_line;
	}
}

void create_and_save_button_with_corners(
	const std::string& filename_template,
	const button_with_corners_metadata in
) {
	typedef augs::image img;

	const auto save = [&](const button_corner_type t, const img& im) {
		im.save(typesafe_sprintf(filename_template, get_filename_for(t)));
	};

	{
		img l;
		img t;
		img r;
		img b;

		l.create(in.lower_side, 1u);
		l.execute(img::paint_line_command{ { 1, 0 }, { in.lower_side - 1, 0 }, in.inside_color });

		r.create(in.upper_side, 1u);
		r.execute(img::paint_line_command{ { in.upper_side - 2, 0 }, { 0, 0 }, in.inside_color } );

		b.create(1u, in.lower_side);
		b.execute(img::paint_line_command{{ 0, in.lower_side - 2 }, { 0, 0 }, in.inside_color });

		t.create(1u, in.upper_side);
		t.execute(img::paint_line_command{{ 0, 1 }, { 0, in.upper_side - 1 }, in.inside_color });

		save(button_corner_type::L, l);
		save(button_corner_type::T, t);
		save(button_corner_type::R, r);
		save(button_corner_type::B, b);
	}

	{
		img lt;
		img rt;
		img rb;
		img lb;

		img lb_complement;
		img lb_complement_border;

		lt.create(in.lower_side, in.upper_side);
		lt.fill(in.inside_color);
		lt.execute(img::paint_line_command{{ 0, 0 }, { in.lower_side - 1, 0 }, { 0, 0, 0, 0 }} );
		lt.execute(img::paint_line_command{{ 0, 0 }, { 0, in.upper_side - 1 }, { 0, 0, 0, 0 }} );

		rt.create(in.upper_side, in.upper_side);
		rt.fill({ 0, 0, 0, 0 });

		for (unsigned i = 1; i < in.upper_side; ++i) {
			rt.execute(img::paint_line_command{{ 0, i }, { in.upper_side - i - 1, in.upper_side - 1 }, in.inside_color });
		}

		rb.create(in.upper_side, in.lower_side);
		rb.fill(in.inside_color);
		rb.execute(img::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { 0, in.lower_side - 1 }, { 0, 0, 0, 0 }});
		rb.execute(img::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { in.upper_side - 1, 0 }, { 0, 0, 0, 0 }});

		lb.create(in.lower_side, in.lower_side);
		lb.fill({ 0, 0, 0, 0 });

		for (unsigned i = 1; i < in.lower_side; ++i) {
			lb.execute(img::paint_line_command{ { i, 0 }, { in.lower_side - 1, in.lower_side - 1 - i }, in.inside_color });
		}

		lb_complement.create(in.lower_side, in.lower_side);

		for (unsigned i = 1; i < in.lower_side; ++i) {
			lb_complement.execute(img::paint_line_command{{ 0, i }, { in.lower_side - 1 - i, in.lower_side - 1 }, in.inside_color });
		}

		save(button_corner_type::LT, lt);
		save(button_corner_type::RT, rt);
		save(button_corner_type::RB, rb);
		save(button_corner_type::LB, lb);

		if (in.make_lb_complement) {
			save(button_corner_type::LB_COMPLEMENT, lb_complement);
		}
	}

	{
		img l;
		img t;
		img r;
		img b;

		l.create(in.lower_side, 1u);
		l.pixel({ 0, 0 }) = in.border_color;

		r.create(in.upper_side, 1u);
		r.pixel({ in.upper_side - 1, 0 }) = in.border_color;

		b.create(1u, in.lower_side);
		b.pixel({ 0, in.lower_side - 1 }) = in.border_color;

		t.create(1u, in.upper_side);
		t.pixel({ 0, 0 }) = in.border_color;

		save(button_corner_type::L_BORDER, l);
		save(button_corner_type::T_BORDER, t);
		save(button_corner_type::R_BORDER, r);
		save(button_corner_type::B_BORDER, b);
	}

	{
		img lt;
		img rt;
		img rb;
		img lb;

		img lb_complement;

		lt.create(in.lower_side, in.upper_side);
		lt.execute(img::paint_line_command{{ 0, 0 }, { 0, in.upper_side - 1 }, in.border_color});
		lt.execute(img::paint_line_command{{ 0, 0 }, { in.lower_side - 1, 0 }, in.border_color});

		rt.create(in.upper_side, in.upper_side);
		rt.fill({ 0, 0, 0, 0 });

		rt.execute(img::paint_line_command{ { 0, 0 }, { in.upper_side - 1, in.upper_side - 1 }, in.border_color });

		rb.create(in.upper_side, in.lower_side);
		rb.execute(img::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { 0, in.lower_side - 1 }, in.border_color});
		rb.execute(img::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { in.upper_side - 1, 0 }, in.border_color});

		lb.create(in.lower_side, in.lower_side);
		lb.fill({ 0, 0, 0, 0 });

		lb.execute(img::paint_line_command{ { 0, 0 }, { in.lower_side - 1, in.lower_side - 1 }, in.border_color });

		lb_complement.create(in.lower_side, in.lower_side);
		lb_complement.execute(img::paint_line_command{{ 0, in.lower_side - 1 }, { in.lower_side - 1, in.lower_side - 1 }, in.border_color });
		lb_complement.execute(img::paint_line_command{{ 0, in.lower_side - 1 }, { 0, 0 }, in.border_color });

		save(button_corner_type::LT_BORDER, lt);
		save(button_corner_type::RT_BORDER, rt);
		save(button_corner_type::RB_BORDER, rb);
		save(button_corner_type::LB_BORDER, lb);

		if (in.make_lb_complement) {
			save(button_corner_type::LB_COMPLEMENT_BORDER, lb_complement);
		}
	}

	{
		img lt;
		img rt;
		img rb;
		img lb;

		img lb_complement;

		lt.create(in.lower_side, in.upper_side);
		lt.execute(img::paint_line_command{{ in.inside_border_padding, in.inside_border_padding }, { in.inside_border_padding, in.upper_side - 1 }, in.border_color});
		lt.execute(img::paint_line_command{{ in.inside_border_padding, in.inside_border_padding }, { in.lower_side - 1, in.inside_border_padding }, in.border_color});

		rt.create(in.upper_side, in.upper_side);
		rt.fill({ 0, 0, 0, 0 });
		rt.execute(img::paint_line_command{{ 0, in.inside_border_padding }, { in.upper_side - 1 - in.inside_border_padding, in.upper_side - 1 }, in.border_color });

		rb.create(in.upper_side, in.lower_side);
		rb.execute(img::paint_line_command{{ in.upper_side - 1 - in.inside_border_padding, in.lower_side - 1 - in.inside_border_padding }, { 0, in.lower_side - 1 - in.inside_border_padding }, in.border_color});
		rb.execute(img::paint_line_command{{ in.upper_side - 1 - in.inside_border_padding, in.lower_side - 1 - in.inside_border_padding }, { in.upper_side - 1 - in.inside_border_padding, 0 }, in.border_color});

		lb.create(in.lower_side, in.lower_side);
		lb.fill({ 0, 0, 0, 0 });
		lb.execute(img::paint_line_command{{ in.inside_border_padding, 0 }, { in.lower_side - 1, in.lower_side - 1 - in.inside_border_padding }, in.border_color });

		save(button_corner_type::LT_INTERNAL_BORDER, lt);
		save(button_corner_type::RT_INTERNAL_BORDER, rt);
		save(button_corner_type::RB_INTERNAL_BORDER, rb);
		save(button_corner_type::LB_INTERNAL_BORDER, lb);
	}

	{
		img inside;
		inside.create(100u, 100u);
		inside.fill(in.inside_color);
		save(button_corner_type::INSIDE, inside);
	}
}