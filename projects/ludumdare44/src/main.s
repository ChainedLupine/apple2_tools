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

.forceimport HANDLE_INPUT_SELECTOR
.forceimport ui_selector_selected, ui_selector_selected_extra
.forceimport GW_PRINT_CURR_ROOM_DESC
.forceimport HANDLE_INPUT_EXAMINE
.forceimport SEL_GET_SELECTED_ITEM

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

		
		JMP SKIP_INTRO
		
		.include "titlescreen.inc"

	SKIP_INTRO:
	
		JSR UI_LOAD_TILES
	
		; -- going to graphics mode ---------------------------

		JSR ROM::GFX_MODE_DHGR_MIXED
		JSR ROM::GFX_DHGR_PAGE1

		;JSR UI_COPY_TILES_TO_PAGE1

		JSR GS_RESET

		JSR TE_CLEAR_TEXT

		JSR UI_CLEAR_TOP_PANEL
		
		JSR UI_CLEAR_RIGHT_PANEL

		JSR UI_CLEAR_DISK_LOAD
				
		JMP @NOCPY
		
		lda #05
		lda #06
		lda #07
		
		LDA #<txt_test
		STA arg1w
		LDA #>txt_test
		STA arg1w+1
		JSR TE_PRINT_TEXT

		LDA #<txt_test2
		STA arg1w
		LDA #>txt_test2
		STA arg1w+1
		JSR TE_PRINT_TEXT

		
		
		;JSR ROM::GETKEY

		LDA #<txt_test2
		STA arg1w
		LDA #>txt_test2
		STA arg1w+1
		JSR TE_PRINT_TEXT
		
		JMP EXIT_GETKEY
		
		
		JMP EXIT_GETKEY
		
@NOCPY:
		; -- main game loop --------------------------

		
		
	
	@MAINLOOP:

		JSR UI_CHECK_INPUT

		JMP @MAINLOOP

		JSR UI_DRAW_INVENTORY
		
		
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
							   ;01234567890123456789012345678901234567890123456789012345678901234567890123456789
	txt_test:			.asciiz "This is a fairly long line to test our text writing routine on.  We want to see^", "if it can handle multiple lines and so on.  This is real simple kind of test^", "here, ayuh.^"
	
	txt_test2:			.asciiz "another test!"
	
		
	
		
