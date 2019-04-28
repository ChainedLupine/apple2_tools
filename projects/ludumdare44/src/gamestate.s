; variables for tracking game state

.include "gamestate.inc"

.segment "CODE_H"

GS_RESET:
		lda #04
		sta state_health
		lda #02
		sta state_time
		RTS

.segment "DATA_H"

	state_health:
		.byte $04
		
	state_time:
		.byte $04
		
		