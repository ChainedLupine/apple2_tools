
.segment "CODE_H"

; talking is driven by a jump table to each possible talking situation, based on the typeid of the nearby that is talked to.
;
; all nearbys must have a talk jump table entry

TALK_NOTHING:
    PRINTR txt_talk_nothing
    RTS

TALK_CRAZY:
    lda #oss_mask_doing_something_stupid
    sta arg1
    JSR GS_ONE_SHOT_SANITY_DEDUCT

    PRINTR txt_talk_crazy
    RTS

TALK_TESTPERSON:
    lda #typeid_inv_ramen
    sta arg1
    jSR GS_CHECK_INV_PRESENT
    bcs :+

		lda #typeid_inv_ramen
		sta arg1
		lda #tileid_inv_ramen
		sta arg2
		JSR GS_ADD_INV_ITEM
    PRINTR txt_talk_testperson
    rts

  :
    PRINTR txt_talk_testperson2
    RTS

.segment "RODATA_H"

  talk_nearby_jumps:
    .addr TALK_CRAZY-1            ; refrig
    .addr TALK_TESTPERSON-1       ; test person
    .addr TALK_CRAZY-1            ; bed


  ; talk texts
  txt_talk_nothing: 
    ;        ------------------------------------------------------------------------------
    .asciiz "There's nothing to be said in this situation. "
  
  txt_talk_crazy: 
    .asciiz "What am I doing?  Maybe I'm going crazy... "

  txt_talk_testperson:
    .asciiz "'I am test person.  I give ramen.  It is my only function.  Please kill me.' "

  txt_talk_testperson2:
    .asciiz "'You already have my ramen.  It was the last thing I owned.  Please leave me^alone.' "
