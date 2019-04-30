.segment "CODE_H"

; descriptions are driven by a jump table to each possible description, based on the typeid.
;
; You can examine any object, so they must all have entries in the jump tables.

DESC_NOTHING:
    PRINTR txt_desc_nothing
    RTS

DESC_INV_ME:
    PRINTR txt_desc_me
    RTS

DESC_INV_RAMEN:
    PRINTR txt_desc_ramen
    RTS

DESC_NB_REFRIG:
    PRINTR txt_desc_refrig
    RTS

DESC_NB_TESTPERSON:
    PRINTR txt_desc_testperson
    RTS

DESC_NB_BED:
    PRINTR txt_desc_bed
    RTS

DESC_ROOM_APT:
    PRINTR txt_desc_apartment
    RTS

DESC_ROOM_STREET:
    PRINTR txt_desc_street
    RTS

DESC_EXIT_APT2STREET:
    PRINTR txt_desc_apt2str
    RTS

DESC_EXIT_STREET2APT:
    PRINTR txt_desc_str2apt
    RTS

.segment "RODATA_H"

  desc_inv_jumps:
    .addr DESC_INV_ME-1
    .addr DESC_INV_RAMEN-1

  desc_nearby_jumps:
    .addr DESC_NB_REFRIG-1          ; fridge
    .addr DESC_NB_TESTPERSON-1      ; test person
    .addr DESC_NB_BED-1             ; bed

  desc_room_jumps:
    .addr DESC_ROOM_APT-1
    .addr DESC_ROOM_STREET-1

  desc_exit_jumps:
    .addr DESC_EXIT_APT2STREET-1
    .addr DESC_EXIT_STREET2APT-1

  ; description texts
  txt_desc_nothing:
    .asciiz "Nothing to see here, really."

  txt_desc_apartment: 
    ;        ------------------------------------------------------------------------------+-----------------------------------------------------------------------------+
    .asciiz "This is my apartment.  I consider it a decent affair.  A bit small but comfy.^It's in a good location too. "
  
  txt_desc_street: 
    .asciiz "It's the street outside of your apartment! "

  txt_desc_unknown: 
    .asciiz "No idea what this is?  :-) "

  txt_desc_refrig: 
    .asciiz "I open the refrigerator and cupboard looking for food. "

  txt_desc_bed: 
    .asciiz "It's an okay looking bed.  It's inviting, but maybe I should get some stuff^done today. "

  txt_desc_testperson: 
    .asciiz "There is a generic person standing here.  They're holding a single insta-cup^of ramen, and when I make eye contact it seems they are begging me to take it. "

  txt_desc_me:
    ;        ------------------------------------------------------------------------------
    .asciiz "I take out my phone and hold it up so that I can see my face in the glossy^surface. "

  txt_desc_ramen:
    .asciiz "Instant ramen.  Cheap, disguisting, unhealthy. "

  txt_desc_apt2str:
    .asciiz "The simple door of my apartment leads to the street outside. "

  txt_desc_str2apt:
    .asciiz "I can see the door to my apartment over here. "
