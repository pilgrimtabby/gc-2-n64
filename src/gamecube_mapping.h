#ifndef _gamecube_mapping_h__
#define _gamecube_mapping_h__

#include <stdint.h>
#include "mapper.h"
#include "gamecube.h"

extern struct mapping_controller_unit g_gamecube_status[];
void gc_report_to_mapping(const unsigned char gcr[GC_REPORT_SIZE], struct mapping_controller_unit *gcs);
void setOriginsFromReport(const unsigned char gcr[GC_REPORT_SIZE]);

/*
 * Read the current post-calb() axis values for a raw GC report, with
 * notch calibration bypassed.  Used by the calibration wizard.
 */
void gc_get_raw_axes(const unsigned char gcr[GC_REPORT_SIZE], int8_t *out_x, int8_t *out_y);

#define MAP_GC_BTN_A            0
#define MAP_GC_BTN_B            1
#define MAP_GC_BTN_Z            2
#define MAP_GC_BTN_START        3
#define MAP_GC_BTN_L            4
#define MAP_GC_BTN_R            5

#define MAP_GC_AXB_C_UP         6
#define MAP_GC_AXB_C_DOWN       7
#define MAP_GC_AXB_C_LEFT       8
#define MAP_GC_AXB_C_RIGHT      9

#define MAP_GC_BTN_DPAD_UP      10
#define MAP_GC_BTN_DPAD_DOWN    11
#define MAP_GC_BTN_DPAD_LEFT    12
#define MAP_GC_BTN_DPAD_RIGHT   13

#define MAP_GC_AXIS_LEFT_RIGHT  14
#define MAP_GC_AXIS_UP_DOWN     15
#define MAP_GC_BTN_X            16
#define MAP_GC_BTN_Y            17

#define MAP_GC_AXB_JOY_UP       18
#define MAP_GC_AXB_JOY_DOWN     19
#define MAP_GC_AXB_JOY_LEFT     20
#define MAP_GC_AXB_JOY_RIGHT    21

#define MAP_GC_AXIS_C_LEFT_RIGHT    22
#define MAP_GC_AXIS_C_UP_DOWN       23

#define MAP_GC_L_05  24
#define MAP_GC_L_10  25
#define MAP_GC_L_15  26
#define MAP_GC_L_20  27
#define MAP_GC_L_25  28
#define MAP_GC_L_30  29
#define MAP_GC_L_35  30
#define MAP_GC_L_40  31
#define MAP_GC_L_45  32
#define MAP_GC_L_50  33
#define MAP_GC_L_55  34
#define MAP_GC_L_60  35
#define MAP_GC_L_65  36
#define MAP_GC_L_70  37
#define MAP_GC_L_75  38
#define MAP_GC_L_80  39
#define MAP_GC_L_85  40
#define MAP_GC_L_90  41
#define MAP_GC_L_95  42
#define MAP_GC_L_100 43

#define MAP_GC_R_05  44
#define MAP_GC_R_10  45
#define MAP_GC_R_15  46
#define MAP_GC_R_20  47
#define MAP_GC_R_25  48
#define MAP_GC_R_30  49
#define MAP_GC_R_35  50
#define MAP_GC_R_40  51
#define MAP_GC_R_45  52
#define MAP_GC_R_50  53
#define MAP_GC_R_55  54
#define MAP_GC_R_60  55
#define MAP_GC_R_65  56
#define MAP_GC_R_70  57
#define MAP_GC_R_75  58
#define MAP_GC_R_80  59
#define MAP_GC_R_85  60
#define MAP_GC_R_90  61
#define MAP_GC_R_95  62
#define MAP_GC_R_100 63

#define MAP_GC_NONE  64

#endif // _gamecube_mapping_h__
