/* GC to N64 : Gamecube controller to N64 adapter firmware */
#include <stdlib.h>
#include "gamecube_mapping.h"
#include "eeprom.h"
#include "main.h"
#include "notch_cal.h"

extern uint8_t notch_cal_is_active(void);

/* Rescaled so that 15% and 85% act as the practical endpoints (loosest /
 * tightest), since the trigger's real squeeze range only spans roughly
 * that much of the full theoretical -127..127 scale. Percentages below
 * 15 or above 85 are clamped to those endpoints instead of being
 * unreachable or always-true. */
#define THRES_PCT(pct) (98 - ((pct) * 196 / 100))

// input
struct mapping_controller_unit g_gamecube_status[] = {
    [MAP_GC_BTN_A]          = { .type = TYPE_BTN, },
    [MAP_GC_BTN_B]          = { .type = TYPE_BTN, },
    [MAP_GC_BTN_Z]          = { .type = TYPE_BTN, },
    [MAP_GC_BTN_START]      = { .type = TYPE_BTN, },
    [MAP_GC_BTN_L]          = { .type = TYPE_BTN, },
    [MAP_GC_BTN_R]          = { .type = TYPE_BTN, },
    
    [MAP_GC_AXB_C_UP]       = { .type = TYPE_AXIS_TO_BTN, .thres = THRES_25_NEG, },
    [MAP_GC_AXB_C_DOWN]     = { .type = TYPE_AXIS_TO_BTN, .thres = THRES_25_POS, },
    [MAP_GC_AXB_C_LEFT]     = { .type = TYPE_AXIS_TO_BTN, .thres = THRES_25_NEG, },
    [MAP_GC_AXB_C_RIGHT]    = { .type = TYPE_AXIS_TO_BTN, .thres = THRES_25_POS, },
    
    [MAP_GC_BTN_DPAD_UP]    = { .type = TYPE_BTN, },
    [MAP_GC_BTN_DPAD_DOWN]  = { .type = TYPE_BTN, },
    [MAP_GC_BTN_DPAD_LEFT]  = { .type = TYPE_BTN, },
    [MAP_GC_BTN_DPAD_RIGHT] = { .type = TYPE_BTN, },
    
    [MAP_GC_AXIS_LEFT_RIGHT]= { .type = TYPE_AXIS },
    [MAP_GC_AXIS_UP_DOWN]   = { .type = TYPE_AXIS },
    [MAP_GC_BTN_X]          = { .type = TYPE_BTN, },
    [MAP_GC_BTN_Y]          = { .type = TYPE_BTN, },
    
    [MAP_GC_AXB_JOY_UP]     = { .type = TYPE_AXIS_TO_BTN, .thres = THRES_25_NEG, },
    [MAP_GC_AXB_JOY_DOWN]   = { .type = TYPE_AXIS_TO_BTN, .thres = THRES_25_POS, },
    [MAP_GC_AXB_JOY_LEFT]   = { .type = TYPE_AXIS_TO_BTN, .thres = THRES_25_NEG, },
    [MAP_GC_AXB_JOY_RIGHT]  = { .type = TYPE_AXIS_TO_BTN, .thres = THRES_25_POS, },
        
    [MAP_GC_AXIS_C_LEFT_RIGHT] = { .type = TYPE_AXIS },
    [MAP_GC_AXIS_C_UP_DOWN] = { .type = TYPE_AXIS },
    
    [MAP_GC_L_05]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(5), },
    [MAP_GC_L_10]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(10), },
    [MAP_GC_L_15]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(15), },
    [MAP_GC_L_20]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(20), },
    [MAP_GC_L_25]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(25), },
    [MAP_GC_L_30]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(30), },
    [MAP_GC_L_35]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(35), },
    [MAP_GC_L_40]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(40), },
    [MAP_GC_L_45]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(45), },
    [MAP_GC_L_50]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(50), },
    [MAP_GC_L_55]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(55), },
    [MAP_GC_L_60]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(60), },
    [MAP_GC_L_65]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(65), },
    [MAP_GC_L_70]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(70), },
    [MAP_GC_L_75]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(75), },
    [MAP_GC_L_80]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(80), },
    [MAP_GC_L_85]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(85), },
    [MAP_GC_L_90]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(90), },
    [MAP_GC_L_95]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(95), },
    [MAP_GC_L_100] = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(100), },
    
    [MAP_GC_R_05]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(5), },
    [MAP_GC_R_10]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(10), },
    [MAP_GC_R_15]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(15), },
    [MAP_GC_R_20]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(20), },
    [MAP_GC_R_25]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(25), },
    [MAP_GC_R_30]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(30), },
    [MAP_GC_R_35]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(35), },
    [MAP_GC_R_40]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(40), },
    [MAP_GC_R_45]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(45), },
    [MAP_GC_R_50]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(50), },
    [MAP_GC_R_55]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(55), },
    [MAP_GC_R_60]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(60), },
    [MAP_GC_R_65]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(65), },
    [MAP_GC_R_70]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(70), },
    [MAP_GC_R_75]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(75), },
    [MAP_GC_R_80]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(80), },
    [MAP_GC_R_85]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(85), },
    [MAP_GC_R_90]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(90), },
    [MAP_GC_R_95]  = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(95), },
    [MAP_GC_R_100] = { .type = TYPE_NEG_SLD_TO_BTN, .thres = THRES_PCT(100), },
    
    [MAP_GC_NONE] = { .type = TYPE_NONE },
};

static char gc_x_origin = 0;
static char gc_y_origin = 0;

void setOriginsFromReport(const unsigned char gcr[GC_REPORT_SIZE])
{
    gc_x_origin = gcr[0]^0x80;
    gc_y_origin = gcr[1]^0x80;
}

int calb(char orig, unsigned char val)
{
    short tmp;
    long mult = 26000;
    char dz=0;

    if (g_eeprom_data.conversion_mode == CONVERSION_MODE_OLD_1v5) {
        mult = 25000;
    }

    if (g_eeprom_data.deadzone_enabled) {
        dz = 12;
        mult = 30000;
        if (g_eeprom_data.conversion_mode == CONVERSION_MODE_OLD_1v5) {
            mult = 29000;
        }
    }

    tmp = (signed char)(val^0x80) - orig;

    if (dz) {
        if (tmp > 0) {
            if (tmp < dz) tmp = 0;
            else tmp -= dz;
        } else if (tmp < 0) {
            if (tmp > -dz) tmp = 0;
            else tmp += dz;
        }
    }

    if (g_eeprom_data.conversion_mode != CONVERSION_MODE_EXTENDED) {
        tmp = tmp * mult / 32000L;
    }

    if (tmp<=-127) tmp = -127;
    if (tmp>127) tmp = 127;

    return tmp;
}

void gamecubeXYtoN64(unsigned char x, unsigned char y, char *dst_x, char *dst_y)
{
    unsigned char abs_y, abs_x;
    long sig_x, sig_y;
    long sx, sy;
    int n64_maxval = 80;
    long l = 256;

    if (g_eeprom_data.conversion_mode == CONVERSION_MODE_EXTENDED) {
        sig_x = calb(gc_x_origin, x);
        sig_y = calb(gc_y_origin, y);
        
        if (notch_cal_is_active()) {
            int8_t cx, cy;
            /* Cartesian Bubble: Invert Y entering Zenith, Invert Y exiting Zenith */
            notch_cal_correct((int8_t)sig_x, (int8_t)-sig_y, &cx, &cy);
            sx = cx;
            sy = -cy; 
        } else {
            sx = sig_x;
            sy = sig_y;
        }
        *dst_x = sx;
        *dst_y = sy;
        return;
    }

    if (g_eeprom_data.conversion_mode == CONVERSION_MODE_OLD_1v5) {
        l = 512;
        n64_maxval = 127;
    }

    sig_x = calb(gc_x_origin, x);
    sig_y = calb(gc_y_origin, y);

    if (notch_cal_is_active()) {
        int8_t cx, cy;
        /* Cartesian Bubble: Invert Y entering Zenith, Invert Y exiting Zenith */
        notch_cal_correct((int8_t)sig_x, (int8_t)-sig_y, &cx, &cy);
        sx = cx;
        sy = -cy; 
    } else {
        abs_y = abs(sig_y);
        abs_x = abs(sig_x);
        if (1) {
            sx = sig_x + sig_x * abs_y / l;
            sy = sig_y + sig_y * abs_x / l;
        } else {
            sx = sig_x;
            sy = sig_y;
        }
    }

    if (sx<=-n64_maxval) sx = -n64_maxval;
    if (sx>n64_maxval) sx = n64_maxval;
    if (sy<=-n64_maxval) sy = -n64_maxval;
    if (sy>n64_maxval) sy = n64_maxval;

    *dst_x = sx;
    *dst_y = sy;
}

void gc_report_to_mapping(const unsigned char gcr[GC_REPORT_SIZE], struct mapping_controller_unit *gcs)
{
    /* Digital buttons normalized to strict 1 or 0 to prevent RetroSpy bit-shift overflows */
    gcs[MAP_GC_BTN_A].value = (gcr[6] & 0x10) ? 1 : 0;
    gcs[MAP_GC_BTN_B].value = (gcr[6] & 0x08) ? 1 : 0;
    gcs[MAP_GC_BTN_Z].value = (gcr[6] & 0x80) ? 1 : 0;
    gcs[MAP_GC_BTN_START].value = (gcr[6] & 0x01) ? 1 : 0;
    gcs[MAP_GC_BTN_L].value = (gcr[6] & 0x20) ? 1 : 0;
    gcs[MAP_GC_BTN_R].value = (gcr[6] & 0x40) ? 1 : 0;

    gcs[MAP_GC_BTN_X].value = (gcr[6] & 0x04) ? 1 : 0;
    gcs[MAP_GC_BTN_Y].value = (gcr[6] & 0x02) ? 1 : 0;

    gcs[MAP_GC_BTN_DPAD_UP].value = (gcr[7] & 0x01) ? 1 : 0;
    gcs[MAP_GC_BTN_DPAD_DOWN].value = (gcr[7] & 0x02) ? 1 : 0;
    gcs[MAP_GC_BTN_DPAD_LEFT].value = (gcr[7] & 0x08) ? 1 : 0;
    gcs[MAP_GC_BTN_DPAD_RIGHT].value = (gcr[7] & 0x04) ? 1 : 0;

    /* Analog inputs rely on raw hardware bounds and do not need normalization */
    gcs[MAP_GC_AXB_C_UP].value = gcr[3] ^ 0x80;
    gcs[MAP_GC_AXB_C_DOWN].value = gcr[3] ^ 0x80;
    gcs[MAP_GC_AXB_C_LEFT].value = gcr[2] ^ 0x80;
    gcs[MAP_GC_AXB_C_RIGHT].value = gcr[2] ^ 0x80;

    gamecubeXYtoN64(gcr[0], gcr[1], &gcs[MAP_GC_AXIS_LEFT_RIGHT].value, &gcs[MAP_GC_AXIS_UP_DOWN].value);

    gcs[MAP_GC_AXB_JOY_UP].value = gcs[MAP_GC_AXIS_UP_DOWN].value;
    gcs[MAP_GC_AXB_JOY_DOWN].value = gcs[MAP_GC_AXIS_UP_DOWN].value;
    gcs[MAP_GC_AXB_JOY_LEFT].value = gcs[MAP_GC_AXIS_LEFT_RIGHT].value;
    gcs[MAP_GC_AXB_JOY_RIGHT].value = gcs[MAP_GC_AXIS_LEFT_RIGHT].value;

    int i;
    for(i = MAP_GC_L_05; i <= MAP_GC_L_100; i++) {
        gcs[i].value = gcr[4] ^ 0x80;
    }
    for(i = MAP_GC_R_05; i <= MAP_GC_R_100; i++) {
        gcs[i].value = gcr[5] ^ 0x80;
    }
}

void gc_get_raw_axes(const unsigned char gcr[GC_REPORT_SIZE], int8_t *out_x, int8_t *out_y)
{
    /* Provide pure Cartesian values (+Up) to the Zenith wizard */
    *out_x = (int8_t)calb(gc_x_origin, gcr[0]);
    *out_y = (int8_t)-calb(gc_y_origin, gcr[1]); 
}
