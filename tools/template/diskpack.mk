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

.PHONY: dsk cleandsk startaddr

# builds a disk image w/ self-starting prodos
makeprodos:
	-cp -f $(DISK_PATH)/prodos-203.po $(DISK_TO_MAKE)
	$(AC) -n $(DISK_TO_MAKE) $(DISK_VOLUME_TO_MAKE)

# no prodos
makeblankprodos:
	-cp -f $(DISK_PATH)/prodos-203-blank.po $(DISK_TO_MAKE)
	$(AC) -n $(DISK_TO_MAKE) $(DISK_VOLUME_TO_MAKE)
	
# inserts bin w/ code
loadbin:
	$(AC) -p $(DISK_TO_MAKE) $(BIN_TO_LOAD) bin $(BIN_TO_LOAD_START) <$(BIN_TO_LOAD).BIN

# inserts bin w/ code plus the cc65 boostrapper
loadbin_selfstartprodos:
	@echo Using start address of $(BIN_TO_LOAD_START)
	$(AC) -p $(DISK_TO_MAKE) $(BIN_TO_LOAD) bin $(BIN_TO_LOAD_START) <$(BIN_TO_LOAD).BIN
	$(AC) -p $(DISK_TO_MAKE) $(BIN_TO_LOAD).SYSTEM sys 0x2000 <$(LOADER_PATH)\loader.system

startaddr:
	@echo Using start address of $(BINSTART)