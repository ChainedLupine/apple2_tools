.include "apple2.inc"
.include "apple2extra.inc"

.segment "RODATA_H"

HELLO_TXT: .asciiz "Loaded at $6000, press key to advance"

.global HELLO_TXT