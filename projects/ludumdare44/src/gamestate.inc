; variables for game state

.global state_health, state_time
.global GS_RESET, GS_RESET_NEARBY, GS_RESET_EXITS
.global GS_ADD_INV_ITEM, GS_REMOVE_INV_ITEM, GS_CHECK_INV_PRESENT
.global GS_ADD_EXIT, GS_ADD_NEARBY

.global GS_ADD_SANITY, GS_REMOVE_SANITY, GS_ONE_SHOT_SANITY_DEDUCT
.global GS_REMOVE_TIME

.global one_shot_sanity_sinks, curr_room_flags

oss_mask_doing_something_stupid       = $01

max_inv_items = 5
.global inventory_start, inventory_slot1, inventory_slot2, inventory_slot3, inventory_slot4, inventory_slot5

max_nearby_items = 5
.global nearby_start, nearby_slot1, nearby_slot2, nearby_slot3, nearby_slot4, nearby_slot5

max_exits_items = 5
.global exit_start, exit_1, exit_2, exit_3, exit_4, exit_5

.global curr_room_desc_txt, curr_room_typeid

; note: TYPEs are UNIQUE IDs to each thing in the game

.struct nearby_struct
  tileid      .byte
  typeid      .byte ; what kind it is
.endstruct

.struct inventory_struct
  tileid      .byte
	typeid 			.byte
.endstruct

.struct exit_struct
  tileid      .byte
  typeid      .byte
.endstruct

.macro clear_inventory addr
    lda #0
    sta addr+inventory_struct::typeid
.endmacro

.macro clear_nearby addr
		lda #0
    sta addr+nearby_struct::typeid
.endmacro

.macro clear_exit addr
		lda #0
    sta addr+exit_struct::typeid
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
