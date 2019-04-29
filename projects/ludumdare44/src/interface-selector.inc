selector_y_exits = 17
selector_y_nearby = 12
selector_y_inv = 7

selector_x_min = 10

.export HANDLE_INPUT_SELECTOR

HANDLE_INPUT_SELECTOR:

		JSR UI_CLEAR_RIGHT_PANEL

    JSR UI_DRAW_RIGHT_PANEL

    ; temp3 = 0=inv, 1=nearby, 2=exits
    ; temp4 = cursor x
    ; temp5 = cursor y

    lda #selector_x_min
    sta temp4  ; cursor x    

    ; find out what section is active, starting from bottom going up
    lda ui_selector_filter
    and #selector_filter_mask_inv
    beq @no_inv
    lda #0
    sta temp3
    jmp @done
  @no_inv:

	  lda ui_selector_filter
	  and #selector_filter_mask_nearby
    beq @no_nearby
    lda #1
    sta temp3
    jmp @done
  @no_nearby:

    lda #2
    sta temp3
  @no_exits:

  @done:
    lda temp3
    JSR SEL_SET_CURSOR_Y_ON_LINE

    JSR SEL_DRAW_ARROW

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
    cmp #kb_space
    beq @HANDLE_KB_SPACE

		JMP @WAITFORINPUT

  @EXIT_INPUT:
    RTS

  @HANDLE_KB_SPACE:
    JSR PLAY_BEEP_SELECT
    ; write to first sl
    ;todo
    JSR SEL_GET_SELECTED_ITEM

    lda ui_prev_state
    sta ui_state
    JSR TE_CLEAR_TEXT

    ;JSR UI_CLEAR_RIGHT_PANEL

    JMP @EXIT_INPUT

	@HANDLE_KB_LEFT:
    JSR @MOVE_ARROW_LEFT
    JMP @WAITFORINPUT

	@HANDLE_KB_RIGHT:
    jsr @MOVE_ARROW_RIGHT
    JMP @WAITFORINPUT

	@HANDLE_KB_UP:
    jsr @MOVE_ARROW_UP
    JMP @WAITFORINPUT

	@HANDLE_KB_DOWN:
    jsr @MOVE_ARROW_DOWN
    JMP @WAITFORINPUT

@HANDLE_KB_ESCAPE:
    JSR PLAY_BEEP_DESELECT

		lda #interface_state_4way
		sta ui_state

    lda #$ff
    sta ui_prev_state

    JSR TE_CLEAR_TEXT

    JSR UI_CLEAR_RIGHT_PANEL

    JMP @EXIT_INPUT

@MOVE_ARROW_LEFT:
    JSR SEL_CLEAR_ARROW
    ; move left
    JSR SEL_GET_ACTIVE_AMT_ON_LINE
    beq :++
    lda temp4
    dec
    dec
    cmp #selector_x_min
    bcs :+
    JSR PLAY_ERROR_BEEP
    lda #selector_x_min
  :
    sta temp4

    ;JSR DRAW_SELECTOR_ELEMENTS
    JSR SEL_DRAW_ARROW
    JSR PLAY_BEEP_SELECT
    rts
  :
    JSR PLAY_ERROR_BEEP
    JSR SEL_DRAW_ARROW
    rts

@MOVE_ARROW_RIGHT:
    JSR SEL_CLEAR_ARROW
    ; move right
    JSR SEL_GET_ACTIVE_AMT_ON_LINE
    beq :++
    dec
    asl a
    clc
    adc #selector_x_min ; add 10 to active amt
    adc #2
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

    ;JSR DRAW_SELECTOR_ELEMENTS
    JSR SEL_DRAW_ARROW
    JSR PLAY_BEEP_SELECT
    rts
  :
    JSR PLAY_ERROR_BEEP
    JSR SEL_DRAW_ARROW
    rts

@MOVE_ARROW_UP:
    JSR SEL_CAN_MOVE_UP
    bcc :+

    JSR SEL_CLEAR_ARROW

    JSR SEL_NEXT_LINE_UP

    sta temp3
    JSR SEL_SET_CURSOR_Y_ON_LINE

    lda #selector_x_min
    sta temp4

    ;JSR DRAW_SELECTOR_ELEMENTS

    JSR SEL_DRAW_ARROW

    JSR PLAY_BEEP_SELECT
    rts
  :
    JSR PLAY_ERROR_BEEP
    rts

@MOVE_ARROW_DOWN:
    JSR SEL_CAN_MOVE_DOWN
    bcc :+

    JSR SEL_CLEAR_ARROW

    JSR SEL_NEXT_LINE_DOWN
    sta temp3
    JSR SEL_SET_CURSOR_Y_ON_LINE

    lda #selector_x_min
    sta temp4

    ;JSR DRAW_SELECTOR_ELEMENTS

    JSR SEL_DRAW_ARROW

    JSR PLAY_BEEP_SELECT
    rts
  :
    JSR PLAY_ERROR_BEEP
    rts

SEL_DRAW_ARROW:
    draw_single_largetile #tileid_select_arrow, temp4, temp5
    RTS

SEL_CLEAR_ARROW:
    draw_single_largetile #0, temp4, temp5
    RTS


.export SEL_CAN_MOVE_DOWN
    ; carry set if possible, otherwise no
SEL_CAN_MOVE_DOWN:
    lda temp3   ; inv line
    cmp #0
    bne @skip_to_nearby

    ; our next line down is nearby
	  lda ui_selector_filter
	  and #selector_filter_mask_nearby
    beq :+ ; not active
    sec
    rts
  :
    ; our next line down is exits
	  lda ui_selector_filter
	  and #selector_filter_mask_exits
    beq :+ ; not active
    sec
    rts
  :
    clc
    rts ; no valid next

  @skip_to_nearby:
    ; now check if nearby
    lda temp3   ; nearby line
    cmp #1
    bne @skip_to_exit

    ; our next line down is exits
	  lda ui_selector_filter
	  and #selector_filter_mask_exits
    beq :+ ; not active
    sec
    rts
  :

  @skip_to_exit:
    ; easy, this is last entry, can never go down
    clc
    rts

    ; carry set if possible, otherwise no
SEL_CAN_MOVE_UP:
    lda temp3   ; exits line
    cmp #2
    bne @skip_to_nearby

    ; our prev line up is nearby
	  lda ui_selector_filter
	  and #selector_filter_mask_nearby
    beq :+ ; not active
    sec
    rts
  :
    ; our prev line up is inv
	  lda ui_selector_filter
	  and #selector_filter_mask_inv
    beq :+ ; not active
    sec
    rts
  :
    clc
    rts ; no valid prev

  @skip_to_nearby:
    lda temp3   ; nearby line
    cmp #1
    bne @skip_to_inv

    ; our prev line up is inv
	  lda ui_selector_filter
	  and #selector_filter_mask_inv
    beq :+ ; not active
    sec
    rts
  :

  @skip_to_inv:
    ; easy, this is top entry, can never go up
    clc
    rts

    ; input: 
    ;   none
    ; return amount active in a
SEL_GET_ACTIVE_AMT_ON_LINE:
    lda temp3
    cmp #1
    bne :+
    lda ui_selector_active_nearby_count
    rts
  :
    cmp #2
    bne :+
    lda ui_selector_active_exits_count
    rts
  :
    lda ui_selector_active_inv_count
    RTS

    ; input: 
    ;   a is cursor y
    ; return is active line (0=inv, 1=nearby, 2=exits)
SEL_GET_ACTIVE_TYPE_LINE:
    cmp #selector_y_nearby
    bne :+
    lda #1
    rts
  :
    cmp #selector_y_exits
    bne :+
    lda #2
    rts
  :
    lda #0
    RTS

    ; a = line to use (0=inv, 1=nearby, 2=exits)
    ; returns nothing
SEL_SET_CURSOR_Y_ON_LINE:
    cmp #2
    bne :+
    lda #selector_y_exits
    sta temp5  ; cursor y
    rts
  :
    cmp #1
    bne :+
    lda #selector_y_nearby
    sta temp5  ; cursor y
    rts
  :  
    lda #selector_y_inv
    sta temp5  ; cursor y
    rts

SEL_NEXT_LINE_DOWN:
    lda temp3
    cmp #0 ; start at inv
    bne @check_at_nearby
    
    lda ui_selector_filter
    and #selector_filter_mask_nearby
    beq :+ ; not active
    lda #1 ; select nearby line
    rts
  :
    lda ui_selector_filter
    and #selector_filter_mask_exits
    beq :+ ; not active
    lda #2 ; select exit line
  :
    rts
  
  @check_at_nearby:
    lda ui_selector_filter
    and #selector_filter_mask_exits
    beq :+ ; not active
    lda #2 ; select exit line
  :
    rts

    ; returns new line state in a (0=inv, 1=nearby, 2=exit)
SEL_NEXT_LINE_UP:
    lda temp3
    cmp #2 ; if we are at exits
    bne @check_at_nearby

    lda ui_selector_filter
    and #selector_filter_mask_nearby
    beq :+ ; not active
    lda #1 ; select nearby line
    rts
  :
    lda ui_selector_filter
    and #selector_filter_mask_inv
    beq :+ ; not active
    lda #0 ; select inv line
  :
    rts

  @check_at_nearby:
    lda ui_selector_filter
    and #selector_filter_mask_inv
    beq :+ ; not active
    lda #0 ; select inv line
  :
    rts

  ; writes into ui_selector_selected
.export SEL_GET_SELECTED_ITEM
SEL_GET_SELECTED_ITEM:
    lda temp4
    sec
    sbc #selector_x_min
    lsr
    sta temp4 ; should now be a 0-based index into list

    lda temp3
    cmp #0
    bne @try_nearby

    ; we are selecting an inv item
    lda ui_selector_active_inv_count
    beq @NO_ITEM

    ldy temp4
    lda ui_selector_active_inv_l,y
    sta ui_selector_selected
    lda ui_selector_active_inv_h,y
    sta ui_selector_selected+1
    jmp @DONE

  @try_nearby:
    lda temp3
    cmp #1
    bne @try_exits

    ; we are selecting a nearby item
    lda ui_selector_active_nearby_count
    beq @NO_ITEM

    ldy temp4
    lda ui_selector_active_nearby_l,y
    sta ui_selector_selected
    lda ui_selector_active_nearby_h,y
    sta ui_selector_selected+1
    jmp @DONE

  @try_exits:

    ; we are selecting an exit
    lda ui_selector_active_exits_count
    beq @NO_ITEM

    ldy temp4
    lda ui_selector_active_exits_l,y
    sta ui_selector_selected
    lda ui_selector_active_exits_h,y
    sta ui_selector_selected+1
    jmp @DONE

  @NO_ITEM:
      ; no item!
      lda #$ff
      sta ui_selector_selected
      lda #$ff
      sta ui_selector_selected+1
      rts

  @DONE:
      rts
