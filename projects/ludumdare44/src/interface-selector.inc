
.export HANDLE_INPUT_SELECTOR

HANDLE_INPUT_SELECTOR:

		JSR UI_CLEAR_RIGHT_PANEL
		JSR UI_DRAW_RIGHT_PANEL

		;Util_LOAD_SYM txt_debug, arg1w
		;JSR TE_PRINT_TEXT

    JSR UI_DRAW_RIGHT_PANEL

    lda #10
    sta temp4  ; cursor x

    lda #7
    sta temp5  ; cursor y

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
		cmp #kb_escape
		beq @HANDLE_KB_ESCAPE

		JMP @WAITFORINPUT

	@HANDLE_KB_LEFT:

    JSR DRAW_SELECTOR_ELEMENTS

    JSR SELECTOR_DRAW_ARROW

    JMP @WAITFORINPUT

@HANDLE_KB_ESCAPE:

		lda #interface_mode_4way
		sta ui_state

    JSR TE_CLEAR_TEXT

    JSR UI_CLEAR_RIGHT_PANEL

    RTS


SELECTOR_DRAW_ARROW:

    draw_single_largetile #tileid_select_arrow, temp4, temp5

    RTS
