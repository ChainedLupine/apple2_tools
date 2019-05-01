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

    lda #selector_filter_mask_inv
    ora #selector_filter_mask_nearby
		sta ui_selector_filter

		; save what we selected
		lda ui_selector_selected
		sta ui_selector_selected_extra
		
		lda ui_selector_selected+1
		sta ui_selector_selected_extra+1

    RTS

	@have_our_second_selection:
		JSR TE_CLEAR_TEXT

		; check to see if second item is bogus
		lda ui_selector_selected_extra+1
		cmp #$ff
		beq @NO_VALID_ITEM

		; selected = second item
		lda ui_selector_selected
		sta temp1w
		lda ui_selector_selected+1
		sta temp1w+1

		; load dst into x
		ldy #inventory_struct::typeid
		lda (temp1w),y
		tax

		; where extra = first item
		lda ui_selector_selected_extra
		sta temp1w
		lda ui_selector_selected_extra+1
		sta temp1w+1

		; load source into a
		ldy #inventory_struct::typeid
		lda (temp1w),y

		JSR GW_HANDLE_INTERACTION
		; carry returns success
		bcc :+
			JMP @EXIT
		:

	@NO_VALID_ITEM:
		JSR PLAY_BEEP_SAD

		Util_LOAD_SYM txt_interact_error, arg1w
		JSR TE_PRINT_TEXT
		
	@EXIT:
		JSR UI_WAIT_FOR_KEY

		JSR TE_CLEAR_TEXT

		lda #interface_state_4way
		sta ui_state

		lda #$ff
		sta ui_prev_state

    RTS
