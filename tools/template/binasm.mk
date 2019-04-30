# requires:
#    make v3.81 or higher
#    BINFILE = name of the binfile to make
#    OBJS = list of objects to assemble
#    TARGET_ARCH = arch to compile for (apple2, apple2enh)

# configuration for paths to find stuff
TEMPLATE_PATH	:= $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
TOOLS_PATH		:= $(TEMPLATE_PATH)..
CC65_PATH 		:= $(TEMPLATE_PATH)../cc65-snapshot-win32/bin
# extra, global includes for all source files
INCLUDE_PATH	:= $(TEMPLATE_PATH)include
LIBAPPLE_PATH	:= $(TEMPLATE_PATH)libapple
ASMCFG_PATH		:= $(TEMPLATE_PATH)asmconfig
PYTHON_PATH		:= $(TEMPLATE_PATH)python

OBJ_PATH = obj
OBJS_PATH = $(addprefix $(OBJ_PATH)/, $(OBJS)) $(addprefix $(OBJ_PATH)/, $(LIBS))

# compiler/linker flags
AFLAGS = -t $(TARGET_ARCH) -I $(subst /,\,$(INCLUDE_PATH)) -v
#-W2
LFLAGS = -C $(ASMCFG_PATH)/$(TARGET_ASM_MAP) -m linkmap.txt -v

# location of tools
CA65 = $(CC65_PATH)/ca65.exe
LD65 = $(CC65_PATH)/ld65.exe
AC = java -jar $(AC_PATH)

BINFILE_FULL = $(BINFILE).bin

$(BINFILE_FULL): $(OBJS_PATH)
	$(LD65) $(LFLAGS) $(OBJS_PATH) -o $(BINFILE_FULL)

$(OBJ_PATH)/apple2rom.o: $(LIBAPPLE_PATH)/apple2rom.s $(INCLUDE_PATH)/apple2rom.inc $(INCLUDE_PATH)/utils.inc
	$(CA65) $(AFLAGS) $< -o $@

$(OBJ_PATH)/prodos.o: $(LIBAPPLE_PATH)/prodos.s $(INCLUDE_PATH)/prodos.inc
	$(CA65) $(AFLAGS) $< -o $@
	
.DEFAULT_GOAL := all
	
.PHONY: bin cleanbin

bin: $(BINFILE_FULL)
	
cleanbin:
	-rm -f $(OBJ_PATH)/*.o $(BINFILE_FULL)
	