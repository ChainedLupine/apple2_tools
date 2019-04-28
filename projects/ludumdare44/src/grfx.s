; -- grfx utils --------------------------

.include "zeropage.inc"
.include "utils.inc"

.include "grfx.inc"
.include "apple2rom.inc"

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

; congrats grfx.s, you also play sound too
PLAY_ERROR_BEEP:
		ZP_SAVE
		
		lda #$10
		sta arg1
		Util_LOAD_SYM $0110, arg1w
		JSR PLAY_NOTE
		
		lda #$f0
		sta arg1
		Util_LOAD_SYM $0040, arg1w
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
  LDY arg1w    ;duration_lo
  INC arg1w+1 ;duration_hi
  ;Starting to vibe ;)
  ;These loop should fit in one page to respect the timings
@loop_half_period:
  LDA $C030             ;4 cycles //SPEAKER
  LDX arg1 ;half_period       ;3 cycles
@loop_nops:
  NOP                   ;2 cycles
  NOP                   ;2 cycles
  NOP                   ;2 cycles
  NOP                   ;2 cycles
  DEX                   ;2 cycles
  BNE @loop_nops         ;3 cycles
  ;Testing duration loop
  DEY                   ;2 cycles
  BNE @loop_half_period  ;3 cycles
  DEC arg1w+1 ;duration_hi       ;5 cycles
  BNE @loop_half_period  ;3 cycles		

	RTS



.segment "RODATA_H"

	
	