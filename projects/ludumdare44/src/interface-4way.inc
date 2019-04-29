
HANDLE_INPUT_4WAY_MODE:
    ; does not clear text, cuz previous interaction might be displaying something

    JSR UI_DRAW_RIGHT_PANEL

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
		
    cmp #kb_right_arrow
		beq @HANDLE_KB_RIGHT
		
    cmp #kb_up_arrow
		beq @HANDLE_KB_UP
		
    cmp #kb_down_arrow
		beq @HANDLE_KB_DOWN
		
    cmp #kb_escape
		beq @HANDLE_KB_ESCAPE

		JMP @WAITFORINPUT

  ; examine
	@HANDLE_KB_LEFT:
    JSR PLAY_BEEP_SELECT

		Util_LOAD_SYM txt_examine, arg1w
		JSR TE_PRINT_TEXT

		lda #interface_state_examine
    JMP @NEW_UI

  ; talk
	@HANDLE_KB_RIGHT:
    JSR PLAY_BEEP_SELECT

		Util_LOAD_SYM txt_talk, arg1w
		JSR TE_PRINT_TEXT

		lda #interface_state_talk
    JMP @NEW_UI

  ; interact
	@HANDLE_KB_UP:
    JSR PLAY_BEEP_SELECT

		Util_LOAD_SYM txt_walk, arg1w
		JSR TE_PRINT_TEXT

		lda #interface_state_walk
    JMP @NEW_UI

  ; walk
	@HANDLE_KB_DOWN:
    JSR PLAY_BEEP_SELECT

		Util_LOAD_SYM txt_interact, arg1w
		JSR TE_PRINT_TEXT

		lda #interface_state_interact
    JMP @NEW_UI

  @HANDLE_KB_ESCAPE:

    JSR PLAY_BEEP_DESELECT
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
    
  @NEW_UI:
  		sta ui_state
      lda #$ff
      sta ui_prev_state


  @DONE:
    RTS
