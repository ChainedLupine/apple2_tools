
.segment "CODE_H"

TALK_NOTHING:
    PRINTR txt_talk_nothing
    RTS

TALK_FRIDGE:
    lda one_shot_sanity_sinks
    and #$01
    bne :+
    lda one_shot_sanity_sinks
    ora #$01
    sta one_shot_sanity_sinks
    JSR GS_REMOVE_SANITY
  :
    PRINTR txt_talk_crazy
    RTS

.segment "RODATA_H"

  talk_nearby_jumps:
    .addr TALK_FRIDGE-1      ; refrig


  ; talk texts
  txt_talk_nothing: 
    .asciiz "There's nothing to be said here. "
  
  txt_talk_crazy: 
    .asciiz "What am I doing?  Maybe I'm going crazy... "
