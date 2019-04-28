; game interface
.include "zeropage.inc"
.include "utils.inc"
.include "apple2rom.inc"

.include "interface.inc"
.include "tileengine.inc"
.include "gamestate.inc"
.include "file.inc"

.segment "CODE_H"

largetile_width 		= 10
smalltime_width			= 20
; icon locations in tilemap
tileid_blank				= 0
tileid_disk_icon		= (largetile_width * 11 + 9)

tileid_moon1_icon		= (largetile_width * 10 + 2)
tileid_moon2_icon		= (largetile_width * 11 + 2)
tileid_moon3_icon		= (largetile_width * 11 + 3)
tileid_moon4_icon		= (largetile_width * 10 + 3)

tileid_health1_icon		= (largetile_width * 10 + 4)
tileid_health2_icon		= (largetile_width * 11 + 4)
tileid_health3_icon		= (largetile_width * 11 + 5)
tileid_health4_icon		= (largetile_width * 10 + 5)

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
		CMP #0
		BNE @DRAW_4_ARROW
		
		CMP #1
		BEQ @DRAW_OTHER
		
		RTS		
	
	@DRAW_4_ARROW:

		RTS

	@DRAW_OTHER:
	
		RTS
		
UI_DRAW_HEALTH:
		draw_tile_prepare

		; upper left
		lda #tileid_health1_icon
		sta temp1

		lda state_health
		cmp #1
		bcs :+ ; if >=1
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #16, #0

		; lower left
		lda #tileid_health2_icon
		sta temp1

		lda state_health
		cmp #2
		bcs :+ ; if >=2
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #16, #2

		; lower right
		lda #tileid_health3_icon
		sta temp1

		lda state_health
		cmp #3
		bcs :+ ; if >=3
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #18, #2

		; upper right
		lda #tileid_health4_icon
		sta temp1

		lda state_health
		cmp #4
		bcs :+ ; if >=4
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #18, #0

		RTS
		
UI_DRAW_TIME:

		draw_tile_prepare

		; upper left
		lda #tileid_moon1_icon
		sta temp1

		lda state_time
		cmp #1
		bcs :+ ; if >=1
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #10, #0

		; lower left
		lda #tileid_moon2_icon
		sta temp1

		lda state_time
		cmp #2
		bcs :+ ; if >=2
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #10, #2

		; lower right
		lda #tileid_moon3_icon
		sta temp1

		lda state_time
		cmp #3
		bcs :+ ; if >=3
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #12, #2

		; upper right
		lda #tileid_moon4_icon
		sta temp1

		lda state_time
		cmp #4
		bcs :+ ; if >=4
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #12, #0

		RTS
	

UI_SHOW_DISK_LOAD:
		draw_tile_prepare

		draw_single_largetile #tileid_disk_icon, #18, #18
		
		RTS
		
UI_CLEAR_DISK_LOAD:
		draw_tile_prepare

		draw_single_largetile #tileid_blank, #18, #18

		RTS

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
		
UI_CHECK_INPUT:

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

		RTS



.segment "DATA_H"

	tiles_img_main: .res $2000, $00   ; 8192 bytes
	tiles_img_aux: .res $2000, $00   ; 8192 bytes
	
	ui_state:
		.byte $00  ; 4-arrow mode
		

.segment "RODATA_H"

		tiles_filename:		Util_LSTR "INGAME.DHGR"
