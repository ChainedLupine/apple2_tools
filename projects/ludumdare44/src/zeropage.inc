; -- zeropage definitions --------------------
; note: TOTALLY stomps all over Applesoft/Integer BASIC, do not use together!

.ifndef ZP_INC_GUARD
.define ZP_INC_GUARD

  zpsave_addr_l  := $56
  zpsave_addr_h  := $57

  ; save pages
  ; $0900
  ; $0A00
  ; $0B00
  ; $0C00
  ; $0D00
  ; $0E00
  ; $1F00

  ; arguments for subroutines
  arg1	:= $60
  arg2	:= $61
  arg3	:= $62
  arg4	:= $63
  arg5	:= $64
  arg6	:= $65

  arg1w	:= $66
  arg2w	:= $68
  arg3w	:= $6A
  arg4w	:= $6C
  arg5w	:= $6E

  ; for use in subroutines
  temp1	:= $70
  temp2	:= $71
  temp3	:= $72
  temp4	:= $73
  temp5	:= $74
  temp6	:= $75

  temp1w	:= $76
  temp2w	:= $78
  temp3w	:= $7A
  temp4w	:= $7C
  temp5w	:= $7E
      
  ; intended for variables in main program
  var1	:= $80
  var2	:= $81
  var3	:= $82
  var4	:= $83
  var5	:= $84
  var6	:= $85

  var1w	:= $86
  var2w	:= $88
  var3w	:= $8A
  var4w	:= $8C
  var5w	:= $8E


    
.endif

  .macro ZP_SETUP
    lda #$00
    sta zpsave_addr_l
    lda #$09
    sta zpsave_addr_h
  .endmacro

  .macro ZP_SAVE
    ldy #$8E
    :
      lda $00,y
      sta (zpsave_addr_l),y
      dey
      cmp #$60
      bne :-

    ZP_SAVE_INC
  .endmacro

  .macro ZP_RESTORE
    ZP_SAVE_DEC
    
    ldy #$8E
    :
      lda (zpsave_addr_l),y
      sta $00,y
      dey
      cmp #$60
      bne :-

  .endmacro

  .macro ZP_SAVE_INC
    lda zpsave_addr_h
    inc
    sta zpsave_addr_h
  .endmacro

  .macro ZP_SAVE_DEC
    lda zpsave_addr_h
    dec
    sta zpsave_addr_h
  .endmacro

