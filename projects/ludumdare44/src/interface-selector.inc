selector_y_exits = 17
selector_y_nearby = 12
selector_y_inv = 7

.export HANDLE_INPUT_SELECTOR

HANDLE_INPUT_SELECTOR:

		JSR UI_CLEAR_RIGHT_PANEL
		JSR UI_DRAW_RIGHT_PANEL

		;Util_LOAD_SYM txt_debug, arg1w
		;JSR TE_PRINT_TEXT

    JSR UI_DRAW_RIGHT_PANEL

    lda #10
    sta temp4  ; cursor x

    ; find out what section is active, starting from bottom going up
	  lda ui_selector_state
	  and #selector_state_mask_exits
    beq :+
    lda #selector_y_exits
    sta temp5  ; cursor y
    :

	  lda ui_selector_state
	  and #selector_state_mask_nearby
    beq :+
    lda #selector_y_nearby
    sta temp5  ; cursor y
    :

	  lda ui_selector_state
	  and #selector_state_mask_inv
    beq :+
    lda #selector_y_inv
    sta temp5  ; cursor y
    :

    JSR SELECTOR_DRAW_ARROW

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

	@HANDLE_KB_LEFT:

    JSR SELECTOR_CLEAR_ARROW
    ; move left
    lda temp4
    dec
    dec
    cmp #10
    bcs :+
    JSR PLAY_ERROR_BEEP
    lda #10
  :
    sta temp4

    JSR DRAW_SELECTOR_ELEMENTS

    JSR SELECTOR_DRAW_ARROW

    JMP @WAITFORINPUT

	@HANDLE_KB_RIGHT:
    JSR SELECTOR_CLEAR_ARROW
    ; move right
    JSR GET_ACTIVE_AMT_ON_LINE
    dec
    asl a
    clc
    adc #12 ; add 10 to active amt
    sta temp1
    lda temp4
    inc
    inc
    cmp temp1
    bcc :+
    JSR PLAY_ERROR_BEEP
    lda temp1
    dec
    dec
  :
    sta temp4

    JSR DRAW_SELECTOR_ELEMENTS

    JSR SELECTOR_DRAW_ARROW

    JMP @WAITFORINPUT

	@HANDLE_KB_UP:

    JSR SELECTOR_CLEAR_ARROW

    JSR DRAW_SELECTOR_ELEMENTS

    JSR SELECTOR_DRAW_ARROW

    JMP @WAITFORINPUT

	@HANDLE_KB_DOWN:

    JSR SELECTOR_CLEAR_ARROW

    JSR DRAW_SELECTOR_ELEMENTS

    JSR SELECTOR_DRAW_ARROW

    JMP @WAITFORINPUT

@HANDLE_KB_ESCAPE:

    JSR PLAY_BEEP_DESELECT

		lda #interface_mode_4way
		sta ui_state

    JSR TE_CLEAR_TEXT

    JSR UI_CLEAR_RIGHT_PANEL

    RTS


SELECTOR_DRAW_ARROW:

    draw_single_largetile #tileid_select_arrow, temp4, temp5

    RTS

SELECTOR_CLEAR_ARROW:

    draw_single_largetile #0, temp4, temp5

    RTS


    ; return in a
GET_ACTIVE_AMT_ON_LINE:
    lda temp5
    cmp #selector_y_inv
    bne :+
    lda ui_selector_active_inv
    RTS
  :
    cmp #selector_y_nearby
    bne :+
    lda ui_selector_active_nearby
    rts
  :
    cmp #selector_y_exits
    bne :+
    lda ui_selector_active_exits
    rts
  :
    lda #0

    RTS



    
