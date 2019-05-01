; -- grfx utils --------------------------

.include "zeropage.inc"
.include "utils.inc"

.include "interface.inc"
.include "gameworld.inc"
.include "grfx.inc"
.include "file.inc"
.include "apple2rom.inc"
.include "tileengine.inc"

.segment "CODE_END_H"

	; arg1 = color to use
GRFX::DHGR_CLEAR_TO_COLOR_PAGE1:
		CLI
		LDA ROM::SW_HIRES 		; need HIRES before we can use the MAIN/AUX writes below
		STA ROM::SW_80STOREON	; enable PAGE1/2 to control MAIN/AUX banks for $2000-$3FFF
		STA ROM::SW_PAGE1		; MAIN bank
		ROM_MEMFILL ROM::MEM_HGR_PAGE1, arg1, $2000
		STA ROM::SW_PAGE2		; AUX bank
		ROM_MEMFILL ROM::MEM_HGR_PAGE1, arg1, $2000
		
		STA ROM::SW_PAGE1		; MAIN bank
		STA ROM::SW_80STOREOFF
		SEI

	RTS

	; arg1 = color to use
GRFX::DHGR_CLEAR_TO_COLOR_SLOW:
		CLI
		LDA ROM::SW_HIRES 		; need HIRES before we can use the MAIN/AUX writes below
		STA ROM::SW_80STOREON	; enable PAGE1/2 to control MAIN/AUX banks for $2000-$3FFF
		STA ROM::SW_PAGE1		; MAIN bank
		ROM_MEMFILL ROM::MEM_HGR_PAGE1, arg1, $2000
		STA ROM::SW_PAGE2		; AUX bank
		ROM_MEMFILL ROM::MEM_HGR_PAGE1, arg1, $2000
		
		STA ROM::SW_PAGE1		; MAIN bank
		STA ROM::SW_80STOREOFF
		SEI

	RTS

GRFX_LOAD_TILES_IMAGE:
		; -- load our tile image ------------------------------		
		Util_LOAD_SYM tiles_filename, arg1w
		Util_LOAD_SYM tiles_img_aux, arg2w
		Util_LOAD_SYM tiles_img_main, arg3w		
		
		JSR FILE_LOAD_DHGR_TO_RAM		
		
		RTS

	; arg1 - typeid of room to load
GRFX_LOAD_ROOM_IMAGE:

		; get filename into arg1w and panel into arg1
		JSR GW_GET_ROOM_FILENAME_AND_PANEL		

		Util_LOAD_SYM tiles_img_aux, arg2w
		Util_LOAD_SYM tiles_img_main, arg3w		
		
		JSR FILE_LOAD_DHGR_TO_RAM

		rts

	; arg1 - panel to show (0=left, 1=right)
	; image should already be loaded into the tiles_img buffer
GRFX_SHOW_ROOM_PANEL:
		lda arg1
		beq :+
			lda #5
			sta arg1
		:
		JSR GRFX_COPY_HALF_LINE_TILES_BUFFER_TO_PAGE1
		
		rts

GRFX_COPY_TILES_BUFFER_TO_PAGE1:
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

		; arg1 = tile offset
GRFX_COPY_HALF_LINE_TILES_BUFFER_TO_PAGE1:
		lda arg1
		sta temp2
		sta temp3

		draw_tile_prepare

		ldx #0
	@DRAW_LINE:
		stx temp1
		
		phx
		lda temp3
		sta temp2
		draw_single_largetile temp2, #0, temp1
		inc temp2
		draw_single_largetile temp2, #2, temp1
		inc temp2
		draw_single_largetile temp2, #4, temp1
		inc temp2
		draw_single_largetile temp2, #6, temp1
		inc temp2
		draw_single_largetile temp2, #8, temp1
		plx

		lda temp3
		clc
		adc #10
		sta temp3

		inx
		inx
		cpx #20
		bcc @DRAW_LINE
		RTS


; congrats grfx.s, you also play sound too
PLAY_ERROR_BEEP:
		ZP_SAVE
		
		lda #$30
		sta arg1
		Util_LOAD_SYM $0090, arg1w
		JSR PLAY_NOTE
		
		lda #$f0
		sta arg1
		Util_LOAD_SYM $0040, arg1w
		JSR PLAY_NOTE

		ZP_RESTORE

		RTS

PLAY_BEEP_SAD:
		ZP_SAVE
		
		lda #$e0
		sta arg1
		Util_LOAD_SYM $0025, arg1w
		JSR PLAY_NOTE
		
		lda #$e7
		sta arg1
		Util_LOAD_SYM $0040, arg1w
		JSR PLAY_NOTE

		ZP_RESTORE

		RTS

PLAY_BEEP_MENTAL_LOSS:
		ZP_SAVE
		
		lda #$10
		sta arg1
		Util_LOAD_SYM $0225, arg1w
		JSR PLAY_NOTE
		
		lda #$15
		sta arg1
		Util_LOAD_SYM $0180, arg1w
		JSR PLAY_NOTE

		lda #$35
		sta arg1
		Util_LOAD_SYM $0150, arg1w
		JSR PLAY_NOTE

		ZP_RESTORE

		RTS


PLAY_BEEP_DESELECT:
		ZP_SAVE
		
		lda #$30
		sta arg1
		Util_LOAD_SYM $030, arg1w
		JSR PLAY_NOTE
		
		lda #$60
		sta arg1
		Util_LOAD_SYM $0030, arg1w
		JSR PLAY_NOTE

		ZP_RESTORE

		RTS

PLAY_BEEP_SELECT:
		ZP_SAVE
		
		lda #$40
		sta arg1
		Util_LOAD_SYM $0020, arg1w
		JSR PLAY_NOTE
		
		lda #$20
		sta arg1
		Util_LOAD_SYM $0040, arg1w
		JSR PLAY_NOTE

		ZP_RESTORE

		RTS

PLAY_NOTE:
	; arg1 = frequency
	; arg1w = duration	
  CLC
  LDY arg1w    	;duration
  INC arg1w+1 	;duration
@loop_freq:
  LDA $C030
  LDX arg1
@idleloop:
  NOP
  NOP
  NOP
  NOP
  DEX
  BNE @idleloop
  
  DEY
  BNE @loop_freq
  DEC arg1w+1
  BNE @loop_freq

	RTS

.segment "DATA_H"
	tiles_img_main: .res $2000, $00   ; 8192 bytes
	tiles_img_aux: .res $2000, $00   ; 8192 bytes	

.segment "RODATA_H"

	tiles_filename:		Util_LSTR "INGAME.DHGR"
	

	