.include "apple2rom.inc"
.include "prodos.inc"
.include "utils.inc"
.include "file.inc"
.include "zeropage.inc"

.segment "CODE_END_H"

		IMG_BUFFER_READ_AMOUNT = $2000

		; ZP input:
		; arg1w = addr to filename path
		; arg2w = addr to aux buffer (must be $2000 bytes)
		; arg3w = addr to main buffer (must be $2000 bytes)

FILE_LOAD_DHGR_TO_RAM:
		; ------- attempt to load a file into some buffers -----
		Util_LOAD_ADDR arg1w, open_param+ProDOS::STRUCT_OPEN_PARAM::PATHNAME
		
		ProDOS_OPEN open_param		
		bcc     :+
        jmp     PRODOSERROR
		
		; file is open, so now read
:		LDA open_param+ProDOS::STRUCT_OPEN_PARAM::REF_NUM		
		STA read_param+ProDOS::STRUCT_READ_PARAM::REF_NUM
		STA close_param+ProDOS::STRUCT_CLOSE_PARAM::REF_NUM
		
		; -- handle MAIN load -------------------------
		
		Util_LOAD_ADDR arg2w, read_param+ProDOS::STRUCT_READ_PARAM::DATA_BUFFER
				
		LDA #<IMG_BUFFER_READ_AMOUNT
		STA read_param+ProDOS::STRUCT_READ_PARAM::REQUEST_COUNT
		LDA #>IMG_BUFFER_READ_AMOUNT
		STA read_param+ProDOS::STRUCT_READ_PARAM::REQUEST_COUNT+1

		ProDOS_READ read_param
		bcc     :+
        jmp     PRODOSERROR
		
		; now compare data read
:		LDA #<IMG_BUFFER_READ_AMOUNT
		CMP read_param+ProDOS::STRUCT_READ_PARAM::TRANS_COUNT
		BEQ :+
		JMP READERROR
:		LDA #>IMG_BUFFER_READ_AMOUNT
		CMP read_param+ProDOS::STRUCT_READ_PARAM::TRANS_COUNT+1
		BEQ :+
		JMP READERROR
:		; all good
		
		; -- handle AUX load -------------------------
		
		Util_LOAD_ADDR arg3w, read_param+ProDOS::STRUCT_READ_PARAM::DATA_BUFFER
				
		LDA #<IMG_BUFFER_READ_AMOUNT
		STA read_param+ProDOS::STRUCT_READ_PARAM::REQUEST_COUNT
		LDA #>IMG_BUFFER_READ_AMOUNT
		STA read_param+ProDOS::STRUCT_READ_PARAM::REQUEST_COUNT+1

		ProDOS_READ read_param		
		bcc     :+
        jmp     PRODOSERROR

		; now compare data read
:		LDA #<IMG_BUFFER_READ_AMOUNT
		CMP read_param+ProDOS::STRUCT_READ_PARAM::TRANS_COUNT
		BEQ :+
		JMP READERROR
:		LDA #>IMG_BUFFER_READ_AMOUNT
		CMP read_param+ProDOS::STRUCT_READ_PARAM::TRANS_COUNT+1
		BEQ :+
		JMP READERROR
:		; all good
		
		; -- closing file ---------------------------
		ProDOS_CLOSE close_param
		bcc     :+
        jmp     PRODOSERROR
:		
		; -- all done -------------------------------
		RTS

		
READERROR:
		JSR ROM::TEXT_MODE_40
		
		ROM_PRINT(error_file_msg)
		JSR ROM::GETKEY ; GETKEY

		JMP EXIT_PRODOS
		
PRODOSERROR:
		PHA
		JSR ROM::TEXT_MODE_40
		ROM_PRINT(error_msg)
		PLA
		JSR ROM::PRBYTE
		JSR ROM::GETKEY ; GETKEY

		JMP EXIT_PRODOS

; --------------------------
; some ProDOS entrance stuff
; --------------------------

EXIT_PRODOS:
		JSR ProDOS::RAMDISK_RECONNECT
		ProDOS_QUIT (quit_param)
		RTS
		
START_PRODOS:
		JSR ProDOS::RAMDISK_DISCONNECT
		RTS


.segment "DATA_H"

	error_msg: 		.asciiz "Error, code = "
	error_file_msg:	.asciiz "File read error!"

	open_param:
		ProDOS_DEFINE_OPEN_PARAM
	
	quit_param:
		ProDOS_DEFINE_QUIT_PARAM 

	read_param:
		ProDOS_DEFINE_READ_PARAM
		
	close_param:
		ProDOS_DEFINE_CLOSE_PARAM
