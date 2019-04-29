HANDLE_INPUT_EXAMINE:

		lda ui_prev_state
		cmp #interface_state_examine
		beq @have_our_selection

		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_examine_what, arg1w
		JSR TE_PRINT_TEXT

		; set ourselves as previous state
		lda #interface_state_examine
		sta ui_prev_state

		; switch UI to select mode
		lda #interface_state_selector
		sta ui_state

    ; set selector filter (inv, nearby)
    lda #selector_filter_mask_inv
    ora #selector_filter_mask_nearby
		sta ui_selector_filter

		RTS

	@have_our_selection:
		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_debug, arg1w
		JSR TE_PRINT_TEXT

		JSR ROM::GETKEY

		JSR TE_CLEAR_TEXT

		lda #interface_state_4way
		sta ui_state

		lda #$ff
		sta ui_prev_state

    RTS

