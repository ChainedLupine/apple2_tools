; ------------------------------------------------
; -- UTILITY macros
; ------------------------------------------------

; shove symbol into some memory location
.macro Util_LOAD_SYM addr, dst
		LDA		#<addr
		LDX		#>addr
		STA		dst
		STX		dst+1
.endmacro

; shove one memory location into another memory location
.macro Util_LOAD_ADDR addr, dst
		LDA		addr
		LDX		addr+1
		STA		dst
		STX		dst+1
.endmacro

; PASCAL-like string (first byte is length)
.macro Util_LSTR str
	.byte .strlen(str), str
.endmacro

.macro Util_Inc_16_Addr addr, amt
	.if .blank(amt) 	; increment by 1
		inc addr
		bne :+
		inc addr+1
	:
	.else 					 	; increment by amt
		lda addr
		clc
		adc #amt
		bcc :+
		sta addr
		inc addr+1
	:
	.endif
.endmacro

.macro Util_Inc_16_Addr_Struct addr, struct
	.if .blank(amt) 	; increment by 1
		inc addr
		bne :+
		inc addr+1
	:
	.else 					 	; increment by amt
		lda addr
		clc
		;.byte $69 
		;.byte .sizeof (struct)
		adc #.sizeof(struct)
		bcc :+
		inc addr+1
	:
		sta addr
	.endif
.endmacro

