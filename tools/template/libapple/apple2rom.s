.include "apple2.inc"

.scope ROM

.include "apple2rom-defs.inc"

.segment "CODE_ROM_H"

		JMP SOFTEV	; do a soft reboot if we get here

; uses COUT to print an ASCII string (not HICASE)
_ROM_PRINT:
        sta     A1L
        stx     A1H
        ldx     ROM::VERSION
        ldy     #$00
:       lda     (A1L),y
        beq     :++
        cpx     #$06            ; //e ?
        beq     :+
        cmp     #$60            ; lowercase ?
        bcc     :+
        and     #$5F            ; -> uppercase
:       ora     #$80
        jsr     COUT
        iny
        bne     :--             ; Branch always
: 		rts		

; performs mem copy (in same bank), uses ZP A1/A2/A3/A4
; A1 = dst
; A2 = src
; A3 = size
_MEMCOPY:
		; outer loop
		ldy #0
		
		ldx	A3H
		beq @LREST
		
	@HCOPY:	
		.repeat 2 
		lda	(A2L), Y
		sta (A1L), Y
		iny
		.endrepeat
		
		bne @HCOPY
		
		inc A2H
		inc A1H
		dex
		bne @HCOPY
		
	@LREST:	
		ldx A3L
		beq @DONE
		
	@LCOPY:	
		lda (A2L), Y
		sta (A1L), Y
		iny
		dex
		bne @LCOPY		
		
	@DONE:	
		RTS

; performs mem fill (in same bank), uses ZP A1/A2/A3
; A1 = dst
; A2 = value (low only)
; A3 = size
_MEMFILL:
		; outer loop
		ldy #0
		
		ldx	A3H
		beq @LREST
		
	@HCOPY:	
		.repeat 2 
		lda	A2L
		sta (A1L), Y
		iny
		.endrepeat
		
		bne @HCOPY
		
		inc A2H
		inc A1H
		dex
		bne @HCOPY
		
	@LREST:	
		ldx A3L
		beq @DONE
		
	@LCOPY:	
		lda A2L
		sta (A1L), Y
		iny
		dex
		bne @LCOPY		
		
	@DONE:	
		RTS
		
		
TEXT_MODE_40:
		STA SW_80COLOFF
		LDA SW_TEXT
		LDA SW_CLRAN3
		LDA SW_PAGE1
		JSR CLRTXT
		RTS
		
GFX_MODE_DHGR:
		LDA SW_SETAN3
		LDA SW_HIRES
		LDA SW_GR
		STA SW_80COLON
		LDA SW_FULL
		
		RTS

GFX_MODE_DHGR_MIXED:
		LDA SW_SETAN3
		LDA SW_HIRES
		LDA SW_GR
		LDA SW_MIXED
		STA SW_80COLON
		
		RTS
		
GFX_DHGR_PAGE1:
		STA SW_80STOREOFF
		LDA SW_PAGE1
		RTS

GFX_DHGR_PAGE2:
		STA SW_80STOREOFF
		LDA SW_PAGE2
		RTS
		
GETKEY:
		LDA SW_80STOREOFF
        BPL GETKEY
        STA SW_KEYSTROBE
        RTS

VBLANK:
		@vloop1: lda $c019
				bpl @vloop1
		@vloop:	lda $c019
				bmi @vloop ;wait for beginning of VBL interval
		rts
		
.endscope