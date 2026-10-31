/* SPDX-License-Identifier: GPL-3.0-only */
/* Replaces tests/host.c through the same clang VFS mechanism as FM-106. The preceding fake_fm1.h is AURORA's
 * own: its panel, LCD, MIDI queue, RAM stores and serial audio worker stay unchanged; no script or WAV driver. */
static void platform_poll(void) {}
static void midi_send(uint32_t st, uint32_t d1, uint32_t d2) { fm1core_dg_midi_out(st, d1, d2); }
static void midi_sysex(const uint8_t *p, uint32_t n) { fm1core_dg_sysex_out(p, n); }
