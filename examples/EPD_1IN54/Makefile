TARGET := EPD_1IN54
C_DEFS := \

#-D DEBUG=0

LIBS := -lCH57xRF

LIBDIR += -L"$(SELF_DIR)CH572libs/BLELIB"

NEEDS_UNBRICK = "1"

C_INCLUDES := -I"../../CH572libs/BLELIB"\
-I"./EPD"

C_SOURCES := $(wildcard ./*/*.c)

include ../../ch572.mk
