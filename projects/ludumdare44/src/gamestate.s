; variables for tracking game state
.include "zeropage.inc"
.include "utils.inc"

.include "gamestate.inc"
.include "interface.inc"

.segment "CODE_H"

GS_RESET:
		lda #04
		sta state_health
		lda #04		
		sta state_time

		JSR GS_RESET_INV

		JSR GS_RESET_NEARBY

		JSR GS_RESET_EXITS

		JSR UI_RESET

		RTS

GS_RESET_INV:
		clear_inventory inventory_slot1
		clear_inventory inventory_slot2
		clear_inventory inventory_slot3
		clear_inventory inventory_slot4
		clear_inventory inventory_slot5

		lda #tileid_inv_me
		sta inventory_slot1+inventory_struct::tileid
		lda #tileid_inv_ramen
		sta inventory_slot2+inventory_struct::tileid
		sta inventory_slot3+inventory_struct::tileid
		sta inventory_slot4+inventory_struct::tileid
		sta inventory_slot5+inventory_struct::tileid

		lda #1
		sta inventory_slot1+inventory_struct::type
		lda #0
		sta inventory_slot2+inventory_struct::type
		sta inventory_slot3+inventory_struct::type
		sta inventory_slot4+inventory_struct::type
		sta inventory_slot5+inventory_struct::type

		RTS

		; arg1 = inv type
		; sets carry on successful add, reg_a/x will be addr of slot added to
GS_ADD_INV_ITEM:
		ZP_SAVE
		lda #<inventory_start
		sta temp1w 
		lda #>inventory_start
		sta temp1w+1

		ldx #0
	@LOOP_INV:
		phx
		ldy #inventory_struct::type
		lda (temp1w),y
		beq @OCCUPIED
		; empty slot, select it

		sec ; set carry, we have item
		lda temp1w
		ldx temp1w+1
		pha
		phx
		ZP_RESTORE
		plx
		pla
		RTS

	@OCCUPIED:
		; move on to next item
		Util_Inc_16_Addr_Struct temp1w, inventory_struct
		inx
		cmp #max_inv_items
		bcc @LOOP_INV

		clc
		ZP_RESTORE
		RTS

GS_RESET_NEARBY:
		lda #0
		sta nearby_slot1+nearby_struct::type
		lda #0
		sta nearby_slot2+nearby_struct::type
		lda #0
		sta nearby_slot3+nearby_struct::type
		lda #0
		sta nearby_slot4+nearby_struct::type
		lda #0
		sta nearby_slot5+nearby_struct::type

		lda #tileid_nearby_test
		sta nearby_slot1+nearby_struct::tileid
		sta nearby_slot2+nearby_struct::tileid
		sta nearby_slot3+nearby_struct::tileid
		sta nearby_slot4+nearby_struct::tileid
		sta nearby_slot5+nearby_struct::tileid

		RTS

GS_RESET_EXITS:

		lda #tileid_exit_north
		sta exit_1+exit_struct::tileid
		sta exit_2+exit_struct::tileid
		sta exit_3+exit_struct::tileid
		sta exit_4+exit_struct::tileid
		sta exit_5+exit_struct::tileid

		lda #2
		sta exit_1+exit_struct::leadsto
		sta exit_2+exit_struct::leadsto
		sta exit_3+exit_struct::leadsto
		sta exit_4+exit_struct::leadsto
		lda #0
		sta exit_5+exit_struct::leadsto

		lda #2
		sta exit_1+exit_struct::typereq
		sta exit_2+exit_struct::typereq
		sta exit_3+exit_struct::typereq
		sta exit_4+exit_struct::typereq
		sta exit_5+exit_struct::typereq

		RTS

.segment "DATA_H"

	state_health:
		.byte $04
		
	state_time:
		.byte $04

	state_nearby_cnt:
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

