; variables for tracking game state

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

		JSR UI_RESET

		RTS

GS_RESET_INV:
		clear_inventory inventory_me
		clear_inventory inventory_ramen
		clear_inventory inventory_temp1
		clear_inventory inventory_temp2
		clear_inventory inventory_temp3

		lda #1
		sta inventory_me+inventory_struct::owned ; we always own ourselves (DEEP)
		sta inventory_ramen+inventory_struct::owned
		sta inventory_temp1+inventory_struct::owned
		sta inventory_temp2+inventory_struct::owned
		sta inventory_temp3+inventory_struct::owned

		RTS

GS_RESET_NEARBY:
		lda #1
		sta nearby_slot1+nearby_struct::type
		sta nearby_slot2+nearby_struct::type
		sta nearby_slot3+nearby_struct::type
		sta nearby_slot4+nearby_struct::type
		sta nearby_slot5+nearby_struct::type

		lda #tileid_nearby_test
		sta nearby_slot1+nearby_struct::tileid
		sta nearby_slot2+nearby_struct::tileid
		sta nearby_slot3+nearby_struct::tileid
		sta nearby_slot4+nearby_struct::tileid
		sta nearby_slot5+nearby_struct::tileid

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
	inventory_me:
		; first byte is always our tileid
		.byte tileid_inv_me
		.byte $00
		build_inventory_item

	inventory_ramen:
		.byte tileid_inv_ramen
		.byte $01
		build_inventory_item

	inventory_temp1:
		.byte tileid_inv_ramen
		.byte $02
		build_inventory_item

	inventory_temp2:
		.byte tileid_inv_ramen
		.byte $02
		build_inventory_item

	inventory_temp3:
		.byte tileid_inv_ramen
		.byte $02
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


