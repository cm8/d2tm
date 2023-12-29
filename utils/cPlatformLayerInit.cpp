#include "cPlatformLayerInit.h"

#include "utils/cLog.h"

#include <allegro/config.h>
#include <allegro/system.h>
#include <allegro/timer.h>

#include <cstring>
#include <stdexcept>

#if defined(ALLEGRO_UNIX) || defined(ALLEGRO_QNX)
#include "utils/alsa/midi_alsa_seq.h"
#endif

static bool allegroInitialized = false;

cPlatformLayerInit::cPlatformLayerInit(const std::string& path_to_config_file, const std::string& window_title) {
    cLogger *logger = cLogger::getInstance();
    logger->logHeader("Allegro");

    if (allegroInitialized) {
        throw std::runtime_error("Allegro initialization was attempted twice.");
    }
    allegroInitialized = true;

    set_config_file(path_to_config_file.c_str());

    // Using the install function wihout registering an at_exit function. The cleanup will be done
    // in the destructor of this object.
    if (install_allegro(SYSTEM_AUTODETECT, &errno, nullptr) != 0) {
        const char* error_message = std::strerror(errno);
        logger->log(LOG_FATAL, COMP_ALLEGRO, "Allegro init", error_message, OUTC_FAILED);
        throw std::runtime_error(error_message);
    }

    logger->log(LOG_INFO, COMP_ALLEGRO, "Allegro init", allegro_id, OUTC_SUCCESS);
    #ifdef AINTUNIX_H
    _unix_register_midi_driver(MIDI_ALSA_SEQ, &midi_alsa_seq, TRUE, TRUE);
    logger->log(LOG_INFO, COMP_ALLEGRO, "Registered alsa port-based allegro midi driver", allegro_id, OUTC_SUCCESS);
    #endif

	set_window_title(window_title.c_str());
	logger->log(LOG_INFO, COMP_ALLEGRO, "Set up window title", window_title, OUTC_SUCCESS);
}

cPlatformLayerInit::~cPlatformLayerInit() {
    cLogger *logger = cLogger::getInstance();
    logger->log(LOG_INFO, COMP_ALLEGRO, "Allegro shutdown", "Shutting down...");
    allegro_exit();
    logger->log(LOG_INFO, COMP_ALLEGRO, "Allegro shutdown", "Thanks for playing!", OUTC_SUCCESS);
}
