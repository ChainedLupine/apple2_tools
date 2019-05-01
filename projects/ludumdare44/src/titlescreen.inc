; titlescreen and opening logo

.segment "CODE_H"

OPENING_LOGO:
		Util_LOAD_SYM intro_filename, arg1w
		Util_LOAD_SYM tiles_img_aux, arg2w
		Util_LOAD_SYM tiles_img_main, arg3w		

		JSR FILE_LOAD_DHGR_TO_RAM

		; copy ludum logo to VRAM
		JSR GRFX_COPY_TILES_BUFFER_TO_PAGE1
		
		; and now start loading the titlescreen
		
		Util_LOAD_SYM title_filename, arg1w
		
		JSR FILE_LOAD_DHGR_TO_RAM		

		; wait for key before swap
		JSR UI_WAIT_FOR_KEY
		
		JSR GRFX_COPY_TILES_BUFFER_TO_PAGE1
		
		JSR UI_WAIT_FOR_KEY
				
		JMP SKIP_LOGO

.segment "RODATA_H"

	intro_filename:		Util_LSTR "LD44INTRO.DHGR"
	
	title_filename:		Util_LSTR "TITLESCR.DHGR"
		
		
.segment "CODE_H"

SKIP_LOGO:
