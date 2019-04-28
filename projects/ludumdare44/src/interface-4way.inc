

HANDLE_INPUT_4WAY_MODE:
		Util_LOAD_SYM txt_enter_cmd, arg1w
		JSR TE_PRINT_TEXT

	@WAITFORINPUT:

		sta $c010 ; clear strobe

	:	lda $c000
		bpl :-
		and #$7f
		sta temp1
		sta $c010

		; key press in temp1
		lda temp1
		cmp #kb_left_arrow
		beq @HANDLE_KB_LEFT
		cmp #kb_escape
		beq @HANDLE_KB_ESCAPE

		JMP @WAITFORINPUT

	@HANDLE_KB_LEFT:

		Util_LOAD_SYM txt_examine, arg1w
		JSR TE_PRINT_TEXT

		lda #interface_mode_examine
		sta ui_state

		JMP @DONE

  @HANDLE_KB_ESCAPE:

		;lda #interface_mode_4way
		;sta ui_state

		; dec health, wrapping if <0
		lda state_health
		dec
		bpl :+
		lda #4

		:
		sta state_health

		; dec health, wrapping if <0
		lda state_time
		dec
		bpl :+
		lda #4

		:
		sta state_time

    JSR UI_DRAW_HEALTH
    JSR UI_DRAW_TIME

    JMP @WAITFORINPUT

	@CLEAR_UI:

		JMP @DONE


  @DONE:
    RTS
