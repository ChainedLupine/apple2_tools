HANDLE_INPUT_INTERACT:

		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_interact_what, arg1w
		JSR TE_PRINT_TEXT

		; switch UI to select mode
		lda #interface_mode_selector
		sta ui_state

    ; set selector state (inv)
    lda #selector_state_mask_inv
		sta ui_selector_state

    RTS

