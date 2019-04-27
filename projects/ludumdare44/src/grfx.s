; -- grfx utils --------------------------

.include "grfx.inc"
.include "apple2rom.inc"
.include "zeropage.inc"

.segment "CODE_END_H"

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

.segment "RODATA_H"

	
	