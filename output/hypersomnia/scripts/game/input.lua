custom_intents = create_inverse_enum {
	"QUIT",
	"DROP_WEAPON",
	"ZOOM_CAMERA",
	
	"SWITCH_CLIENT_1",
	"SWITCH_CLIENT_2",
	"SWITCH_CLIENT_3",
	"SWITCH_CLIENT_4"
}

main_input_context = create_input_context {
	intents = { 
		[mouse.raw_motion] 		= intent_message.AIM,
		[keys.W] 				= intent_message.MOVE_FORWARD,
		[keys.S] 				= intent_message.MOVE_BACKWARD,
		[keys.A] 				= intent_message.MOVE_LEFT,
		[keys.D] 				= intent_message.MOVE_RIGHT,
		
		[keys._1] 				= custom_intents.SWITCH_CLIENT_1,
		[keys._2] 				= custom_intents.SWITCH_CLIENT_2,
		[keys._3] 				= custom_intents.SWITCH_CLIENT_3,
		[keys._4] 				= custom_intents.SWITCH_CLIENT_4,
		
		[mouse.ldoubleclick] 	= intent_message.SHOOT,
		[mouse.ltripleclick] 	= intent_message.SHOOT,
		[mouse.ldown] 			= intent_message.SHOOT,

		[keys.LSHIFT] 			= intent_message.SWITCH_LOOK,
		
		[keys.ESC] 				= custom_intents.QUIT,
			
		[mouse.rdown] 			= custom_intents.DROP_WEAPON,
		[mouse.rdoubleclick] 	= custom_intents.DROP_WEAPON,
		[mouse.wheel]			= custom_intents.ZOOM_CAMERA
	}
}