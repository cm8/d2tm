#include "allegro.h"

#include "allegro/internal/aintern.h"

#include ALLEGRO_INTERNAL_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define MIDI_ALSA_SEQ	AL_ID('A','S','E','Q')
AL_VAR(MIDI_DRIVER, midi_alsa_seq);

#ifdef __cplusplus
}
#endif