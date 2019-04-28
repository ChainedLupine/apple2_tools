; variables for game state

.global state_health, state_time
.global GS_RESET

max_inv_items = 5
.global inventory_start, inventory_slot1, inventory_slot2, inventory_slot3, inventory_slot4, inventory_slot5

max_nearby_items = 5
.global nearby_start, nearby_slot1, nearby_slot2, nearby_slot3, nearby_slot4, nearby_slot5

max_exits_items = 5
.global exit_start, exit_1, exit_2, exit_3, exit_4, exit_5

.struct nearby_struct
  tileid    .byte
  type      .byte ; what kind it is
.endstruct

.struct inventory_struct
  tileid    .byte
	type 			.byte
  state     .byte
.endstruct

.struct exit_struct
  tileid    .byte
  leadsto   .byte
  typereq   .byte
.endstruct

.macro clear_inventory addr
		lda #0
    sta addr+inventory_struct::state
    lda #0
    sta addr+inventory_struct::type
.endmacro

.macro clear_nearby addr
		lda #0
    sta addr+nearby_struct::type
.endmacro

.macro clear_exit addr
		lda #0
    sta addr+exit_struct::leadsto
.endmacro

.macro build_inventory_item
	.RES .sizeof(inventory_struct), $00
.endmacro

.macro build_nearby_item
	.RES .sizeof(nearby_struct), $00
.endmacro

.macro build_exit
	.RES .sizeof(exit_struct), $00
.endmacro
