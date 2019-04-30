# be careful, any BINFILE name must be 8 chars max
BINFILE = EXUBER
OBJS = main.o tileengine.o file.o grfx.o textengine.o interface.o gamestate.o gameworld.o
LIBS = apple2rom.o prodos.o
TARGET_ARCH = apple2enh
TARGET_ASM_MAP = apple2enh-asm-prodos-gfx.cfg

DISKIMAGES = EXUBERLD44-D1.PO EXUBERLD44-D2.PO

# disk file to create to hold binfile
DISKIMAGE1 = EXUBERLD44-D1.PO
DISKVOLUME1 = EXUBER1

DISKIMAGE2 = EXUBERLD44-D2.PO
DISKVOLUME2 = EXUBER2

ASSETS_DISK1 = assets/ld44intro.dhgr assets/titlescr.dhgr assets/ingame.dhgr assets/room1.dhgr
ASSETS_DISK2 = assets/ingame.dhgr

include ../../tools/template/binasm.mk
include ../../tools/template/diskpack.mk
include ../../tools/template/assettools.mk

# kinda ugly, recursive makes but didn't want to use .phonys
$(DISKIMAGE1): $(BINFILE_FULL) $(ASSETS_DISK1)
	$(MAKE) DISK_TO_MAKE=$(DISKIMAGE1) DISK_VOLUME_TO_MAKE=$(DISKVOLUME1) makeprodos
	$(MAKE) DISK_TO_MAKE=$(DISKIMAGE1) BIN_TO_LOAD=$(BINFILE) BIN_TO_LOAD_START=$(BINSTART) loadbin_selfstartprodos	
	$(AC) -p $(DISKIMAGE1) ld44intro.dhgr bin 0x0000 <assets/ld44intro.dhgr
	$(AC) -p $(DISKIMAGE1) titlescr.dhgr bin 0x0000 <assets/titlescr.dhgr
	$(AC) -p $(DISKIMAGE1) ingame.dhgr bin 0x0000 <assets/ingame.dhgr
	$(AC) -p $(DISKIMAGE1) room1.dhgr bin 0x0000 <assets/room1.dhgr
	$(MAKE) copyto-disk1

$(DISKIMAGE2): $(ASSETS_DISK2)
	$(MAKE) DISK_TO_MAKE=$(DISKIMAGE2) DISK_VOLUME_TO_MAKE=$(DISKVOLUME2) makeblankprodos
	$(AC) -p $(DISKIMAGE2) ingame.dhgr bin 0x0000 <assets/ingame.dhgr
	$(MAKE) copyto-disk2

assets/ld44intro.dhgr: assets/ld44intro.png
	$(PALETTEDHGR) $<

assets/titlescr.dhgr: assets/titlescr.png
	$(PALETTEDHGR) $<
	
assets/ingame.dhgr: assets/ingame.png
	$(PALETTEDHGR) $<

assets/room1.dhgr: assets/room1.png
	$(PALETTEDHGR) $<

src/tables.inc: utils/buildtables.py
	-python utils/buildtables.py >src/tables.inc

obj/grfx.o: src/grfx.s src/zeropage.inc src/grfx.inc
	$(CA65) $(AFLAGS) $< -o $@
	
obj/tileengine.o: src/tileengine.s src/tables.inc src/zeropage.inc src/tileengine.inc
	$(CA65) $(AFLAGS) $< -o $@
	
obj/file.o: src/file.s src/zeropage.inc src/file.inc
	$(CA65) $(AFLAGS) $< -o $@
	
obj/textengine.o: src/textengine.s src/zeropage.inc src/textengine.inc
	$(CA65) $(AFLAGS) $< -o $@
	
obj/interface.o: src/interface.s src/zeropage.inc src/interface.inc src/gamestate.inc $(INCLUDE_PATH)/utils.inc src/interface-4way.inc src/interface-examine.inc src/interface-selector.inc src/interface-rightpanel.inc src/interface-interact.inc src/interface-walk.inc src/interface-talk.inc
	$(CA65) $(AFLAGS) -l interface.txt $< -o $@

obj/gamestate.o: src/gamestate.s src/zeropage.inc src/gamestate.inc src/gameworld.inc
	$(CA65) $(AFLAGS) $< -o $@

obj/gameworld.o: src/gameworld.s src/zeropage.inc src/gameworld.inc src/gameworld-descs.inc src/gameworld-talks.inc src/gameworld-interacts.inc src/gameworld-rooms.inc
	$(CA65) $(AFLAGS) $< -o $@

obj/main.o: src/main.s src/tables.inc src/zeropage.inc  src/tileengine.inc src/file.inc src/titlescreen.inc src/textengine.inc src/interface.inc src/gamestate.inc src/gameworld.inc $(INCLUDE_PATH)/utils.inc
	$(CA65) $(AFLAGS) $< -o $@
	
copyto-disk1: $(DISKIMAGE1)
	-cp -f $(DISKIMAGE1) ../../ADTPro-2.0.3/disks/CL/Virtual.po

copyto-disk2: $(DISKIMAGE2)
	-cp -f $(DISKIMAGE2) ../../ADTPro-2.0.3/disks/CL/Virtual2.po
	
emu: $(DISKIMAGE1) $(DISKIMAGE2)
	-../../emu/AppleWin1.28.5.0/Applewin.exe -no-print-screen -d1 $(DISKIMAGE1)
	
.PHONY: all clean cleanassets assets copyto-disk1 copyto-disk2 cleandsk dsk

assets: $(ASSETS_DISK1) $(ASSETS_DISK2)

all: | bin dsk

dsk: $(DISKIMAGE1) $(DISKIMAGE2)

cleanassets: $(ASSETS)
	-rm -f $^

cleandsk:
	-rm -f $(DISKIMAGE1) $(DISKIMAGE2)
	

clean: cleanbin cleandsk cleanassets
