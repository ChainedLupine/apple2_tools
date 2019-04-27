# requires:
#    make v3.81 or higher
#	 DISKIMAGE = name of the disk image to create
#    DISKVOLUME = name of the volume to create (if using disk creator)
#    BINFILE, BINFILE_FULL = if using loadbin
#    DISKEXTRAS = extra files to load, if using loadextras
# must be loaded after binasm.mk

# where to locate the starter disk and SYS loader
LOADER_PATH		:= $(TEMPLATE_PATH)loader
DISK_PATH		:= $(TEMPLATE_PATH)disks
AC_PATH			:= $(TEMPLATE_PATH)../ac/AppleCommander-ac-1.5.0.jar

# grab start address from our linker config
# (using python because no shell under Windows)
BINSTART := $(shell python $(PYTHON_PATH)/extractstartaddr.py $(ASMCFG_PATH)/$(TARGET_ASM_MAP))

DISKIMAGE_MAKE_PRODOS := makeprodos

.PHONY: dsk cleandsk startaddr

dsk: $(DISKIMAGE)

# builds a disk image w/ self-starting prodos
makeprodos:
	-cp -f $(DISK_PATH)/prodos-203.po $(DISKIMAGE)
	$(AC) -n $(DISKIMAGE) $(DISKVOLUME)

# inserts bin w/ code
loadbin: $(BINFILE_FULL)
	$(AC) -p $(DISKIMAGE) $(BINFILE) bin $(BINSTART) <$(BINFILE_FULL)

# inserts bin w/ code plus the cc65 boostrapper
loadbin_selfstartprodos: $(BINFILE_FULL)
	@echo Using start address of $(BINSTART)
	$(AC) -p $(DISKIMAGE) $(BINFILE) bin $(BINSTART) <$(BINFILE_FULL)
	$(AC) -p $(DISKIMAGE) $(BINFILE).SYSTEM sys 0x2000 <$(LOADER_PATH)\loader.system

cleandsk:
	-rm -f $(DISKIMAGE)
	
startaddr:
	@echo Using start address of $(BINSTART)