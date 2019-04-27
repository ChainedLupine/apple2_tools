; internal defines for ROM routines

CLRTXT		:= $FC58
COUT 		:= $FDED
PRBYTE		:= $FDDA 

MACHID		:= $BF96

GETKEYJSR	:= $FD0C

AUXMOVE		:= $C311

MEM_HGR_PAGE1	:= $2000
MEM_HGR_PAGE2	:= $4000

; soft switches (apple //e)
SW_80STOREOFF  	:= $C000		; Allow PAGE1/2 to switch between video pagel and page2 (read: key in)
SW_80STOREON	:= $C001		; Allow PAGE1/2 to switch between main and aux. video memory
SW_RDMAINRAM	:= $C002     	; If 80STORE Off: Read Main Mem $0200-$BFFF
SW_RDCARDRAM 	:= $C003		; If 80STORE Off: Read Aux Mem $0200-$BFFF
SW_WRMAINRAM 	:= $C004  		; If 80STORE Off: Write Main Mem $0200-$BFFF
SW_WRCARDRAM 	:= $C005		; If 80STORE Off: Write Aux Mem $0200-$BFFF
SW_KEYSTROBE	:= $C010		; reset keyboard strobe (W)
SW_SETINTC3ROM	:= $C00A		; ROM in Slot 3
SW_SETSLOTC3ROM := $C00B        ; ROM in Aux Slot
SW_80COLOFF    	:= $C00C		; Turn off 80-col display hardware
SW_80COLON 		:= $C00D		; Turn on 80-col display hardware
SW_GR		  	:= $C050		; Graphics mode
SW_TEXT			:= $C051		; Text mode
SW_FULL  		:= $C052		; Fullscreen graphics
SW_MIXED  		:= $C053		; Mixed graphics
SW_PAGE1	 	:= $C054		; Select page1 display (or main video memory)
SW_PAGE2		:= $C055		; Select page2 display (or aux video memory)
SW_LORES		:= $C056		; Select LORES
SW_HIRES	   	:= $C057		; Select HIRES
SW_SETAN3  		:= $C05E		; Turn on annunciator 3 on game-port IO (enable double-width mode)
SW_CLRAN3		:= $C05F		; Turn off annunciator 3 on game-port IO (disable double-width mode)

; flags
FL_PAGE2		:= $C01C		; is page2 on (or aux selected)

; zeropage temps
A1L			:= $3C
A1H   		:= $3D

A2L			:= $3E
A2H   		:= $3F

A3L			:= $40
A3H   		:= $41

A4L			:= $42
A4H   		:= $43

STACK       := $0100
VERSION		:= $FBB3

.global GETKEY
.global GFX_MODE_DHGR, GFX_DHGR_PAGE1, GFX_DHGR_PAGE2
.global TEXT_MODE_40
.global _ROM_PRINT, _MEMCOPY, _MEMFILL
