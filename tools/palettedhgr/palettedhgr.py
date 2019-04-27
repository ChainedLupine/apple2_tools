import sys, os, math
import png
from bitstring import *

pngfilenm = sys.argv[1]
dhgrfilename = os.path.splitext(pngfilenm)[0] + '.dhgr'

#lines_to_process = 120 if sys.argv[2] == "mixed" else 192
lines_to_process = 192

# in order of wiki article palette (also used by the Aseprite A2 palette)
# note: Only grey #1 in here with the same RGB values
# Why?  They show up identically on actual //e hardware
# This does not apply to IIgs but I don't own one so write your own converter to test it!  :^)
# It's easy to add, just add another entry to this table with the correct RGB and pal_hgr_map index
pal_hgr_map = [
#	((0x00, 0x00, 0x00), 0 ),	# black
#	((0x6c, 0x29, 0x40), 1 ),	# magenta
#	((0x40, 0x35, 0x78), 8 ),	# dark blue
#	((0xd9, 0x3c, 0xf0), 9 ),	# purple
#	((0x13, 0x57, 0x40), 4 ),	# dark green
#	((0x80, 0x80, 0x80), 5 ),	# grey #1
#	((0x26, 0x97, 0xf0), 12),	# medium blue
#	((0xbf, 0xb4, 0xf8), 13),	# light blue
#	((0x40, 0x4b, 0x07), 2 ),	# brown
#	((0xd9, 0x68, 0x0f), 3 ),	# orange
#	((0xec, 0xa8, 0xbf), 11),	# pink
#	((0x26, 0xc3, 0x0f), 6 ),	# green
#	((0xbf, 0xca, 0x87), 7 ),	# yellow	
#	((0x93, 0xd6, 0xbf), 14),	# aqua
#	((0xff, 0xff, 0xff), 15)	# white

	((0x00, 0x00, 0x00), 0 ),	# black
	((0x26, 0x43, 0xff), 1 ),	# blue #1
	((0x73, 0x0d, 0x2f), 8 ),	# magenta
	((0xd9, 0x3c, 0xf0), 9 ),	# purple
	((0x21, 0x40, 0x27), 4 ),	# dark green #1
	((0x70, 0x70, 0x70), 5 ),	# grey #1
	((0xf5, 0x84, 0x1b), 12),	# orange
	((0xfd, 0x8f, 0xff), 13),	# light purple
	
	((0x23, 0x61, 0x2a), 2 ),	# dark green #2
	((0x10, 0x81, 0xeb), 3 ),	# light blue
	((0xc6, 0x96, 0xfa), 11),	# light purple
	((0x26, 0xc3, 0x0f), 6 ),	# green
	((0x3e, 0xf7, 0xee), 7 ),	# cyan
	((0xe1, 0xe8, 0x7b), 14),	# yellow
	((0xff, 0xff, 0xff), 15)	# white

]

# in order of bit pattern
hgr_bit_map = [ 
	0b0000,	# 0,black
	0b0001, # 1,magenta (aka red in some references)
	0b0010, # 2,brown										
	0b0011, # 3,orange
	0b0100, # 4,dark green
	0b0101, # 5,grey #1
	0b0110, # 6,green
	0b0111, # 7,yellow
	0b1000, # 8,dark blue
	0b1001, # 9,purple (aka violet in tech ref)
	0b1010, # A,grey #2
	0b1011, # B,pink
	0b1100, # C,medium blue
	0b1101, # D,light blue 13
	0b1110, # E,aqua
	0b1111, # F,white
]

print ("Reading " + pngfilenm)

pngfile = open(pngfilenm, "rb")

pngstream = png.Reader(pngfile)
pngdata = pngstream.read()


sizex = pngdata[0]
sizey = pngdata[1]

if sizex == 140 and sizey == 192:
	#print(pngdata)
	
	img_rows=list(pngdata[2])
	is8bit = pngdata[3]['bitdepth'] == 8
	
	if is8bit:
		palette = pngdata[3]['palette']
		print ("Processing as a paletted image.  Present colors = {}".format(len(palette)))
		
		print ("Mapping palette...")
		# will contain png palette idx and DHGR bit pattern
		mapped_pal = [None] * len(palette)
		for idx in range(0, len(palette)):
			pal_entry = palette[idx]
			found_entry = None
			for p_hgr_e in pal_hgr_map:
				if pal_entry[0] == p_hgr_e[0][0] and pal_entry[1] == p_hgr_e[0][1] and pal_entry[2] == p_hgr_e[0][2]:
					bit_pattern = hgr_bit_map[p_hgr_e[1]]
					found_entry = pal_entry
					mapped_pal[idx] = bit_pattern
					#print ("Mapping idx {:02x} to bit pattern {:04b}".format(idx, bit_pattern))
			if found_entry is None:
				print ("Error, missing DHGR palette entry for idx {}, color {}".format(idx, pal_entry))
				mapped_pal[idx] = 0b1111
				
		# now convert to our bytearrays
		print("Generating aux and main buffer...")
		aux_buffer = bytearray (0x2000)   # 8192 bytes
		main_buffer = bytearray (0x2000)   # 8192 bytes

		bit_patterns = [None] * 7

		#for y in range(0, 1): #192):
		for y in range(0, lines_to_process):
			addr = (int(y/64) | int(0)) * 0x28 + (int(y%8) | int(0)) * 0x400 + (int(y/8) & int(7)) * 0x80
			#print ("addr={:04x}".format(addr))
			row_data = img_rows[y]
			# now process a line
			
			#for x in range(0, 7, 7): #140, 7):
			for x in range(0, 140, 7):
				# time to get LAAAAAAAAAAAAZZZZZZZZZZZZZZZZZZZZYYYYYYYYYYYYYYYYY with bitstring
				
				#print ("pixel ", x,  " of row ", y)
				# lookup bit pattern
				bit_patterns[0] = mapped_pal[row_data[x]]
				bit_patterns[1] = mapped_pal[row_data[x+1]]
				bit_patterns[2] = mapped_pal[row_data[x+2]]
				bit_patterns[3] = mapped_pal[row_data[x+3]]
				bit_patterns[4] = mapped_pal[row_data[x+4]]
				bit_patterns[5] = mapped_pal[row_data[x+5]]
				bit_patterns[6] = mapped_pal[row_data[x+6]]

				ba = BitArray()

				for p_idx in range(0, len(bit_patterns)):
					bp = bit_patterns[p_idx]
					#print(bp)
					bits = BitArray(uint=bp, length=4)
					bits.reverse()
					ba.append (bits)
				
				# insert padding
				ba.insert('uint:1=0', 7)
				ba.insert('uint:1=0', 15)
				ba.insert('uint:1=0', 23)
				ba.insert('uint:1=0', 31)
				
				ba.reverse(0,8)
				ba.reverse(8,16)
				ba.reverse(16,24)
				ba.reverse(24,32)
				#print (ba.bin)
				
				byte_idx = math.floor(x / 7) * 2
				#print (byte_idx)
				
				data_bytes = ba.bytes
				aux_buffer[addr + byte_idx] = ba.bytes[0]
				main_buffer[addr + byte_idx] = ba.bytes[1]
				aux_buffer[addr + byte_idx + 1] = ba.bytes[2]
				main_buffer[addr + byte_idx + 1] = ba.bytes[3]
				
		# write aux and main buffers
		out_file = open (dhgrfilename, 'bw')
		out_file.write(aux_buffer)
		out_file.write(main_buffer)
		
		out_file.close()
				
	else:
		print ("Processing as an RGB image.  TODO.")
		sys.exit(0)
			

else:
	print ("We do not support images which are not 140x192!")
	
print ("Done!")

pngfile.close()


