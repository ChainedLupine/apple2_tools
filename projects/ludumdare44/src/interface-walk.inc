HANDLE_INPUT_WALK:

		lda ui_prev_state
		cmp #interface_state_walk
		beq @have_our_selection

		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_walk_where, arg1w
		JSR TE_PRINT_TEXT

		; set ourselves as previous state
		lda #interface_state_walk
		sta ui_prev_state

		; switch UI to select mode
		lda #interface_state_selector
		sta ui_state

		; set selector state (exits only)
		lda #selector_filter_mask_exits
    sta ui_selector_filter

    RTS

	@have_our_selection:		
		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_debug3, arg1w
		JSR TE_PRINT_TEXT

		JSR ROM::GETKEY

		JSR TE_CLEAR_TEXT

		lda #interface_state_4way
		sta ui_state

		lda #$ff
		sta ui_prev_state

    RTS

