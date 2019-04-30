; exits

.segment "CODE_H"

EXIT_NO_LOCK:
    sec
    rts

EXIT_APT_TO_STREET:
    lda #typeid_inv_ramen
    sta arg1
    JSR GS_CHECK_INV_PRESENT
    ; carry is set if we have the item
    rts


.segment "RODATA_H"

  exit_jumps:
    .addr EXIT_APT_TO_STREET-1        ; apt to street
    .byte typeid_street, $0
    
    .addr EXIT_NO_LOCK-1              ; street to apt
    .byte typeid_apartment, $0
