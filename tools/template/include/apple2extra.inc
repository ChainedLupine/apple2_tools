; Convert characters to screen codes

; Helper macro that converts and outputs one character
.macro _scrcode char
        .if (char < 256)
                .byte   (char + 128)
        .else
                .error  "scrcode: Character constant out of range"
        .endif
.endmacro

.macro  scrcodeZ arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9

        ; Bail out if next argument is empty
        .if     .blank (arg1)
                .exitmacro
        .endif

        ; Check for a string
        .if     .match ({arg1}, "")

                ; Walk over all string chars
                .repeat .strlen (arg1), i
                        _scrcode        {.strat (arg1, i)}
                .endrepeat

        ; Check for a number
        .elseif .match (.left (1, {arg1}), 0)

				
                ; Just output the number
                ;_scrcode        arg1
				.byte 0
				.exitmacro

        ; Check for a character
        .elseif .match (.left (1, {arg1}), 'a')

                ; Just output the character
                _scrcode        arg1

        ; Anything else is an error
        .else

                .error  "scrcode: invalid argument type"

        .endif

        ; Call the macro recursively with the remaining args
        scrcodeZ arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9
.endmacro
