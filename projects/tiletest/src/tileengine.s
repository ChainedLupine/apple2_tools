.include "apple2.inc"
.include "apple2rom.inc"
.include "zeropage.inc"
.include "tables.inc"

.segment "CODE_END_H"

		; input:
		; arg1 = tileid
		; arg2 = dst x (in tiles)
		; arg3 = dst y (in tiles)
		
		; arg1w = addr of tiles_buffer_aux
		; arg2w = addr of tiles_buffer_main
		
		; uses internally:
		; temp1 = calculated y offset in large tiles
		; temp2 = calculated tile_x in bytes
		; temp3 = calculated dst_x in bytes
		; temp4 = calculated dst_y in lines
		
		; temp1w = calculated tile_addr
		; temp2w = calculated dst_addr
		; temp3w = calculated tile addr offset in table
		
		
		.include "tileengine.inc"

		
TILEENGINE_RENDER_LARGE_TILE_PAGE1:
		LDX arg1
		LDA LARGETILE_TILEID_Y_LOOKUP, X ; tile_y = large_tile_y_lookup(tileid)
		STA temp1 	; contains y offset in tile y
	
		LDX arg1
		LDA LARGETILE_X_BYTE_LOOKUP, X ; tile_byte = large_tile_byte_lookup(tileid)
		STA temp2 	; contains tile_x in bytes
	
		LDX arg2
		LDA SMALLTILE_X_BYTE_LOOKUP, X ; dst_byte = small_tile_byte_lookup(dst_x)
		STA temp3	; dst x in bytes

		LDX arg3
		LDA SMALLTILE_Y_LOOKUP, X ; dst_y = small_tile_y_lookup(dst_y)
		STA temp4	; dst y in lines

		CLI
		
		; -- page 2 ------------------------
		;STA ROM::SW_SETINTC3ROM
		; ----------------------------------

		LDX #15		; always process 16 lines at a time for large tiles
	@LINE:
		; calculate offset in our table (based on 64:1 interleave lookups)
		; note this is relative to the actual table memory location and not actual
		; HGR memory addresses  (ie: starts at $0000, not $2000)
		TXA
		clc
		ADC temp1	; y_offset in tiles
		TAY
		LDA DATA_Y_ADDR_LOOKUP_L, Y
		clc 
		adc temp2 ; add in tile byte offset		
		STA temp3w
		LDA DATA_Y_ADDR_LOOKUP_H, Y
		STA temp3w+1
		
		; calculate dst in our table (based on actual HGR address lookups)
		clc
		TXA
		ADC temp4 ; dst_y in lines
		TAY
		LDA HGR_Y_ADDR_LOOKUP_L, Y
		clc
		adc temp3 ; add in dst byte offset
		STA temp2w
		LDA HGR_Y_ADDR_LOOKUP_H, Y
		; -- page 2 ------------------------
		;adc #$20
		; ----------------------------------
		STA temp2w+1
		
		; add tile_addr_offset to tiles_buffer_aux addr for AUX bank transfer
		clc
		LDA temp3w
		adc arg1w  		; tiles_buffer_aux
		STA temp1w	
		
		LDA temp3w+1
		adc arg1w+1 	; tiles_buffer_aux
		STA temp1w+1

		; do aux transfer
		; -- page 1 ------------------------
		STA ROM::SW_80STOREON	; enable PAGE1/2 to control MAIN/AUX banks for $2000-$3FFF
		STA ROM::SW_PAGE2		; AUX bank
		; -- page 2 ------------------------
		;STA ROM::SW_80STOREOFF	; turn off so we can use WRCARDRAM/WRMAINRAM
		;STA ROM::SW_WRCARDRAM	; AUX bank
		; ----------------------------------
		
		;   #0
		LDA (temp1w)
		STA (temp2w)
		LDY #1
		LDA (temp1w),Y
		STA (temp2w),Y
		LDY #2
		LDA (temp1w),Y
		STA (temp2w),Y
		LDY #3
		LDA (temp1w),Y
		STA (temp2w),Y
		; -- page 1 ------------------------
		STA ROM::SW_PAGE1		; MAIN bank
		STA ROM::SW_80STOREOFF		
		; -- page 2 ------------------------
		;STA ROM::SW_WRMAINRAM	; MAIN bank
		; ----------------------------------

		; add tile_addr_offset to tiles_buffer_main addr for MAIN bank transfer
		clc
		LDA temp3w
		adc arg2w		; tiles_buffer_main
		STA temp1w
		
		LDA temp3w+1
		adc arg2w+1		; tiles_buffer_main
		STA temp1w+1

		; do main transfer
		LDA (temp1w)
		STA (temp2w)
		LDY #1		
		LDA (temp1w),Y
		STA (temp2w),Y
		LDY #2
		LDA (temp1w),Y
		STA (temp2w),Y
		LDY #3
		LDA (temp1w),Y
		STA (temp2w),Y
		

		DEX
		BMI @DONE_LINES
		JMP @LINE
		
	@DONE_LINES:
		SEI
		RTS

