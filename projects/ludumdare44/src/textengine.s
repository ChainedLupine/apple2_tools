.include "textengine.inc"
.include "zeropage.inc"
.include "apple2rom.inc"

.segment "CODE_END_H"

SPACE = $A0

TE_CLEAR_TEXT:
		LDA #20		
	:
		STA arg1
		PHA
		JSR TE_CLEAR_TEXTLINE
		PLA
		INC
		CMP #24
		BCC :-
		
		; reset cursor also
		LDA #0
		STA TLINE_CURSOR_X
		LDA #20
		STA TLINE_CURSOR_Y
		
		RTS	

	; arg1 = line to clear
TE_CLEAR_TEXTLINE:
		
		LDY arg1
		LDA TLINE_ADDR_L,Y
		STA TLINE_PTR_TEMP
		
		LDA TLINE_ADDR_H,Y
		STA TLINE_PTR_TEMP+1
		
		ROM_MEMFILL_ADDR_ABS TLINE_PTR_TEMP, SPACE, $28
		CLI
		STA ROM::SW_80STOREON	; enable PAGE1/2 to control MAIN/AUX banks for $2000-$3FFF
		STA ROM::SW_PAGE2		
		; text lines are 40/0x28 bytes wide
		ROM_MEMFILL_ADDR_ABS TLINE_PTR_TEMP, SPACE, $28
		STA ROM::SW_PAGE1		; MAIN bank
		STA ROM::SW_80STOREOFF
		SEI
		
		RTS

	; arg1w = ptr to text
	; uses whatever the cursor is set to
TE_PRINT_TEXT:		
		lda arg1w
		ldx arg1w+1
		sta temp1w
		stx temp1w+1 
		
	@HPRINT:	
		lda (temp1w)
		BEQ @DONE
		
		cmp #$5E ; caret is CR
		bne @NO_LINE_ADV
		
		; detected line advance
		JSR TE_CARRIAGE_RETURN
		JMP @NEXT
		
	@NO_LINE_ADV:		
		sta arg1
		JSR TE_OUTPUT_CHAR

	@NEXT:
		inc temp1w
		BNE @HPRINT
		
		; increment hi byte
		inc temp1w+1
		
		bne @HPRINT		
		
	@DONE:
		JSR TE_SET_VISUAL_CURSOR
		
		RTS
		
		; arg1 = char
TE_OUTPUT_CHAR:
		ldy TLINE_CURSOR_Y
		LDA TLINE_ADDR_L,Y
		STA temp5w
		
		LDA TLINE_ADDR_H,Y
		STA temp5w+1
		
		lda TLINE_CURSOR_X ; check to see if cursor is on even or odd row
		bit #%00000001
		beq @EVEN_X
		; odd x

		lda TLINE_CURSOR_X
		lsr
		tay		

		lda arg1
		ora #$80
		;ora #$40
		STA (temp5w),Y
		JMP :+

	@EVEN_X:
		CLI
		STA ROM::SW_80STOREON
		STA ROM::SW_PAGE2
		; text lines are 40/0x28 bytes wide
		lda TLINE_CURSOR_X
		lsr
		tay		
		
		lda arg1
		ora #$80
		;ora #$40
		STA (temp5w),Y
		STA ROM::SW_PAGE1
		STA ROM::SW_80STOREOFF
		SEI
		
	:	
		JSR TE_ADV_CURSOR
		
		RTS

TE_OUTPUT_CHAR_INV:
		ldy TLINE_CURSOR_Y
		LDA TLINE_ADDR_L,Y
		STA temp5w
		
		LDA TLINE_ADDR_H,Y
		STA temp5w+1
		
		lda TLINE_CURSOR_X ; check to see if cursor is on even or odd row
		bit #%00000001
		beq @EVEN_X
		; odd x

		lda TLINE_CURSOR_X
		lsr
		tay		

		lda arg1
		ora #$40
		STA (temp5w),Y
		JMP :+

	@EVEN_X:
		CLI
		STA ROM::SW_80STOREON
		STA ROM::SW_PAGE2
		; text lines are 40/0x28 bytes wide
		lda TLINE_CURSOR_X
		lsr
		tay		
		
		lda arg1
		ora #$40
		STA (temp5w),Y
		STA ROM::SW_PAGE1
		STA ROM::SW_80STOREOFF
		SEI
		
	:	
		;JSR TE_ADV_CURSOR
		
		RTS
		
TE_ADV_CURSOR:
		lda TLINE_CURSOR_X
		inc
		cmp #80
		bcc @NORESETX
		
		lda TLINE_CURSOR_Y
		inc
		cmp #24
		bcc @NORESETY
		
		lda #20
		
	@NORESETY:
		sta TLINE_CURSOR_Y
		lda #0
		
	@NORESETX:
		sta TLINE_CURSOR_X
		
		RTS
		
TE_CARRIAGE_RETURN:
		lda #0
		sta TLINE_CURSOR_X
		
		lda TLINE_CURSOR_Y
		inc
		cmp #24
		bcc @NORESETY
		
		lda #20
		
	@NORESETY:
		sta TLINE_CURSOR_Y
		
		JSR TE_SET_VISUAL_CURSOR
		
		RTS
		
TE_SET_VISUAL_CURSOR:
		lda arg1
		pha
		lda #$20
		sta arg1
		JSR TE_OUTPUT_CHAR_INV
		pla
		sta arg1
		RTS
	
.segment "RODATA_H"

	TLINE_PTR_TEMP:
		.addr $0000
		
	TLINE_CURSOR_X:
		.byte $00  ; x
	TLINE_CURSOR_Y:
		.byte $00  ; y

	TLINE_ADDR_L:
		.byte $00 ; 0
		.byte $80
		.byte $00
		.byte $80
		.byte $00
		.byte $80
		.byte $00 ; 6
		.byte $80
		.byte $28
		.byte $A8
		.byte $28
		.byte $A8
		.byte $28 ; 12
		.byte $A8
		.byte $28
		.byte $A8
		.byte $50
		.byte $D0
		.byte $50
		.byte $D0 ; 19
		.byte $50 ; 20
		.byte $D0
		.byte $50
		.byte $D0 ; 23
		
	TLINE_ADDR_H:
		.byte $04 ; 0
		.byte $04
		.byte $05
		.byte $05
		.byte $06
		.byte $06
		.byte $07 ; 6
		.byte $07
		.byte $04
		.byte $04
		.byte $05
		.byte $05
		.byte $06 ; 12
		.byte $06
		.byte $07
		.byte $07
		.byte $04
		.byte $04
		.byte $05
		.byte $05 ; 19
		.byte $06
		.byte $06
		.byte $07
		.byte $07 ; 23		