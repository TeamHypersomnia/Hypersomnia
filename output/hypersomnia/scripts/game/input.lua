custom_intents = create_inverse_enum {
	"QUIT",
	"ZOOM_CAMERA",
	
	"SWITCH_CLIENT_1",
	"SWITCH_CLIENT_2",
	"SWITCH_CLIENT_3",
	"SWITCH_CLIENT_4",
	
	"PICK_REQUEST",
	"DROP_REQUEST",
	
	"SELECT_ITEM_1",
	"SELECT_ITEM_2",
	"SELECT_ITEM_3",
	"SELECT_ITEM_4",
	"SELECT_ITEM_5",
	"SELECT_ITEM_6"
}

main_input_context = create_input_context {
	intents = { 
		[mouse.raw_motion] 		= intent_message.AIM,
		[keys.W] 				= intent_message.MOVE_FORWARD,
		[keys.S] 				= intent_message.MOVE_BACKWARD,
		[keys.A] 				= intent_message.MOVE_LEFT,
		[keys.D] 				= intent_message.MOVE_RIGHT,
		
		[keys.G] 				= custom_intents.DROP_REQUEST,
		
		[keys._1] 				= custom_intents.SELECT_ITEM_1,
		[keys._2] 				= custom_intents.SELECT_ITEM_2,
		[keys._3] 				= custom_intents.SELECT_ITEM_3,
		[keys._4] 				= custom_intents.SELECT_ITEM_4,
		[keys._5] 				= custom_intents.SELECT_ITEM_5,
		[keys._6] 				= custom_intents.SELECT_ITEM_6,
		
		[keys._7] 				= custom_intents.SWITCH_CLIENT_1,
		[keys._8] 				= custom_intents.SWITCH_CLIENT_2,
		[keys._9] 				= custom_intents.SWITCH_CLIENT_3,
		[keys._0] 				= custom_intents.SWITCH_CLIENT_4,
		
		[mouse.ldoubleclick] 	= intent_message.SHOOT,
		[mouse.ltripleclick] 	= intent_message.SHOOT,
		[mouse.ldown] 			= intent_message.SHOOT,

		[keys.LSHIFT] 			= intent_message.SWITCH_LOOK,
		
		[keys.ESC] 				= custom_intents.QUIT,
			
		[mouse.rdown] 			= custom_intents.PICK_REQUEST,
		[mouse.rdoubleclick] 	= custom_intents.PICK_REQUEST,
		[mouse.wheel]			= custom_intents.ZOOM_CAMERA
	}
}