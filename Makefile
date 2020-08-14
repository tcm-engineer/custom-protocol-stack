# Modified by Domenico Balsamo

TRG	= rfm12b
SRC	= main.cpp 1_PHY/PHY.cpp 2_1_MAC/csma.cpp 2_2_LLC/LLC.cpp 3_NET/NET.cpp 4_TRAN/TRAN.cpp 5_APP/APP.cpp application/application.cpp rfm12.cpp 
#DEFS += -DID=2
#SUBDIRS	= tft-cpp common

PRGER		= usbasp
MCU_TARGET	= atmega644p
MCU_FREQ	= 12000000

TFT_HARDWARE	= ili9341_parallel
TFT_PORT_CTRL	= A
TFT_PORT_DATA	= C

DEFS	+= -D__PLATFORM_AVR__
LIBS	+= -lm

EFUSE	= 0xFF
HFUSE	= 0x9C
LFUSE	= 0xE7

include Makefile_AVR.defs