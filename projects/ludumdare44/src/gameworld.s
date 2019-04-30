; game world
.include "zeropage.inc"
.include "utils.inc"
.include "apple2rom.inc"

.include "interface.inc"
.include "gamestate.inc"
.include "gameworld.inc"
.include "textengine.inc"

.include "grfx.inc"

.segment "CODE_H"

.macro PRINTR symb
    Util_LOAD_SYM symb, arg1w
    JSR TE_PRINT_TEXT
.endmacro

GW_RETURN_TO_APT:

  lda #typeid_apartment
  sta curr_room_typeid

  lda #$01
  sta arg1
  JSR GRFX::DHGR_CLEAR_TO_COLOR_SLOW

  JSR GW_POPULATE_ROOM

  rts


; loads up a room with nearby and exits
GW_POPULATE_ROOM:
		JSR GS_RESET_NEARBY
		JSR GS_RESET_EXITS

    lda #01
    sta curr_room_flags

    JSR @JMP_CURR_ROOM_TABLE

    RTS

@JMP_CURR_ROOM_TABLE:
    lda curr_room_typeid
    sec
    sbc #typeid_rooms_start
    asl
    tax

    lda room_setup_jumps+1, x
    pha
    lda room_setup_jumps, x
    pha
    rts ; now go to our real setup room handler


GW_PRINT_CURR_ROOM_DESC:
    JSR TE_CLEAR_TEXT

    lda curr_room_typeid
    JSR GW_PRINT_DESC_TYPEID

    RTS

    ; a = typeid
    ; note this MUST BE called as a JSR, as we're using the RTS Trick (https://wiki.nesdev.com/w/index.php/RTS_Trick) for jump tables
    ; note that we will not return back to this routine, but to the routine that called this one
GW_PRINT_DESC_TYPEID:
    ; find out what range we are in
    cmp #typeid_nearbys_start-1
    bcs :+  ; we are not in inventory range
      ; we are inventory item

      sec
      sbc #typeid_invs_start
      asl
      tax
      lda desc_inv_jumps+1, x
      pha
      lda desc_inv_jumps, x
      pha
      rts ; now go to our real print desc routine
    :
    cmp #typeid_rooms_start-1
    bcs :+ ; we are not in nearby range
      ; we are nearby item
      sec
      sbc #typeid_nearbys_start
      asl
      tax
      lda desc_nearby_jumps+1, x
      pha
      lda desc_nearby_jumps, x
      pha
      rts ; now go to our real print desc routine
    :
    cmp #typeid_exits_start-1
    bcs :+ ; we are not in rooms range
      ; we are room item
      
      sec
      sbc #typeid_rooms_start
      asl
      tax
      lda desc_room_jumps+1, x
      pha
      lda desc_room_jumps, x
      pha
      rts ; now go to our real print desc routine
    :
    ; only thing left, we must be an ext
    sec
    sbc #typeid_exits_start
    asl
    tax
    lda desc_exit_jumps+1, x
    pha
    lda desc_exit_jumps, x
    pha
    rts ; now go to our real print desc routine

    ; a = typeid
    ; note this MUST BE called as a RTS, as we're using the RTS Trick (https://wiki.nesdev.com/w/index.php/RTS_Trick) for jump tables
    ; note that we will not return back to this routine, but to the routine that called this one
GW_HANDLE_TALKING:
    ; we are nearby item
    sec
    sbc #typeid_nearbys_start
    asl
    tax
    lda talk_nearby_jumps+1, x
    pha
    lda talk_nearby_jumps, x
    pha
    rts ; now go to our real print desc routine


  .include "gameworld-interacts.inc"

    ; input:  typeid1 in a, typeid2 in x
    ; return = carry if success, clear if not
  GW_HANDLE_INTERACTION:
      sta arg1
      stx arg2

      ; we need to loop over interact_combinations
      lda #<interactions::interact_table
      ldx #>interactions::interact_table
      sta temp1w
      stx temp1w+1

      lda #.sizeof(interactions)
      lsr
      lsr ; divide by two for count
      tax

    @loop_table:

      ldy #interact_struct::typeidsrc
      lda (temp1w),y
      cmp arg1
      bne @keep_looking

      ldy #interact_struct::typeiddst
      lda (temp1w),y
      cmp arg2
      bne @keep_looking

      ; found an interaction, now trigger
      JSR @TRIGGER_HANDLE_INTERACTION
      sec
      rts

    @keep_looking:
      Util_Inc_16_Addr_Struct temp1w, interact_struct
      dex
      bne @loop_table

      ; none found
      clc ; clear carry = error
      rts

    ; temp1w = routine to execute
  @TRIGGER_HANDLE_INTERACTION:
      ldy #interact_struct::handler+1
      lda (temp1w),y
      pha
      ldy #interact_struct::handler
      lda (temp1w),y
      pha
      rts ; jump to our handler



.segment "DATA_H"


.include "gameworld-descs.inc"
.include "gameworld-talks.inc"
.include "gameworld-rooms.inc"

.segment "CODE_H"

  ; no args, looks into curr_room_typeid for room
  ; return: arg1w = filename, arg1=0 for left panel, 1 for right panel
GW_GET_ROOM_FILENAME_AND_PANEL:
      ; we need to loop over interact_combinations
      lda #<room_filenames::room_images_table
      ldx #>room_filenames::room_images_table
      sta temp1w
      stx temp1w+1

      lda curr_room_typeid
      sec
      sbc #typeid_rooms_start
      ; zero-index of room id
      asl ; *2
      asl ; *2
      sta temp1
      
      Util_Inc_16_Addr temp1w, temp1

      ; temp1w should now point to our image entry
      ldy #room_image_table_struct::filenametxt
      lda (temp1w),y
      sta arg1w
      ldy #room_image_table_struct::filenametxt+1
      lda (temp1w),y
      sta arg1w+1

      ldy #room_image_table_struct::panel
      lda (temp1w),y
      sta arg1

    RTS

