.include "apple2rom.inc"
.include "prodos.inc"
.include "zeropage.inc"
.include "utils.inc"

.include "file.inc"
.include "tileengine.inc"
.include "grfx.inc"

.segment "CODE_H"

		; initial ProDOS SYSTEM setup
		JSR START_PRODOS
		
		JSR ROM::TEXT_MODE_40
		JSR ROM::CLRTXT ; home, clrscreen

		ROM_PRINT txt_loading

		; -- load our tile image ------------------------------
		
		Util_LOAD_SYM tiles_filename, arg1w
		Util_LOAD_SYM tiles_img_aux, arg2w
		Util_LOAD_SYM tiles_img_main, arg3w
		
		JSR FILE_LOAD_DHGR_TO_RAM
		
		; -- going to graphics mode ---------------------------

		
		JSR ROM::GFX_MODE_DHGR
		JSR ROM::GFX_DHGR_PAGE1

		LDA #$44
		STA arg1
		JSR GRFX::DHGR_CLEAR_TO_COLOR_PAGE1
		
		;JMP @NOCPY
		JMP EXIT_GETKEY
		
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
		
;		JMP EXIT_GETKEY
		
@NOCPY:
		; -- prepare to render tiles --------------------------

		; place our tile image buffers into arg1 (aux) and arg2 (main)
		Util_LOAD_SYM tiles_img_aux, arg1w
		Util_LOAD_SYM tiles_img_main, arg2w
	
		LDA #0
		STA arg1 ; tileid
		LDA #0
		STA arg2 ; dst x in small tiles
		LDA #0
		STA arg3 ; dst y in small tiles
	
	@MAINLOOP:
		
	@RLOOP:
	
		JSR TILEENGINE_RENDER_LARGE_TILE_PAGE1
		
		;JMP @INC
		;JMP EXIT_GETKEY
		
	@INC:
		; just blindly increment tiles
		INC arg1
		CMP #120
		BNE :+
		LDA #0
		STA arg1
		
	:	LDA arg2
		INC
		INC
		CMP #20
		BNE @NORESETX
		; end of width, reset
		LDA 0
		STA arg2

		; and increment y
		LDA arg3
		INC
		INC
		CMP #4
		BNE :+
		LDA 0
		STA arg1 ; reset tile to 0 too
	:	STA arg3	
	
		JMP @MAINLOOP
		
	@NORESETX:
		STA arg2
	
		JMP @RLOOP
		
EXIT_GETKEY:
		JSR ROM::GETKEY
		
		JSR ROM::TEXT_MODE_40
		
		JMP QUIT_TO_PRODOS
		
		; -- give control back to ProDOS ----------------------

QUIT_TO_PRODOS:
		JSR EXIT_PRODOS
		
; -- data ------------------------------------------------------

.segment "RODATA_H"

	tiles_filename:		Util_LSTR "TILEIMG.DHGR"
	;tiles_filename:		Util_LSTR "HYENAS2.DHGR"
	txt_success:		.asciiz "OK!"
	txt_loading:		.asciiz "Loading..."
	
		
.segment "DATA_H"

	tiles_img_main: .res $2000, $00   ; 8192 bytes
	tiles_img_aux: .res $2000, $00   ; 8192 bytes
	
	
	
	
	
		
