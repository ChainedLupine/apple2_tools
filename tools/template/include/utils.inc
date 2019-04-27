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

