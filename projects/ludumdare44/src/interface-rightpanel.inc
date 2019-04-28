		
UI_DRAW_HEALTH:
		draw_tile_prepare

		; upper left
		lda #tileid_health1_icon
		sta temp1

		lda state_health
		cmp #1
		bcs :+ ; if >=1
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #16, #0

		; lower left
		lda #tileid_health2_icon
		sta temp1

		lda state_health
		cmp #2
		bcs :+ ; if >=2
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #16, #2

		; lower right
		lda #tileid_health3_icon
		sta temp1

		lda state_health
		cmp #3
		bcs :+ ; if >=3
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #18, #2

		; upper right
		lda #tileid_health4_icon
		sta temp1

		lda state_health
		cmp #4
		bcs :+ ; if >=4
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #18, #0

		RTS
		
UI_DRAW_TIME:

		draw_tile_prepare

		; upper left
		lda #tileid_moon1_icon
		sta temp1

		lda state_time
		cmp #1
		bcs :+ ; if >=1
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #10, #0

		; lower left
		lda #tileid_moon2_icon
		sta temp1

		lda state_time
		cmp #2
		bcs :+ ; if >=2
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #10, #2

		; lower right
		lda #tileid_moon3_icon
		sta temp1

		lda state_time
		cmp #3
		bcs :+ ; if >=3
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #12, #2

		; upper right
		lda #tileid_moon4_icon
		sta temp1

		lda state_time
		cmp #4
		bcs :+ ; if >=4
		lda #tileid_blank
		sta temp1
	:
		draw_single_largetile temp1, #12, #0

		RTS
	

UI_SHOW_DISK_LOAD:
		draw_tile_prepare

		draw_single_largetile #tileid_disk_icon, #18, #18
		
		RTS
		
UI_CLEAR_DISK_LOAD:
		draw_tile_prepare

		draw_single_largetile #tileid_blank, #18, #18

		RTS
