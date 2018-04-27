- Build times of debug and release builds to know which one to test
	- Release:
		make all -j8 -C build/current  1327.45s user 36.61s system 665% cpu 3:24.87 total
	- Debug:
		make all -j8 -C build/current  998.23s user 42.75s system 750% cpu 2:18.70 total

- Build times with and without -g on debug
	- without -g:
		make all -j8 -C build/current  911.48s user 39.84s system 697% cpu 2:16.39 total
		make src/application/setups/editor/gui/editor_fae_gui.cpp.o -j8 -C   28.05s user 0.80s system 99% cpu 29.058 total
		- after reordering sources:
			make all -j8 -C build/current  922.41s user 40.03s system 761% cpu 2:06.35 total

	- with -g:
		make all -j8 -C build/current  1049.32s user 46.56s system 684% cpu 2:40.08 total
		make src/application/setups/editor/gui/editor_fae_gui.cpp.o -j8 -C   35.65s user 1.02s system 99% cpu 36.914 total

- ninja vs make:
ninja all -C build/current  1051.75s user 40.63s system 784% cpu 2:19.30 total
make all -j10 -C build/current  1053.82s user 42.51s system 778% cpu 2:20.90 total
