; Apple 2 ROM helper routines
.scope ROM

.include "apple2.inc"
.include "apple2rom-defs.inc"

.macro ROM_PRINT addr
		lda     #<addr
        ldx     #>addr
		jsr ROM::_ROM_PRINT
.endmacro

.enum   EAUXMOVE_MODE
                TO_MAIN	= 0
                TO_AUX	= 1
        .endenum
		
.macro ROM_AUXMOVE dstaddr, srcaddr, size, toaux
		.if (toaux = 1)
			SEC
		.else
			CLC
		.endif
		
		LDA #<srcaddr
		STA ROM::A1L
		LDA #>srcaddr
		STA ROM::A1H
		
		LDA #<(srcaddr + size)
		STA ROM::A2L
		LDA #>(srcaddr + size)
		STA ROM::A2H
		
		LDA #<dstaddr
		STA ROM::A4L
		LDA #>dstaddr
		STA ROM::A4H
		
		JSR ROM::AUXMOVE
.endmacro

.macro ROM_MEMCOPY dstaddr, srcaddr, size
		LDA #<dstaddr
		STA ROM::A1L
		LDA #>dstaddr
		STA ROM::A1H		
		
		LDA #<srcaddr
		STA ROM::A2L
		LDA #>srcaddr
		STA ROM::A2H
				
		LDA #<size
		STA ROM::A3L
		LDA #>size
		STA ROM::A3H
				
		JSR ROM::_MEMCOPY
.endmacro

.macro ROM_MEMFILL dstaddr, val, size
		LDA #<dstaddr
		STA ROM::A1L
		LDA #>dstaddr
		STA ROM::A1H		
		
		LDA val
		STA ROM::A2L
				
		LDA #<size
		STA ROM::A3L
		LDA #>size
		STA ROM::A3H
				
		JSR ROM::_MEMCOPY
.endmacro


.endscope