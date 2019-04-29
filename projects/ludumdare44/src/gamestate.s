; variables for tracking game state
.include "zeropage.inc"
.include "utils.inc"

.include "gameworld.inc"
.include "gamestate.inc"
.include "interface.inc"

.segment "CODE_H"

; complete reset of gamestate and world (ie: loading first time or reloading)
GS_RESET:
		lda #04
		sta state_health
		lda #04		
		sta state_time

		JSR GS_RESET_INV
		JSR UI_RESET

		lda #typeid_apartment
		sta curr_room_typeid
		JSR GW_POPULATE_ROOM

		RTS

GS_ADD_SANITY:
		lda state_health
		inc
		cmp #4
		bcc :+
		lda #4
	:
		sta state_health
		
		JSR UI_DRAW_HEALTH
		RTS

GS_REMOVE_SANITY:
		lda state_health
		dec
		bpl :+
		lda #0
	:
		sta state_health

		JSR UI_DRAW_HEALTH
		RTS

GS_REMOVE_TIME:
		lda state_time
		dec
		bpl :+
		lda #0
	:
		sta state_time

		JSR UI_DRAW_TIME
		RTS

		; arg1 = inv typeidid to add
		; arg2 = inv tileid
		; sets carry on successful add, reg_a/x will be addr of slot added to
GS_ADD_INV_ITEM:
		ZP_SAVE
		lda #<inventory_start
		sta temp1w 
		lda #>inventory_start
		sta temp1w+1

		ldx #0
	@LOOP_INV:
		ldy #inventory_struct::typeid
		lda (temp1w),y
		bne @OCCUPIED
		; empty slot, now fill it

		ldy #inventory_struct::typeid
		lda arg1
		sta (temp1w),y

		ldy #inventory_struct::tileid
		lda arg2
		sta (temp1w),y

		JMP @FOUND

	@OCCUPIED:
		; move on to next item
		Util_Inc_16_Addr_Struct temp1w, inventory_struct
		inx
		cpx #max_inv_items
		bcc @LOOP_INV

	@RANOUT:
		ZP_RESTORE
		clc ; clear carry, no item
		rts

	@FOUND:
		lda temp1w
		ldx temp1w+1
		pha
		phx
		ZP_RESTORE
		plx
		pla
		sec 
		RTS

		; arg1 = inv typeidid to remove
		; sets carry on successful remove
GS_REMOVE_INV_ITEM:
		ZP_SAVE
		lda #<inventory_start
		sta temp1w 
		lda #>inventory_start
		sta temp1w+1

		ldx #0
	@LOOP_INV:
		ldy #inventory_struct::typeid
		lda (temp1w),y
		cmp arg1
		bne @NOT_ITEM
		
		; zero the inv slot
		ldy #inventory_struct::typeid
		lda #0
		sta (temp1w),y

		ldy #inventory_struct::tileid
		lda #0
		sta (temp1w),y

		; done w/ removal
		JMP @RESHUFFLE_INV

	@NOT_ITEM:
		; move on to next item
		Util_Inc_16_Addr_Struct temp1w, inventory_struct
		inx
		cpx #max_inv_items
		bcc @LOOP_INV

		ZP_RESTORE
		clc ; clear carry, no item found
		rts

	@RESHUFFLE_INV:
		; now that we've zeroed the item at temp1w, we need to move the rest upwards
		; first check to see if we're already at end
		cpx #max_inv_items
		bcs	@DONE_RESHUFFLE ; then nothing to do

		lda temp1w
		sta temp2w
		lda temp1w+1
		sta temp2w+1

		Util_Inc_16_Addr_Struct temp2w, inventory_struct
		; temp2w now points to item below us

		; swap
		ldy #inventory_struct::typeid
		lda (temp2w),y
		sta (temp1w),y
		ldy #inventory_struct::tileid
		lda (temp2w),y
		sta (temp1w),y

		; move to next item
		Util_Inc_16_Addr_Struct temp1w, inventory_struct
		inx
		JMP @RESHUFFLE_INV

	@DONE_RESHUFFLE:
		ZP_RESTORE
		sec 
		RTS

		; a = exit typeidid to add
		; x = exit tileid
GS_ADD_EXIT:
		phx
		pha
		ZP_SAVE
		pla
		plx
		sta arg1
		stx arg2

		lda #<exit_start
		sta temp1w 
		lda #>exit_start
		sta temp1w+1

		ldx #0
	@LOOP_INV:
		ldy #exit_struct::typeid
		lda (temp1w),y
		bne @OCCUPIED

		; empty slot, now fill it
		ldy #exit_struct::typeid
		lda arg1
		sta (temp1w),y

		ldy #exit_struct::tileid
		lda arg2
		sta (temp1w),y

		JMP @DONE

	@OCCUPIED:
		; move on to next item
		Util_Inc_16_Addr_Struct temp1w, exit_struct
		inx
		cpx #max_exits_items
		bcc @LOOP_INV

	@DONE:
		ZP_RESTORE
		rts

		; a = nearby typeidid to add
		; x = nearby tileid
GS_ADD_NEARBY:
		phx
		pha
		ZP_SAVE
		pla
		plx
		sta arg1
		stx arg2

		lda #<nearby_start
		sta temp1w 
		lda #>nearby_start
		sta temp1w+1

		ldx #0
	@LOOP_INV:
		ldy #nearby_struct::typeid
		lda (temp1w),y
		bne @OCCUPIED

		; empty slot, now fill it
		ldy #nearby_struct::typeid
		lda arg1
		sta (temp1w),y

		ldy #nearby_struct::tileid
		lda arg2
		sta (temp1w),y

		JMP @DONE

	@OCCUPIED:
		; move on to next item
		Util_Inc_16_Addr_Struct temp1w, nearby_struct
		inx
		cpx #max_nearby_items
		bcc @LOOP_INV

	@DONE:
		ZP_RESTORE
		rts

GS_RESET_INV:
		clear_inventory inventory_slot1
		clear_inventory inventory_slot2
		clear_inventory inventory_slot3
		clear_inventory inventory_slot4
		clear_inventory inventory_slot5

		lda #typeid_inv_me
		sta inventory_slot1+inventory_struct::typeid
		lda #tileid_inv_me
		sta inventory_slot1+inventory_struct::tileid

		lda #typeid_inv_ramen
		sta inventory_slot2+inventory_struct::typeid
		lda #tileid_inv_ramen
		sta inventory_slot2+inventory_struct::tileid

		;lda #typeid_inv_me
		;sta arg1
		;lda #tileid_inv_me
		;sta arg2
		;JSR GS_ADD_INV_ITEM

		;lda #typeid_ramen
		;sta arg1
		;lda #tileid_inv_ramen
		;sta arg2
		;JSR GS_ADD_INV_ITEM

		;lda #typeid_me
		;sta arg1
		;JSR GS_REMOVE_INV_ITEM

		;lda #typeid_me
		;sta arg1
		;lda #tileid_inv_me
		;sta arg2
		;JSR GS_ADD_INV_ITEM

		RTS

GS_RESET_NEARBY:
		clear_nearby nearby_slot1
		clear_nearby nearby_slot2
		clear_nearby nearby_slot3
		clear_nearby nearby_slot4
		clear_nearby nearby_slot5
		RTS

GS_RESET_EXITS:
		clear_exit exit_1
		clear_exit exit_2
		clear_exit exit_3
		clear_exit exit_4
		clear_exit exit_5

		RTS

.segment "DATA_H"

	state_health:
		.byte $04
		
	state_time:
		.byte $04

  curr_room_typeid:
		.byte $00

	curr_room_desc_txt:
		.addr $0000

	one_shot_sanity_sinks:
		.byte $00

	; max 5 inventory items
	inventory_start:
	inventory_slot1:
		; first byte is always our tileid
		build_inventory_item

	inventory_slot2:
		build_inventory_item

	inventory_slot3:
		build_inventory_item

	inventory_slot4:
		build_inventory_item

	inventory_slot5:
		build_inventory_item

	; max 5 nearby items in room
	nearby_start:
	nearby_slot1:
		build_nearby_item
	nearby_slot2:
		build_nearby_item
	nearby_slot3:
		build_nearby_item
	nearby_slot4:
		build_nearby_item
	nearby_slot5:
		build_nearby_item

	; max 5 exits in room
	exit_start:
	exit_1:
		build_exit
	exit_2:
		build_exit
	exit_3:
		build_exit
	exit_4:
		build_exit
	exit_5:
		build_exit

