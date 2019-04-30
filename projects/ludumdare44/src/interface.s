; game interface
.include "zeropage.inc"
.include "utils.inc"
.include "apple2rom.inc"

.include "interface.inc"
.include "tileengine.inc"
.include "gamestate.inc"
.include "file.inc"
.include "textengine.inc"
.include "grfx.inc"
.include "gameworld.inc"

.list on
.segment "CODE_H"

UI_RESET:
		lda #$ff
		sta ui_prev_state

		lda #interface_state_4way
		sta ui_state

		RTS

UI_DRAW_TOP_RIGHT_PANEL:
		JSR UI_DRAW_HEALTH		
		JSR UI_DRAW_TIME

		RTS
		
UI_DRAW_RIGHT_PANEL:
		JSR UI_DRAW_TOP_RIGHT_PANEL

		LDA ui_state
		CMP #interface_state_4way
		BEQ @PREPARE_4_ARROW
		CMP #interface_state_selector
		BEQ @PREPARE_SELECTOR

		RTS		
	
	@PREPARE_4_ARROW:
		JSR DRAW_4WAY_ELEMENTS
		RTS

	@PREPARE_SELECTOR:
		JSR DRAW_SELECTOR_ELEMENTS
		RTS

DRAW_4WAY_ELEMENTS:
	draw_single_largetile #tileid_cmd_move, #14, #13
	draw_single_largetile #tileid_cmd_examine, #12, #15
	draw_single_largetile #tileid_cmd_center, #14, #15
	draw_single_largetile #tileid_cmd_talk, #16, #15
	draw_single_largetile #tileid_cmd_interact, #14, #17

	JSR UI_DRAW_INVENTORY
	JSR UI_DRAW_NEARBY
	RTS

DRAW_SELECTOR_ELEMENTS:

	lda ui_selector_filter
	and #selector_filter_mask_inv
	beq :+
	JSR UI_DRAW_INVENTORY
:

	lda ui_selector_filter
	and #selector_filter_mask_nearby
	beq :+
	JSR UI_DRAW_NEARBY
:

	lda ui_selector_filter
	and #selector_filter_mask_exits
	beq :+
	JSR UI_DRAW_EXITS
:
	RTS

.export UI_DRAW_INVENTORY
UI_DRAW_INVENTORY:
		ZP_SAVE
		draw_single_largetile #tileid_invbar, #10, #4
		draw_single_largetile #tileid_invbar+1, #12, #4
		draw_single_largetile #tileid_invbar+2, #14, #4
		draw_single_largetile #tileid_invbar+3, #16, #4
		draw_single_largetile #tileid_invbar+4, #18, #4

		; draw our contents
		lda #00
		sta ui_selector_active_inv_count

		lda #10
		sta temp1  ; inv x

		lda #5
		sta temp2  ; inv y

		lda #<inventory_start
		sta temp1w 
		lda #>inventory_start
		sta temp1w+1

		ldx #0

	@LOOP_INV:
		ldy #inventory_struct::typeid
		lda (temp1w),y
		beq @END_OF_INV
		; owned item, now draw it

		ldy #inventory_struct::tileid
		lda (temp1w),y
		sta temp3

		phx
		draw_single_largetile temp3, temp1, temp2
		plx

		; write to our temp list of items
		lda temp1w
		sta ui_selector_active_inv_l,x
		lda temp1w+1
		sta ui_selector_active_inv_h,x

		; increate number of active inv items in list
		inc ui_selector_active_inv_count

		; now increment cusor x
		inc temp1
		inc temp1

		Util_Inc_16_Addr_Struct temp1w, inventory_struct
		inx
		cpx #max_exits_items
		; if still items left, draw more
		bcc @LOOP_INV

	@END_OF_INV:
		ZP_RESTORE
		RTS

UI_DRAW_NEARBY:
		ZP_SAVE
		draw_single_largetile #tileid_nearbybar, #10, #9
		draw_single_largetile #tileid_nearbybar+1, #12, #9
		draw_single_largetile #tileid_nearbybar+2, #14, #9
		draw_single_largetile #tileid_nearbybar+3, #16, #9
		draw_single_largetile #tileid_nearbybar+4, #18, #9
		
		lda #00
		sta ui_selector_active_nearby_count

		lda #10
		sta temp1  ; inv x

		lda #10
		sta temp2  ; inv y

		lda #<nearby_start
		sta temp1w 
		lda #>nearby_start
		sta temp1w+1

		ldx #0

	@LOOP_NEARBY:
		ldy #nearby_struct::typeid
		lda (temp1w),y
		beq @END_OF_NEARBY

		ldy #nearby_struct::tileid
		lda (temp1w),y
		sta temp3

		phx
		draw_single_largetile temp3, temp1, temp2
		plx

		; write to our temp list of items
		lda temp1w
		sta ui_selector_active_nearby_l,x
		lda temp1w+1
		sta ui_selector_active_nearby_h,x

		; increate number of active nearby items in list
		inc ui_selector_active_nearby_count

		; now increment cursor x
		inc temp1
		inc temp1

		; nearby item isn't in here
		Util_Inc_16_Addr_Struct temp1w, nearby_struct
		inx
		cpx #max_nearby_items
		; if still items left, draw more
		bcc @LOOP_NEARBY

	@END_OF_NEARBY:
		ZP_RESTORE
		RTS

UI_DRAW_EXITS:
		ZP_SAVE
		draw_single_largetile #tileid_exitsbar, #10, #14
		draw_single_largetile #tileid_exitsbar+1, #12, #14
		draw_single_largetile #tileid_exitsbar+2, #14, #14
		draw_single_largetile #tileid_exitsbar+3, #16, #14
		draw_single_largetile #tileid_exitsbar+4, #18, #14
		
		lda #00
		sta ui_selector_active_exits_count

		lda #10
		sta temp1  ; inv x

		lda #15
		sta temp2  ; inv y

		lda #<exit_start
		sta temp1w 
		lda #>exit_start
		sta temp1w+1

		ldx #0

	@LOOP_EXITS:
		ldy #exit_struct::typeid
		lda (temp1w),y
		beq @END_OF_EXITS

		ldy #exit_struct::tileid
		lda (temp1w),y
		sta temp3

		phx
		draw_single_largetile temp3, temp1, temp2
		plx

		; write to our temp list of items
		lda temp1w
		sta ui_selector_active_exits_l,x
		lda temp1w+1
		sta ui_selector_active_exits_h,x

		; increate number of active exits in list
		inc ui_selector_active_exits_count

		; now increment cursor x
		inc temp1
		inc temp1

		Util_Inc_16_Addr_Struct temp1w, exit_struct
		inx
		cpx #max_exits_items
		; if still items left, draw more
		bcc @LOOP_EXITS

	@END_OF_EXITS:
		ZP_RESTORE
		RTS

.include "interface-rightpanel.inc"

UI_CLEAR_TOP_PANEL:
		draw_tile_prepare

		ldx #0 ; 4 smalltiles high

	@CLEAR_LINE:
		stx temp1
		phx
		draw_single_largetile #tileid_blank, #10, temp1
		draw_single_largetile #tileid_blank, #12, temp1
		draw_single_largetile #tileid_blank, #14, temp1
		draw_single_largetile #tileid_blank, #16, temp1
		draw_single_largetile #tileid_blank, #18, temp1
		plx

		inx
		inx
		cpx #4
		bcc @CLEAR_LINE
		RTS

UI_CLEAR_RIGHT_PANEL:
		draw_tile_prepare

		ldx #16 ; 14 smalltiles high

	@CLEAR_LINE:
		phx
		txa
		clc
		adc #2
		sta temp1
		draw_single_largetile #tileid_blank, #10, temp1
		draw_single_largetile #tileid_blank, #12, temp1
		draw_single_largetile #tileid_blank, #14, temp1
		draw_single_largetile #tileid_blank, #16, temp1
		draw_single_largetile #tileid_blank, #18, temp1
		plx

		dex
		dex
		bne @CLEAR_LINE
		RTS

UI_CHECK_ROOM_TRANSITION:
		; check for room transition
		lda curr_room_flags
		beq :+
			; room needs redrawing/loading
			JSR UI_SHOW_DISK_LOAD
			
			JSR GRFX_LOAD_ROOM_IMAGE
			; gets arg1 from previous load
			JSR GRFX_SHOW_ROOM_PANEL

			; now reload our tiles image
			JSR GRFX_LOAD_TILES_IMAGE
			
			; all done with disk access
			JSR UI_CLEAR_DISK_LOAD

			JSR UI_CLEAR_TOP_PANEL
			JSR UI_DRAW_TOP_RIGHT_PANEL

			lda #0
			sta curr_room_flags
		:

		rts


; blocks while waiting for input		
UI_CHECK_INPUT:

		lda ui_state
		cmp #interface_state_4way
		beq @FOURWAY_START
		cmp #interface_state_examine
		beq @COMMAND_EXAMINE
		cmp #interface_state_interact
		beq @COMMAND_INTERACT
		cmp #interface_state_interact2
		beq @COMMAND_INTERACT2
		cmp #interface_state_talk
		beq @COMMAND_TALK
		cmp #interface_state_walk
		beq @COMMAND_WALK
		cmp #interface_state_selector
		beq @COMMAND_SELECTOR

		JMP @DONE

	@FOURWAY_START:
		JSR HANDLE_INPUT_4WAY_MODE
		JMP @DONE

	@COMMAND_EXAMINE:
		JSR HANDLE_INPUT_EXAMINE
		JMP @DONE

	@COMMAND_INTERACT:
	@COMMAND_INTERACT2:
		JSR HANDLE_INPUT_INTERACT
		JMP @DONE

	@COMMAND_TALK:
		JSR HANDLE_INPUT_TALK
		JMP @DONE

	@COMMAND_WALK:
		JSR HANDLE_INPUT_WALK
		JMP @DONE

	@COMMAND_SELECTOR:
		JSR HANDLE_INPUT_SELECTOR
		JMP @DONE

	@DONE:

		RTS

.include "interface-4way.inc"
.include "interface-examine.inc"
.include "interface-interact.inc"
.include "interface-talk.inc"
.include "interface-walk.inc"
.include "interface-selector.inc"

.segment "DATA_H"

	ui_state:
		.byte $00  
		; 0 = 4-arrow mode
		; 1 = examine mode
		; 2 = talk mode
		; 3 = interact mode
		; 4 = walk mode

	ui_prev_state:
		.byte $00 ; used by selector

	ui_selector_filter:
		.byte $00	; bitflag of what to select

	; number of active inv items
	ui_selector_active_inv_count:
		.byte $00
	ui_selector_active_inv_l:
		.byte $00
		.byte $00
		.byte $00
		.byte $00
		.byte $00
	ui_selector_active_inv_h:
		.byte $00
		.byte $00
		.byte $00
		.byte $00
		.byte $00

	; number of active nearbys
	ui_selector_active_nearby_count:
		.byte $00
	ui_selector_active_nearby_l:
		.byte $00
		.byte $00
		.byte $00
		.byte $00
		.byte $00
	ui_selector_active_nearby_h:
		.byte $00
		.byte $00
		.byte $00
		.byte $00
		.byte $00

	; number of active exits
	ui_selector_active_exits_count:
		.byte $00
	ui_selector_active_exits_l:
		.byte $00
		.byte $00
		.byte $00
		.byte $00
		.byte $00
	ui_selector_active_exits_h:
		.byte $00
		.byte $00
		.byte $00
		.byte $00
		.byte $00

	ui_selector_selected:
		.addr $0000
	ui_selector_selected_extra:
		.addr $0000

.segment "RODATA_H"

		txt_enter_cmd:		.asciiz "What shall I do? [ARROW KEYS=select] "

		txt_examine:			.asciiz "Examine..."

		txt_interact:			.asciiz "Interact..."

		txt_talk:					.asciiz "Talk..."

		txt_walk:					.asciiz "Walk to..."

		txt_examine_what:			.asciiz "What am I examining? [ESC=abort, SPACE=select] "

		txt_examine_error:		.asciiz "Oh, I don't see that here... "

		txt_interact_what1:		.asciiz "What am I using? [ESC=abort, SPACE=select] "

		txt_interact_what2:		.asciiz "And the target? [ESC=abort, SPACE=select] "

		txt_interact_error:		.asciiz "That isn't possible, sorry. "

		txt_talk_what:				.asciiz "Who or what am I talking to? [ESC=abort, SPACE=select] "

		txt_talk_error:				.asciiz "Strange, who am I trying to talk to? "
		
		txt_walk_where:				.asciiz "Where are am I walking to? [ESC=abort, SPACE=select] "

		txt_walk_error:				.asciiz "I'm not sure where I am supposed to go. "

		txt_walk_locked:			.asciiz "I'm missing something.  A key?  Something else? "


		txt_debug:				.asciiz "gothereA"
		txt_debug2:				.asciiz "gothereB"
		txt_debug3:				.asciiz "gothereC"
		txt_debug4:				.asciiz "gothereD"

