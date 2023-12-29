/** port-based ALSA implementation, yet another allegro midi driver
 *  originally imported from OpenDUNE's src/audio/midi_alsa.c */

#include "midi_alsa_seq.h"

#ifndef SCAN_DEPEND
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <errno.h>

  #ifndef __FreeBSD__
    #include <alloca.h>
  #endif /* __FreeBSD__ */

  #if ALLEGRO_ALSA_VERSION == 9
    #define ALSA_PCM_NEW_HW_PARAMS_API 1
    #include <alsa/asoundlib.h>
  #else  /* ALLEGRO_ALSA_VERSION == 5 */
    #include <sys/asoundlib.h>
  #endif
#endif

#include "utils/cLog.h"

int Error(const char* msg, int ret);
void LogMsg(eLogLevel ell, const char* fmt, ...);

static snd_seq_t *s_midi = NULL;
static snd_midi_event_t *s_midiCoder = NULL;
static snd_seq_event_t ev;
static snd_seq_addr_t sender, receiver;
static snd_seq_port_info_t *pinfo = NULL;
static snd_seq_port_subscribe_t *s_midiSubscription = NULL;
static int s_midiPort = -1;

static const char *s_midiCaption = "D2TM MIDI Port";

static int alsa_seq_detect(int input);
static int alsa_seq_init(int input, int voices);
static void alsa_seq_exit(int input);
static void alsa_seq_output(int data);
static void prepare_new_seq_event();

MIDI_DRIVER midi_alsa_seq =
{
   MIDI_ALSA_SEQ,            /* id */
   empty_string,             /* name */
   empty_string,             /* desc */
   "ALSA SEQ",               /* ASCII name */
   0,                        /* voices */
   0,                        /* basevoice */
   0xFFFF,                   /* max_voices */
   0,                        /* def_voices */
   -1,                       /* xmin */
   -1,                       /* xmax */
   alsa_seq_detect,          /* detect */
   alsa_seq_init,            /* init */
   alsa_seq_exit,            /* exit */
   NULL,                     /* set_mixer_volume */
   NULL,                     /* get_mixer_volume */
   alsa_seq_output,          /* raw_midi */
   _dummy_load_patches,      /* load_patches */
   _dummy_adjust_patches,    /* adjust_patches */
   _dummy_key_on,            /* key_on */
   _dummy_noop1,             /* key_off */
   _dummy_noop2,             /* set_volume */
   _dummy_noop3,             /* set_pitch */
   _dummy_noop2,             /* set_pan */
   _dummy_noop2              /* set_vibrato */
};

#define IS_WRITABLE_PORT(pinfo) \
  ((snd_seq_port_info_get_capability(pinfo) & (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE)) \
   == (SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE))

static int alsa_seq_detect(int input) {
	snd_seq_client_info_t *cinfo;
	bool found = false;

	if (input) {
		return FALSE;
	}

	if (snd_seq_open(&s_midi, "default", SND_SEQ_OPEN_OUTPUT, 0) < 0) {
		s_midi = NULL;
		return Error("Failed to initialize MIDI\n", FALSE);
	}
	snd_seq_set_client_name(s_midi, s_midiCaption);

	/* Create a port to work on */
	s_midiPort = snd_seq_create_simple_port(s_midi, s_midiCaption,
			SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
			SND_SEQ_PORT_TYPE_MIDI_GENERIC);
	if (s_midiPort < 0) {
		return Error("Failed to initialize MIDI\n", FALSE);
	}

	/* Try to find a MIDI out */
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_alloca(&cinfo);
	snd_seq_client_info_set_client(cinfo, -1);

	/* If midi client and port is explicitly configured, try that first */
	int cc = get_config_int("sound", "midi_aseq_client", -1);
	int cp = get_config_int("sound", "midi_aseq_port", -1);
	if (cc > 0 && cp >= 0) {
		LogMsg(LOG_INFO, "Read midi aseq client %d and port %d from config.", cc, cp);
		snd_seq_port_info_set_client(pinfo, cc);
		snd_seq_port_info_set_port(pinfo, cp - 1);
		if (snd_seq_query_next_port(s_midi, pinfo) >= 0) {
			if (IS_WRITABLE_PORT(pinfo)) {
				found = true;
			}
		}
	}

	if (!found) {
		LogMsg(LOG_INFO, "Autodetect midi aseq client and port (skips Midi Through and VirMidi).");
	}

	/* Walk all clients and ports, and see if one matches our demands */
	while (!found && snd_seq_query_next_client(s_midi, cinfo) >= 0) {
		int client;

		client = snd_seq_client_info_get_client(cinfo);
		if (client == 0) continue;

		snd_seq_port_info_set_client(pinfo, client);
		snd_seq_port_info_set_port(pinfo, -1);
		while (!found && snd_seq_query_next_port(s_midi, pinfo) >= 0) {
			/* Most linux installations come with a Midi Through Port.
			 *  This is 'hardware' support that mostly ends up on your serial, which
			 *  you most likely do not have connected. So we skip it by default. */
			if (strncmp("Midi Through Port", snd_seq_port_info_get_name(pinfo), 17) == 0) continue;
			if (strncmp("VirMIDI", snd_seq_port_info_get_name(pinfo), 7) == 0) continue;

			if (IS_WRITABLE_PORT(pinfo)) {
				found = true;
			}
		}
	}

	if (!found) {
		return Error("No valid MIDI output ports. Please start Timidity++ or Fluidsynth.", FALSE);
	} else {
		LogMsg(LOG_INFO, "Using midi aseq client %d and writable port %d.",
			snd_seq_port_info_get_client(pinfo),
			snd_seq_port_info_get_port(pinfo)
		);
		return TRUE;
	}
}

static int alsa_seq_init(int input, int voices)
{
	(void) input;
	(void) voices;

	midi_alsa_seq.desc = "ALSA SEQ port-based midi synth";

	/* Subscribe ourself to the port */
	receiver.client = snd_seq_port_info_get_client(pinfo);
	receiver.port = snd_seq_port_info_get_port(pinfo);
	sender.client = snd_seq_client_id(s_midi);
	sender.port = s_midiPort;

	snd_seq_port_subscribe_malloc(&s_midiSubscription);
	snd_seq_port_subscribe_set_sender(s_midiSubscription, &sender);
	snd_seq_port_subscribe_set_dest(s_midiSubscription, &receiver);
	snd_seq_port_subscribe_set_time_update(s_midiSubscription, 1);
	snd_seq_port_subscribe_set_time_real(s_midiSubscription, 1);
	if (snd_seq_subscribe_port(s_midi, s_midiSubscription) < 0) {
		return Error("Failed to subscribe to MIDI output port\n", -1);
	}

	/* Start the MIDI decoder */
	if (snd_midi_event_new(4, &s_midiCoder) < 0) {
		return Error("Failed to initialize MIDI decoder\n", -1);
	}
	snd_midi_event_init(s_midiCoder);

	/* Prepare for first event */
	prepare_new_seq_event();

	return 0;
}

static void alsa_seq_exit(int input)
{
	(void) input;

	if (s_midi != NULL) {
		snd_midi_event_free(s_midiCoder);
		snd_seq_port_subscribe_free(s_midiSubscription);
		snd_seq_delete_port(s_midi, s_midiPort);
		snd_seq_close(s_midi);

		s_midi = NULL;
	}
}

static void prepare_new_seq_event()
{
	snd_seq_ev_clear(&ev);
	snd_seq_ev_set_source(&ev, s_midiPort);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);
}

/* allegro sends portions of one byte in data per call */
static void alsa_seq_output(int data)
{
	int r;

	if (s_midi != NULL) {
		r = snd_midi_event_encode_byte(s_midiCoder, data & 0xff, &ev);
		if (r < 0) {
			LogMsg(LOG_WARN, "snd_midi_event_encode_byte() failed to send byte %02X. err=%d\n", data & 0xff, r);
		}
		if (r == 1) {
			snd_seq_event_output(s_midi, &ev);
			snd_seq_drain_output(s_midi);
		}
		if (r != 0) {
			/* if r == 0 then more bytes are required to complete the event */
			prepare_new_seq_event();
		}
	}
}

/* clears any pending event (unlike alsa_seq_output and midi_send_string)
 * tries to encode a midi event given in data
 * sends encoded, completed midi event
 * clears event in any case before returning
 */
static void midi_send_byte_triple(uint32_t data)
{
	const char* msg = "snd_midi_event_encode_byte() failed to send";
	int r;

	if (s_midi != NULL) {
		prepare_new_seq_event();

		r = snd_midi_event_encode_byte(s_midiCoder, data & 0xff, &ev);	/* status byte */
		if (r < 0) {
			LogMsg(LOG_WARN, "%s status byte %02X. err=%d\n", msg, data & 0xff, r);
		} else if (r == 0) {
			/* snd_midi_event_encode_byte() returns 0 when more bytes are required to complete the event */
			r = snd_midi_event_encode_byte(s_midiCoder, (data >> 8) & 0xff, &ev);	/* 1st data byte */
			if (r < 0) {
				LogMsg(LOG_WARN, "%s 1st data byte %02X. err=%d\n", msg, (data >> 8) & 0xff, r);
			} else if (r == 0) {
				r = snd_midi_event_encode_byte(s_midiCoder, (data >> 16) & 0xff, &ev);	/* 2nd data byte */
				if (r < 0) {
					LogMsg(LOG_WARN, "%s 2nd data byte %02X. err=%d\n", msg, (data >> 16) & 0xff, r);
				} else if (r == 0) {
					LogMsg(LOG_WARN, "midi_send_byte_triple() incomplete event, not sending: %02X %02X %02X\n",
						 data & 0xff, (data >> 8) & 0xff, (data >> 16) & 0xff);
				}
			}
		}

		if (r == 1) {
			snd_seq_event_output(s_midi, &ev);
			snd_seq_drain_output(s_midi);
		}

		prepare_new_seq_event();
	}
}

static uint16_t midi_send_string(const uint8_t * data, uint16_t len)
{
	int r = 0;

	if (s_midi != NULL) {
		r = snd_midi_event_encode(s_midiCoder, data, len, &ev);
		if (r < 0) {
			LogMsg(LOG_WARN, "snd_midi_event_encode() failed. err=%d\n", r);
			prepare_new_seq_event();
		} else if (ev.type != SND_SEQ_EVENT_NONE) {
			snd_seq_event_output(s_midi, &ev);
			snd_seq_drain_output(s_midi);
			prepare_new_seq_event();
		}
	}

	return len - r;
}

static void midi_reset()
{
	if (s_midi != NULL) {
		snd_midi_event_reset_encode(s_midiCoder);
	}
}

int Error(const char* msg, int ret) {
	cLogger::getInstance()->log(LOG_ERROR, COMP_SOUND, "midi_alsa_seq", msg, OUTC_FAILED);

	if (s_midi != NULL) {
		if (s_midiPort >= 0) {
			snd_seq_delete_port(s_midi, s_midiPort);
			s_midiPort = -1;
		}
		snd_seq_close(s_midi);
		s_midi = NULL;
	}
	return ret;
}

void LogMsg(eLogLevel ell, const char* fmt, ...) {
	int size = 0;
	char *msg = NULL;
	va_list ap;

	/* Determine required size */

	va_start(ap, fmt);
	size = vsnprintf(msg, size, fmt, ap);
	va_end(ap);

	if (size < 0)
		return;

	size++; // for '\0'
	msg = (char *) malloc(size);
	if (msg == NULL)
		return;

	va_start(ap, fmt);
	size = vsnprintf(msg, size, fmt, ap);
	va_end(ap);

	if (size >= 0)
		cLogger::getInstance()->log(ell, COMP_SOUND, "midi_alsa_seq", msg);

	free(msg);
}