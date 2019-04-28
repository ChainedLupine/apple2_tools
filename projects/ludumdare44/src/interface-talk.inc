HANDLE_INPUT_TALK:

		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_talk_what, arg1w
		JSR TE_PRINT_TEXT

		; switch UI to select mode
		lda #interface_mode_selector
		sta ui_state


		; set selector state (nearby only)
		lda #selector_state_mask_nearby
		sta ui_selector_state

    RTS

