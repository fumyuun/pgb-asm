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
.data 0x48,0x45,0x4C,0x4C,0x4F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

.data 0x00			# 0x00: GB, 0x80: CGB
.data 0x00,0x00		# licensee (new)
.data 0x00			# 0x00: GB, 0x03: SGB
.data 0x00			# ROM-type
.data 0x00			# ROM-size
.data 0x00			# RAM-size
.data 0x01			# 0x00: JP, 0x01: other
.data 0x33			# licensee (old)
.data 0x00			# Mask ROM version number
# CHECKSUM STOP

.data 0x3F			# Header checksum (complement check)
.data 0x00,0x00		# Checksum (ignored)
#HEADER END

#0x100, entry point
NOP
JP main

#logo 
.data 0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D
.data 0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99
.data 0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E

# CHECKSUM START
# title 
.data 0x48,0x45,0x4C,0x4C,0x4F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

.data 0x00			# 0x00: GB, 0x80: CGB
.data 0x00,0x00		# licensee (new)
.data 0x00			# 0x00: GB, 0x03: SGB
.data 0x00			# ROM-type
.data 0x00			# ROM-size
.data 0x00			# RAM-size
.data 0x01			# 0x00: JP, 0x01: other
.data 0x33			# licensee (old)
.data 0x00			# Mask ROM version number
# CHECKSUM STOP

.data 0x3F			# Header checksum (complement check)
.data 0x00,0x00		# Checksum (ignored)
#HEADER END

# Tiles for ascii table below (Commodore PET font, yeah!)
tileset:
.data 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
.data 0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,
.data 0x24,0x00,0x24,0x00,0x24,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
.data 0x24,0x00,0x24,0x00,0x7E,0x00,0x24,0x00,0x7E,0x00,0x24,0x00,0x24,0x00,0x00,0x00,
.data 0x08,0x00,0x1E,0x00,0x28,0x00,0x1C,0x00,0x0A,0x00,0x3C,0x00,0x08,0x00,0x00,0x00,
.data 0x00,0x00,0x62,0x00,0x64,0x00,0x08,0x00,0x10,0x00,0x26,0x00,0x46,0x00,0x00,0x00,
.data 0x30,0x00,0x48,0x00,0x48,0x00,0x30,0x00,0x4A,0x00,0x44,0x00,0x3A,0x00,0x00,0x00,
.data 0x04,0x00,0x08,0x00,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
.data 0x04,0x00,0x08,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x08,0x00,0x04,0x00,0x00,0x00,
.data 0x20,0x00,0x10,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x10,0x00,0x20,0x00,0x00,0x00,
.data 0x08,0x00,0x2A,0x00,0x1C,0x00,0x3E,0x00,0x1C,0x00,0x2A,0x00,0x08,0x00,0x00,0x00,
.data 0x00,0x00,0x08,0x00,0x08,0x00,0x3E,0x00,0x08,0x00,0x08,0x00,0x00,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x08,0x00,0x10,0x00,
.data 0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x18,0x00,0x00,0x00,
.data 0x00,0x00,0x02,0x00,0x04,0x00,0x08,0x00,0x10,0x00,0x20,0x00,0x40,0x00,0x00,0x00,
.data 0x3C,0x00,0x42,0x00,0x46,0x00,0x5A,0x00,0x62,0x00,0x42,0x00,0x3C,0x00,0x00,0x00,
.data 0x08,0x00,0x18,0x00,0x28,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x3E,0x00,0x00,0x00,
.data 0x3C,0x00,0x42,0x00,0x02,0x00,0x0C,0x00,0x30,0x00,0x40,0x00,0x7E,0x00,0x00,0x00,
.data 0x3C,0x00,0x42,0x00,0x02,0x00,0x1C,0x00,0x02,0x00,0x42,0x00,0x3C,0x00,0x00,0x00,
.data 0x60,0x00,0x0A,0x00,0x12,0x00,0x22,0x00,0x7F,0x00,0x02,0x00,0x02,0x00,0x00,0x00,
.data 0x7E,0x00,0x40,0x00,0x78,0x00,0x04,0x00,0x02,0x00,0x44,0x00,0x38,0x00,0x00,0x00,
.data 0x1C,0x00,0x20,0x00,0x40,0x00,0x7C,0x00,0x42,0x00,0x42,0x00,0x3C,0x00,0x00,0x00,
.data 0x7E,0x00,0x42,0x00,0x04,0x00,0x08,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x00,0x00,
.data 0x3C,0x00,0x42,0x00,0x42,0x00,0x3C,0x00,0x42,0x00,0x42,0x00,0x3C,0x00,0x00,0x00,
.data 0x3C,0x00,0x42,0x00,0x42,0x00,0x3E,0x00,0x02,0x00,0x04,0x00,0x38,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x08,0x00,0x10,0x00,
.data 0x0E,0x00,0x18,0x00,0x30,0x00,0x60,0x00,0x30,0x00,0x18,0x00,0x0E,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
.data 0x70,0x00,0x18,0x00,0x0C,0x00,0x06,0x00,0x0C,0x00,0x18,0x00,0x70,0x00,0x00,0x00,
.data 0x3C,0x00,0x42,0x00,0x02,0x00,0x0C,0x00,0x10,0x00,0x00,0x00,0x10,0x00,0x00,0x00,
.data 0x1C,0x00,0x22,0x00,0x4A,0x00,0x56,0x00,0x4C,0x00,0x20,0x00,0x1E,0x00,0x00,0x00,
.data 0x18,0x00,0x24,0x00,0x42,0x00,0x7E,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x00,0x00,
.data 0x7C,0x00,0x22,0x00,0x22,0x00,0x3C,0x00,0x22,0x00,0x22,0x00,0x7C,0x00,0x00,0x00,
.data 0x1C,0x00,0x22,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x22,0x00,0x1C,0x00,0x00,0x00,
.data 0x78,0x00,0x24,0x00,0x22,0x00,0x22,0x00,0x22,0x00,0x24,0x00,0x78,0x00,0x00,0x00,
.data 0x7E,0x00,0x40,0x00,0x40,0x00,0x78,0x00,0x40,0x00,0x40,0x00,0x7E,0x00,0x00,0x00,
.data 0x7E,0x00,0x40,0x00,0x40,0x00,0x78,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x00,0x00,
.data 0x1C,0x00,0x22,0x00,0x40,0x00,0x4E,0x00,0x42,0x00,0x22,0x00,0x1C,0x00,0x00,0x00,
.data 0x42,0x00,0x42,0x00,0x42,0x00,0x7E,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x00,0x00,
.data 0x1C,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x1C,0x00,0x00,0x00,
.data 0x0E,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x44,0x00,0x38,0x00,0x00,0x00,
.data 0x42,0x00,0x44,0x00,0x48,0x00,0x70,0x00,0x48,0x00,0x44,0x00,0x42,0x00,0x00,0x00,
.data 0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x7E,0x00,0x00,0x00,
.data 0x42,0x00,0x66,0x00,0x5A,0x00,0x5A,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x00,0x00,
.data 0x42,0x00,0x62,0x00,0x52,0x00,0x4A,0x00,0x46,0x00,0x42,0x00,0x42,0x00,0x00,0x00,
.data 0x18,0x00,0x24,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x24,0x00,0x18,0x00,0x00,0x00,
.data 0x7C,0x00,0x42,0x00,0x42,0x00,0x7C,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x00,0x00,
.data 0x18,0x00,0x24,0x00,0x42,0x00,0x42,0x00,0x4A,0x00,0x24,0x00,0x1A,0x00,0x00,0x00,
.data 0x7C,0x00,0x42,0x00,0x42,0x00,0x7C,0x00,0x48,0x00,0x44,0x00,0x42,0x00,0x00,0x00,
.data 0x3C,0x00,0x42,0x00,0x40,0x00,0x3C,0x00,0x02,0x00,0x42,0x00,0x3C,0x00,0x00,0x00,
.data 0x3E,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x00,0x00,
.data 0x42,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x3C,0x00,0x00,0x00,
.data 0x42,0x00,0x42,0x00,0x42,0x00,0x24,0x00,0x24,0x00,0x18,0x00,0x18,0x00,0x00,0x00,
.data 0x42,0x00,0x42,0x00,0x42,0x00,0x5A,0x00,0x5A,0x00,0x66,0x00,0x42,0x00,0x00,0x00,
.data 0x42,0x00,0x42,0x00,0x24,0x00,0x18,0x00,0x24,0x00,0x42,0x00,0x42,0x00,0x00,0x00,
.data 0x22,0x00,0x22,0x00,0x22,0x00,0x1C,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x00,0x00,
.data 0x7E,0x00,0x02,0x00,0x04,0x00,0x18,0x00,0x20,0x00,0x40,0x00,0x7E,0x00,0x00,0x00,
.data 0x3C,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x20,0x00,0x3C,0x00,0x00,0x00,
.data 0x00,0x00,0x40,0x00,0x20,0x00,0x10,0x00,0x08,0x00,0x04,0x00,0x02,0x00,0x00,0x00,
.data 0x3C,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x3C,0x00,0x00,0x00,
.data 0x08,0x00,0x14,0x00,0x22,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,
.data 0x0C,0x00,0x10,0x00,0x10,0x00,0x3C,0x00,0x10,0x00,0x70,0x00,0x6E,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x38,0x00,0x04,0x00,0x3C,0x00,0x44,0x00,0x3A,0x00,0x00,0x00,
.data 0x40,0x00,0x40,0x00,0x5C,0x00,0x62,0x00,0x42,0x00,0x62,0x00,0x5C,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x3C,0x00,0x42,0x00,0x40,0x00,0x42,0x00,0x3C,0x00,0x00,0x00,
.data 0x02,0x00,0x02,0x00,0x3A,0x00,0x46,0x00,0x42,0x00,0x46,0x00,0x3A,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x3C,0x00,0x42,0x00,0x7E,0x00,0x40,0x00,0x3C,0x00,0x00,0x00,
.data 0x0C,0x00,0x12,0x00,0x10,0x00,0x7C,0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x3A,0x00,0x46,0x00,0x46,0x00,0x3A,0x00,0x02,0x00,0x3C,0x00,
.data 0x40,0x00,0x40,0x00,0x5C,0x00,0x62,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x00,0x00,
.data 0x08,0x00,0x00,0x00,0x18,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x1C,0x00,0x00,0x00,
.data 0x04,0x00,0x00,0x00,0x0C,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x24,0x00,0x18,0x00,
.data 0x00,0x00,0x20,0x00,0x22,0x00,0x24,0x00,0x28,0x00,0x34,0x00,0x22,0x00,0x00,0x00,
.data 0x18,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x1C,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x76,0x00,0x49,0x00,0x49,0x00,0x49,0x00,0x41,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x5C,0x00,0x62,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x3C,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x3C,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x5C,0x00,0x62,0x00,0x62,0x00,0x5C,0x00,0x40,0x00,0x40,0x00,
.data 0x00,0x00,0x00,0x00,0x3A,0x00,0x46,0x00,0x46,0x00,0x3A,0x00,0x02,0x00,0x02,0x00,
.data 0x00,0x00,0x00,0x00,0x5C,0x00,0x62,0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x3E,0x00,0x40,0x00,0x3C,0x00,0x02,0x00,0x7C,0x00,0x00,0x00,
.data 0x10,0x00,0x10,0x00,0x7C,0x00,0x10,0x00,0x10,0x00,0x12,0x00,0x0C,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x46,0x00,0x3A,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x42,0x00,0x42,0x00,0x42,0x00,0x24,0x00,0x18,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x41,0x00,0x49,0x00,0x49,0x00,0x49,0x00,0x36,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x42,0x00,0x24,0x00,0x18,0x00,0x24,0x00,0x42,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x42,0x00,0x42,0x00,0x46,0x00,0x3A,0x00,0x02,0x00,0x3C,0x00,
.data 0x00,0x00,0x00,0x00,0x7E,0x00,0x04,0x00,0x18,0x00,0x20,0x00,0x7E,0x00,0x00,0x00,
.data 0x06,0x00,0x08,0x00,0x08,0x00,0x30,0x00,0x08,0x00,0x08,0x00,0x06,0x00,0x00,0x00,
.data 0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x00,0x00,
.data 0x30,0x00,0x08,0x00,0x08,0x00,0x06,0x00,0x08,0x00,0x08,0x00,0x30,0x00,0x00,0x00,
.data 0x31,0x00,0x49,0x00,0x46,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
.data 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

# Null terminated string to print
hellostring:
.data "Hoi wereld!"
.data 0x00


# memcpy copy up to 0xFF bytes *dest = (HL), *source = (BC), n = D
memcpy8:
	LD A, (BC)
	LDI (HL), A
	INC BC
	DEC D
	JR NZ memcpy8
	RET

# memcpy copy up to 0xFFFF bytes *dest = (HL), *source = (BC), nn = DE
memcpy16:
	LD A, (BC)
	LDI (HL), A
	INC BC
	DEC DE
	LD A, D
	CP 0x00
	JR NZ memcpy16
	LD A, E
	CP 0x00
	JR NZ memcpy16
	RET

# puts prints null-terminated string pointed by (BC)
puts:
	LD HL, 0x9800
puts_:
	LD A, (BC)
	CP 0x00
	RET Z
	SUB 0x20
	LDI (HL), A
	INC BC
	JR puts_

# wait until next vsync period
vsync:
	LDH A, (0x44)
	CP 0x90
	JR NZ, vsync
	RET

main:
	#disable display
	CALL vsync
	LD A, 0x11
	LDH (0x40), A
	
	# clear screen
	LD HL, 0x9800
	LD DE, 0x400
clear_loop:
	LD A, 0xFF
	LDI (HL), A
	DEC DE
	LD A, D
	CP 0x00
	JR NZ clear_loop
	LD A, E
	CP 0x00
	JR NZ clear_loop

	#init tiles
	LD HL, 0x8000
	LD BC, tileset
	LD DE, 0x600
	CALL memcpy16
	# print string
	LD BC, hellostring
	CALL puts
	# activate display
	LD A, 0x91
	LDH (0x40), A
	LD A, 0xFC
	LDH (0x47), A
loop:
	JR loop