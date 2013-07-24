#include <gl\glew.h>
#include "../augmentations.h"

#include "../texture_baker/font.h"

#include "../config/config.h"
#include "../window_framework/window.h"


int main() {
	augmentations::init();
	using namespace augmentations;

	config::input_file cfg("window_config.txt");

	window::glwindow gl;
	gl.create(cfg, rects::wh(100, 100), window::glwindow::RESIZABLE);
	
	util::fpstimer fps;
	fps.set_max_fps(9999999);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glDisable(GL_DEPTH_TEST); 
	glClearColor(0.0, 0.0, 0.0, 1.0);

	glEnableClientState(GL_VERTEX_ARRAY);  
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);  
	glEnableClientState(GL_COLOR_ARRAY);  

	gl.set_minimum_size(rects::wh(10, 10));
	gl.set_maximum_size(rects::wh(1680, 1050));

	window::event::message msg;

	bool quit = false;
	gl.set_show(gl.SHOW);
	unsigned id = 0, in = 0;
	
	const int IMAGES = 40;
	texture_baker::image images[IMAGES];
	texture_baker::texture textures[IMAGES];

	images[0].from_file(L"C:/VVV/fruit.png");
	images[1].from_file(L"C:/VVV/head.png");
	images[2].from_file(L"C:/VVV/enemy.png");
	images[3].from_file(L"C:/VVV/segment.png");
	images[4].from_file(L"C:/VVV/fiz_room.png");
	images[5].from_file(L"C:/VVV/splash_screen_bottom_text.png");
	images[6].from_file(L"C:/VVV/splash_screen_hero.png");
	images[7].from_file(L"C:/VVV/splash_screen_top_small_text.png");
	images[8].from_file(L"C:/VVV/splash_screen_top_text.png");
	images[10].from_file(L"C:/VVV/dyrexp.png");
	images[11].from_file(L"C:/VVV/cwexp.png");
	images[12].from_file(L"C:/VVV/cat.png");
	images[13].from_file(L"C:/VVV/enemy_cwioro.png");
	images[14].from_file(L"C:/VVV/bio_room.png");
	images[15].from_file(L"C:/VVV/fruit_box.png");
	images[16].from_file(L"C:/VVV/glejt.png");
	images[17].from_file(L"C:/VVV/np.png");
	images[18].from_file(L"C:/VVV/cp.png");
	images[23].from_file(L"C:/VVV/enemy_slowiak.png");
	images[24].from_file(L"C:/VVV/geo_room.jpg");
	images[25].from_file(L"C:/VVV/enemy_francuz.png");
	images[26].from_file(L"C:/VVV/nb.png");
	images[27].from_file(L"C:/VVV/enemy_suder.png");
	images[28].from_file(L"C:/VVV/server.png");
	images[29].from_file(L"C:/VVV/kanc_room.png");
	images[30].from_file(L"C:/VVV/explosion.png");
	images[31].from_file(L"C:/VVV/bomb.png");
	images[32].from_file(L"C:/VVV/error.png");
	images[33].from_file(L"C:/VVV/lightning.png");
	images[35].from_file(L"C:/VVV/shell.png");
	images[36].from_file(L"C:/VVV/shotgun_shell.png");
	images[37].from_file(L"C:/VVV/pistol_shell.png");
	images[38].from_file(L"C:/VVV/sniper_shell.png");
	images[39].create(4, 4, 1);
	images[39].fill(255);

	const int FONTS = 4;
	texture_baker::font_file fontf[FONTS];
	texture_baker::font      fonts[FONTS];

	wchar_t* str = L" qvxQVXaπbcÊdeÍfghijkl≥mnÒoÛprsútuwyzüøA•BC∆DE FGHIJKL£MN—O”PRSåTUWYZèØ0123456789.!@#$%^&*()_+-=[];'\\,./{}:\"|<>?";
	
	fontf[0].open("SDK/resources/arial.ttf",   25, str);
	fontf[1].open("SDK/resources/arialbd.ttf", 25, str);
	fontf[2].open("SDK/resources/ariali.ttf",  25, str);
	fontf[3].open("SDK/resources/arialbi.ttf", 25, str);
	
	fonts->set_styles(fonts + 1, fonts + 2, fonts + 3);
	
    texture_baker::atlas atl;

	atl.quick_add(images, textures, IMAGES, fontf, fonts, FONTS);
	atl.pack();
	atl.create_image(4, false);
	atl.build(false, false);
	atl.img.destroy();
	atl.nearest();

	textures[IMAGES].translate_uv(vec2<float>(2, 2));
	textures[IMAGES].scale_uv(0.000000001f, 0.000000001f);


	gl.resize = [](window::glwindow& gl) {
		gl.current();
		glViewport(0, 0, gl.get_window_rect().w, gl.get_window_rect().h);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, gl.get_window_rect().w, gl.get_window_rect().h, 0, 0, 1);
		glMatrixMode(GL_MODELVIEW);
	};

	gl.resize(gl);

	atl.bind();
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	while(true) {
		if(gl.poll_events(msg)) {
			if(msg == window::event::close) 
				break;
		}

		if(fps.render()) { 
			gl.current();
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLoadIdentity();

			glBegin (GL_QUADS);
			glTexCoord2f (0.0, 0.0);
			glVertex2i (0, 0);
			glTexCoord2f (1.0, 0.0);
			glVertex2i (gl.get_window_rect().w, 0);
			glTexCoord2f (1.0, 1.0);
			glVertex2i (gl.get_window_rect().w, gl.get_window_rect().h);
			glTexCoord2f (0.0, 1.0);
			glVertex2i (0, gl.get_window_rect().h);
			glEnd();

			if(!gl.swap_buffers()) break;

			fps.reset();
		}
		fps.loop();
	}
	augmentations::deinit();
	return 0;
}



