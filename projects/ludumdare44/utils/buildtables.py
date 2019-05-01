print(";-- tables ----------------------------------")
print("; need a godawful amount of these for DHGR :(")
print(";--------- ----------------------------------")
print("")

print(".segment \"RODATA_H\"")

print("")

print("HGR_Y_ADDR_LOOKUP_L:")
for y in range(0, 192):
	int_y = int(y)
	addr = 0x2000 + (int(int_y/64) | int(0)) * 0x28 + (int(int_y%8) | int(0)) * 0x400 + (int(int_y/8) & int(7)) * 0x80
	str = "{0:04x}".format(addr)
	print("\t.byte ${0} ; {1}".format(str[2:], y))

print("")	
print("HGR_Y_ADDR_LOOKUP_H:")
for y in range(0, 192):
	int_y = int(y)
	addr = 0x2000 + (int(int_y/64) | int(0)) * 0x28 + (int(int_y%8) | int(0)) * 0x400 + (int(int_y/8) & int(7)) * 0x80
	str = "{0:04x}".format(addr)
	print("\t.byte ${0} ; {1}".format(str[:2], y))
	
#print("")	
#print("DATA_Y_ADDR_LOOKUP_L:")
#for y in range(0, 192):
#	int_y = int(y)
#	addr = (int(int_y/64) | int(0)) * 0x28 + (int(int_y%8) | int(0)) * 0x400 + (int(int_y/8) & int(7)) * 0x80
#	str = "{0:04x}".format(addr)
#	print("\t.byte ${0} ; {1}".format(str[2:], y))

print("")	
print("DATA_Y_ADDR_LOOKUP_H:")
for y in range(0, 192):
	int_y = int(y)
	addr = (int(int_y/64) | int(0)) * 0x28 + (int(int_y%8) | int(0)) * 0x400 + (int(int_y/8) & int(7)) * 0x80
	str = "{0:04x}".format(addr)
	print("\t.byte ${0} ; {1}".format(str[:2], y))
	
print("")
print("LARGETILE_X_BYTE_LOOKUP:")
for tile in range(0, 120):
	int_x = int(tile % 10)
	offset = int(int_x * 4)
	print("\t.byte ${0:02x} ; {1}".format(offset, tile))

print("")
print("LARGETILE_TILEID_Y_LOOKUP:")
for tile in range(0, 120):
	int_y = int(tile / 10) * 16
	print("\t.byte ${0:02x} ; {1}".format(int_y, tile))

print("")
print("SMALLTILE_X_BYTE_LOOKUP:")
for x in range(0, 20):
	offset = int(x * 2)
	print("\t.byte ${0:02x} ; {1}".format(offset, x))
	
print("")
print("SMALLTILE_Y_LOOKUP:")
for y in range(0, 24):
	offset = int(y * 8)
	print("\t.byte ${0:02x} ; {1}".format(offset, y))
		