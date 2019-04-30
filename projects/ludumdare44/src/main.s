.include "apple2rom.inc"
.include "prodos.inc"
.include "zeropage.inc"
.include "utils.inc"

.include "file.inc"
.include "tileengine.inc"
.include "grfx.inc"
.include "textengine.inc"
.include "interface.inc"
.include "gamestate.inc"
.include "gameworld.inc"

.segment "CODE_H"

		ZP_SETUP

		; initial ProDOS SYSTEM setup
		JSR START_PRODOS
		
		JSR ROM::TEXT_MODE_40
		JSR ROM::CLRTXT ; home, clrscreen

		ROM_PRINT txt_loading

		LDA #$FF
		STA arg1
		JSR GRFX::DHGR_CLEAR_TO_COLOR_PAGE1
		
		JSR ROM::GFX_MODE_DHGR
		JSR ROM::GFX_DHGR_PAGE1
		
		;JMP SKIP_LOGO
		
		.include "titlescreen.inc"

		JSR GRFX_LOAD_TILES_IMAGE
	
		; -- going to graphics mode ---------------------------

		LDA #$00
		STA arg1
		JSR GRFX::DHGR_CLEAR_TO_COLOR_PAGE1

		JSR ROM::GFX_MODE_DHGR_MIXED
		JSR ROM::GFX_DHGR_PAGE1

		;JSR GRFX_COPY_TILES_TO_PAGE1

		JSR GS_RESET

		JSR TE_CLEAR_TEXT

		JSR UI_CLEAR_TOP_PANEL
		
		JSR UI_CLEAR_RIGHT_PANEL

		JSR UI_CLEAR_DISK_LOAD				
		
		; -- main game loop --------------------------
	@MAINLOOP:

		JSR UI_CHECK_ROOM_TRANSITION

		JSR UI_CHECK_INPUT

		JMP @MAINLOOP

EXIT_GETKEY:
		JSR ROM::GETKEY
		
		JSR ROM::TEXT_MODE_40
		
		JMP QUIT_TO_PRODOS
		
		; -- give control back to ProDOS ----------------------

QUIT_TO_PRODOS:
		JSR EXIT_PRODOS
		
; -- data ------------------------------------------------------

.segment "RODATA_H"

	txt_success:		.asciiz "OK!"
	txt_loading:		.asciiz "Loading..."
	
		
	
		
