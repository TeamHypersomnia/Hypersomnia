#include "augs/log.h"
#include "augs/gui/button_corners.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/image/image.h"

#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"

#include "view/viewables/regeneration/buttons_with_corners.h"

#include "augs/readwrite/byte_file.h"
#include "augs/readwrite/to_bytes.h"

void regenerate_button_with_corners(
	const augs::path_type& output_path_template,
	const button_with_corners_input input,
	const bool force_regenerate
) {
	button_with_corners_stamp new_stamp = input;

	const auto untemplatized_path = augs::path_type(typesafe_sprintf(output_path_template.string(), ""));
	const auto button_with_corners_stamp_path = augs::path_type(untemplatized_path).replace_extension(".stamp");

	const auto new_stamp_bytes = augs::to_bytes(new_stamp);

	bool should_regenerate = force_regenerate;

	for (size_t i = 0; i < static_cast<int>(button_corner_type::COUNT); ++i) {
		const auto type = static_cast<button_corner_type>(i);

		if (
			!augs::exists(
				typesafe_sprintf(
					output_path_template.string(),
					get_filename_for(type)
				)
			)
		) {
			should_regenerate = true;
			break;
		}
	}

	if (!should_regenerate) {
		if (!augs::exists(button_with_corners_stamp_path)) {
			should_regenerate = true;
		}
		else {
			const auto existent_stamp_bytes = augs::file_to_bytes(button_with_corners_stamp_path);
			const bool are_stamps_identical = (new_stamp_bytes == existent_stamp_bytes);

			if (!are_stamps_identical) {
				should_regenerate = true;
			}
		}
	}

	if (should_regenerate) {
		LOG("Regenerating button with corners: %x", untemplatized_path);
		
		/* stamp is in the same directory as all images */
		augs::create_directories_for(button_with_corners_stamp_path);

		create_and_save_button_with_corners(
			output_path_template,
			new_stamp
		);

		augs::bytes_to_file(new_stamp_bytes, button_with_corners_stamp_path);
	}
}

void create_and_save_button_with_corners(
	const augs::path_type& path_template,
	const button_with_corners_input in
) {
	ensure(in.upper_side > 0);
	ensure(in.lower_side > 0);

	using img = augs::image;

	const auto save = [&](const button_corner_type t, const img& im) {
		im.save(typesafe_sprintf(path_template.string(), get_filename_for(t)));
	};

	{
		img l = vec2u(in.lower_side, 1u);
		img t = vec2u(1u, in.upper_side);
		img r = vec2u(in.upper_side, 1u);
		img b = vec2u(1u, in.lower_side);

		l.execute(augs::paint_line_command{ { 1, 0 }, { in.lower_side - 1, 0 }, in.inside_color });
		t.execute(augs::paint_line_command{ { 0, 1 },{ 0, in.upper_side - 1 }, in.inside_color });
		r.execute(augs::paint_line_command{ { in.upper_side - 2, 0 }, { 0, 0 }, in.inside_color } );
		b.execute(augs::paint_line_command{{ 0, in.lower_side - 2 }, { 0, 0 }, in.inside_color });

		save(button_corner_type::L, l);
		save(button_corner_type::T, t);
		save(button_corner_type::R, r);
		save(button_corner_type::B, b);
	}

	{
		img lt = vec2u(in.lower_side, in.upper_side);
		img rt = vec2u(in.upper_side, in.upper_side);
		img rb = vec2u(in.upper_side, in.lower_side);
		img lb = vec2u(in.lower_side, in.lower_side);

		img lb_complement = vec2u(in.lower_side, in.lower_side);

		lt.fill(in.inside_color);
		lt.execute(augs::paint_line_command{{ 0, 0 }, { in.lower_side - 1, 0 }, { 0, 0, 0, 0 }} );
		lt.execute(augs::paint_line_command{{ 0, 0 }, { 0, in.upper_side - 1 }, { 0, 0, 0, 0 }} );

		rt.fill({ 0, 0, 0, 0 });

		for (unsigned i = 1; i < in.upper_side; ++i) {
			rt.execute(augs::paint_line_command{{ 0, i }, { in.upper_side - i - 1, in.upper_side - 1 }, in.inside_color });
		}

		rb.fill(in.inside_color);
		rb.execute(augs::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { 0, in.lower_side - 1 }, { 0, 0, 0, 0 }});
		rb.execute(augs::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { in.upper_side - 1, 0 }, { 0, 0, 0, 0 }});

		lb.fill({ 0, 0, 0, 0 });

		for (unsigned i = 1; i < in.lower_side; ++i) {
			lb.execute(augs::paint_line_command{ { i, 0 }, { in.lower_side - 1, in.lower_side - 1 - i }, in.inside_color });
		}

		for (unsigned i = 1; i < in.lower_side; ++i) {
			lb_complement.execute(augs::paint_line_command{{ 0, i }, { in.lower_side - 1 - i, in.lower_side - 1 }, in.inside_color });
		}

		save(button_corner_type::LT, lt);
		save(button_corner_type::RT, rt);
		save(button_corner_type::RB, rb);
		save(button_corner_type::LB, lb);

		save(button_corner_type::LB_COMPLEMENT, lb_complement);
	}

	{
		img l = vec2u(in.lower_side, 1u);
		img t = vec2u(1u, in.upper_side);
		img r = vec2u(in.upper_side, 1u);
		img b = vec2u(1u, in.lower_side);

		l.pixel({ 0, 0 }) = in.border_color;
		t.pixel({ 0, 0 }) = in.border_color;
		r.pixel({ in.upper_side - 1, 0 }) = in.border_color;
		b.pixel({ 0, in.lower_side - 1 }) = in.border_color;

		save(button_corner_type::L_BORDER, l);
		save(button_corner_type::T_BORDER, t);
		save(button_corner_type::R_BORDER, r);
		save(button_corner_type::B_BORDER, b);
	}

	{
		img lt = vec2u(in.lower_side, in.upper_side);
		img rt = vec2u(in.upper_side, in.upper_side);
		img rb = vec2u(in.upper_side, in.lower_side);
		img lb = vec2u(in.lower_side, in.lower_side);
		img lb_complement = vec2u(in.lower_side, in.lower_side);

		lt.execute(augs::paint_line_command{{ 0, 0 }, { 0, in.upper_side - 1 }, in.border_color});
		lt.execute(augs::paint_line_command{{ 0, 0 }, { in.lower_side - 1, 0 }, in.border_color});

		rt.fill({ 0, 0, 0, 0 });

		rt.execute(augs::paint_line_command{ { 0, 0 }, { in.upper_side - 1, in.upper_side - 1 }, in.border_color });

		rb.execute(augs::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { 0, in.lower_side - 1 }, in.border_color});
		rb.execute(augs::paint_line_command{{ in.upper_side - 1, in.lower_side - 1 }, { in.upper_side - 1, 0 }, in.border_color});

		lb.fill({ 0, 0, 0, 0 });

		lb.execute(augs::paint_line_command{ { 0, 0 }, { in.lower_side - 1, in.lower_side - 1 }, in.border_color });

		lb_complement.execute(augs::paint_line_command{{ 0, in.lower_side - 1 }, { in.lower_side - 1, in.lower_side - 1 }, in.border_color });
		lb_complement.execute(augs::paint_line_command{{ 0, in.lower_side - 1 }, { 0, 0 }, in.border_color });

		save(button_corner_type::LT_BORDER, lt);
		save(button_corner_type::RT_BORDER, rt);
		save(button_corner_type::RB_BORDER, rb);
		save(button_corner_type::LB_BORDER, lb);

		save(button_corner_type::LB_COMPLEMENT_BORDER, lb_complement);
	}

	{
		img lt = vec2u(in.lower_side, in.upper_side);
		img rt = vec2u(in.upper_side, in.upper_side);
		img rb = vec2u(in.upper_side, in.lower_side);
		img lb = vec2u(in.lower_side, in.lower_side);

		lt.execute(augs::paint_line_command{{ in.inside_border_padding, in.inside_border_padding }, { in.inside_border_padding, in.upper_side - 1 }, in.border_color});
		lt.execute(augs::paint_line_command{{ in.inside_border_padding, in.inside_border_padding }, { in.lower_side - 1, in.inside_border_padding }, in.border_color});

		rt.fill({ 0, 0, 0, 0 });
		rt.execute(augs::paint_line_command{{ 0, in.inside_border_padding }, { in.upper_side - 1 - in.inside_border_padding, in.upper_side - 1 }, in.border_color });

		rb.execute(augs::paint_line_command{{ in.upper_side - 1 - in.inside_border_padding, in.lower_side - 1 - in.inside_border_padding }, { 0, in.lower_side - 1 - in.inside_border_padding }, in.border_color});
		rb.execute(augs::paint_line_command{{ in.upper_side - 1 - in.inside_border_padding, in.lower_side - 1 - in.inside_border_padding }, { in.upper_side - 1 - in.inside_border_padding, 0 }, in.border_color});

		lb.fill({ 0, 0, 0, 0 });
		lb.execute(augs::paint_line_command{{ in.inside_border_padding, 0 }, { in.lower_side - 1, in.lower_side - 1 - in.inside_border_padding }, in.border_color });

		save(button_corner_type::LT_INTERNAL_BORDER, lt);
		save(button_corner_type::RT_INTERNAL_BORDER, rt);
		save(button_corner_type::RB_INTERNAL_BORDER, rb);
		save(button_corner_type::LB_INTERNAL_BORDER, lb);
	}

	{
		img inside;
		inside = vec2u(100u, 100u);
		inside.fill(in.inside_color);
		save(button_corner_type::INSIDE, inside);
	}
}