/*

  Dune II - The Maker

  Author : Stefan Hendriks
  Contact: stefan@fundynamic.com
  Website: http://dune2themaker.fundynamic.com

  2001 - 2021 (c) code by Stefan Hendriks

  */

// TODO: Clean this class up big time.

#ifndef D2TM_GAME_H
#define D2TM_GAME_H

//#include <controls/cGameControlsContext.h>
//#include <player/cPlayer.h>

#include <controls/cMouse.h>
#include <observers/cScenarioObserver.h>
#include <data/cAllegroDataRepository.h>

// forward declaration :/ sigh should really look into these includes and such
class cRectangle;
class cAbstractMentat;
class cPlayer;
class cGameControlsContext;
class cInteractionManager;

class cGame : public cScenarioObserver {

public:

	cGame();
	~cGame();

	std::string game_filename;

	bool windowed;				// windowed
	char version[15];			// version number, or name.

    // Alpha (for fading in/out)
    int iAlphaScreen;           // 255 = opaque , anything else
    int iFadeAction;            // 0 = NONE, 1 = fade out (go to 0), 2 = fade in (go to 255)

    // resolution of the game
	int screen_x;
	int screen_y;
    int ini_screen_width;
    int ini_screen_height;

    bool bPlaySound;            // play sound?
    bool bDisableAI;            // disable AI thinking?
    bool bOneAi;                // disable all but one AI brain? (default == false)
    bool bDisableReinforcements;// disable any reinforcements from scenario ini file?
    bool bDrawUsages;           // draw the amount of structures/units/bullets used during combat
    bool bDrawUnitDebug;        // draw the unit debug info (rects, paths, etc)
    bool bNoAiRest;             // Campaign AI does not have long initial REST time
    bool bPlayMusic;            // play any music?
    bool bMp3;                  // use mp3 files instead of midi

	bool bPlaying;				// playing or not
    bool bSkirmish;             // playing a skirmish game  or not
	int  iSkirmishMap;			// what map is selected
	int screenshot;				// screenshot taking number
	int iSkirmishStartPoints;	// random map startpoints

	void init();		// initialize all game variables
	void mission_init(); // initialize variables for mission loading only
	void run();			// run the game

    int iRegion;        // what region is selected? (changed by cSelectYourNextConquestState class)
	int iMission;		// what mission are we playing? (= techlevel)

	int selected_structure;
	int hover_unit;

	int paths_created;

    int iMusicVolume;       // volume of the mp3 / midi


    int iMusicType;

    // Condition to win the mission:
    int iWinQuota;              // > 0 means, get this to win the mission, else, destroy all!

    void think_winlose();
    void winning();       // winning
    void losing();        // losing

    // TODO: This belongs to a Mouse state?
	bool bPlaceIt;		// placing something? (for structures only)
	bool bPlacedIt;		// for remembering, for combat_mouse stuff..

    // TODO: This belongs to a Mouse state?
	bool bDeployIt;		// deploying something? (for palace)
	bool bDeployedIt;   // for remembering, for combat_mouse stuff..

	void setup_players();

    void think_music();

	void think_mentat();

    void FADE_OUT(); // fade out with current screen_bmp, this is a little game loop itself!

    void prepareMentatForPlayer();

	bool setupGame();
	void shutdown();

	bool isState(int thisState);
	void setState(int newState);

	int getMaxVolume() { return iMaxVolume; }

	cSoundPlayer * getSoundPlayer() {
	    return soundPlayer;
	}

    void combat_mouse();

	int getGroupNumberFromKeyboard();
	void destroyAllUnits(bool);
	void destroyAllStructures(bool);

    int getColorFadeSelected(int r, int g, int b) {
        // Fade with all rgb
        return getColorFadeSelected(r, g, b, true, true, true);
    }

    int getColorFadeSelectedRed(int r, int g, int b) {
        return getColorFadeSelected(r, g, b, true, false, false);
    }

    int getColorFadeSelectedGreen(int r, int g, int b) {
        return getColorFadeSelected(r, g, b, false, true, false);
    }

    int getColorFadeSelectedBlue(int r, int g, int b) {
        return getColorFadeSelected(r, g, b, false, false, true);
    }

    int getColorFadeSelected(int r, int g, int b, bool rFlag, bool gFlag, bool bFlag);

    int getColorFadeSelected(int color);

    void think_fading();

    cRectangle * mapViewport;

    void init_skirmish() const;

    void createAndPrepareMentatForHumanPlayer();

    void loadScenario();

    void think_state();

    cMouse *getMouse() {
        return mouse; // NOOOO
    }

    void shakeScreen(int duration);

    void setPlayerToInteractFor(cPlayer *pPlayer);

    void onNotify(const s_GameEvent &event) override;
    void onEventDiscovered(const s_GameEvent &event);
    void onEventSpecialLaunch(const s_GameEvent &event);

    static const char* stateString(const int &state) {
        switch (state) {
            case GAME_INITIALIZE: return "GAME_INITIALIZE";
            case GAME_OVER: return "GAME_OVER";
            case GAME_MENU: return "GAME_MENU";
            case GAME_PLAYING: return "GAME_PLAYING";
            case GAME_BRIEFING: return "GAME_BRIEFING";
            case GAME_EDITING: return "GAME_EDITING";
            case GAME_OPTIONS: return "GAME_OPTIONS";
            case GAME_REGION: return "GAME_REGION";
            case GAME_SELECT_HOUSE: return "GAME_SELECT_HOUSE";
            case GAME_TELLHOUSE: return "GAME_TELLHOUSE";
            case GAME_WINNING: return "GAME_WINNING";
            case GAME_WINBRIEF: return "GAME_WINBRIEF";
            case GAME_LOSEBRIEF: return "GAME_LOSEBRIEF";
            case GAME_LOSING: return "GAME_LOSING";
            case GAME_SETUPSKIRMISH: return "GAME_SETUPSKIRMISH";
            default:
                assert(false);
                break;
        }
        return "";
    }

    void reduceShaking();

    cAllegroDataRepository * getDataRepository() {
        return m_dataRepository;
    }

    int getColorPlaceNeutral();

    int getColorPlaceBad();

    int getColorPlaceGood();

private:
    cInteractionManager *_interactionManager;
    cAllegroDataRepository *m_dataRepository;

    cMouse *mouse;

	void updateState();
	void combat();		// the combat part (main) of the game
	bool isMusicPlaying();

    void setup_skirmish();  // set up a skirmish game
	void stateSelectHouse();		// house selection
	void stateMentat(cAbstractMentat *pMentat);  // state mentat talking and interaction
	void menu();		// main menu
	void stateSelectYourNextConquest();		// region selection

	void runGameState();
	void shakeScreenAndBlitBuffer();
	void handleTimeSlicing();

    bool isResolutionInGameINIFoundAndSet();
    void setScreenResolutionFromGameIniSettings();

    void install_bitmaps();

	/** game state **/
	int state;

	int iMaxVolume;

	cSoundPlayer *soundPlayer;
	cAbstractMentat *pMentat;

    float fade_select;        // fade color when selected
    bool bFadeSelectDir;    // fade select direction
    void prepareMentatToTellAboutHouse(int house);

    // screen shaking
    int shake_x;
    int shake_y;
    int TIMER_shake;

    void combat_mouse_normalCombatInteraction(cGameControlsContext *context, cPlayer &humanPlayer,
                                              bool &bOrderingUnits) const;

    void mouse_combat_dragViewportInteraction() const;

    void mouse_combat_resetDragViewportInteraction() const;

    void mouse_combat_hoverOverStructureInteraction(cPlayer &humanPlayer, cGameControlsContext *context, bool bOrderingUnits) const;

    void mouseOnBattlefield(cGameControlsContext *context, int mouseCell, bool &bOrderingUnits) const;
};

#endif