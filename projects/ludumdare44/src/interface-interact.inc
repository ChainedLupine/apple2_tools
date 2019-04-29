HANDLE_INPUT_INTERACT:

		lda ui_prev_state
		cmp #interface_state_interact
		beq @have_our_first_selection
		cmp #interface_state_interact2
		beq @have_our_second_selection

		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_interact_what1, arg1w
		JSR TE_PRINT_TEXT

		; set ourselves as previous state
		lda #interface_state_interact
		sta ui_prev_state

		; switch UI to select mode
		lda #interface_state_selector
		sta ui_state

    ; set selector state (inv)
    lda #selector_filter_mask_inv
		sta ui_selector_filter

    RTS

	@have_our_first_selection:
		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_interact_what2, arg1w
		JSR TE_PRINT_TEXT

		lda #interface_state_selector
		sta ui_state

		lda #interface_state_interact2
		sta ui_prev_state

    lda #selector_filter_mask_nearby
		sta ui_selector_filter

		; save what we selected
		lda ui_selector_selected
		sta ui_selector_selected_extra
		
		lda ui_selector_selected+1
		sta ui_selector_selected_extra+1

    RTS

	@have_our_second_selection:
		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_debug2, arg1w
		JSR TE_PRINT_TEXT

		JSR ROM::GETKEY

		JSR TE_CLEAR_TEXT

		lda #interface_state_4way
		sta ui_state

		lda #$ff
		sta ui_prev_state

    RTS
