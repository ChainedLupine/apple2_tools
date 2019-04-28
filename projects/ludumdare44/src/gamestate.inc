; variables for game state

.global state_health, state_time
.global GS_RESET

max_inv_items = 5
.global inventory_start, inventory_me, inventory_ramen, inventory_temp1, inventory_temp2, inventory_temp3

max_nearby_items = 5
.global nearby_start, nearby_slot1, nearby_slot2, nearby_slot3, nearby_slot4, nearby_slot5


.struct nearby_struct
  tileid    .byte
  type      .byte ; what kind it is
.endstruct

.struct inventory_struct
  tileid    .byte
	type 			.byte
	owned 		.byte
  state     .byte
.endstruct

.macro clear_inventory addr
		lda #0
    sta addr+inventory_struct::state
.endmacro

.macro clear_nearby addr
		lda #0
    sta addr+nearby_struct::type
.endmacro

.macro build_inventory_item
	.RES .sizeof(inventory_struct) - 2, $00
.endmacro

.macro build_nearby_item
	.RES .sizeof(nearby_struct), $00
.endmacro
