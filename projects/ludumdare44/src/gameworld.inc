; game world

.global GW_POPULATE_ROOM, GW_PRINT_CURR_ROOM_DESC
.global GW_PRINT_DESC_TYPEID
.global GW_HANDLE_TALKING

; global typeid table

; inventory items
typeid_invs_start         = $01
typeid_inv_me             = typeid_invs_start + 0
typeid_inv_ramen          = typeid_invs_start + 1

; nearbys
typeid_nearbys_start      = $30
typeid_nearby_refrig      = typeid_nearbys_start + 0

; rooms
typeid_rooms_start        = $60
typeid_apartment          = typeid_rooms_start + 0
typeid_street             = typeid_rooms_start + 1

; exits
typeid_exits_start        = $90
typeid_exit_apt_street    = typeid_exits_start + 0
typeid_exit_street_apt    = typeid_exits_start + 1


tileid_exit_north       = (largetile_width * 3) + 0
tileid_exit_west        = (largetile_width * 3) + 1
tileid_exit_east        = (largetile_width * 3) + 2
tileid_exit_south       = (largetile_width * 3) + 3

tileid_inv_me					  = (largetile_width * 1) + 0
tileid_inv_ramen			  = (largetile_width * 1) + 1

tileid_nearby_test		  = (largetile_width * 2) + 0
tileid_nearby_refrig	  = (largetile_width * 2) + 1
