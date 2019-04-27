# be careful, any BINFILE name must be 8 chars max
BINFILE = tile
OBJS = main.o tileengine.o file.o grfx.o
LIBS = apple2rom.o prodos.o
TARGET_ARCH = apple2enh
TARGET_ASM_MAP = apple2enh-asm-prodos-gfx.cfg

# disk file to create to hold binfile
DISKIMAGE = tiletest.po
DISKVOLUME = TILE

ASSETS = assets/tileimg.dhgr assets/hyenas2.dhgr

include ../../tools/template/binasm.mk
include ../../tools/template/diskpack.mk
include ../../tools/template/assettools.mk

# kinda ugly, recursive makes but didn't want to use .phonys
$(DISKIMAGE): $(BINFILE_FULL) $(ASSETS)
	$(MAKE) makeprodos
	$(MAKE) loadbin_selfstartprodos	
	$(AC) -p $(DISKIMAGE) hyenas2.dhgr bin 0x0000 <assets/hyenas2.dhgr
	$(AC) -p $(DISKIMAGE) tileimg.dhgr bin 0x0000 <assets/tileimg.dhgr
	$(MAKE) copyto
	
assets/tileimg.dhgr: assets/tileimg.png
	$(PALETTEDHGR) $<
	
assets/hyenas2.dhgr: assets/hyenas2.png
	$(TOHGR) -dhgr $<
	
src/tables.inc: utils/buildtables.py
	-python utils/buildtables.py >src/tables.inc

obj/grfx.o: src/grfx.s src/zeropage.inc src/grfx.inc
	$(CA65) $(AFLAGS) $< -o $@
	
obj/tileengine.o: src/tileengine.s src/tables.inc src/zeropage.inc src/tileengine.inc
	$(CA65) $(AFLAGS) $< -o $@
	
obj/file.o: src/file.s src/zeropage.inc src/file.inc
	$(CA65) $(AFLAGS) $< -o $@

obj/main.o: src/main.s src/tables.inc src/zeropage.inc  src/tileengine.inc src/file.inc
	$(CA65) $(AFLAGS) $< -o $@
	
copyto: $(DISKIMAGE)
	-cp -f $(DISKIMAGE) ../../ADTPro-2.0.3/disks/CL/Virtual.po

emu: $(DISKIMAGE)
	-../../emu/AppleWin1.28.5.0/Applewin.exe -no-print-screen -d1 $(DISKIMAGE)
	
.PHONY: all clean cleanassets assets

assets: $(ASSETS)

all: | bin dsk

cleanassets: $(ASSETS)
	-rm -f $^

clean: cleanbin cleandsk cleanassets
