struct arena_buy_items_gui {
	// GEN INTROSPECTOR struct arena_buy_items_gui
	bool show = false;
	// END GEN INTROSPECTOR

	template <class M>
	void perform_imgui(
		draw_mode_gui_input, 
		const M& mode, 
		const typename M::input&
	);
};
