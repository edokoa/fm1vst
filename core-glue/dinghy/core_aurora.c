/* SPDX-License-Identifier: GPL-3.0-only */
/* AURORA's unchanged app and fake FM-1, with only tests/host.c replaced (core_aurora.cmake). Its app_init /
 * app_loop split is driven on the Dinghy coroutine. The host worker runs submit/join serially, as upstream.
 * Settings use the device's DNGY record at 0xFC000; the sixteen user patches occupy the raw sector at 0x98000.
 * Analog key sensing is hardware-only, as in upstream's host: MIDI velocity and aftertouch remain available. */
#define DINGHY_HOST 1
#define APP_VER FM1CORE_VERSION
#include "dinghy_pre.h"
#include "aurora.c"
#include "exit_end.h"

#define FM1CORE_AUR_SETTINGS_AT 0x000FC000u
#define FM1CORE_AUR_BANK_AT 0x00098000u
_Static_assert(sizeof bank <= BANK_LEN, "AURORA's user patches must fit in their flash sector");
extern fm1panel_t fm1core_dg_panel;

static void app_main(void)
{
    app_init();
    for (;;) {
        fm1core_dg_poll();
        app_loop();
    }
}

static void fm1core_dg_hal_in(uint32_t notes, uint32_t held, uint32_t down, uint32_t up, const int32_t *enc)
{
    uint32_t i;
    fake_keys(notes);
    fm1_in.buttons = held;
    fake_pressed |= down;
    fake_released |= up;
    for (i = 0; i < FM1P_NENC; i++)
        fake_enc[i] += enc[i];
    fake_master = fm1core_dg_panel.master;
}
static uint8_t fm1core_dg_led_level(uint32_t id) { return fake_leds >> id & 1u ? FM1P_LED_LIT : FM1P_LED_OFF; }
static int fm1core_dg_midi_put(uint8_t st, uint8_t d1, uint8_t d2) { return fake_midi_put(st, d1, d2); }

static uint32_t fm1core_aur_settings_saved, fm1core_aur_bank_saved;
static void fm1core_dg_stores_load(void)
{
    uint32_t n = 0;
    memset(fake_store, 0, sizeof fake_store);
    if (!fm1core_dg_sector_read(FM1CORE_AUR_SETTINGS_AT, fake_store + 4, sizeof fake_store - 4u, &n)) {
        fake_store[0] = 1;
        fake_store[1] = (uint8_t)n;
        fake_store[2] = (uint8_t)(n >> 8);
    }
    memcpy(fake_bank, fm1nor_image + FM1CORE_AUR_BANK_AT, sizeof fake_bank);
    fm1core_aur_settings_saved = fake_saves;
    fm1core_aur_bank_saved = fake_bank_saves;
}
static void fm1core_dg_stores_poll(void)
{
    if (fake_saves != fm1core_aur_settings_saved) {
        const uint32_t n = fake_store[1] | (uint32_t)fake_store[2] << 8;
        const int rc = fm1core_dg_sector_save(FM1CORE_AUR_SETTINGS_AT, fake_store + 4, n);
        if (rc)
            printf("fm1core aurora: settings save failed (%d); will retry\n", rc);
        else
            fm1core_aur_settings_saved = fake_saves;
    }
    if (fake_bank_saves != fm1core_aur_bank_saved) {
        int rc = fm1nor_erase(FM1CORE_AUR_BANK_AT);
        if (!rc)
            rc = fm1nor_prog_span(FM1CORE_AUR_BANK_AT, fake_bank, sizeof bank);
        if (rc)
            printf("fm1core aurora: user bank save failed (%d); will retry\n", rc);
        else
            fm1core_aur_bank_saved = fake_bank_saves;
    }
}
static int fm1core_dg_sync(void)
{
    if (dirty) {
        const int rc = settings_save(&set, sizeof set);
        if (rc) {
            printf("fm1core aurora: settings staging failed (%d)\n", rc);
            return 0;
        }
        dirty = 0;
    }
    fm1core_dg_stores_poll();
    return fake_saves == fm1core_aur_settings_saved && fake_bank_saves == fm1core_aur_bank_saved;
}
static void fm1core_dg_info(char *buf, uint32_t n)
{
    snprintf(buf, n, "page %u, preset %s%s%s", (unsigned)page, preset_name(set.preset),
             voices_sounding() ? ", voices" : "", autopilot ? ", autopilot" : "");
}

#define FELUCCA_ID "FM-1_4747"
#define FM1CORE_DG_OTA_C "../../cores/aurora/firmware/src/ota.c"
#define FM1CORE_DG_SYSEX_IN(p, n) fm1core_dg_ota_frame((p), (n))
#include "dinghy_glue.h"
