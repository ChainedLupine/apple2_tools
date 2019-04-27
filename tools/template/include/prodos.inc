; ProDOS routines

.scope ProDOS

.global RAMDISK_DISCONNECT, RAMDISK_RECONNECT

.include "prodos-defs.inc"

; ------------------------------------------------
; -- STRUCTS for parameters
; ------------------------------------------------
.struct STRUCT_GET_FILE_INFO_PARAM
	PARAM_COUNT		.byte
	PATHNAME		.addr
	ACCESS			.byte
	FILE_TYPE		.byte
	AUX_TYPE		.word
	STORAGE_TYPE	.byte
	BLOCKS_USED		.word
	MOD_DATE		.word
	MOD_TIME		.word
	CREATE_DATE		.word
	CREATE_TIME		.word
.endstruct

.struct STRUCT_OPEN_PARAM
	PARAM_COUNT		.byte
	PATHNAME		.addr
	IO_BUFFER		.addr	; 1024 bytes required, must fall on page boundary (ie: multiple of $100)
	REF_NUM			.byte
.endstruct

.struct STRUCT_READ_PARAM
	PARAM_COUNT		.byte
	REF_NUM			.byte
	DATA_BUFFER		.addr
	REQUEST_COUNT	.word
	TRANS_COUNT		.word
.endstruct

.struct STRUCT_CLOSE_PARAM
	PARAM_COUNT		.byte
	REF_NUM			.byte
.endstruct


.endscope

; ------------------------------------------------
; -- PROCs for STRUCTS
; ------------------------------------------------

.macro ProDOS_DEFINE_GET_FILE_INFO_PARAM
	.byte $0A ; 11 params
	.RES .sizeof(ProDOS::STRUCT_GET_FILE_INFO_PARAM) - 1, $00
.endmacro

.macro ProDOS_DEFINE_OPEN_PARAM
	.byte $03 ; 3 params
	.addr $0000
	.addr ProDOS::FILE_IO_BUFFER_ADDR
	.byte $00
.endmacro

.macro ProDOS_DEFINE_QUIT_PARAM
	.byte   $04             ;PARAM_COUNT
    .byte   $00             ;QUIT_TYPE
    .word   $0000           ;RESERVED
    .byte   $00             ;RESERVED
    .word   $0000           ;RESERVED
.endmacro

.macro ProDOS_DEFINE_READ_PARAM
	.byte	$04				;PARAM_COUNT
	.RES .sizeof(ProDOS::STRUCT_READ_PARAM) - 1, $00
.endmacro

.macro ProDOS_DEFINE_CLOSE_PARAM
	.byte	$01				;PARAM_COUNT
	.RES .sizeof(ProDOS::STRUCT_CLOSE_PARAM) - 1, $00
.endmacro

; ------------------------------------------------
; -- MACROS for MLI functions
; ------------------------------------------------

.macro ProDOS_QUIT param_addr
		INC $3F4
		JSR ProDOS::MLI
		.byte ProDOS::CALL_QUIT
		.word param_addr
.endmacro

.macro ProDOS_OPEN param_addr
		JSR ProDOS::MLI
		.byte ProDOS::CALL_OPEN
		.word param_addr
.endmacro

.macro ProDOS_READ param_addr
		JSR ProDOS::MLI
		.byte ProDOS::CALL_READ
		.word param_addr
.endmacro

.macro ProDOS_CLOSE param_addr
		JSR ProDOS::MLI
		.byte ProDOS::CALL_CLOSE
		.word param_addr
.endmacro
