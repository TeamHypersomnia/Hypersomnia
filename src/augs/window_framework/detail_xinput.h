/** Opcode for xcb_input_raw_motion. */
#define XCB_INPUT_RAW_MOTION 17

typedef enum xcb_input_xi_event_mask_t {
    XCB_INPUT_XI_EVENT_MASK_DEVICE_CHANGED = 2,
    XCB_INPUT_XI_EVENT_MASK_KEY_PRESS = 4,
    XCB_INPUT_XI_EVENT_MASK_KEY_RELEASE = 8,
    XCB_INPUT_XI_EVENT_MASK_BUTTON_PRESS = 16,
    XCB_INPUT_XI_EVENT_MASK_BUTTON_RELEASE = 32,
    XCB_INPUT_XI_EVENT_MASK_MOTION = 64,
    XCB_INPUT_XI_EVENT_MASK_ENTER = 128,
    XCB_INPUT_XI_EVENT_MASK_LEAVE = 256,
    XCB_INPUT_XI_EVENT_MASK_FOCUS_IN = 512,
    XCB_INPUT_XI_EVENT_MASK_FOCUS_OUT = 1024,
    XCB_INPUT_XI_EVENT_MASK_HIERARCHY = 2048,
    XCB_INPUT_XI_EVENT_MASK_PROPERTY = 4096,
    XCB_INPUT_XI_EVENT_MASK_RAW_KEY_PRESS = 8192,
    XCB_INPUT_XI_EVENT_MASK_RAW_KEY_RELEASE = 16384,
    XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_PRESS = 32768,
    XCB_INPUT_XI_EVENT_MASK_RAW_BUTTON_RELEASE = 65536,
    XCB_INPUT_XI_EVENT_MASK_RAW_MOTION = 131072,
    XCB_INPUT_XI_EVENT_MASK_TOUCH_BEGIN = 262144,
    XCB_INPUT_XI_EVENT_MASK_TOUCH_UPDATE = 524288,
    XCB_INPUT_XI_EVENT_MASK_TOUCH_END = 1048576,
    XCB_INPUT_XI_EVENT_MASK_TOUCH_OWNERSHIP = 2097152,
    XCB_INPUT_XI_EVENT_MASK_RAW_TOUCH_BEGIN = 4194304,
    XCB_INPUT_XI_EVENT_MASK_RAW_TOUCH_UPDATE = 8388608,
    XCB_INPUT_XI_EVENT_MASK_RAW_TOUCH_END = 16777216,
    XCB_INPUT_XI_EVENT_MASK_BARRIER_HIT = 33554432,
    XCB_INPUT_XI_EVENT_MASK_BARRIER_LEAVE = 67108864
} xcb_input_xi_event_mask_t;
/**
 * @brief xcb_input_fp3232_t
 **/
typedef struct xcb_input_fp3232_t {
    int32_t  integral;
    uint32_t frac;
} xcb_input_fp3232_t;

typedef uint16_t xcb_input_device_id_t;

typedef struct xcb_input_raw_button_press_event_t {
    uint8_t               response_type;
    uint8_t               extension;
    uint16_t              sequence;
    uint32_t              length;
    uint16_t              event_type;
    xcb_input_device_id_t deviceid;
    xcb_timestamp_t       time;
    uint32_t              detail;
    xcb_input_device_id_t sourceid;
    uint16_t              valuators_len;
    uint32_t              flags;
    uint8_t               pad0[4];
    uint32_t              full_sequence;
} xcb_input_raw_button_press_event_t;

typedef xcb_input_raw_button_press_event_t xcb_input_raw_motion_event_t;

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

