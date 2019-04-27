; titlescreen and opening logo

.segment "CODE_H"

OPENING_LOGO:
		Util_LOAD_SYM intro_filename, arg1w

		JSR FILE_LOAD_DHGR_TO_RAM

		; copy ludum logo to VRAM
		JSR CPY_IMAGE_TO_VRAM
		
		; and now start loading the titlescreen
		
		Util_LOAD_SYM title_filename, arg1w
		
		JSR FILE_LOAD_DHGR_TO_RAM		

		; wait for key before swap
		JSR ROM::GETKEY
		
		JSR CPY_IMAGE_TO_VRAM
		
		JSR ROM::GETKEY
				
		JMP SKIP_LOGO

.segment "CODE_END_H"

	CPY_IMAGE_TO_VRAM:
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
		RTS

.segment "RODATA_H"

	intro_filename:		Util_LSTR "LD44INTRO.DHGR"
	
	title_filename:		Util_LSTR "TITLESCR.DHGR"
		
		
.segment "CODE_H"

SKIP_LOGO:
