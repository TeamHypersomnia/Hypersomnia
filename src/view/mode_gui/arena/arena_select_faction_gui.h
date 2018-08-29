struct arena_select_faction_gui {
	// GEN INTROSPECTOR struct arena_select_faction_gui
	bool show = false;
	// END GEN INTROSPECTOR

	template <class M>
	void perform_imgui(
		draw_mode_gui_input, 
		const M& mode, 
		const typename M::input&
	);
};
