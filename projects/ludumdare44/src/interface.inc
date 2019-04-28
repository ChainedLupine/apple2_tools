; interface

.global UI_LOAD_TILES, UI_COPY_TILES_TO_PAGE1
.global UI_CLEAR_RIGHT_PANEL, UI_CLEAR_TOP_PANEL
.global UI_DRAW_RIGHT_PANEL, UI_DRAW_HEALTH, UI_DRAW_TIME
.global UI_SHOW_DISK_LOAD, UI_CLEAR_DISK_LOAD
.global UI_CHECK_INPUT

.global UI_DRAW_INVENTORY
.global UI_DRAW_TOP_RIGHT_PANEL
.global UI_RESET

.global tiles_img_main, tiles_img_aux

;.ifndef INTERFACE_INC_GUARD
;.define INTERFACE_INC_GUARD

largetile_width 		= 10
smalltime_width			= 20
; icon locations in tilemap
tileid_blank				= 0
tileid_disk_icon		= (largetile_width * 11 + 9)

tileid_moon1_icon		= (largetile_width * 10 + 2)
tileid_moon2_icon		= (largetile_width * 11 + 2)
tileid_moon3_icon		= (largetile_width * 11 + 3)
tileid_moon4_icon		= (largetile_width * 10 + 3)

tileid_health1_icon		= (largetile_width * 10 + 4)
tileid_health2_icon		= (largetile_width * 11 + 4)
tileid_health3_icon		= (largetile_width * 11 + 5)
tileid_health4_icon		= (largetile_width * 10 + 5)

tileid_cmd_move				= (largetile_width * 9 + 7)
tileid_cmd_center			= (largetile_width * 10 + 7)
tileid_cmd_interact		= (largetile_width * 11 + 7)
tileid_cmd_examine		= (largetile_width * 10 + 6)
tileid_cmd_talk				= (largetile_width * 10 + 8)

tileid_invbar					= (largetile_width * 8) + 5
tileid_nearbybar			= (largetile_width * 7) + 5
tileid_exits    			= (largetile_width * 6) + 5

tileid_inv_me					= (largetile_width * 1) + 0
tileid_inv_ramen			= (largetile_width * 1) + 1

tileid_nearby_test		= (largetile_width * 2) + 0

tileid_select_arrow   = (largetile_width * 0) + 1

kb_left_arrow     = $08
kb_right_arrow    = $15
kb_up_arrow       = $0b
kb_down_arrow     = $0a
kb_escape         = $1b

interface_mode_4way     = 0
interface_mode_examine  = 1
interface_mode_talk     = 2
interface_mode_interact = 3
interface_mode_walk     = 4
interface_mode_selector = 5

selector_state_mask_inv       = $01
selector_state_mask_nearby    = $02
selector_state_mask_exits     = $04

;.endif