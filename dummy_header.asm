# HEADER START
0x00:	#RST
0x08:
0x10:
0x18:
0x20:
0x28:
0x30:
0x40:	#VBLANK INTERRUPT
0x48:	#LCDC INTERRUPT
0x50:	#TIMER INTERRUPT
0x58:	#SERIAL INTERRUPT
0x60:	#HIGH-TO-LOW INTERRUPT
0x68:	#UNUSED
0x100:

#0x100, entry point
NOP
JP main

#logo 
.data 0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D
.data 0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99
.data 0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E

# CHECKSUM START
# title 
.data "HELLO"

0x143:
.data 0x00			# 0x00: GB, 0x80: CGB
.data 0x00,0x00		# licensee (new)
.data 0x00			# 0x00: GB, 0x03: SGB
.data 0x00			# CART-type
.data 0x00			# ROM-size
.data 0x00			# RAM-size
.data 0x01			# 0x00: JP, 0x01: other
.data 0x33			# licensee (old)
.data 0x00			# Mask ROM version number
# CHECKSUM STOP

.data 0x3F			# Header checksum (complement check)
.data 0x00,0x00		# Checksum (ignored)
#HEADER END