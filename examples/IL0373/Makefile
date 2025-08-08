TARGET := IL0373
C_DEFS := \

#-D DEBUG=0


#LIBDIR += -L"$(SELF_DIR)CH572libs/BLELIB"

NEEDS_UNBRICK = defined

C_INCLUDES := -I"../../CH572libs/BLELIB" -I"./EPD"

C_SOURCES += $(wildcard ./EPD/*.c)

include ../../ch572.mk
