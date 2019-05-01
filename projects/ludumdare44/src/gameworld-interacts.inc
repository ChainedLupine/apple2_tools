; interactions

; interactions are similiar to talks/descs, though it works via one layer of indirection.
;
; an interaction is a combination of two typeids.  The first is always an inventory item, and the second is an inv or nearby item.
;
; to keep the UI simple, we always have to select an inv item to operate on the second item.  So selecting "me" and interacting
; with something is the game equivalent to singular engagement mode.  It can be considered "pick this up" or "i want to push this button.""
;
; using a non-me item is equivalent to a nearby is equivalent to "use item1 upon item2."
;
; because this would be a huge amount of jumptables to keep track of if we were to do a n^m comparison.  So instead we will use a layer
; of indirection in another table.  It will contain two typeids -- first is the source, and second is a target.  If interact can't match
; these values for a combination, then it will give a generic "can't do that" message.  If a match is found, it will then jump to that
; entry.

.segment "RODATA_H"

  .proc interactions
  
  interact_table:
          ; source
          ; target
          ; result
    .byte typeid_inv_me,              typeid_inv_ramen
    .addr INTERACT_ME_WITH_RAMEN-1

    .byte typeid_inv_me,              typeid_nearby_person
    .addr INTERACT_UNKNOWN-1

    .byte typeid_inv_me,              typeid_nearby_bed
    .addr INTERACT_BED-1

    .byte typeid_inv_ramen,           typeid_nearby_refrig
    .addr INTERACT_COOK_RAMEN-1

  .endproc


.segment "RODATA_H"

  txt_interact_me_ramen:
    ;        ------------------------------------------------------------------------------
    .asciiz "That's a solid block of uncooked freeze dried ramen.  Am I really that insane^I will try to eat that? "

  txt_interact_cook_ramen:
    ;        ------------------------------------------------------------------------------
    .asciiz "I'll microwave this instant-ramen.  I really need some real food around^here...  "

  txt_interact_bed:
    .asciiz "I think I will go to sleep now.  It's a good way to waste time;  And my life,^for that matter. "


  txt_interact_unknown:
    .asciiz "I don't really know what would happen in that sort of situation.  Best to just^leave it. "


.segment "CODE_H"

  INTERACT_COOK_RAMEN:
    lda #typeid_inv_ramen    
    JSR GS_REMOVE_INV_ITEM

    PRINTR txt_interact_cook_ramen
    RTS

  INTERACT_ME_WITH_RAMEN:
    lda #oss_mask_doing_something_stupid
    sta arg1
    JSR GS_ONE_SHOT_SANITY_DEDUCT

    PRINTR txt_interact_me_ramen
    RTS

  INTERACT_BED:
    PRINTR txt_interact_bed

    JSR GW_RETURN_TO_APT
    JSR GS_REMOVE_TIME
    RTS

  INTERACT_UNKNOWN:
    JSR PLAY_BEEP_SAD
    PRINTR txt_interact_unknown
    RTS
