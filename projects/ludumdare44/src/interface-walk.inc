HANDLE_INPUT_WALK:

		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_walk_where, arg1w
		JSR TE_PRINT_TEXT

		; switch UI to select mode
		lda #interface_mode_selector
		sta ui_state

		; set selector state (exits only)
		lda #selector_state_mask_exits
    sta ui_selector_state

    RTS

