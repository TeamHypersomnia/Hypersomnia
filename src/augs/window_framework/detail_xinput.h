uint32_t *
xcb_input_raw_button_press_valuator_mask (const xcb_input_raw_button_press_event_t *R)
{
	return (uint32_t *) (R + 1);
}

xcb_generic_iterator_t
xcb_input_raw_button_press_valuator_mask_end (const xcb_input_raw_button_press_event_t *R)
{
    xcb_generic_iterator_t i;
    i.data = ((uint32_t *) (R + 1)) + (R->valuators_len);
    i.rem = 0;
    i.index = (char *) i.data - (char *) R;
    return i;
}

xcb_input_fp3232_t *
xcb_input_raw_button_press_axisvalues (const xcb_input_raw_button_press_event_t *R)
{
    xcb_generic_iterator_t prev = xcb_input_raw_button_press_valuator_mask_end(R);
    return (xcb_input_fp3232_t *) ((char *) prev.data + XCB_TYPE_PAD(xcb_input_fp3232_t, prev.index) + 0);
}

int
xcb_input_raw_button_press_axisvalues_raw_length (const xcb_input_raw_button_press_event_t *R)
{
    int xcb_pre_tmp_17; /* sumof length */
    int xcb_pre_tmp_18; /* sumof loop counter */
    int64_t xcb_pre_tmp_19; /* sumof sum */
    const uint32_t* xcb_pre_tmp_20; /* sumof list ptr */
    /* sumof start */
    xcb_pre_tmp_17 = R->valuators_len;
    xcb_pre_tmp_19 = 0;
    xcb_pre_tmp_20 = xcb_input_raw_button_press_valuator_mask(R);
    for (xcb_pre_tmp_18 = 0; xcb_pre_tmp_18 < xcb_pre_tmp_17; xcb_pre_tmp_18++) {
        const uint32_t *xcb_listelement = xcb_pre_tmp_20;
        xcb_pre_tmp_19 += xcb_popcount((*xcb_listelement));
        xcb_pre_tmp_20++;
    }
    /* sumof end. Result is in xcb_pre_tmp_19 */
    return xcb_pre_tmp_19;
}

