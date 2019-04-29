; game world
.include "zeropage.inc"
.include "utils.inc"

.include "interface.inc"
.include "gamestate.inc"
.include "gameworld.inc"
.include "textengine.inc"

.segment "CODE_H"

.macro PRINTR symb
    Util_LOAD_SYM symb, arg1w
    JSR TE_PRINT_TEXT
.endmacro

; loads up a room with nearby and exits
GW_POPULATE_ROOM:
		JSR GS_RESET_NEARBY
		JSR GS_RESET_EXITS

    lda curr_room_typeid
    cmp #typeid_apartment
    bne :+

      ; apartment exits
      lda #typeid_exit_apt_street
      ldx #tileid_exit_north
      JSR GS_ADD_EXIT

      lda #typeid_nearby_refrig
      ldx #tileid_nearby_refrig
      JSR GS_ADD_NEARBY
    :
    lda curr_room_typeid
    cmp #typeid_street
    bne :+


    :

    RTS

GW_PRINT_CURR_ROOM_DESC:
    JSR TE_CLEAR_TEXT

    lda curr_room_typeid
    JSR GW_PRINT_DESC_TYPEID

    RTS

    ; a = typeid
    ; note this MUST BE called as a RTS, as we're using the RTS Trick (https://wiki.nesdev.com/w/index.php/RTS_Trick) for jump tables
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
      jmp @DONE
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
      jmp @DONE
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
      jmp @DONE
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

  @DONE:
    rts    

    ; a = typeid
    ; note this MUST BE called as a RTS, as we're using the RTS Trick (https://wiki.nesdev.com/w/index.php/RTS_Trick) for jump tables
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

    rts    

.segment "DATA_H"


.include "gameworld-descs.inc"
.include "gameworld-talks.inc"

