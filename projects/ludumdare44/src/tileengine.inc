; common interface for the tile engine

.global TILEENGINE_RENDER_LARGE_TILE
.global TILEENGINE_RENDER_LARGE_TILE_ANYLINE

.macro emit_lda param
		.if (.match (.left (1, {param}), #))
			lda param
		.else
			lda param
		.endif
.endmacro

.macro emit_ldx param
		.if (.match (.left (1, {param}), #))
			ldx param
		.else
			ldx param
		.endif
.endmacro

.macro emit_ldy param
		.if (.match (.left (1, {param}), #))
			ldy param
		.else
			ldy param
		.endif
.endmacro


.macro draw_tile_prepare
		; make sure we've selected the tile image
		lda #<tiles_img_aux
		sta arg1w
		lda #>tiles_img_aux
		sta arg1w+1
		
		lda #<tiles_img_main
		sta arg2w
		lda #>tiles_img_main
		sta arg2w+1
.endmacro

.macro draw_single_largetile src_tileid, dst_x, dst_y
		emit_lda src_tileid
		sta arg1
		
		emit_lda dst_x
		sta arg2
		
		emit_lda dst_y
		sta arg3
		
		JSR TILEENGINE_RENDER_LARGE_TILE

.endmacro

