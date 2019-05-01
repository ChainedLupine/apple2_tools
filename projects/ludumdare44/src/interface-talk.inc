HANDLE_INPUT_TALK:

		lda ui_prev_state
		cmp #interface_state_talk
		beq @have_our_selection

		JSR TE_CLEAR_TEXT

		Util_LOAD_SYM txt_talk_what, arg1w
		JSR TE_PRINT_TEXT

		; set ourselves as previous state
		lda #interface_state_talk
		sta ui_prev_state

		; switch UI to select mode
		lda #interface_state_selector
		sta ui_state

		; set selector filter (nearby only)
		lda #selector_filter_mask_nearby
		sta ui_selector_filter

    RTS

	@have_our_selection:
		JSR TE_CLEAR_TEXT
		; check to see if it's a bogus item
		lda ui_selector_selected+1
		cmp #$ff
		beq @NO_VALID_ITEM

		; prepare for ZP
		lda ui_selector_selected
		sta temp1w
		lda ui_selector_selected+1
		sta temp1w+1

		ldy #inventory_struct::typeid
		lda (temp1w),y
		JSR GW_HANDLE_TALKING

		JSR UI_WAIT_FOR_KEY

		JMP @EXIT

	@NO_VALID_ITEM:
		JSR PLAY_BEEP_SAD

		Util_LOAD_SYM txt_talk_error, arg1w
		JSR TE_PRINT_TEXT
		
		JSR UI_WAIT_FOR_KEY

	@EXIT:
		JSR TE_CLEAR_TEXT

		lda #interface_state_4way
		sta ui_state

		lda #$ff
		sta ui_prev_state

    RTS
