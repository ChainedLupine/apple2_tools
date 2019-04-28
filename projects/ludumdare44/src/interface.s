; game interface
.include "zeropage.inc"
.include "utils.inc"
.include "apple2rom.inc"

.include "interface.inc"
.include "tileengine.inc"
.include "gamestate.inc"
.include "file.inc"
.include "textengine.inc"

.segment "CODE_H"

UI_RESET:
		lda #interface_mode_4way
		sta ui_state

		RTS


UI_LOAD_TILES:
		; -- load our tile image ------------------------------
		
		Util_LOAD_SYM tiles_filename, arg1w

		Util_LOAD_SYM tiles_img_aux, arg2w
		Util_LOAD_SYM tiles_img_main, arg3w		
		
		JSR FILE_LOAD_DHGR_TO_RAM		
		
		RTS

UI_COPY_TILES_TO_PAGE1:
		CLI		
		LDA ROM::SW_HIRES 		; need HIRES before we can use the MAIN/AUX writes below
		STA ROM::SW_80STOREON	; enable PAGE1/2 to control MAIN/AUX banks for $2000-$3FFF
		STA ROM::SW_PAGE1		; MAIN bank
		; now move load_buffer_main to $2000 in MAIN
		ROM_MEMCOPY ROM::MEM_HGR_PAGE1, tiles_img_main, $2000
		STA ROM::SW_PAGE2		; AUX bank
		; now AUXMOVE load_buffer_aux to $2000 in AUX
		ROM_MEMCOPY ROM::MEM_HGR_PAGE1, tiles_img_aux, $2000
		
		STA ROM::SW_PAGE1		; MAIN bank
		STA ROM::SW_80STOREOFF
		SEI

		rts


UI_DRAW_TOP_RIGHT_PANEL:
		; draw health icon
		JSR UI_DRAW_HEALTH		
		JSR UI_DRAW_TIME

		RTS
		
UI_DRAW_RIGHT_PANEL:
		JSR UI_DRAW_TOP_RIGHT_PANEL

		LDA ui_state
		CMP #interface_mode_4way
		BEQ @PREPARE_4_ARROW
		
		CMP #interface_mode_selector
		BEQ @PREPARE_SELECTOR
		
		RTS		
	
	@PREPARE_4_ARROW:

		JSR DRAW_4WAY_ELEMENTS

		RTS

	@PREPARE_SELECTOR:

		JSR DRAW_SELECTOR_ELEMENTS
	
		RTS

DRAW_4WAY_ELEMENTS:
	draw_single_largetile #tileid_cmd_move, #14, #14
	draw_single_largetile #tileid_cmd_examine, #12, #16
	draw_single_largetile #tileid_cmd_center, #14, #16
	draw_single_largetile #tileid_cmd_talk, #16, #16
	draw_single_largetile #tileid_cmd_interact, #14, #18

	JSR UI_DRAW_INVENTORY

	JSR UI_DRAW_NEARBY

	RTS

DRAW_SELECTOR_ELEMENTS:

	lda ui_selector_state
	and #selector_state_mask_inv
	beq :+
	JSR UI_DRAW_INVENTORY
:

	lda ui_selector_state
	and #selector_state_mask_nearby
	beq :+
	JSR UI_DRAW_NEARBY
:

	lda ui_selector_state
	and #selector_state_mask_exits
	beq :+
	;JSR UI_DRAW_EXITS
:

	RTS


UI_DRAW_INVENTORY:
		draw_single_largetile #tileid_invbar, #10, #4
		draw_single_largetile #tileid_invbar+1, #12, #4
		draw_single_largetile #tileid_invbar+2, #14, #4
		draw_single_largetile #tileid_invbar+3, #16, #4
		draw_single_largetile #tileid_invbar+4, #18, #4

		; draw our contents

		ldx #max_inv_items

		lda #10
		sta temp1  ; inv x

		lda #5
		sta temp2  ; inv y

		lda #<inventory_start
		sta temp1w 
		lda #>inventory_start
		sta temp1w+1

	@LOOP_INV:
		phx
		ldy #inventory_struct::owned
		lda (temp1w),y
		beq @DONT_OWN
		; owned item, now draw it

		ldy #inventory_struct::tileid
		lda (temp1w),y
		sta temp3

		draw_single_largetile temp3, temp1, temp2

		; now increment x to next slot
		lda temp1
		inc
		inc
		cmp #20
		bcc :+
		lda temp2
		inc
		inc
		sta temp2
		lda #10
	:
		sta temp1

	@DONT_OWN:
		; don't own item
		Util_Inc_16_Addr_Struct temp1w, inventory_struct
		plx
		dex
		; if still items left, draw more
		beq :+
		JMP @LOOP_INV
	:


		RTS

UI_DRAW_NEARBY:

		draw_single_largetile #tileid_nearbybar, #10, #9
		draw_single_largetile #tileid_nearbybar+1, #12, #9
		draw_single_largetile #tileid_nearbybar+2, #14, #9
		draw_single_largetile #tileid_nearbybar+3, #16, #9
		draw_single_largetile #tileid_nearbybar+4, #18, #9
		
		ldx #max_nearby_items

		lda #10
		sta temp1  ; inv x

		lda #10
		sta temp2  ; inv y

		lda #<nearby_start
		sta temp1w 
		lda #>nearby_start
		sta temp1w+1

	@LOOP_NEARBY:
		phx
		ldy #nearby_struct::type
		lda (temp1w),y
		beq @NOT_PRESENT
		; owned item, now draw it

		ldy #nearby_struct::tileid
		lda (temp1w),y
		sta temp3

		draw_single_largetile temp3, temp1, temp2

		; now increment x to next slot
		lda temp1
		inc
		inc
		cmp #20
		bcc :+
		lda temp2
		inc
		inc
		sta temp2
		lda #10
	:
		sta temp1

	@NOT_PRESENT:
		; nearby item isn't in here
		Util_Inc_16_Addr_Struct temp1w, nearby_struct
		plx
		dex
		; if still items left, draw more
		beq :+
		JMP @LOOP_NEARBY
	:

		RTS

.include "interface-rightpanel.inc"

UI_CLEAR_TOP_PANEL:
		draw_tile_prepare

		ldx #0 ; 4 smalltiles high

	@CLEAR_LINE:
		phx
		stx temp1
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

; blocks while waiting for input		
UI_CHECK_INPUT:

		JSR UI_DRAW_RIGHT_PANEL

		lda ui_state
		cmp #interface_mode_4way
		beq @FOURWAY_START
		cmp #interface_mode_examine
		beq @COMMAND_EXAMINE
		cmp #interface_mode_selector
		beq @SELECTOR

		JMP @DONE

	@FOURWAY_START:

		JSR HANDLE_INPUT_4WAY_MODE

		JMP @DONE

	@COMMAND_EXAMINE:

		JSR HANDLE_INPUT_EXAMINE

		JMP @DONE

	@SELECTOR:

		JSR HANDLE_INPUT_SELECTOR

		JMP @DONE

	@DONE:

		RTS

.include "interface-4way.inc"
.include "interface-examine.inc"
.include "interface-selector.inc"

.segment "DATA_H"

	tiles_img_main: .res $2000, $00   ; 8192 bytes
	tiles_img_aux: .res $2000, $00   ; 8192 bytes
	
	ui_state:
		.byte $00  
		; 0 = 4-arrow mode
		; 1 = examine mode
		; 2 = talk mode
		; 3 = interact mode
		; 4 = walk mode

	ui_selector_state:
		.byte $00	; 
		

.segment "RODATA_H"

		tiles_filename:		Util_LSTR "INGAME.DHGR"

		txt_enter_cmd:		.asciiz "What shall you do? "

		txt_examine:			.asciiz "Examine..."

		txt_examine_what:	.asciiz "What will you examine? [ESC to abort] "

		txt_debug:				.asciiz "gothere"
