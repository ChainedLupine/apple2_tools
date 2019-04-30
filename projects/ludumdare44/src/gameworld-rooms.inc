; rooms

.segment "CODE_H"

ROOM_APARTMENT:
    ; apartment exits
    lda #typeid_exit_apt_street
    ldx #tileid_exit_north
    ldy #0 ; no item required
    JSR GS_ADD_EXIT

    lda #typeid_exit_apt_street
    ldx #tileid_exit_north
    ldy #typeid_inv_ramen ; no item required
    JSR GS_ADD_EXIT

    lda #typeid_nearby_refrig
    ldx #tileid_nearby_refrig
    JSR GS_ADD_NEARBY

    lda #typeid_nearby_person
    ldx #tileid_nearby_test
    JSR GS_ADD_NEARBY

    lda #typeid_nearby_bed
    ldx #tileid_nearby_bed
    JSR GS_ADD_NEARBY

    rts


.segment "RODATA_H"

  room_setup_jumps:
    ; goes by typeid
    .addr ROOM_APARTMENT-1

  filename_room1:
    Util_LSTR "ROOM1.DHGR"

.proc room_filenames
  room_images_table:
    ; panel 0=left, 1=right, second is padding
    .addr filename_room1        ; apartment
      .byte $0, $0
    .addr filename_room1        ; street
      .byte $1, $0
.endproc
