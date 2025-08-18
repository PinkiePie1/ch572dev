TARGET := BLE_EPD_AHT
C_DEFS := \

#-D DEBUG=0

LIBS := -lCH572BLE_PERI

NEEDS_UNBRICK = defined

LIBDIR += -L"$(SELF_DIR)CH572libs/BLELIB"

C_INCLUDES := -I"../../CH572libs/BLELIB"\
-I"./APP/include"\
-I"./HAL/include"\
-I"./EPD"

C_SOURCES += \
$(wildcard ./*/*.c)



include ../../ch572.mk
