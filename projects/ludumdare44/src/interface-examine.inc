HANDLE_INPUT_EXAMINE:

		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_examine_what, arg1w
		JSR TE_PRINT_TEXT

		; switch UI to select mode
		lda #interface_mode_selector
		sta ui_state


		; set selector state (inv only)
		;lda #selector_state_mask_inv
		;sta ui_selector_state

		; set selector state (nearby only)
		;lda #selector_state_mask_nearby
		;sta ui_selector_state

    ; set selector state (inv, nearby)
    lda #selector_state_mask_inv
    ora #selector_state_mask_nearby
		sta ui_selector_state

		; set selector state (exits only)
		;lda #selector_state_mask_exits
    ;sta ui_selector_state

    RTS

