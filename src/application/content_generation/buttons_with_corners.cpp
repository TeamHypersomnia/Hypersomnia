#include "buttons_with_corners.h"
#include "augs/gui/button_corners.h"

#include "augs/misc/streams.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

void regenerate_buttons_with_corners(
	const bool force_regenerate
) {
	const auto buttons_with_corners_directory = "generated/buttons_with_corners/";

	augs::create_directories(buttons_with_corners_directory);

	const auto lines = augs::get_file_lines("generators/buttons_with_corners_generator_input.cfg");
	size_t current_line = 0;

	while (current_line < lines.size()) {
		button_with_corners_stamp new_stamp;

		const auto target_stem = lines[current_line];
		
		++current_line;

		{
			std::istringstream in(lines[current_line]);
			in >> new_stamp.border_color;
		}

		++current_line;

		{
			std::istringstream in(lines[current_line]);
			in >> new_stamp.inside_color;
		}
		
		++current_line;

		{
			std::istringstream in(lines[current_line]);
			in >> new_stamp.lower_side >> new_stamp.upper_side >> new_stamp.inside_border_padding >> new_stamp.make_lb_complement;
		}

		// skip separating newline
		++current_line;

		// go to the next path line
		++current_line;

		const auto button_with_corners_path_template = buttons_with_corners_directory + target_stem + "_%x.png";
		const auto button_with_corners_stamp_path = buttons_with_corners_directory + target_stem + ".stamp";

		augs::stream new_stamp_stream;
		augs::write(new_stamp_stream, new_stamp);

		bool should_regenerate = force_regenerate;

		for (size_t i = 0; i < static_cast<int>(button_corner_type::COUNT); ++i) {
			const auto type = static_cast<button_corner_type>(i);
			
			if (is_lb_complement(type) && !new_stamp.make_lb_complement) {
				continue;
			}

			if (
				!augs::file_exists(
					typesafe_sprintf(
						button_with_corners_path_template,
						get_filename_for(type)
					)
				)
			) {
				should_regenerate = true;
				break;
			}
		}

		if (!should_regenerate) {
			if (!augs::file_exists(button_with_corners_stamp_path)) {
				should_regenerate = true;
			}
			else {
				augs::stream existent_stamp_stream;
				augs::get_file_contents_binary_into(button_with_corners_stamp_path, existent_stamp_stream);

				const bool are_stamps_identical = (new_stamp_stream == existent_stamp_stream);

				if (!are_stamps_identical) {
					should_regenerate = true;
				}
			}
		}

		if (should_regenerate) {
			LOG("Regenerating button with corners: %x", target_stem);

			create_and_save_button_with_corners(
				button_with_corners_path_template,
				new_stamp
			);

			augs::create_binary_file(button_with_corners_stamp_path, new_stamp_stream);
		}
	}
}

void create_and_save_button_with_corners(
	const std::string& path_template,
	const button_with_corners_stamp in
) {
	typedef augs::image img;

	const auto save = [&](const button_corner_type t, const img& im) {
		im.save(typesafe_sprintf(path_template, get_filename_for(t)));
	};

	{
		img l;
		img t;
		img r;
		img b;

		l.create(in.lower_side, 1u);
		l.execute(augs::paint_line_command{ { 1, 0 }, { in.lower_side - 1, 0 }, in.inside_color });

		r.create(in.upper_side, 1u);
		r.execute(augs::paint_line_command{ { in.upper_side - 2, 0 }, { 0, 0 }, in.inside_color } );

		b.create(1u, in.lower_side);
		b.execute(augs::paint_line_command{{ 0, in.lower_side - 2 }, { 0, 0 }, in.inside_color });

		t.create(1u, in.upper_side);
		t.execute(augs::paint_line_command{{ 0, 1 }, { 0, in.upper_side - 1 }, in.inside_color });

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
		lt.execute(augs::paint_line_command{{ 0, 0 }, { in.lower_side - 1, 0 }, { 0, 0, 0, 0 }} );
		lt.execute(augs::paint_line_command{{ 0, 0 }, { 0, in.upper_side - 1 }, { 0, 0, 0, 0 }} );

		rt.create(in.upper_side, in.upper_side);
		rt.fill({ 0, 0, 0, 0 });

		for (unsigned i = 1; i < in.upper_side; ++i) {
			rt.execute(augs::paint_line_command{{ 0, i }, { in.upper_side - i - 1, in.upper_side - 1 }, in.inside_color });
		}

		rb.create(in.upper_side, in.lower_side);
		rb.fill(in.inside_color);
		rb.execute(augs::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { 0, in.lower_side - 1 }, { 0, 0, 0, 0 }});
		rb.execute(augs::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { in.upper_side - 1, 0 }, { 0, 0, 0, 0 }});

		lb.create(in.lower_side, in.lower_side);
		lb.fill({ 0, 0, 0, 0 });

		for (unsigned i = 1; i < in.lower_side; ++i) {
			lb.execute(augs::paint_line_command{ { i, 0 }, { in.lower_side - 1, in.lower_side - 1 - i }, in.inside_color });
		}

		lb_complement.create(in.lower_side, in.lower_side);

		for (unsigned i = 1; i < in.lower_side; ++i) {
			lb_complement.execute(augs::paint_line_command{{ 0, i }, { in.lower_side - 1 - i, in.lower_side - 1 }, in.inside_color });
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
		lt.execute(augs::paint_line_command{{ 0, 0 }, { 0, in.upper_side - 1 }, in.border_color});
		lt.execute(augs::paint_line_command{{ 0, 0 }, { in.lower_side - 1, 0 }, in.border_color});

		rt.create(in.upper_side, in.upper_side);
		rt.fill({ 0, 0, 0, 0 });

		rt.execute(augs::paint_line_command{ { 0, 0 }, { in.upper_side - 1, in.upper_side - 1 }, in.border_color });

		rb.create(in.upper_side, in.lower_side);
		rb.execute(augs::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { 0, in.lower_side - 1 }, in.border_color});
		rb.execute(augs::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { in.upper_side - 1, 0 }, in.border_color});

		lb.create(in.lower_side, in.lower_side);
		lb.fill({ 0, 0, 0, 0 });

		lb.execute(augs::paint_line_command{ { 0, 0 }, { in.lower_side - 1, in.lower_side - 1 }, in.border_color });

		lb_complement.create(in.lower_side, in.lower_side);
		lb_complement.execute(augs::paint_line_command{{ 0, in.lower_side - 1 }, { in.lower_side - 1, in.lower_side - 1 }, in.border_color });
		lb_complement.execute(augs::paint_line_command{{ 0, in.lower_side - 1 }, { 0, 0 }, in.border_color });

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
		lt.execute(augs::paint_line_command{{ in.inside_border_padding, in.inside_border_padding }, { in.inside_border_padding, in.upper_side - 1 }, in.border_color});
		lt.execute(augs::paint_line_command{{ in.inside_border_padding, in.inside_border_padding }, { in.lower_side - 1, in.inside_border_padding }, in.border_color});

		rt.create(in.upper_side, in.upper_side);
		rt.fill({ 0, 0, 0, 0 });
		rt.execute(augs::paint_line_command{{ 0, in.inside_border_padding }, { in.upper_side - 1 - in.inside_border_padding, in.upper_side - 1 }, in.border_color });

		rb.create(in.upper_side, in.lower_side);
		rb.execute(augs::paint_line_command{{ in.upper_side - 1 - in.inside_border_padding, in.lower_side - 1 - in.inside_border_padding }, { 0, in.lower_side - 1 - in.inside_border_padding }, in.border_color});
		rb.execute(augs::paint_line_command{{ in.upper_side - 1 - in.inside_border_padding, in.lower_side - 1 - in.inside_border_padding }, { in.upper_side - 1 - in.inside_border_padding, 0 }, in.border_color});

		lb.create(in.lower_side, in.lower_side);
		lb.fill({ 0, 0, 0, 0 });
		lb.execute(augs::paint_line_command{{ in.inside_border_padding, 0 }, { in.lower_side - 1, in.lower_side - 1 - in.inside_border_padding }, in.border_color });

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