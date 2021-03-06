/*-------------------------------------------------------------------------------

	BARONY
	File: menu.cpp
	Desc: contains code for all menu buttons in the game

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include <list>
#include "main.hpp"
#include "draw.hpp"
#include "game.hpp"
#include "stat.hpp"
#include "messages.hpp"
#include "entity.hpp"
#include "files.hpp"
#include "menu.hpp"
#include "classdescriptions.hpp"
#include "interface/interface.hpp"
#include "magic/magic.hpp"
#include "sound.hpp"
#include "items.hpp"
#include "init.hpp"
#include "shops.hpp"
#include "monster.hpp"
#include "scores.hpp"
#include "menu.hpp"
#include "net.hpp"
#ifdef STEAMWORKS
#include <steam/steam_api.h>
#include "steam.hpp"
#endif
#include "prng.hpp"
#include "credits.hpp"
#include "paths.hpp"
#include "collision.hpp"
#include "player.hpp"
#include "cppfuncs.hpp"
#include "colors.hpp"

#ifdef STEAMWORKS
//Helper func. //TODO: Bugger.
void* cpp_SteamMatchmaking_GetLobbyOwner(void* steamIDLobby)
{
	CSteamID* id = new CSteamID();
	*id = SteamMatchmaking()->GetLobbyOwner(*static_cast<CSteamID*>(steamIDLobby));
	return id; //Still don't like this method.
}
// get player names in a lobby
void* cpp_SteamMatchmaking_GetLobbyMember(void* steamIDLobby, int index)
{
	CSteamID* id = new CSteamID();
	*id = SteamMatchmaking()->GetLobbyMemberByIndex(*static_cast<CSteamID*>(currentLobby), index);
	return id;
}
uint64 SteamAPICall_NumPlayersOnline = 0;
NumberOfCurrentPlayers_t NumberOfCurrentPlayers;
int steamOnlinePlayers = 0;
#endif

// menu variables
bool lobby_window = false;
bool settings_window = false;
int connect_window = 0;
int charcreation_step = 0;
int loadGameSaveShowRectangle = 0; // stores the current amount of savegames available, to use when drawing load game window boxes.
bool singleplayerSavegameExists = false; // used on multiplayer/single player select window to store if savefile exists. 
bool multiplayerSavegameExists = false; // used on multiplayer/single player select window to store if savefile exists. 
/*
 * settings_tab
 * valid values:
 *		- 0 = Video settings
 *		- 1 = Audio settings
 *		- 2 = Keyboard/binding settings
 *		- 3 = Mouse settings
 *		- 4 = Gamepad bindings
 *		- 5 = Gamepad settings
 *		- 6 = Misc settings
 */
int settings_tab = 0;
button_t* button_video_tab = nullptr;
button_t* button_audio_tab = nullptr;
button_t* button_keyboard_tab = nullptr;
button_t* button_mouse_tab = nullptr;
button_t* button_gamepad_bindings_tab = nullptr;
button_t* button_gamepad_settings_tab = nullptr;
button_t* button_misc_tab = nullptr;

int score_window = 0;
int score_window_to_delete = 0;

// gamemods window stuff.
int gamemods_window = 0;
std::list<std::string> currentDirectoryFiles;
std::list<std::string> directoryFilesListToUpload;
std::string directoryToUpload;
std::string directoryPath;
int gamemods_window_scroll = 0;
int gamemods_window_fileSelect = 0;
int gamemods_uploadStatus = 0;
char gamemods_uploadTitle[32] = "Title";
char gamemods_uploadDescription[32] = "Description";
int gamemods_currentEditField = 0;
bool gamemods_workshopSetPropertyReturn[3] = { false, false, false };
int gamemods_subscribedItemsStatus = 0;
char gamemods_newBlankDirectory[32] = "";
char gamemods_newBlankDirectoryOldName[32] = "";
int gamemods_newBlankDirectoryStatus = 0;
int gamemods_numCurrentModsLoaded = -1;
const int gamemods_maxTags = 10;
std::vector<std::pair<std::string, std::string>> gamemods_mountedFilepaths;
std::list<std::string> gamemods_localModFoldernames;
bool gamemods_modelsListRequiresReload = false;
bool gamemods_modelsListLastStartedUnmodded = false; // if starting regular game that had to reset model list, use this to reinit custom models.
bool gamemods_soundListRequiresReload = false;
bool gamemods_soundsListLastStartedUnmodded = false; // if starting regular game that had to reset sounds list, use this to reinit custom sounds.
bool gamemods_tileListRequireReloadUnmodded = false;
bool gamemods_booksRequireReloadUnmodded = false;
bool gamemods_musicRequireReloadUnmodded = false;
bool gamemods_disableSteamAchievements = false;
#ifdef STEAMWORKS
std::vector<SteamUGCDetails_t *> workshopSubscribedItemList;
std::vector<std::pair<std::string, uint64>> gamemods_workshopLoadedFileIDMap;
#endif // STEAMWORKS


bool scoreDisplayMultiplayer = false;
int settings_xres, settings_yres;

typedef std::tuple<int, int> resolution;
std::list<resolution> resolutions;
Uint32 settings_fov;
Uint32 settings_fps;
bool settings_smoothlighting;
int settings_fullscreen, settings_shaking, settings_bobbing;
real_t settings_gamma;
int settings_sfxvolume, settings_musvolume;
int settings_impulses[NUMIMPULSES];
int settings_joyimpulses[NUM_JOY_IMPULSES];
int settings_reversemouse;
real_t settings_mousespeed;
bool settings_broadcast;
bool settings_nohud;
bool settings_colorblind;
bool settings_spawn_blood;
bool settings_light_flicker;
char portnumber_char[6];
char connectaddress[64];
char classtoquickstart[256] = "";
bool spawn_blood = true;
int multiplayerselect = SINGLE;
int menuselect = 0;
bool settings_auto_hotbar_new_items = true;
bool settings_auto_hotbar_categories[NUM_HOTBAR_CATEGORIES] = { true, true, true, true,
																true, true, true, true,
																true, true, true, true };
int settings_autosort_inventory_categories[NUM_AUTOSORT_CATEGORIES] = {	0, 0, 0, 0,
																		0, 0, 0, 0,
																		0, 0, 0, 0 };
bool settings_hotbar_numkey_quick_add = false;
bool settings_disable_messages = true;
bool settings_right_click_protect = false;
bool settings_auto_appraise_new_items = true;
bool playing_random_char = false;
bool colorblind = false;
bool settings_lock_right_sidebar = false;
bool settings_show_game_timer_always = false;
Sint32 oslidery = 0;

//Gamepad settings.
bool settings_gamepad_leftx_invert = false;
bool settings_gamepad_lefty_invert = false;
bool settings_gamepad_rightx_invert = false;
bool settings_gamepad_righty_invert = false;
bool settings_gamepad_menux_invert = false;
bool settings_gamepad_menuy_invert = false;

int settings_gamepad_deadzone = 1;
int settings_gamepad_rightx_sensitivity = 1;
int settings_gamepad_righty_sensitivity = 1;
int settings_gamepad_menux_sensitivity = 1;
int settings_gamepad_menuy_sensitivity = 1;

Uint32 colorWhite = 0xFFFFFFFF;

int firstendmoviealpha[30];
int secondendmoviealpha[30];
int thirdendmoviealpha[30];
int fourthendmoviealpha[30];
int intromoviealpha[30];
int rebindkey = -1;
int rebindaction = -1;

Sint32 gearrot = 0;
Sint32 gearsize = 5000;
Uint16 logoalpha = 0;
int credittime = 0;
int creditstage = 0;
int intromovietime = 0;
int intromoviestage = 0;
int firstendmovietime = 0;
int firstendmoviestage = 0;
int secondendmovietime = 0;
int secondendmoviestage = 0;
int thirdendmoviestage = 0;
int thirdendmovietime = 0;
int thirdEndNumLines = 6;
int fourthendmoviestage = 0;
int fourthendmovietime = 0;
int fourthEndNumLines = 13;
real_t drunkextend = 0;
bool losingConnection[4] = { false };
bool subtitleVisible = false;
int subtitleCurrent = 0;

//Confirm resolution window stuff.
bool resolutionChanged = false;
bool confirmResolutionWindow = false;
int resolutionConfirmationTimer = 0;
Sint32 oldXres;
Sint32 oldYres;
Sint32 oldFullscreen;
real_t oldGamma;
button_t* revertResolutionButton = nullptr;

void buttonCloseSettingsSubwindow(button_t* my);

button_t* getSettingsTabButton()
{
	switch ( settings_tab )
	{
		case SETTINGS_VIDEO_TAB:
			return button_video_tab;
		case SETTINGS_AUDIO_TAB:
			return button_audio_tab;
		case SETTINGS_KEYBOARD_TAB:
			return button_keyboard_tab;
		case SETTINGS_MOUSE_TAB:
			return button_mouse_tab;
		case SETTINGS_GAMEPAD_BINDINGS_TAB:
			return button_gamepad_bindings_tab;
		case SETTINGS_GAMEPAD_SETTINGS_TAB:
			return button_gamepad_settings_tab;
		case SETTINGS_MISC_TAB:
			return button_misc_tab;
	}

	return nullptr;
}

void changeSettingsTab(int option)
{
	if ( getSettingsTabButton() )
	{
		getSettingsTabButton()->outline = false;
	}

	settings_tab = option;

	if ( settings_tab < 0 )
	{
		settings_tab = NUM_SETTINGS_TABS - 1;
	}
	if ( settings_tab >= NUM_SETTINGS_TABS )
	{
		settings_tab = 0;
	}

	if ( getSettingsTabButton() )
	{
		button_t* button = getSettingsTabButton();
		button->outline = true;
		int x = button->x + (button->sizex / 2);
		int y = button->y + (button->sizey / 2);
		SDL_WarpMouseInWindow(screen, x, y);
	}
}

void navigateMainMenuItems(bool mode)
{
	int warpx, warpy;
	if (menuselect == 0)
	{
		//No menu item selected.
		if ( keystatus[SDL_SCANCODE_UP] || (*inputPressed(joyimpulses[INJOY_DPAD_UP]) && rebindaction == -1) )
		{
			keystatus[SDL_SCANCODE_UP] = 0;
			if ( rebindaction == -1 )
			{
				*inputPressed(joyimpulses[INJOY_DPAD_UP]) = 0;
			}
			draw_cursor = false;
			menuselect = 1;
			//Warp cursor to menu item, for gamepad convenience.
			warpx = 50 + 18;
			warpy = (yres / 4) + 80 + (18 / 2); //I am a wizard. I hate magic numbers.
			SDL_WarpMouseInWindow(screen, warpx, warpy);
		}
		else if ( keystatus[SDL_SCANCODE_DOWN] || (*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) && rebindaction == -1) )
		{
			keystatus[SDL_SCANCODE_DOWN] = 0;
			if ( rebindaction == -1 )
			{
				*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) = 0;
			}
			draw_cursor = false;
			menuselect = 1;
			warpx = 50 + 18;
			warpy = (yres / 4) + 80 + (18 / 2);
			SDL_WarpMouseInWindow(screen, warpx, warpy);
		}
	}
	else
	{
		if ( keystatus[SDL_SCANCODE_UP] || (*inputPressed(joyimpulses[INJOY_DPAD_UP]) && rebindaction == -1) )
		{
			keystatus[SDL_SCANCODE_UP] = 0;
			if ( rebindaction == -1 )
			{
				*inputPressed(joyimpulses[INJOY_DPAD_UP]) = 0;
			}
			draw_cursor = false;
			menuselect--;
			if (menuselect == 0)
			{
				if (mode)
				{
#ifdef STEAMWORKS
					menuselect = 8;
#else
					menuselect = 7;
#endif
				}
				else
				{
					menuselect = 4 + (multiplayer != CLIENT);
				}
			}

			warpx = 50 + 18;
			warpy = (((yres / 4) + 80 + (18 / 2)) + ((menuselect - 1) * 24));
			SDL_WarpMouseInWindow(screen, warpx, warpy);
		}
		else if (keystatus[SDL_SCANCODE_DOWN] || (*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) && rebindaction == -1) )
		{
			keystatus[SDL_SCANCODE_DOWN] = 0;
			if ( rebindaction == -1 )
			{
				*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) = 0;
			}
			draw_cursor = false;
			menuselect++;
			if (mode)
			{
#ifdef STEAMWORKS
				if (menuselect > 8 )
				{
					menuselect = 1;
				}
#else
				if ( menuselect > 7 )
				{
					menuselect = 1;
				}
#endif // STEAMWORKS

			}
			else
			{
				if (menuselect > 4 + ( multiplayer != CLIENT))
				{
					menuselect = 1;
				}
			}

			warpx = 50 + 18;
			warpy = (((yres / 4) + 80 + (18 / 2)) + ((menuselect - 1) * 24));
			SDL_WarpMouseInWindow(screen, warpx, warpy);
		}
	}
}

void inline printJoybindingNames(const SDL_Rect& currentPos, int c, bool &rebindingaction)
{
	ttfPrintText(ttf8, currentPos.x, currentPos.y, language[1948 + c]);
	if ( mousestatus[SDL_BUTTON_LEFT] && !rebindingaction )
	{
		if ( omousex >= currentPos.x && omousex < subx2 - 24 )
		{
			if ( omousey >= currentPos.y && omousey < currentPos.y + 12 )
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				if ( settings_joyimpulses[c] != UNBOUND_JOYBINDING )
				{
					settings_joyimpulses[c] = UNBOUND_JOYBINDING; //Unbind the joybinding if clicked on.
				}
				else
				{
					lastkeypressed = 0;
					rebindingaction = true;
					rebindaction = c;
				}
			}
		}
	}

	if ( c != rebindaction )
	{
		if ( !strcmp(getInputName(settings_joyimpulses[c]), "Unassigned key" ))
		{
			ttfPrintTextColor(ttf8, currentPos.x + 232, currentPos.y, uint32ColorBaronyBlue(*mainsurface), true, getInputName(settings_joyimpulses[c]));
		}
		else if ( !strcmp(getInputName(settings_joyimpulses[c]), "Unknown key") || !strcmp(getInputName(settings_joyimpulses[c]), "Unknown trigger") )
		{
			ttfPrintTextColor(ttf8, currentPos.x + 232, currentPos.y, uint32ColorRed(*mainsurface), true, getInputName(settings_joyimpulses[c]));
		}
		else
		{
			ttfPrintText(ttf8, currentPos.x + 232, currentPos.y, getInputName(settings_joyimpulses[c]));
		}
	}
	else
	{
		ttfPrintTextColor(ttf8, currentPos.x + 232, currentPos.y, uint32ColorGreen(*mainsurface), true, "...");
	}
}

/*-------------------------------------------------------------------------------

	handleMainMenu

	draws & processes the game menu; if passed true, does the whole menu,
	otherwise just handles the reduced ingame menu

-------------------------------------------------------------------------------*/

void handleMainMenu(bool mode)
{
	SDL_Rect pos, src, dest;
	int x, c;
	//int y;
	bool b;
	//int tilesreceived=0;
	//Mix_Music **music, *intromusic, *splashmusic, *creditsmusic;
	node_t* node, *nextnode;
	Entity* entity;
	FILE* fp;
	//SDL_Surface *sky_bmp;
	button_t* button;

	if ( !movie )
	{
		// title pic
		src.x = 0;
		src.y = 0;
		src.w = title_bmp->w;
		src.h = title_bmp->h;
		dest.x = 20;
		dest.y = 20;
		dest.w = xres;
		dest.h = yres;
		if ( mode || introstage != 5 )
		{
			drawImage(title_bmp, &src, &dest);
		}
		if ( mode && subtitleVisible )
		{
			Uint32 colorYellow = SDL_MapRGBA(mainsurface->format, 255, 255, 0, 255);
			ttfPrintTextColor(ttf16, 176, 20 + title_bmp->h - 24, colorYellow, true, language[1910 + subtitleCurrent]);
		}

		// gray text color
		Uint32 colorGray = SDL_MapRGBA(mainsurface->format, 128, 128, 128, 255);

		// print game version
		if ( mode || introstage != 5 )
		{
			char version[64];
			strcpy(version, __DATE__ + 7);
			strcat(version, ".");
			if ( !strncmp(__DATE__, "Jan", 3) )
			{
				strcat(version, "01");
			}
			else if ( !strncmp(__DATE__, "Feb", 3) )
			{
				strcat(version, "02");
			}
			else if ( !strncmp(__DATE__, "Mar", 3) )
			{
				strcat(version, "03");
			}
			else if ( !strncmp(__DATE__, "Apr", 3) )
			{
				strcat(version, "04");
			}
			else if ( !strncmp(__DATE__, "May", 3) )
			{
				strcat(version, "05");
			}
			else if ( !strncmp(__DATE__, "Jun", 3) )
			{
				strcat(version, "06");
			}
			else if ( !strncmp(__DATE__, "Jul", 3) )
			{
				strcat(version, "07");
			}
			else if ( !strncmp(__DATE__, "Aug", 3) )
			{
				strcat(version, "08");
			}
			else if ( !strncmp(__DATE__, "Sep", 3) )
			{
				strcat(version, "09");
			}
			else if ( !strncmp(__DATE__, "Oct", 3) )
			{
				strcat(version, "10");
			}
			else if ( !strncmp(__DATE__, "Nov", 3) )
			{
				strcat(version, "11");
			}
			else if ( !strncmp(__DATE__, "Dec", 3) )
			{
				strcat(version, "12");
			}
			strcat(version, ".");
			int day = atoi(__DATE__ + 4);
			if (day >= 10)
			{
				strncat(version, __DATE__ + 4, 2);
			}
			else
			{
				strcat(version, "0");
				strncat(version, __DATE__ + 5, 1);
			}
			int w, h;
			TTF_SizeUTF8(ttf8, version, &w, &h);
			ttfPrintTextFormatted(ttf8, xres - 8 - w, yres - 4 - h, "%s", version);
			int h2 = h;
			TTF_SizeUTF8(ttf8, VERSION, &w, &h);
			ttfPrintTextFormatted(ttf8, xres - 8 - w, yres - 8 - h - h2, VERSION);
			if ( gamemods_numCurrentModsLoaded >= 0 )
			{
				ttfPrintTextFormatted(ttf8, xres - 8 - TTF8_WIDTH * 16, yres - 12 - h - h2 * 2, "%2d mod(s) loaded", gamemods_numCurrentModsLoaded);
			}
#ifdef STEAMWORKS
			if ( gamemods_disableSteamAchievements || (intro == false && conductGameChallenges[CONDUCT_CHEATS_ENABLED]) )
			{
				TTF_SizeUTF8(ttf8, language[3003], &w, &h);
				if ( gamemods_numCurrentModsLoaded < 0 )
				{
					h = -4;
				}
				ttfPrintTextFormatted(ttf8, xres - 8 - w, yres - 16 - h - h2 * 3, language[3003]);
			}
#endif // STEAMWORKS

#ifdef STEAMWORKS
			TTF_SizeUTF8(ttf8, language[2549], &w, &h);
			if ( (omousex >= xres - 8 - w && omousex < xres && omousey >= 8 && omousey < 8 + h)
				&& subwindow == 0
				&& introstage == 1
				&& SteamUser()->BLoggedOn() )
			{
				if ( mousestatus[SDL_BUTTON_LEFT] )
				{
					mousestatus[SDL_BUTTON_LEFT] = 0;
					playSound(139, 64);
					SteamAPICall_NumPlayersOnline = SteamUserStats()->GetNumberOfCurrentPlayers();
				}
				ttfPrintTextFormattedColor(ttf8, xres - 8 - w, 8, colorGray, language[2549], steamOnlinePlayers);
			}
			else if ( SteamUser()->BLoggedOn() )
			{
				ttfPrintTextFormatted(ttf8, xres - 8 - w, 8, language[2549], steamOnlinePlayers);
			}
			if ( intro == false )
			{
				if ( conductGameChallenges[CONDUCT_CHEATS_ENABLED] )
				{
					TTF_SizeUTF8(ttf8, language[2986], &w, &h);
					ttfPrintTextFormatted(ttf8, xres - 8 - w, 8 + h, language[2986]);
				}
			}
			if ( SteamUser()->BLoggedOn() && SteamAPICall_NumPlayersOnline == 0 && ticks % 250 == 0 )
			{
				SteamAPICall_NumPlayersOnline = SteamUserStats()->GetNumberOfCurrentPlayers();
			}
			bool bFailed = false;
			if ( SteamUser()->BLoggedOn() )
			{
				SteamUtils()->GetAPICallResult(SteamAPICall_NumPlayersOnline, &NumberOfCurrentPlayers, sizeof(NumberOfCurrentPlayers_t), 1107, &bFailed);
				if ( NumberOfCurrentPlayers.m_bSuccess )
				{
					steamOnlinePlayers = NumberOfCurrentPlayers.m_cPlayers;
				}
			}
#endif // STEAMWORKS
		}
		// navigate with arrow keys
		if (!subwindow)
		{
			navigateMainMenuItems(mode);
		}

		// draw menu
		if ( mode )
		{
			/*
			 * Mouse menu item select/highlight implicitly handled here.
			 */

			/*
			bool mouseover = false;
			if ( ((omousex >= 50 && omousex < 50 + strlen(language[1303]) * 18 && omousey >= yres / 4 + 80 && omousey < yres / 4 + 80 + 18)) ) {
				//Mouse hovering over a menu item.
				mouseover = true;
				menuselect = 1;
			}

			if ( (mouseover || (menuselect == 1)) && subwindow == 0 && introstage == 1 ) {
				ttfPrintTextFormattedColor(ttf16, 50, yres/4+80, colorGray, language[1303]);
				//...etc
			 */
			if ( keystatus[SDL_SCANCODE_L] && (keystatus[SDL_SCANCODE_LCTRL] || keystatus[SDL_SCANCODE_RCTRL]) )
			{
				buttonOpenCharacterCreationWindow(nullptr);
				keystatus[SDL_SCANCODE_L] = 0;
				keystatus[SDL_SCANCODE_LCTRL] = 0;
				keystatus[SDL_SCANCODE_RCTRL] = 0;
				multiplayerselect = SERVER;
				charcreation_step = 6;
				camera_charsheet_offsetyaw = (330) * PI / 180;
				directConnect = true;
				strcpy(portnumber_char, "12345");
				buttonHostLobby(nullptr);
			}

			if ( keystatus[SDL_SCANCODE_M] && (keystatus[SDL_SCANCODE_LCTRL] || keystatus[SDL_SCANCODE_RCTRL]) )
			{
				buttonOpenCharacterCreationWindow(nullptr);

				keystatus[SDL_SCANCODE_M] = 0;
				keystatus[SDL_SCANCODE_LCTRL] = 0;
				keystatus[SDL_SCANCODE_RCTRL] = 0;
				multiplayerselect = CLIENT;
				charcreation_step = 6;
				camera_charsheet_offsetyaw = (330) * PI / 180;
				directConnect = true;
				strcpy(connectaddress, "localhost:12345");
				buttonJoinLobby(nullptr);
			}

			//"Start Game" button.
			if ( ((omousex >= 50 && omousex < 50 + strlen(language[1303]) * 18 && omousey >= yres / 4 + 80 && omousey < yres / 4 + 80 + 18) || (menuselect == 1)) && subwindow == 0 && introstage == 1 )
			{
				menuselect = 1;
				ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 80, colorGray, language[1303]);
				if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
				{
					mousestatus[SDL_BUTTON_LEFT] = 0;
					keystatus[SDL_SCANCODE_RETURN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
					}
					playSound(139, 64);

					// look for a save game
					bool reloadModels = false;
					int modelsIndexUpdateStart = 1;
					int modelsIndexUpdateEnd = nummodels;
					if ( physfsSearchModelsToUpdate() || !gamemods_modelsListModifiedIndexes.empty() )
					{
						reloadModels = true; // we had some models already loaded which should be reset
					}
					bool reloadSounds = false;
					if ( physfsSearchSoundsToUpdate() )
					{
						reloadSounds = true; // we had some sounds already loaded which should be reset
					}

					gamemodsClearAllMountedPaths();

					if ( reloadModels )
					{
						// print a loading message
						drawClearBuffers();
						int w, h;
						TTF_SizeUTF8(ttf16, language[2990], &w, &h);
						ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[2990]);
						GO_SwapBuffers(screen);

						physfsModelIndexUpdate(modelsIndexUpdateStart, modelsIndexUpdateEnd, true);
						generatePolyModels(modelsIndexUpdateStart, modelsIndexUpdateEnd, false);
						gamemods_modelsListLastStartedUnmodded = true;
					}
					if ( reloadSounds )
					{
						// print a loading message
						drawClearBuffers();
						int w, h;
						TTF_SizeUTF8(ttf16, language[2988], &w, &h);
						ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[2988]);
						GO_SwapBuffers(screen);
						physfsReloadSounds(true);
						gamemods_soundsListLastStartedUnmodded = true;
					}

					if ( gamemods_tileListRequireReloadUnmodded )
					{
						drawClearBuffers();
						int w, h;
						TTF_SizeUTF8(ttf16, language[3004], &w, &h);
						ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[3004]);
						GO_SwapBuffers(screen);
						physfsReloadTiles(true);
						gamemods_tileListRequireReloadUnmodded = false;
					}

					if ( gamemods_booksRequireReloadUnmodded )
					{
						drawClearBuffers();
						int w, h;
						TTF_SizeUTF8(ttf16, language[2992], &w, &h);
						ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[2992]);
						GO_SwapBuffers(screen);
						physfsReloadBooks();
						gamemods_booksRequireReloadUnmodded = false;
					}

					if ( gamemods_musicRequireReloadUnmodded )
					{
						drawClearBuffers();
						int w, h;
						TTF_SizeUTF8(ttf16, language[2994], &w, &h);
						ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[2994]);
						GO_SwapBuffers(screen);
						physfsSearchMusicToUpdate();
						gamemods_musicRequireReloadUnmodded = false;
					}

					gamemods_disableSteamAchievements = false;

					if ( saveGameExists(true) || saveGameExists(false) )
					{
						openLoadGameWindow(NULL);
					}
					else
					{
						buttonOpenCharacterCreationWindow(NULL);
					}
				}
			}
			else
			{
				ttfPrintText(ttf16, 50, yres / 4 + 80, language[1303]);
			}
			//"Introduction" button.
			if ( ((omousex >= 50 && omousex < 50 + strlen(language[1304]) * 18 && omousey >= yres / 4 + 104 && omousey < yres / 4 + 104 + 18) || (menuselect == 2)) && subwindow == 0 && introstage == 1 )
			{
				menuselect = 2;
				ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 104, colorGray, language[1304]);
				if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
				{
					mousestatus[SDL_BUTTON_LEFT] = 0;
					keystatus[SDL_SCANCODE_RETURN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
					}
					playSound(139, 64);
					introstage = 6; // goes to intro movie
					fadeout = true;
#ifdef MUSIC
					playmusic(introductionmusic, true, true, false);
#endif
				}
			}
			else
			{
				ttfPrintText(ttf16, 50, yres / 4 + 104, language[1304]);
			}
			//"Statistics" Button.
			if ( ((omousex >= 50 && omousex < 50 + strlen(language[1305]) * 18 && omousey >= yres / 4 + 128 && omousey < yres / 4 + 128 + 18) || (menuselect == 3)) && subwindow == 0 && introstage == 1 )
			{
				menuselect = 3;
				ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 128, colorGray, language[1305]);
				if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
				{
					mousestatus[SDL_BUTTON_LEFT] = 0;
					keystatus[SDL_SCANCODE_RETURN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
					}
					playSound(139, 64);

					buttonOpenScoresWindow(nullptr);
				}
			}
			else
			{
				ttfPrintText(ttf16, 50, yres / 4 + 128, language[1305]);
			}
			//"Settings" button.
			if ( ((omousex >= 50 && omousex < 50 + strlen(language[1306]) * 18 && omousey >= yres / 4 + 152 && omousey < yres / 4 + 152 + 18) || (menuselect == 4)) && subwindow == 0 && introstage == 1 )
			{
				menuselect = 4;
				ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 152, colorGray, language[1306]);
				if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
				{
					mousestatus[SDL_BUTTON_LEFT] = 0;
					keystatus[SDL_SCANCODE_RETURN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
					}
					playSound(139, 64);
					openSettingsWindow();
				}
			}
			else
			{
				ttfPrintText(ttf16, 50, yres / 4 + 152, language[1306]);
			}
			//"Credits" button
			if ( ((omousex >= 50 && omousex < 50 + strlen(language[1307]) * 18 && omousey >= yres / 4 + 176 && omousey < yres / 4 + 176 + 18) || (menuselect == 5)) && subwindow == 0 && introstage == 1 )
			{
				menuselect = 5;
				ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 176, colorGray, language[1307]);
				if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
				{
					mousestatus[SDL_BUTTON_LEFT] = 0;
					keystatus[SDL_SCANCODE_RETURN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
					}
					playSound(139, 64);
					introstage = 4; // goes to credits
					fadeout = true;
				}
			}
			else
			{
				ttfPrintText(ttf16, 50, yres / 4 + 176, language[1307]);
			}
			//"Custom content" button.
			int customContent_pady = 0;
			int customContentMenuSelectOffset = 0;
			if ( ((omousex >= 50 && omousex < 50 + strlen(language[2978]) * 18 && omousey >= yres / 4 + 200 && omousey < yres / 4 + 200 + 18) || (menuselect == 6)) && subwindow == 0 && introstage == 1 )
			{
				menuselect = 6;
				ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 200, colorGray, language[2978]);
				if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
				{
					mousestatus[SDL_BUTTON_LEFT] = 0;
					keystatus[SDL_SCANCODE_RETURN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
					}
					playSound(139, 64);
					gamemodsCustomContentInit();
				}
			}
			else
			{
				ttfPrintText(ttf16, 50, yres / 4 + 200, language[2978]);
			}
#ifdef STEAMWORKS
			if ( ((omousex >= 50 && omousex < 50 + strlen(language[2979]) * 18 && omousey >= yres / 4 + 224 && omousey < yres / 4 + 224 + 18) || (menuselect == 7)) && subwindow == 0 && introstage == 1 )
			{
				menuselect = 7;
				ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 224, colorGray, language[2979]);
				if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
				{
					mousestatus[SDL_BUTTON_LEFT] = 0;
					keystatus[SDL_SCANCODE_RETURN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
					}
					playSound(139, 64);
					gamemodsSubscribedItemsInit();
				}
			}
			else
			{
				ttfPrintText(ttf16, 50, yres / 4 + 224, language[2979]);
			}
			customContent_pady = 24;
			customContentMenuSelectOffset = 1;
#endif
			//"Quit" button.
			if ( ((omousex >= 50 && omousex < 50 + strlen(language[1308]) * 18 && omousey >= yres / 4 + 224 + customContent_pady && omousey < yres / 4 + 224 + customContent_pady + 18) || (menuselect == 7 + customContentMenuSelectOffset)) && subwindow == 0 && introstage == 1 )
			{
				menuselect = 7 + customContentMenuSelectOffset;
				ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 224 + customContent_pady, colorGray, language[1308]);
				if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
				{
					mousestatus[SDL_BUTTON_LEFT] = 0;
					keystatus[SDL_SCANCODE_RETURN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
					}
					playSound(139, 64);

					// create confirmation window
					subwindow = 1;
					subx1 = xres / 2 - 128;
					subx2 = xres / 2 + 128;
					suby1 = yres / 2 - 40;
					suby2 = yres / 2 + 40;
					strcpy(subtext, language[1128]);

					// close button
					button = newButton();
					strcpy(button->label, "x");
					button->x = subx2 - 20;
					button->y = suby1;
					button->sizex = 20;
					button->sizey = 20;
					button->action = &buttonCloseSubwindow;
					button->visible = 1;
					button->focused = 1;
					button->key = SDL_SCANCODE_ESCAPE;
					button->joykey = joyimpulses[INJOY_MENU_CANCEL];

					// yes button
					button = newButton();
					strcpy(button->label, language[1314]);
					button->x = subx1 + 8;
					button->y = suby2 - 28;
					button->sizex = strlen(language[1314]) * 12 + 8;
					button->sizey = 20;
					button->action = &buttonQuitConfirm;
					button->visible = 1;
					button->focused = 1;
					button->key = SDL_SCANCODE_RETURN;
					button->joykey = joyimpulses[INJOY_MENU_NEXT];

					// no button
					button = newButton();
					strcpy(button->label, language[1315]);
					button->x = subx2 - strlen(language[1315]) * 12 - 16;
					button->y = suby2 - 28;
					button->sizex = strlen(language[1315]) * 12 + 8;
					button->sizey = 20;
					button->action = &buttonCloseSubwindow;
					button->visible = 1;
					button->focused = 1;
				}
			}
			else
			{
				ttfPrintText(ttf16, 50, yres / 4 + 224 + customContent_pady, language[1308]);
			}
		}
		else
		{
			if ( introstage != 5 )
			{
				if ( ((omousex >= 50 && omousex < 50 + strlen(language[1309]) * 18 && omousey >= yres / 4 + 80 && omousey < yres / 4 + 80 + 18) || (menuselect == 1)) && subwindow == 0 && introstage == 1 )
				{
					menuselect = 1;
					ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 80, colorGray, language[1309]);
					if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
					{
						if ( rebindaction == -1 )
						{
							*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
						}
						mousestatus[SDL_BUTTON_LEFT] = 0;
						keystatus[SDL_SCANCODE_RETURN] = 0;
						playSound(139, 64);
						pauseGame(1, MAXPLAYERS);
					}
				}
				else
				{
					ttfPrintText(ttf16, 50, yres / 4 + 80, language[1309]);
				}
				if ( ((omousex >= 50 && omousex < 50 + strlen(language[1306]) * 18 && omousey >= yres / 4 + 104 && omousey < yres / 4 + 104 + 18) || (menuselect == 2)) && subwindow == 0 && introstage == 1 )
				{
					menuselect = 2;
					ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 104, colorGray, language[1306]);
					if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						keystatus[SDL_SCANCODE_RETURN] = 0;
						if ( rebindaction == -1 )
						{
							*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
						}
						playSound(139, 64);
						openSettingsWindow();
					}
				}
				else
				{
					ttfPrintText(ttf16, 50, yres / 4 + 104, language[1306]);
				}
				char* endgameText = NULL;
				if ( multiplayer == SINGLE )
				{
					endgameText = language[1310];
				}
				else
				{
					endgameText = language[1311];
				}
				if ( ((omousex >= 50 && omousex < 50 + strlen(endgameText) * 18 && omousey >= yres / 4 + 128 && omousey < yres / 4 + 128 + 18) || (menuselect == 3)) && subwindow == 0 && introstage == 1 )
				{
					menuselect = 3;
					ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 128, colorGray, endgameText);
					if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						keystatus[SDL_SCANCODE_RETURN] = 0;
						if ( rebindaction == -1 )
						{
							*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
						}
						playSound(139, 64);

						// create confirmation window
						subwindow = 1;
						subx1 = xres / 2 - 140;
						subx2 = xres / 2 + 140;
						suby1 = yres / 2 - 48;
						suby2 = yres / 2 + 48;
						strcpy(subtext, language[1129]);

						// close button
						button = newButton();
						strcpy(button->label, "x");
						button->x = subx2 - 20;
						button->y = suby1;
						button->sizex = 20;
						button->sizey = 20;
						button->action = &buttonCloseSubwindow;
						button->visible = 1;
						button->focused = 1;
						button->key = SDL_SCANCODE_ESCAPE;
						button->joykey = joyimpulses[INJOY_MENU_CANCEL];

						// yes button
						button = newButton();
						strcpy(button->label, language[1314]);
						button->x = subx1 + 8;
						button->y = suby2 - 28;
						button->sizex = strlen(language[1314]) * 12 + 8;
						button->sizey = 20;
						button->action = &buttonEndGameConfirm;
						button->visible = 1;
						button->focused = 1;
						button->key = SDL_SCANCODE_RETURN;
						button->joykey = joyimpulses[INJOY_MENU_NEXT];

						// no button
						button = newButton();
						strcpy(button->label, language[1315]);
						button->x = subx2 - strlen(language[1315]) * 12 - 16;
						button->y = suby2 - 28;
						button->sizex = strlen(language[1315]) * 12 + 8;
						button->sizey = 20;
						button->action = &buttonCloseSubwindow;
						button->visible = 1;
						button->focused = 1;
					}
				}
				else
				{
					ttfPrintText(ttf16, 50, yres / 4 + 128, endgameText);
				}
				if ( multiplayer != CLIENT )
				{
					if ( ((omousex >= 50 && omousex < 50 + strlen(language[1312]) * 18 && omousey >= yres / 4 + 152 && omousey < yres / 4 + 152 + 18) || (menuselect == 4)) && subwindow == 0 && introstage == 1 )
					{
						menuselect = 4;
						ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 152, colorGray, language[1312]);
						if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
						{
							mousestatus[SDL_BUTTON_LEFT] = 0;
							keystatus[SDL_SCANCODE_RETURN] = 0;
							if ( rebindaction == -1 )
							{
								*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
							}
							playSound(139, 64);

							// create confirmation window
							subwindow = 1;
							subx1 = xres / 2 - 164;
							subx2 = xres / 2 + 164;
							suby1 = yres / 2 - 48;
							suby2 = yres / 2 + 48;
							strcpy(subtext, language[1130]);

							// close button
							button = newButton();
							strcpy(button->label, "x");
							button->x = subx2 - 20;
							button->y = suby1;
							button->sizex = 20;
							button->sizey = 20;
							button->action = &buttonCloseSubwindow;
							button->visible = 1;
							button->focused = 1;
							button->key = SDL_SCANCODE_ESCAPE;
							button->joykey = joyimpulses[INJOY_MENU_CANCEL];

							// yes button
							button = newButton();
							strcpy(button->label, language[1314]);
							button->x = subx1 + 8;
							button->y = suby2 - 28;
							button->sizex = strlen(language[1314]) * 12 + 8;
							button->sizey = 20;
							if ( multiplayer == SINGLE )
							{
								button->action = &buttonStartSingleplayer;
							}
							else
							{
								button->action = &buttonStartServer;
							}
							button->visible = 1;
							button->focused = 1;
							button->key = SDL_SCANCODE_RETURN;
							button->joykey = joyimpulses[INJOY_MENU_NEXT];

							// no button
							button = newButton();
							strcpy(button->label, language[1315]);
							button->x = subx2 - strlen(language[1315]) * 12 - 16;
							button->y = suby2 - 28;
							button->sizex = strlen(language[1315]) * 12 + 8;
							button->sizey = 20;
							button->action = &buttonCloseSubwindow;
							button->visible = 1;
							button->focused = 1;
						}
					}
					else
					{
						ttfPrintText(ttf16, 50, yres / 4 + 152, language[1312]);
					}
				}
				if ( ((omousex >= 50 && omousex < 50 + strlen(language[1313]) * 18 && omousey >= yres / 4 + 152 + 24 * (multiplayer != CLIENT) && omousey < yres / 4 + 152 + 18 + 24 * (multiplayer != CLIENT)) || (menuselect == 4 + (multiplayer != CLIENT))) && subwindow == 0 && introstage == 1 )
				{
					menuselect = 4 + (multiplayer != CLIENT);
					ttfPrintTextFormattedColor(ttf16, 50, yres / 4 + 152 + 24 * (multiplayer != CLIENT), colorGray, language[1313]);
					if ( mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_RETURN] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						keystatus[SDL_SCANCODE_RETURN] = 0;
						if ( rebindaction == -1 )
						{
							*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
						}
						playSound(139, 64);

						// create confirmation window
						subwindow = 1;
						subx1 = xres / 2 - 188;
						subx2 = xres / 2 + 188;
						suby1 = yres / 2 - 64;
						suby2 = yres / 2 + 64;
						strcpy(subtext, language[1131]);

						// yes button
						button = newButton();
						strcpy(button->label, language[1314]);
						button->x = subx1 + 8;
						button->y = suby2 - 28;
						button->sizex = strlen(language[1314]) * 12 + 8;
						button->sizey = 20;
						button->action = &buttonQuitConfirm;
						button->visible = 1;
						button->focused = 1;
						button->key = SDL_SCANCODE_RETURN;
						button->joykey = joyimpulses[INJOY_MENU_NEXT]; //TODO: Select which button to activate via dpad.

						// no button
						button = newButton();
						strcpy(button->label, language[1315]);
						button->sizex = strlen(language[1315]) * 12 + 8;
						button->sizey = 20;
						button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
						button->y = suby2 - 28;
						button->action = &buttonQuitNoSaveConfirm;
						button->visible = 1;
						button->focused = 1;

						// cancel button
						button = newButton();
						strcpy(button->label, language[1316]);
						button->x = subx2 - strlen(language[1316]) * 12 - 16;
						button->y = suby2 - 28;
						button->sizex = strlen(language[1316]) * 12 + 8;
						button->sizey = 20;
						button->action = &buttonCloseSubwindow;
						button->visible = 1;
						button->focused = 1;

						// close button
						button = newButton();
						strcpy(button->label, "x");
						button->x = subx2 - 20;
						button->y = suby1;
						button->sizex = 20;
						button->sizey = 20;
						button->action = &buttonCloseSubwindow;
						button->visible = 1;
						button->focused = 1;
						button->key = SDL_SCANCODE_ESCAPE;
						button->joykey = joyimpulses[INJOY_MENU_CANCEL];
					}
				}
				else
				{
					ttfPrintText(ttf16, 50, yres / 4 + 152 + 24 * (multiplayer != CLIENT), language[1313]);
				}
			}
		}

#ifdef STEAMWORKS
		if ( intro )
		{
			// lobby list request succeeded
			if ( !requestingLobbies && !strcmp(subtext, language[1132]) )
			{
				openSteamLobbyBrowserWindow(NULL);
			}

			// lobby entered
			if ( !connectingToLobby && connectingToLobbyWindow )
			{
				connectingToLobbyWindow = false;
				connectingToLobby = false;

				// close current window
				buttonCloseSubwindow(NULL);
				list_FreeAll(&button_l);
				deleteallbuttons = true;

				// we are assuming here that the lobby join was successful
				// otherwise, the callback would've flipped off the connectingToLobbyWindow and opened an error window

				// get number of lobby members (capped to game limit)

				// record CSteamID of lobby owner (and nobody else)
				int lobbyMembers = SteamMatchmaking()->GetNumLobbyMembers(*static_cast<CSteamID*>(currentLobby));
				if ( steamIDRemote[0] )
				{
					cpp_Free_CSteamID(steamIDRemote[0]);
				}
				steamIDRemote[0] = cpp_SteamMatchmaking_GetLobbyOwner(currentLobby); //TODO: Bugger void pointers!
				int c;
				for ( c = 1; c < MAXPLAYERS; c++ )
				{
					if ( steamIDRemote[c] )
					{
						cpp_Free_CSteamID(steamIDRemote[c]);
						steamIDRemote[c] = NULL;
					}
				}
				for ( c = 1; c < lobbyMembers; ++c )
				{
					steamIDRemote[c] = cpp_SteamMatchmaking_GetLobbyMember(currentLobby, c);
				}
				buttonJoinLobby(NULL);
			}
		}
#endif

		//Confirm Resolution Change Window
		if ( confirmResolutionWindow )
		{
			subx1 = xres / 2 - 128;
			subx2 = xres / 2 + 128;
			suby1 = yres / 2 - 40;
			suby2 = yres / 2 + 40;
			drawWindowFancy(subx1, suby1, subx2, suby2);

			if ( SDL_GetTicks() >= resolutionConfirmationTimer + RESOLUTION_CONFIRMATION_TIME )
			{
				//Automatically revert.
				buttonRevertResolution(revertResolutionButton);
			}
		}

		// draw subwindow
		if ( subwindow )
		{
			drawWindowFancy(subx1, suby1, subx2, suby2);
			if ( loadGameSaveShowRectangle > 0 )
			{
				SDL_Rect saveBox;
				saveBox.x = subx1 + 4;
				saveBox.y = suby1 + TTF12_HEIGHT * 2;
				saveBox.w = subx2 - subx1 - 8;
				saveBox.h = TTF12_HEIGHT * 3;
				drawWindowFancy(saveBox.x, saveBox.y, saveBox.x + saveBox.w, saveBox.y + saveBox.h);
				if ( gamemods_numCurrentModsLoaded >= 0 )
				{
					drawRect(&saveBox, uint32ColorGreen(*mainsurface), 32);
				}
				else
				{
					drawRect(&saveBox, uint32ColorBaronyBlue(*mainsurface), 32);
				}
				if ( loadGameSaveShowRectangle == 2 )
				{
					saveBox.y = suby1 + TTF12_HEIGHT * 5 + 2;
					//drawTooltip(&saveBox);
					drawWindowFancy(saveBox.x, saveBox.y, saveBox.x + saveBox.w, saveBox.y + saveBox.h);
					if ( gamemods_numCurrentModsLoaded >= 0 )
					{
						drawRect(&saveBox, uint32ColorGreen(*mainsurface), 32);
					}
					else
					{
						drawRect(&saveBox, uint32ColorBaronyBlue(*mainsurface), 32);
					}
				}
			}
			if ( gamemods_window == 1 || gamemods_window == 2 || gamemods_window == 5 )
			{
				drawWindowFancy(subx1 + 4, suby1 + 44 + 10 * TTF12_HEIGHT,
					subx2 - 4, suby2 - 4);
			}
			if ( subtext != NULL )
			{
				if ( strncmp(subtext, language[740], 12) )
				{
					ttfPrintTextFormatted(ttf12, subx1 + 8, suby1 + 8, subtext);
				}
				else
				{
					ttfPrintTextFormatted(ttf16, subx1 + 8, suby1 + 8, subtext);
				}
			}
			if ( loadGameSaveShowRectangle > 0 && gamemods_numCurrentModsLoaded >= 0 )
			{
				ttfPrintTextFormattedColor(ttf12, subx1 + 8, suby2 - TTF12_HEIGHT * 5, uint32ColorBaronyBlue(*mainsurface), "%s", language[2982]);
			}
		}
		else
		{
			loadGameSaveShowRectangle = 0;
		}

		// process button actions
		handleButtons();
	}

	// character creation screen
	if ( charcreation_step >= 1 && charcreation_step < 6 )
	{
		if ( gamemods_numCurrentModsLoaded >= 0 )
		{
			ttfPrintText(ttf16, subx1 + 8, suby1 + 8, language[2980]);
		}
		else
		{
			ttfPrintText(ttf16, subx1 + 8, suby1 + 8, language[1318]);
		}

		// draw character window
		if (players[clientnum] != nullptr && players[clientnum]->entity != nullptr)
		{
			camera_charsheet.x = players[clientnum]->entity->x / 16.0 + 1.118 * cos(camera_charsheet_offsetyaw); // + 1
			camera_charsheet.y = players[clientnum]->entity->y / 16.0 + 1.118 * sin(camera_charsheet_offsetyaw); // -.5
			camera_charsheet.z = players[clientnum]->entity->z * 2;
			camera_charsheet.ang = atan2(players[clientnum]->entity->y / 16.0 - camera_charsheet.y, players[clientnum]->entity->x / 16.0 - camera_charsheet.x);
			camera_charsheet.vang = PI / 24;
			camera_charsheet.winw = 360;
			camera_charsheet.winy = suby1 + 32;
			camera_charsheet.winh = suby2 - 96 - camera_charsheet.winy;
			camera_charsheet.winx = subx2 - camera_charsheet.winw - 32;
			pos.x = camera_charsheet.winx;
			pos.y = camera_charsheet.winy;
			pos.w = camera_charsheet.winw;
			pos.h = camera_charsheet.winh;
			drawRect(&pos, 0, 255);
			b = players[clientnum]->entity->flags[BRIGHT];
			players[clientnum]->entity->flags[BRIGHT] = true;
			if (!playing_random_char)
			{
				if ( !players[clientnum]->entity->flags[INVISIBLE] )
				{
					real_t ofov = fov;
					fov = 50;
					glDrawVoxel(&camera_charsheet, players[clientnum]->entity, REALCOLORS);
					fov = ofov;
				}
				players[clientnum]->entity->flags[BRIGHT] = b;
				c = 0;
				for ( node = players[clientnum]->entity->children.first; node != NULL; node = node->next )
				{
					if ( c == 0 )
					{
						c++;
					}
					entity = (Entity*) node->element;
					if ( !entity->flags[INVISIBLE] )
					{
						b = entity->flags[BRIGHT];
						entity->flags[BRIGHT] = true;
						real_t ofov = fov;
						fov = 50;
						glDrawVoxel(&camera_charsheet, entity, REALCOLORS);
						fov = ofov;
						entity->flags[BRIGHT] = b;
					}
					c++;
				}
			}
			SDL_Rect rotateBtn;
			rotateBtn.w = 24;
			rotateBtn.h = 24;
			rotateBtn.x = camera_charsheet.winx + camera_charsheet.winw - rotateBtn.w;
			rotateBtn.y = camera_charsheet.winy + camera_charsheet.winh - rotateBtn.h;
			drawWindow(rotateBtn.x, rotateBtn.y, rotateBtn.x + rotateBtn.w, rotateBtn.y + rotateBtn.h);
			if ( mouseInBounds(rotateBtn.x, rotateBtn.x + rotateBtn.w, rotateBtn.y, rotateBtn.y + rotateBtn.h) )
			{
				if ( mousestatus[SDL_BUTTON_LEFT] )
				{
					camera_charsheet_offsetyaw += 0.05;
					if ( camera_charsheet_offsetyaw > 2 * PI )
					{
						camera_charsheet_offsetyaw -= 2 * PI;
					}
					drawDepressed(rotateBtn.x, rotateBtn.y, rotateBtn.x + rotateBtn.w, rotateBtn.y + rotateBtn.h);
				}
			}
			ttfPrintText(ttf12, rotateBtn.x + 4, rotateBtn.y + 6, ">");

			rotateBtn.x = camera_charsheet.winx + camera_charsheet.winw - rotateBtn.w * 2 - 4;
			rotateBtn.y = camera_charsheet.winy + camera_charsheet.winh - rotateBtn.h;
			drawWindow(rotateBtn.x, rotateBtn.y, rotateBtn.x + rotateBtn.w, rotateBtn.y + rotateBtn.h);
			if ( mouseInBounds(rotateBtn.x, rotateBtn.x + rotateBtn.w, rotateBtn.y, rotateBtn.y + rotateBtn.h) )
			{
				if ( mousestatus[SDL_BUTTON_LEFT] )
				{
					camera_charsheet_offsetyaw -= 0.05;
					if ( camera_charsheet_offsetyaw < 0.f )
					{
						camera_charsheet_offsetyaw += 2 * PI;
					}
					drawDepressed(rotateBtn.x, rotateBtn.y, rotateBtn.x + rotateBtn.w, rotateBtn.y + rotateBtn.h);
				}
			}
			ttfPrintText(ttf12, rotateBtn.x + 4, rotateBtn.y + 6, "<");
		}

		// sexes
		if ( charcreation_step == 1 )
		{
			ttfPrintText(ttf16, subx1 + 24, suby1 + 32, language[1319]);
			if ( stats[0]->sex == 0 )
			{
				ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 56, "[o] %s", language[1321]);
				ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 72, "[ ] %s", language[1322]);

				ttfPrintTextFormatted(ttf12, subx1 + 8, suby2 - 80, language[1320], language[1321]);
			}
			else
			{
				ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 56, "[ ] %s", language[1321]);
				ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 72, "[o] %s", language[1322]);

				ttfPrintTextFormatted(ttf12, subx1 + 8, suby2 - 80, language[1320], language[1322]);
			}
			if ( mousestatus[SDL_BUTTON_LEFT] )
			{
				if ( omousex >= subx1 + 40 && omousex < subx1 + 72 )
				{
					if ( omousey >= suby1 + 56 && omousey < suby1 + 72 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						stats[0]->sex = MALE;
					}
					else if ( omousey >= suby1 + 72 && omousey < suby1 + 88 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						stats[0]->sex = FEMALE;
					}
				}
			}
			if ( keystatus[SDL_SCANCODE_UP] || (*inputPressed(joyimpulses[INJOY_DPAD_UP]) && rebindaction == -1) )
			{
				keystatus[SDL_SCANCODE_UP] = 0;
				if ( rebindaction == -1 )
				{
					*inputPressed(joyimpulses[INJOY_DPAD_UP]) = 0;
				}
				draw_cursor = false;
				stats[0]->sex = static_cast<sex_t>((stats[0]->sex == MALE));
			}
			if ( keystatus[SDL_SCANCODE_DOWN] || (*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) && rebindaction == -1) )
			{
				keystatus[SDL_SCANCODE_DOWN] = 0;
				if ( rebindaction == -1 )
				{
					*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) = 0;
				}
				draw_cursor = false;
				stats[0]->sex = static_cast<sex_t>((stats[0]->sex == MALE));
			}
		}

		// classes
		else if ( charcreation_step == 2 )
		{
			ttfPrintText(ttf16, subx1 + 24, suby1 + 32, language[1323]);
			for ( c = 0; c < NUMCLASSES; c++ )
			{
				if ( c == client_classes[0] )
				{
					ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 56 + 16 * c, "[o] %s", playerClassLangEntry(c));
				}
				else
				{
					ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 56 + 16 * c, "[ ] %s", playerClassLangEntry(c));
				}

				if ( mousestatus[SDL_BUTTON_LEFT] )
				{
					if ( omousex >= subx1 + 40 && omousex < subx1 + 72 )
					{
						if ( omousey >= suby1 + 56 + 16 * c && omousey < suby1 + 72 + 16 * c )
						{
							mousestatus[SDL_BUTTON_LEFT] = 0;
							client_classes[0] = c;

							// reset class loadout
							stats[0]->clearStats();
							initClass(0);
						}
					}
				}
				if ( keystatus[SDL_SCANCODE_UP] || (*inputPressed(joyimpulses[INJOY_DPAD_UP]) && rebindaction == -1) )
				{
					keystatus[SDL_SCANCODE_UP] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_DPAD_UP]) = 0;
					}
					draw_cursor = false;
					client_classes[0]--;
					if (client_classes[0] < 0)
					{
						client_classes[0] = NUMCLASSES - 1;
					}

					// reset class loadout
					stats[0]->clearStats();
					initClass(0);
				}
				if ( keystatus[SDL_SCANCODE_DOWN] || (*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) && rebindaction == -1) )
				{
					keystatus[SDL_SCANCODE_DOWN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) = 0;
					}
					draw_cursor = false;
					client_classes[0]++;
					if ( client_classes[0] > NUMCLASSES - 1 )
					{
						client_classes[0] = 0;
					}

					// reset class loadout
					stats[0]->clearStats();
					initClass(0);
				}
			}

			// class description
			ttfPrintText(ttf12, subx1 + 8, suby2 - 80, playerClassDescription(client_classes[0]));
		}

		// faces
		else if ( charcreation_step == 3 )
		{
			ttfPrintText(ttf16, subx1 + 24, suby1 + 32, language[1324]);
			for ( c = 0; c < NUMAPPEARANCES; c++ )
			{
				if ( stats[0]->appearance == c )
				{
					ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 56 + c * 16, "[o] %s", language[20 + c]);
					ttfPrintText(ttf12, subx1 + 8, suby2 - 80, language[38 + c]);
				}
				else
				{
					ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 56 + c * 16, "[ ] %s", language[20 + c]);
				}
				if ( mousestatus[SDL_BUTTON_LEFT] )
				{
					if ( omousex >= subx1 + 40 && omousex < subx1 + 72 )
					{
						if ( omousey >= suby1 + 56 + 16 * c && omousey < suby1 + 72 + 16 * c )
						{
							mousestatus[SDL_BUTTON_LEFT] = 0;
							stats[0]->appearance = c;
						}
					}
				}
				if ( keystatus[SDL_SCANCODE_UP] || (*inputPressed(joyimpulses[INJOY_DPAD_UP]) && rebindaction == -1) )
				{
					keystatus[SDL_SCANCODE_UP] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_DPAD_UP]) = 0;
					}
					draw_cursor = false;
					stats[0]->appearance--;
					if (stats[0]->appearance >= NUMAPPEARANCES)
					{
						stats[0]->appearance = NUMAPPEARANCES - 1;
					}
				}
				if ( keystatus[SDL_SCANCODE_DOWN] || (*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) && rebindaction == -1) )
				{
					keystatus[SDL_SCANCODE_DOWN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) = 0;
					}
					draw_cursor = false;
					stats[0]->appearance++;
					if (stats[0]->appearance >= NUMAPPEARANCES)
					{
						stats[0]->appearance = 0;
					}
				}
			}
		}

		// name
		else if ( charcreation_step == 4 )
		{
			ttfPrintText(ttf16, subx1 + 24, suby1 + 32, language[1325]);
			drawDepressed(subx1 + 40, suby1 + 56, subx1 + 364, suby1 + 88);
			ttfPrintText(ttf16, subx1 + 48, suby1 + 64, stats[0]->name);
			ttfPrintText(ttf12, subx1 + 8, suby2 - 80, language[1326]);

			// enter character name
			if ( !SDL_IsTextInputActive() )
			{
				inputstr = stats[0]->name;
				SDL_StartTextInput();
			}
			//strncpy(stats[0]->name,inputstr,16);
			inputlen = 22;
			if (lastname != "" && strlen(inputstr) == 0)
			{
				strncat(inputstr, lastname.c_str(), std::max<size_t>(0, inputlen - strlen(inputstr)));
				lastname = ""; // Set this to nothing while we're currently editing so it doesn't keep filling it.  We'll save it again if we leave this tab.
			}

			if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
			{
				int x;
				TTF_SizeUTF8(ttf16, stats[0]->name, &x, NULL);
				ttfPrintText(ttf16, subx1 + 48 + x, suby1 + 64, "_");
			}
		}

		// gamemode
		else if ( charcreation_step == 5 )
		{
			ttfPrintText(ttf16, subx1 + 24, suby1 + 32, language[1327]);

			int nummodes = 3;
#ifdef STEAMWORKS
			nummodes += 2;
#endif

			for ( c = 0; c < nummodes; c++ )
			{
				if ( multiplayerselect == c )
				{
					switch ( c )
					{
						case 0:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 56, "[o] %s", language[1328]);
							ttfPrintText(ttf12, subx1 + 8, suby2 - 80, language[1329]);
							break;
						case 1:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 76, "[o] %s", language[1330]);
							ttfPrintText(ttf12, subx1 + 8, suby2 - 80, language[1331]);
							break;
						case 2:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 96, "[o] %s", language[1332]);
							ttfPrintText(ttf12, subx1 + 8, suby2 - 80, language[1333]);
							break;
						case 3:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 136, "[o] %s\n     %s", language[1330], language[1537]);
							ttfPrintText(ttf12, subx1 + 8, suby2 - 80, language[1538]);
							break;
						case 4:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 176, "[o] %s\n     %s", language[1332], language[1537]);
							ttfPrintText(ttf12, subx1 + 8, suby2 - 80, language[1539]);
							break;
					}
				}
				else
				{
					switch ( c )
					{
						case 0:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 56, "[ ] %s", language[1328]);
							break;
						case 1:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 76, "[ ] %s", language[1330]);
							break;
						case 2:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 96, "[ ] %s", language[1332]);
							break;
						case 3:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 136, "[ ] %s\n     %s", language[1330], language[1537]);
							break;
						case 4:
							ttfPrintTextFormatted(ttf16, subx1 + 32, suby1 + 176, "[ ] %s\n     %s", language[1332], language[1537]);
							break;
					}
				}
				if ( multiplayerselect == 0 )
				{
					if ( singleplayerSavegameExists )
					{
						ttfPrintTextColor(ttf12, subx1 + 8, suby2 - 60, uint32ColorOrange(*mainsurface), true, language[2965]);
					}
				}
				else if ( multiplayerselect > 0 )
				{
					if ( multiplayerSavegameExists )
					{
						ttfPrintTextColor(ttf12, subx1 + 8, suby2 - 60, uint32ColorOrange(*mainsurface), true, language[2966]);
					}
					if ( gamemods_numCurrentModsLoaded >= 0 )
					{
						ttfPrintTextColor(ttf12, subx1 + 8, suby2 - 60 - TTF12_HEIGHT * 6, uint32ColorOrange(*mainsurface), true, language[2981]);
					}
				}
				if ( gamemods_numCurrentModsLoaded >= 0 )
				{
					ttfPrintTextColor(ttf12, subx1 + 8, suby2 - 60 + TTF12_HEIGHT, uint32ColorBaronyBlue(*mainsurface), true, language[2982]);
				}
				if ( mousestatus[SDL_BUTTON_LEFT] )
				{
					if ( omousex >= subx1 + 40 && omousex < subx1 + 72 )
					{
						if ( c < 3 )
						{
							if ( omousey >= suby1 + 56 + 20 * c && omousey < suby1 + 74 + 20 * c )
							{
								mousestatus[SDL_BUTTON_LEFT] = 0;
								multiplayerselect = c;
							}
						}
						else
						{
							if ( omousey >= suby1 + 136 + 40 * (c - 3) && omousey < suby1 + 148 + 40 * (c - 3) )
							{
								mousestatus[SDL_BUTTON_LEFT] = 0;
								multiplayerselect = c;
							}
						}
					}
				}
				if (keystatus[SDL_SCANCODE_UP] || (*inputPressed(joyimpulses[INJOY_DPAD_UP]) && rebindaction == -1) )
				{
					keystatus[SDL_SCANCODE_UP] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_DPAD_UP]) = 0;
					}
					draw_cursor = false;
					multiplayerselect--;
					if (multiplayerselect < 0)
					{
						multiplayerselect = nummodes - 1;
					}
				}
				if ( keystatus[SDL_SCANCODE_DOWN] || (*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) && rebindaction == -1) )
				{
					keystatus[SDL_SCANCODE_DOWN] = 0;
					if ( rebindaction == -1 )
					{
						*inputPressed(joyimpulses[INJOY_DPAD_DOWN]) = 0;
					}
					draw_cursor = false;
					multiplayerselect++;
					if (multiplayerselect > nummodes - 1)
					{
						multiplayerselect = 0;
					}
				}
			}
		}
	}

	// steam lobby browser
#ifdef STEAMWORKS
	if ( subwindow && !strcmp(subtext, language[1334]) )
	{
		drawDepressed(subx1 + 8, suby1 + 24, subx2 - 32, suby2 - 64);
		drawDepressed(subx2 - 32, suby1 + 24, subx2 - 8, suby2 - 64);

		// slider
		slidersize = std::min<int>(((suby2 - 65) - (suby1 + 25)), ((suby2 - 65) - (suby1 + 25)) / ((real_t)std::max(numSteamLobbies + 1, 1) / 20));
		slidery = std::min(std::max(suby1 + 25, slidery), suby2 - 65 - slidersize);
		drawWindowFancy(subx2 - 31, slidery, subx2 - 9, slidery + slidersize);

		// directory list offset from slider
		Sint32 y2 = ((real_t)(slidery - suby1 - 20) / ((suby2 - 52) - (suby1 + 20))) * (numSteamLobbies + 1);
		if ( mousestatus[SDL_BUTTON_LEFT] && omousex >= subx2 - 32 && omousex < subx2 - 8 && omousey >= suby1 + 24 && omousey < suby2 - 64 )
		{
			slidery = oslidery + mousey - omousey;
		}
		else if ( mousestatus[SDL_BUTTON_WHEELUP] || mousestatus[SDL_BUTTON_WHEELDOWN] )
		{
			slidery += 16 * mousestatus[SDL_BUTTON_WHEELDOWN] - 16 * mousestatus[SDL_BUTTON_WHEELUP];
			mousestatus[SDL_BUTTON_WHEELUP] = 0;
			mousestatus[SDL_BUTTON_WHEELDOWN] = 0;
		}
		else
		{
			oslidery = slidery;
		}
		slidery = std::min(std::max(suby1 + 25, slidery), suby2 - 65 - slidersize);
		y2 = ((real_t)(slidery - suby1 - 20) / ((suby2 - 52) - (suby1 + 20))) * (numSteamLobbies + 1);

		// server flags tooltip variables
		SDL_Rect flagsBox;
		char flagsBoxText[256];
		int hoveringSelection = -1;

		// select/inspect lobbies
		if ( omousex >= subx1 + 8 && omousex < subx2 - 32 && omousey >= suby1 + 26 && omousey < suby2 - 64 )
		{
			//Something is flawed somewhere in here, because commit 1bad2c5d9f67e0a503ca79f93b03101fbcc7c7ba had to fix the game using an inappropriate hoveringSelection.
			//Perhaps it's as simple as setting hoveringSelection back to -1 if lobbyIDs[hoveringSelection] is in-fact null.
			hoveringSelection = std::min(std::max(0, y2 + ((omousey - suby1 - 24) >> 4)), MAX_STEAM_LOBBIES);

			// lobby info tooltip
			if ( lobbyIDs[hoveringSelection] )
			{
				const char* lobbySvFlagsChar = SteamMatchmaking()->GetLobbyData( *static_cast<CSteamID*>(lobbyIDs[hoveringSelection]), "svFlags" );
				Uint32 lobbySvFlags = atoi(lobbySvFlagsChar);

				int numSvFlags = 0, c;
				for ( c = 0; c < NUM_SERVER_FLAGS; ++c )
				{
					if ( lobbySvFlags & power(2, c) )
					{
						++numSvFlags;
					}
				}

				const char* serverNumModsChar = SteamMatchmaking()->GetLobbyData(*static_cast<CSteamID*>(lobbyIDs[hoveringSelection]), "svNumMods");
				int serverNumModsLoaded = atoi(serverNumModsChar);

				flagsBox.x = mousex + 8;
				flagsBox.y = mousey + 8;
				flagsBox.w = strlen(language[1335]) * 12 + 4;
				flagsBox.h = 4 + (TTF_FontHeight(ttf12) * (std::max(2, numSvFlags + 2)));
				if ( serverNumModsLoaded > 0 )
				{
					flagsBox.h += TTF12_HEIGHT;
					flagsBox.w += 16;
				}
				strcpy(flagsBoxText, language[1335]);
				strcat(flagsBoxText, "\n");

				if ( !numSvFlags )
				{
					strcat(flagsBoxText, language[1336]);
				}
				else
				{
					int y = 2;
					for ( c = 0; c < NUM_SERVER_FLAGS; c++ )
					{
						if ( lobbySvFlags & power(2, c) )
						{
							y += TTF_FontHeight(ttf12);
							strcat(flagsBoxText, "\n");
							char flagStringBuffer[256] = "";
							if ( c < 5 )
							{
								strcpy(flagStringBuffer, language[153 + c]);
							}
							else
							{
								strcpy(flagStringBuffer, language[2917 - 5 + c]);
							}
							strcat(flagsBoxText, flagStringBuffer);
						}
					}
				}
				if ( serverNumModsLoaded > 0 )
				{
					strcat(flagsBoxText, "\n");
					char numModsBuffer[32];
					snprintf(numModsBuffer, 32, "%2d mod(s) loaded", serverNumModsLoaded);
					strcat(flagsBoxText, numModsBuffer);
				}
			}

			// selecting lobby
			if ( mousestatus[SDL_BUTTON_LEFT] )
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				selectedSteamLobby = hoveringSelection;
			}
		}
		selectedSteamLobby = std::min(std::max(y2, selectedSteamLobby), std::min(std::max(numSteamLobbies - 1, 0), y2 + 17));
		pos.x = subx1 + 10;
		pos.y = suby1 + 26 + (selectedSteamLobby - y2) * 16;
		pos.w = subx2 - subx1 - 44;
		pos.h = 16;
		drawRect(&pos, SDL_MapRGB(mainsurface->format, 64, 64, 64), 255);

		// print all lobby entries
		Sint32 x = subx1 + 10;
		Sint32 y = suby1 + 28;
		if ( numSteamLobbies > 0 )
		{
			Sint32 z;
			c = std::min(numSteamLobbies, 18 + y2);
			for (z = y2; z < c; z++)
			{
				ttfPrintTextFormatted(ttf12, x, y, lobbyText[z]); // name
				ttfPrintTextFormatted(ttf12, subx2 - 72, y, "%d/4", lobbyPlayers[z]); // player count
				y += 16;
			}
		}
		else
		{
			ttfPrintText(ttf12, x, y, language[1337]);
		}

		// draw server flags tooltip (if applicable)
		if ( hoveringSelection >= 0 && numSteamLobbies > 0 && hoveringSelection < numSteamLobbies )
		{
			drawTooltip(&flagsBox);
			ttfPrintTextFormatted(ttf12, flagsBox.x + 2, flagsBox.y + 4, flagsBoxText);
		}
	}
#endif

	// settings window
	if ( settings_window == true )
	{
		drawWindowFancy(subx1 + 16, suby1 + 44, subx2 - 16, suby2 - 32);

		int hovering_selection = -1; //0 to NUM_SERVER_FLAGS used for the game flags settings, e.g. are traps enabled, are cheats enabled, is minotaur enabled, etc.
		SDL_Rect tooltip_box;

		if ( *inputPressed(joyimpulses[INJOY_MENU_SETTINGS_NEXT]) && rebindaction == -1 )
		{
			*inputPressed(joyimpulses[INJOY_MENU_SETTINGS_NEXT]) = 0;;
			changeSettingsTab(settings_tab + 1);
		}
		if ( *inputPressed(joyimpulses[INJOY_MENU_SETTINGS_PREV]) && rebindaction == -1 )
		{
			*inputPressed(joyimpulses[INJOY_MENU_SETTINGS_PREV]) = 0;
			changeSettingsTab(settings_tab - 1);
		}

		// video tab
		if ( settings_tab == SETTINGS_VIDEO_TAB )
		{
			// resolution
			ttfPrintText(ttf12, subx1 + 24, suby1 + 60, language[1338]);
			c=0;
			for ( auto cur : resolutions )
			{
				int width, height;
				std::tie (width, height) = cur;
				if ( settings_xres == width && settings_yres == height )
				{
					ttfPrintTextFormatted(ttf12, subx1 + 32, suby1 + 84 + c * 16, "[o] %dx%d", width, height);
				}
				else
				{
					ttfPrintTextFormatted(ttf12, subx1 + 32, suby1 + 84 + c * 16, "[ ] %dx%d", width, height);
				}
				if ( mousestatus[SDL_BUTTON_LEFT] )
				{
					if ( omousex >= subx1 + 38 && omousex < subx1 + 62 )
					{
						if ( omousey >= suby1 + 84 + c * 16 && omousey < suby1 + 96 + c * 16 )
						{
							mousestatus[SDL_BUTTON_LEFT] = 0;
							settings_xres = width;
							settings_yres = height;
							resolutionChanged = true;
						}
					}
				}
				c++;
			}

			// extra options
			ttfPrintText(ttf12, subx1 + 224, suby1 + 60, language[1339]);
			if ( settings_smoothlighting )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 84, "[x] %s", language[1340]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 84, "[ ] %s", language[1340]);
			}
			if ( settings_fullscreen )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 108, "[x] %s", language[1341]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 108, "[ ] %s", language[1341]);
			}
			if ( settings_shaking )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 132, "[x] %s", language[1342]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 132, "[ ] %s", language[1342]);
			}
			if ( settings_bobbing )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 156, "[x] %s", language[1343]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 156, "[ ] %s", language[1343]);
			}
			if ( settings_spawn_blood )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 180, "[x] %s", language[1344]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 180, "[ ] %s", language[1344]);
			}
			if ( settings_colorblind )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 204, "[x] %s", language[1345]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 204, "[ ] %s", language[1345]);
			}
			if ( settings_light_flicker )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 228, "[x] %s", language[2967]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 236, suby1 + 228, "[ ] %s", language[2967]);
			}

			if ( mousestatus[SDL_BUTTON_LEFT] )
			{
				if ( omousex >= subx1 + 242 && omousex < subx1 + 266 )
				{
					if ( omousey >= suby1 + 84 && omousey < suby1 + 84 + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_smoothlighting = (settings_smoothlighting == 0);
					}
					else if ( omousey >= suby1 + 108 && omousey < suby1 + 108 + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_fullscreen = (settings_fullscreen == 0);
					}
					else if ( omousey >= suby1 + 132 && omousey < suby1 + 132 + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_shaking = (settings_shaking == 0);
					}
					else if ( omousey >= suby1 + 156 && omousey < suby1 + 156 + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_bobbing = (settings_bobbing == 0);
					}
					else if ( omousey >= suby1 + 180 && omousey < suby1 + 180 + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_spawn_blood = (settings_spawn_blood == 0);
					}
					else if ( omousey >= suby1 + 204 && omousey < suby1 + 204 + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_colorblind = (settings_colorblind == false);
					}
					else if ( omousey >= suby1 + 228 && omousey < suby1 + 228 + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_light_flicker = (settings_light_flicker == false);
					}
				}
			}

			// fov slider
			ttfPrintText(ttf12, subx1 + 24, suby2 - 174, language[1346]);
			doSlider(subx1 + 24, suby2 - 148, 14, 40, 100, 1, (int*)(&settings_fov));

			// gamma slider
			ttfPrintText(ttf12, subx1 + 24, suby2 - 128, language[1347]);
			doSliderF(subx1 + 24, suby2 - 104, 14, 0.25, 2.f, 0.25, &settings_gamma);

			// fps slider
			ttfPrintText(ttf12, subx1 + 24, suby2 - 80, language[2411]);
			doSlider(subx1 + 24, suby2 - 56, 14, 60, 144, 1, (int*)(&settings_fps));
		}

		// audio tab
		if ( settings_tab == SETTINGS_AUDIO_TAB )
		{
			ttfPrintText(ttf12, subx1 + 24, suby1 + 60, language[1348]);
			doSlider(subx1 + 24, suby1 + 84, 15, 0, 128, 0, &settings_sfxvolume);
			ttfPrintText(ttf12, subx1 + 24, suby1 + 108, language[1349]);
			doSlider(subx1 + 24, suby1 + 132, 15, 0, 128, 0, &settings_musvolume);
		}

		// keyboard tab
		if ( settings_tab == SETTINGS_KEYBOARD_TAB )
		{
			ttfPrintText(ttf12, subx1 + 24, suby1 + 60, language[1350]);

			bool rebindingkey = false;
			if ( rebindkey != -1 )
			{
				rebindingkey = true;
			}

			int c;
			for ( c = 0; c < NUMIMPULSES; c++ )
			{
				if ( c < 14 )
				{
					ttfPrintText(ttf12, subx1 + 24, suby1 + 84 + 16 * c, language[1351 + c]);
				}
				else if ( c < 16 )
				{
					ttfPrintText(ttf12, subx1 + 24, suby1 + 84 + 16 * c, language[1940 + (c - 14)]);
				}
				else
				{
					ttfPrintText(ttf12, subx1 + 24, suby1 + 84 + 16 * c, language[1981 + (c - 16)]);
				}
				if ( mousestatus[SDL_BUTTON_LEFT] && !rebindingkey )
				{
					if ( omousex >= subx1 + 24 && omousex < subx2 - 24 )
					{
						if ( omousey >= suby1 + 84 + c * 16 && omousey < suby1 + 96 + c * 16 )
						{
							mousestatus[SDL_BUTTON_LEFT] = 0;
							lastkeypressed = 0;
							rebindingkey = true;
							rebindkey = c;
						}
					}
				}
				if ( c != rebindkey )
				{
					if ( !strcmp(getInputName(settings_impulses[c]), "Unassigned key" ))
					{
						ttfPrintTextColor(ttf12, subx1 + 256, suby1 + 84 + c * 16, uint32ColorBaronyBlue(*mainsurface), true, getInputName(settings_impulses[c]));
					}
					else if ( !strcmp(getInputName(settings_impulses[c]), "Unknown key") || !strcmp(getInputName(settings_impulses[c]), "Unknown trigger") )
					{
						ttfPrintTextColor(ttf12, subx1 + 256, suby1 + 84 + c * 16, uint32ColorRed(*mainsurface), true, getInputName(settings_impulses[c]));
					}
					else
					{
						ttfPrintText(ttf12, subx1 + 256, suby1 + 84 + c * 16, getInputName(settings_impulses[c]));
					}
				}
				else
				{
					ttfPrintTextColor(ttf12, subx1 + 256, suby1 + 84 + c * 16, uint32ColorGreen(*mainsurface), true, "...");
				}
			}

			if ( rebindkey != -1 && lastkeypressed )
			{
				if ( lastkeypressed == SDL_SCANCODE_ESCAPE )
				{
					keystatus[SDL_SCANCODE_ESCAPE] = 0;
					lastkeypressed = 0;
					rebindkey = -1;
				}
				else
				{
					settings_impulses[rebindkey] = lastkeypressed;
					if ( lastkeypressed == 283 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;  // fixes mouse-left not registering bug
					}
					rebindkey = -1;
				}
			}
		}

		// mouse tab
		if ( settings_tab == SETTINGS_MOUSE_TAB )
		{
			ttfPrintText(ttf12, subx1 + 24, suby1 + 60, language[1365]);
			doSliderF(subx1 + 24, suby1 + 84, 11, 0, 128, 1, &settings_mousespeed);

			// checkboxes
			if ( settings_reversemouse )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 24, suby1 + 108, "[x] %s", language[1366]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 24, suby1 + 108, "[ ] %s", language[1366]);
			}
			if ( settings_smoothmouse )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 24, suby1 + 132, "[x] %s", language[1367]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 24, suby1 + 132, "[ ] %s", language[1367]);
			}
			if ( mousestatus[SDL_BUTTON_LEFT] )
			{
				if ( omousex >= subx1 + 30 && omousex < subx1 + 54 )
				{
					if ( omousey >= suby1 + 108 && omousey < suby1 + 120 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_reversemouse = (settings_reversemouse == 0);
					}
					if ( omousey >= suby1 + 132 && omousey < suby1 + 144 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_smoothmouse = (settings_smoothmouse == 0);
					}
				}
			}
		}

		//Gamepad tab
		if (settings_tab == SETTINGS_GAMEPAD_BINDINGS_TAB)
		{
			SDL_Rect startPos;
			startPos.x = subx1 + 24;
			startPos.y = suby1 + 60;
			SDL_Rect currentPos = startPos;
			ttfPrintText(ttf8, currentPos.x, currentPos.y, language[1992]);
			currentPos.y += 24;

			bool rebindingaction = false;
			if (rebindaction != -1)
			{
				rebindingaction = true;
			}

			//Print out the bi-functional bindings.
			for ( int c = 0; c < INDEX_JOYBINDINGS_START_MENU; ++c, currentPos.y += 12 )
			{
				printJoybindingNames(currentPos, c, rebindingaction);
			}

			//Print out the menu-exclusive bindings.
			currentPos.y += 12;
			drawLine(subx1 + 24, currentPos.y - 6, subx2 - 24, currentPos.y - 6, uint32ColorGray(*mainsurface), 255);
			ttfPrintText(ttf8, currentPos.x, currentPos.y, language[1990]);
			currentPos.y += 18;
			for ( c = INDEX_JOYBINDINGS_START_MENU; c < INDEX_JOYBINDINGS_START_GAME; ++c, currentPos.y += 12 )
			{
				printJoybindingNames(currentPos, c, rebindingaction);
			}

			//Print out the game-exclusive bindings.
			currentPos.y += 12;
			drawLine(subx1 + 24, currentPos.y - 6, subx2 - 24, currentPos.y - 6, uint32ColorGray(*mainsurface), 255);
			ttfPrintText(ttf8, currentPos.x, currentPos.y, language[1991]);
			currentPos.y += 18;
			for ( c = INDEX_JOYBINDINGS_START_GAME; c < NUM_JOY_IMPULSES; ++c, currentPos.y += 12 )
			{
				printJoybindingNames(currentPos, c, rebindingaction);
			}

			if (rebindaction != -1 && lastkeypressed)
			{

				if (lastkeypressed >= 299)   /* Is a joybutton. */
				{
					settings_joyimpulses[rebindaction] = lastkeypressed;
					*inputPressed(lastkeypressed) = 0; //To prevent bugs where the button will still be treated as pressed after assigning it, potentially doing wonky things.
					rebindaction = -1;
				}
				else
				{
					if (lastkeypressed == SDL_SCANCODE_ESCAPE)
					{
						keystatus[SDL_SCANCODE_ESCAPE] = 0;
					}
					lastkeypressed = 0;
					rebindaction = -1;
				}
			}
		}

		//General gamepad settings
		if (settings_tab == SETTINGS_GAMEPAD_SETTINGS_TAB)
		{
			int current_option_x = subx1 + 24;
			int current_option_y = suby1 + 60;

			//Checkboxes.
			if (settings_gamepad_leftx_invert)
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[x] %s", language[2401]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[ ] %s", language[2401]);
			}

			if (mousestatus[SDL_BUTTON_LEFT] && mouseInBounds(current_option_x, current_option_x + strlen("[x]")*TTF12_WIDTH, current_option_y, current_option_y + TTF12_HEIGHT))
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				settings_gamepad_leftx_invert = !settings_gamepad_leftx_invert;
			}

			current_option_y += 24;

			if (settings_gamepad_lefty_invert)
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[x] %s", language[2402]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[ ] %s", language[2402]);
			}

			if (mousestatus[SDL_BUTTON_LEFT] && mouseInBounds(current_option_x, current_option_x + strlen("[x]")*TTF12_WIDTH, current_option_y, current_option_y + TTF12_HEIGHT))
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				settings_gamepad_lefty_invert = !settings_gamepad_lefty_invert;
			}

			current_option_y += 24;

			if (settings_gamepad_rightx_invert)
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[x] %s", language[2403]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[ ] %s", language[2403]);
			}

			if (mousestatus[SDL_BUTTON_LEFT] && mouseInBounds(current_option_x, current_option_x + strlen("[x]")*TTF12_WIDTH, current_option_y, current_option_y + TTF12_HEIGHT))
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				settings_gamepad_rightx_invert = !settings_gamepad_rightx_invert;
			}

			current_option_y += 24;

			if (settings_gamepad_righty_invert)
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[x] %s", language[2404]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[ ] %s", language[2404]);
			}

			if (mousestatus[SDL_BUTTON_LEFT] && mouseInBounds(current_option_x, current_option_x + strlen("[x]")*TTF12_WIDTH, current_option_y, current_option_y + TTF12_HEIGHT))
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				settings_gamepad_righty_invert = !settings_gamepad_righty_invert;
			}

			current_option_y += 24;

			if (settings_gamepad_menux_invert)
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[x] %s", language[2405]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[ ] %s", language[2405]);
			}

			if (mousestatus[SDL_BUTTON_LEFT] && mouseInBounds(current_option_x, current_option_x + strlen("[x]")*TTF12_WIDTH, current_option_y, current_option_y + TTF12_HEIGHT))
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				settings_gamepad_menux_invert = !settings_gamepad_menux_invert;
			}

			current_option_y += 24;

			if (settings_gamepad_menuy_invert)
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[x] %s", language[2406]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, current_option_x, current_option_y, "[ ] %s", language[2406]);
			}

			if (mousestatus[SDL_BUTTON_LEFT] && mouseInBounds(current_option_x, current_option_x + strlen("[x]")*TTF12_WIDTH, current_option_y, current_option_y + TTF12_HEIGHT))
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;
				settings_gamepad_menuy_invert = !settings_gamepad_menuy_invert;
			}

			current_option_y += 24;

			ttfPrintText(ttf12, current_option_x, current_option_y, language[2407]);
			current_option_y += 24;
			//doSlider(current_option_x, current_option_y, 11, 1, 2000, 200, &settings_gamepad_rightx_sensitivity, font8x8_bmp, 12); //Doesn't like any fonts besides the default.
			doSlider(current_option_x, current_option_y, 11, 1, 4096, 100, &settings_gamepad_rightx_sensitivity);

			current_option_y += 24;

			ttfPrintText(ttf12, current_option_x, current_option_y, language[2408]);
			current_option_y += 24;
			//doSlider(current_option_x, current_option_y, 11, 1, 2000, 200, &settings_gamepad_righty_sensitivity, font8x8_bmp, 12);
			doSlider(current_option_x, current_option_y, 11, 1, 4096, 100, &settings_gamepad_righty_sensitivity);

			current_option_y += 24;

			ttfPrintText(ttf12, current_option_x, current_option_y, language[2409]);
			current_option_y += 24;
			//doSlider(current_option_x, current_option_y, 11, 1, 2000, 200, &settings_gamepad_menux_sensitivity, font8x8_bmp, 12);
			doSlider(current_option_x, current_option_y, 11, 1, 4096, 100, &settings_gamepad_menux_sensitivity);

			current_option_y += 24;

			ttfPrintText(ttf12, current_option_x, current_option_y, language[2410]);
			current_option_y += 24;
			//doSlider(current_option_x, current_option_y, 11, 1, 2000, 200, &settings_gamepad_menuy_sensitivity, font8x8_bmp, 12);
			doSlider(current_option_x, current_option_y, 11, 1, 4096, 100, &settings_gamepad_menuy_sensitivity);
		}

		// miscellaneous options
		if (settings_tab == SETTINGS_MISC_TAB)
		{
			int current_x = subx1;
			int current_y = suby1 + 60;

			ttfPrintText(ttf12, subx1 + 24, current_y, language[1371]);
			current_y += 24;

			int options_start_y = current_y;
			if ( settings_broadcast )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", language[1372]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", language[1372]);
			}
			current_y += 16;
			if ( settings_nohud )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", language[1373]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", language[1373]);
			}
			current_y += 16;
			int hotbar_options_x = subx1 + 72 + 256;
			int hotbar_options_y = current_y;
			if ( settings_auto_hotbar_new_items )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", language[1374]);
				int pad_x = hotbar_options_x;
				int pad_y = hotbar_options_y;
				drawWindowFancy(pad_x - 16, pad_y - 32, pad_x + 4 * 128 + 16, pad_y + 48 + 16);
				ttfPrintTextFormatted(ttf12, pad_x, current_y - 16, "%s", language[2583]);
				for ( int i = 0; i < (NUM_HOTBAR_CATEGORIES); ++i )
				{
					if ( settings_auto_hotbar_categories[i] == true )
					{
						ttfPrintTextFormatted(ttf12, pad_x, pad_y, "[x] %s", language[2571 + i]);
					}
					else
					{
						ttfPrintTextFormatted(ttf12, pad_x, pad_y, "[ ] %s", language[2571 + i]);
					}
					pad_x += 128;
					if ( i == 3 || i == 7 )
					{
						pad_x = hotbar_options_x;
						pad_y += 16;
					}
				}
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", language[1374]);
			}

			// autosort inventory categories
			int autosort_options_x = subx1 + 72 + 256;
			int autosort_options_y = current_y + 112;
			int pad_x = autosort_options_x;
			int pad_y = autosort_options_y;
			drawWindowFancy(pad_x - 16, pad_y - 32, pad_x + 4 * 128 + 16, pad_y + 48 + 16);
			ttfPrintTextFormatted(ttf12, pad_x, current_y - 16 + 112, "%s", language[2912]);

			// draw the values for autosort
			for ( int i = 0; i < (NUM_AUTOSORT_CATEGORIES); ++i )
			{
				ttfPrintTextFormatted(ttf12, pad_x, pad_y, "<");
				Uint32 autosortColor = uint32ColorGreen(*mainsurface);
				int padValue_x = pad_x;
				if ( settings_autosort_inventory_categories[i] < 0 )
				{
					autosortColor = uint32ColorRed(*mainsurface);
					padValue_x += 4; // centre the negative numbers.
				}
				else if ( settings_autosort_inventory_categories[i] == 0 )
				{
					autosortColor = uint32ColorWhite(*mainsurface);
				}
				ttfPrintTextFormattedColor(ttf12, padValue_x, pad_y, autosortColor, " %2d", settings_autosort_inventory_categories[i]);
				if ( i == NUM_AUTOSORT_CATEGORIES - 1 )
				{
					ttfPrintTextFormatted(ttf12, pad_x, pad_y, "    > %s", language[2916]);
				}
				else
				{
					ttfPrintTextFormatted(ttf12, pad_x, pad_y, "    > %s", language[2571 + i]);
				}
				pad_x += 128;
				if ( i == 3 || i == 7 )
				{
					pad_x = autosort_options_x;
					pad_y += 16;
				}
			}

			pad_x = autosort_options_x + (strlen(language[2912]) - 3) * (TTF12_WIDTH) + 8; // 3 chars from the end of string.
			pad_y = autosort_options_y;
			// hover text for autosort title text
			if ( mouseInBounds(pad_x - 4, pad_x + 3 * TTF12_WIDTH + 8, current_y - 16 + 112, current_y - 16 + 124) )
			{
				tooltip_box.x = omousex - TTF12_WIDTH * 32;
				tooltip_box.y = omousey - (TTF12_HEIGHT * 3 + 16);
				tooltip_box.w = strlen(language[2914]) * TTF12_WIDTH + 8;
				tooltip_box.h = TTF12_HEIGHT * 3 + 8;
				drawTooltip(&tooltip_box);
				ttfPrintTextFormatted(ttf12, tooltip_box.x + 4, tooltip_box.y + 4, language[2913]);
				ttfPrintTextFormatted(ttf12, tooltip_box.x + 4, tooltip_box.y + 4 + TTF12_HEIGHT, language[2914]);
				ttfPrintTextFormatted(ttf12, tooltip_box.x + 4, tooltip_box.y + 4 + TTF12_HEIGHT * 2, language[2915]);
			}

			current_y += 16;
			if ( settings_auto_appraise_new_items )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", language[1997]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", language[1997]);
			}
			current_y += 16;
			if ( settings_disable_messages )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", language[1536]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", language[1536]);
			}
			current_y += 16;
			if ( settings_right_click_protect )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", language[1998]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", language[1998]);
			}
			current_y += 16;
			if ( settings_hotbar_numkey_quick_add )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", language[2590]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", language[2590]);
			}
			current_y += 16;
			if ( settings_lock_right_sidebar )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", language[2598]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", language[2598]);
			}
			current_y += 16;
			if ( settings_show_game_timer_always )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", language[2983]);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", language[2983]);
			}
			current_y += 32;

			// server flag elements
			ttfPrintText(ttf12, subx1 + 24, current_y, language[1375]);
			current_y += 24;


			int server_flags_start_y = current_y;
			int i;
			for ( i = 0; i < NUM_SERVER_FLAGS; i++, current_y += 16 )
			{
				char flagStringBuffer[256] = "";
				if ( i < 5 )
				{
					strncpy(flagStringBuffer, language[153 + i], 255);
				}
				else
				{
					strncpy(flagStringBuffer, language[2917 - 5 + i], 255);
				}
				if ( svFlags & power(2, i) )
				{
					ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[x] %s", flagStringBuffer);
				}
				else
				{
					ttfPrintTextFormatted(ttf12, subx1 + 36, current_y, "[ ] %s", flagStringBuffer);
				}
				if (mouseInBounds(subx1 + 36 + 6, subx1 + 36 + 24 + 6, current_y, current_y + 12))   //So many gosh dang magic numbers ._.
				{
					if ( i < 5 )
					{
						strncpy(flagStringBuffer, language[1942 + i], 255);
					}
					else
					{
						strncpy(flagStringBuffer, language[2921 - 5 + i], 255);
					}
					if (strlen(flagStringBuffer) > 0)   //Don't bother drawing a tooltip if the file doesn't say anything.
					{
						hovering_selection = i;
#ifndef STEAMWORKS
						if ( hovering_selection == 0 )
						{
							hovering_selection = -1; // don't show cheats tooltip about disabling achievements.
						}
#endif // STEAMWORKS
						tooltip_box.x = omousex + 16;
						tooltip_box.y = omousey + 8; //I hate magic numbers :|. These should probably be replaced with omousex + mousecursorsprite->width, omousey + mousecursorsprite->height, respectively.
						tooltip_box.w = strlen(flagStringBuffer) * TTF12_WIDTH + 8; //MORE MAGIC NUMBERS. HNNGH. I can guess what they all do, but dang.
						tooltip_box.h = TTF12_HEIGHT + 8;
					}
				}
			}

			if (hovering_selection > -1)
			{
				drawTooltip(&tooltip_box);
				if (hovering_selection < NUM_SERVER_FLAGS)
				{
					char flagStringBuffer[256] = "";
					if ( hovering_selection < 5 )
					{
						strncpy(flagStringBuffer, language[1942 + hovering_selection], 255);
					}
					else
					{
						strncpy(flagStringBuffer, language[2921 - 5 + hovering_selection], 255);
					}
					ttfPrintTextFormatted(ttf12, tooltip_box.x + 4, tooltip_box.y + 4, flagStringBuffer);
				}
			}

			current_y = options_start_y;

			if ( mousestatus[SDL_BUTTON_LEFT] )
			{
				if ( omousex >= subx1 + 42 && omousex < subx1 + 66 )
				{
					if (omousey >= current_y && omousey < current_y + 12)
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_broadcast = (settings_broadcast == false);
					}
					else if (omousey >= (current_y += 16) && omousey < current_y + 12)
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_nohud = (settings_nohud == false);
					}
					else if (omousey >= (current_y += 16) && omousey < current_y + 12)
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_auto_hotbar_new_items = (settings_auto_hotbar_new_items == false);
					}
					else if (omousey >= (current_y += 16) && omousey < current_y + 12)
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_auto_appraise_new_items = (settings_auto_appraise_new_items == false);
					}
					else if (omousey >= (current_y += 16) && omousey < current_y + 12)
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_disable_messages = (settings_disable_messages == false);
					}
					else if (omousey >= (current_y += 16) && omousey < current_y + 12)
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_right_click_protect = (settings_right_click_protect == false);
					}
					else if ( omousey >= (current_y += 16) && omousey < current_y + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_hotbar_numkey_quick_add = (settings_hotbar_numkey_quick_add == false);
					}
					else if ( omousey >= (current_y += 16) && omousey < current_y + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_lock_right_sidebar = (settings_lock_right_sidebar == false);
					}
					else if ( omousey >= (current_y += 16) && omousey < current_y + 12 )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;
						settings_show_game_timer_always = (settings_show_game_timer_always == false);
					}
				}
				else
				{
					if ( settings_auto_hotbar_new_items )
					{
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							for ( i = 0; i < NUM_HOTBAR_CATEGORIES; ++i )
							{
								if ( mouseInBounds(hotbar_options_x, hotbar_options_x + 24, hotbar_options_y, hotbar_options_y + 12) )
								{
									settings_auto_hotbar_categories[i] = !settings_auto_hotbar_categories[i];
									mousestatus[SDL_BUTTON_LEFT] = 0;
								}
								hotbar_options_x += 128;
								if ( i == 3 || i == 7 )
								{
									hotbar_options_x -= (128 * 4);
									hotbar_options_y += 16;
								}
							}
						}
					}

					// autosort category toggles
					if ( mousestatus[SDL_BUTTON_LEFT] )
					{
						for ( i = 0; i < NUM_AUTOSORT_CATEGORIES; ++i )
						{
							if ( mouseInBounds(autosort_options_x, autosort_options_x + 16, autosort_options_y, autosort_options_y + 12) )
							{
								--settings_autosort_inventory_categories[i];
								if ( settings_autosort_inventory_categories[i] < -9 )
								{
									settings_autosort_inventory_categories[i] = 9;
								}
								mousestatus[SDL_BUTTON_LEFT] = 0;
							}
							else if(mouseInBounds(autosort_options_x + 36, autosort_options_x + 52, autosort_options_y, autosort_options_y + 12))
							{
								++settings_autosort_inventory_categories[i];
								if ( settings_autosort_inventory_categories[i] > 9 )
								{
									settings_autosort_inventory_categories[i] = -9;
								}
								mousestatus[SDL_BUTTON_LEFT] = 0;
							}
							autosort_options_x += 128;
							if ( i == 3 || i == 7 )
							{
								autosort_options_x -= (128 * 4);
								autosort_options_y += 16;
							}
						}
					}
				}

				if ( multiplayer != CLIENT )
				{
					current_y = server_flags_start_y;
					for (i = 0; i < NUM_SERVER_FLAGS; i++, current_y += 16)
					{
						if ( mouseInBounds(subx1 + 36 + 6, subx1 + 36 + 24 + 6, current_y, current_y + 12) )
						{
							mousestatus[SDL_BUTTON_LEFT] = 0;

							// toggle flag
							svFlags ^= power(2, i);

							if ( multiplayer == SERVER )
							{
								// update client flags
								strcpy((char*)net_packet->data, "SVFL");
								SDLNet_Write32(svFlags, &net_packet->data[4]);
								net_packet->len = 8;

								int c;
								for (c = 1; c < MAXPLAYERS; ++c)
								{
									if (client_disconnected[c])
									{
										continue;
									}
									net_packet->address.host = net_clients[c - 1].host;
									net_packet->address.port = net_clients[c - 1].port;
									sendPacketSafe(net_sock, -1, net_packet, c - 1);
									messagePlayer(c, language[276]);
								}
								messagePlayer(clientnum, language[276]);
							}
						}
					}
				}
			}
		}
	}

	// connect window
	if ( connect_window )
	{
		if ( connect_window == SERVER )
		{
			drawDepressed(subx1 + 8, suby1 + 40, subx2 - 8, suby1 + 64);
			ttfPrintText(ttf12, subx1 + 12, suby1 + 46, portnumber_char);

			// enter port number
			if ( !SDL_IsTextInputActive() )
			{
				SDL_StartTextInput();
				inputstr = portnumber_char;
			}
			//strncpy(portnumber_char,inputstr,5);
			inputlen = 5;
			if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
			{
				int x;
				TTF_SizeUTF8(ttf12, portnumber_char, &x, NULL);
				ttfPrintText(ttf12, subx1 + 12 + x, suby1 + 46, "_");
			}
		}
		else if ( connect_window == CLIENT )
		{
			drawDepressed(subx1 + 8, suby1 + 40, subx2 - 8, suby1 + 64);
			if ( !broadcast )
			{
				ttfPrintText(ttf12, subx1 + 12, suby1 + 46, connectaddress);
			}
			else
			{
				int i;
				for ( i = 0; i < strlen(connectaddress); i++ )
				{
					ttfPrintText(ttf12, subx1 + 12 + 12 * i, suby1 + 46, "*");
				}
			}

			// enter address
			if ( !SDL_IsTextInputActive() )
			{
				SDL_StartTextInput();
				inputstr = connectaddress;
			}
			//strncpy(connectaddress,inputstr,31);
			inputlen = 31;
			if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
			{
				int x;
				TTF_SizeUTF8(ttf12, connectaddress, &x, NULL);
				ttfPrintText(ttf12, subx1 + 12 + x, suby1 + 46, "_");
			}
		}
	}

	// communicating with clients
	if ( multiplayer == SERVER && mode )
	{
		//void *newSteamID = NULL; //TODO: Bugger void pointers!
#ifdef STEAMWORKS
		CSteamID newSteamID;
#endif

		// hosting the lobby
		int numpacket;
		for ( numpacket = 0; numpacket < PACKET_LIMIT; numpacket++ )
		{
			if ( directConnect )
			{
				if ( !SDLNet_UDP_Recv(net_sock, net_packet) )
				{
					break;
				}
			}
			else
			{
#ifdef STEAMWORKS
				uint32_t packetlen = 0;
				if ( !SteamNetworking()->IsP2PPacketAvailable(&packetlen, 0) )
				{
					break;
				}
				packetlen = std::min<int>(packetlen, NET_PACKET_SIZE - 1);
				/*if ( newSteamID ) {
					cpp_Free_CSteamID( newSteamID );
					newSteamID = NULL;
				}*/
				//newSteamID = c_AllocateNew_CSteamID();
				Uint32 bytesRead = 0;
				if ( !SteamNetworking()->ReadP2PPacket(net_packet->data, packetlen, &bytesRead, &newSteamID, 0) )
				{
					continue;
				}
				net_packet->len = packetlen;
				if ( packetlen < sizeof(DWORD) )
				{
					continue;    // junk packet, skip //TODO: Investigate the cause of this. During earlier testing, we were getting bombarded with untold numbers of these malformed packets, as if the entire steam network were being routed through this game.
				}

				CSteamID mySteamID = SteamUser()->GetSteamID();
				if ( mySteamID.ConvertToUint64() == newSteamID.ConvertToUint64() )
				{
					continue;
				}
#endif
			}

			if ( handleSafePacket() )
			{
				continue;
			}
			if (!strncmp((char*)net_packet->data, "BARONY_JOIN_REQUEST", 19))
			{
#ifdef STEAMWORKS
				if ( !directConnect )
				{
					bool skipJoin = false;
					for ( c = 0; c < MAXPLAYERS; c++ )
					{
						if ( client_disconnected[c] || !steamIDRemote[c] )
						{
							continue;
						}
						if ( newSteamID.ConvertToUint64() == (static_cast<CSteamID* >(steamIDRemote[c]))->ConvertToUint64() )
						{
							// we've already accepted this player. NEXT!
							skipJoin = true;
							break;
						}
					}
					if ( skipJoin )
					{
						continue;
					}
				}
#endif
				if ( strcmp( VERSION, (char*)net_packet->data + 54 ) )
				{
					c = MAXPLAYERS + 1; // wrong version number
				}
				else
				{
					Uint32 clientlsg = SDLNet_Read32(&net_packet->data[68]);
					Uint32 clientms = SDLNet_Read32(&net_packet->data[64]);
					if ( net_packet->data[63] == 0 )
					{
						// client will enter any player spot
						for ( c = 0; c < MAXPLAYERS; c++ )
						{
							if ( client_disconnected[c] == true )
							{
								break;    // no more player slots
							}
						}
					}
					else
					{
						// client is joining a particular player spot
						c = net_packet->data[63];
						if ( !client_disconnected[c] )
						{
							c = MAXPLAYERS;  // client wants to fill a space that is already filled
						}
					}
					if ( clientlsg != loadingsavegame && loadingsavegame == 0 )
					{
						c = MAXPLAYERS + 2;  // client shouldn't load save game
					}
					else if ( clientlsg == 0 && loadingsavegame != 0 )
					{
						c = MAXPLAYERS + 3;  // client is trying to join a save game without a save of their own
					}
					else if ( clientlsg != loadingsavegame )
					{
						c = MAXPLAYERS + 4;  // client is trying to join the game with an incompatible save
					}
					else if ( loadingsavegame && getSaveGameMapSeed(false) != clientms )
					{
						c = MAXPLAYERS + 5;  // client is trying to join the game with a slightly incompatible save (wrong level)
					}
				}
				if ( c >= MAXPLAYERS )
				{
					// on error, client gets a player number that is invalid (to be interpreted as an error code)
					net_clients[MAXPLAYERS - 1].host = net_packet->address.host;
					net_clients[MAXPLAYERS - 1].port = net_packet->address.port;
					if ( directConnect )
						while ((net_tcpclients[MAXPLAYERS - 1] = SDLNet_TCP_Accept(net_tcpsock)) == NULL);
					net_packet->address.host = net_clients[MAXPLAYERS - 1].host;
					net_packet->address.port = net_clients[MAXPLAYERS - 1].port;
					net_packet->len = 4;
					SDLNet_Write32(c, &net_packet->data[0]); // error code for client to interpret
					if ( directConnect )
					{
						SDLNet_TCP_Send(net_tcpclients[MAXPLAYERS - 1], net_packet->data, net_packet->len);
						SDLNet_TCP_Close(net_tcpclients[MAXPLAYERS - 1]);
					}
					else
					{
#ifdef STEAMWORKS
						SteamNetworking()->SendP2PPacket(newSteamID, net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
						SteamNetworking()->SendP2PPacket(newSteamID, net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
						SteamNetworking()->SendP2PPacket(newSteamID, net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
						SteamNetworking()->SendP2PPacket(newSteamID, net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
						SteamNetworking()->SendP2PPacket(newSteamID, net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
#endif
					}
				}
				else
				{
					// on success, client gets legit player number
					strcpy(stats[c]->name, (char*)(&net_packet->data[19]));
					client_disconnected[c] = false;
					client_classes[c] = (int)SDLNet_Read32(&net_packet->data[42]);
					stats[c]->sex = static_cast<sex_t>((int)SDLNet_Read32(&net_packet->data[46]));
					stats[c]->appearance = (int)SDLNet_Read32(&net_packet->data[50]);
					net_clients[c - 1].host = net_packet->address.host;
					net_clients[c - 1].port = net_packet->address.port;
					if ( directConnect )
					{
						while ((net_tcpclients[c - 1] = SDLNet_TCP_Accept(net_tcpsock)) == NULL);
						const char* clientaddr = SDLNet_ResolveIP(&net_packet->address);
						printlog("client %d connected from %s:%d\n", c, clientaddr, net_packet->address.port);
					}
					else
					{
						printlog("client %d connected.\n", c);
					}
					client_keepalive[c] = ticks;

					// send existing clients info on new client
					for ( x = 1; x < MAXPLAYERS; x++ )
					{
						if ( client_disconnected[x] || c == x )
						{
							continue;
						}
						strcpy((char*)(&net_packet->data[0]), "NEWPLAYER");
						net_packet->data[9] = c; // clientnum
						net_packet->data[10] = client_classes[c]; // class
						net_packet->data[11] = stats[c]->sex; // sex
						strcpy((char*)(&net_packet->data[12]), stats[c]->name);  // name
						net_packet->address.host = net_clients[x - 1].host;
						net_packet->address.port = net_clients[x - 1].port;
						net_packet->len = 12 + strlen(stats[c]->name) + 1;
						sendPacketSafe(net_sock, -1, net_packet, x - 1);
					}
					char shortname[11] = { 0 };
					strncpy(shortname, stats[c]->name, 10);

					newString(&lobbyChatboxMessages, 0xFFFFFFFF, "\n***   %s has joined the game   ***\n", shortname);

					// send new client their id number + info on other clients
					SDLNet_Write32(c, &net_packet->data[0]);
					for ( x = 0; x < MAXPLAYERS; x++ )
					{
						net_packet->data[4 + x * (3 + 16)] = client_classes[x]; // class
						net_packet->data[5 + x * (3 + 16)] = stats[x]->sex; // sex
						net_packet->data[6 + x * (3 + 16)] = client_disconnected[x]; // connectedness :p
						strcpy((char*)(&net_packet->data[7 + x * (3 + 16)]), stats[x]->name);  // name
					}
					net_packet->address.host = net_clients[c - 1].host;
					net_packet->address.port = net_clients[c - 1].port;
					net_packet->len = 4 + MAXPLAYERS * (3 + 16);
					if ( directConnect )
					{
						SDLNet_TCP_Send(net_tcpclients[c - 1], net_packet->data, net_packet->len);
					}
					else
					{
#ifdef STEAMWORKS
						if ( steamIDRemote[c - 1] )
						{
							cpp_Free_CSteamID( steamIDRemote[c - 1] );
						}
						steamIDRemote[c - 1] = new CSteamID();
						*static_cast<CSteamID*>(steamIDRemote[c - 1]) = newSteamID;
						SteamNetworking()->SendP2PPacket(*static_cast<CSteamID* >(steamIDRemote[c - 1]), net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
						SteamNetworking()->SendP2PPacket(*static_cast<CSteamID* >(steamIDRemote[c - 1]), net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
						SteamNetworking()->SendP2PPacket(*static_cast<CSteamID* >(steamIDRemote[c - 1]), net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
						SteamNetworking()->SendP2PPacket(*static_cast<CSteamID* >(steamIDRemote[c - 1]), net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
						SteamNetworking()->SendP2PPacket(*static_cast<CSteamID* >(steamIDRemote[c - 1]), net_packet->data, net_packet->len, k_EP2PSendReliable, 0);
						SDL_Delay(5);
#endif
					}
				}
				continue;
			}

			// got a chat message
			else if (!strncmp((char*)net_packet->data, "CMSG", 4))
			{
				int i;
				for ( i = 0; i < MAXPLAYERS; i++ )
				{
					if ( client_disconnected[i] )
					{
						continue;
					}
					net_packet->address.host = net_clients[i - 1].host;
					net_packet->address.port = net_clients[i - 1].port;
					sendPacketSafe(net_sock, -1, net_packet, i - 1);
				}
				newString(&lobbyChatboxMessages, 0xFFFFFFFF, (char*)(&net_packet->data[4]));
				playSound(238, 64);
				continue;
			}

			// player disconnected
			else if (!strncmp((char*)net_packet->data, "PLAYERDISCONNECT", 16))
			{
				client_disconnected[net_packet->data[16]] = true;
				for ( c = 1; c < MAXPLAYERS; c++ )
				{
					if ( client_disconnected[c] )
					{
						continue;
					}
					net_packet->address.host = net_clients[c - 1].host;
					net_packet->address.port = net_clients[c - 1].port;
					net_packet->len = 17;
					sendPacketSafe(net_sock, -1, net_packet, c - 1);
				}
				char shortname[11] = { 0 };
				strncpy(shortname, stats[net_packet->data[16]]->name, 10);
				newString(&lobbyChatboxMessages, 0xFFFFFFFF, language[1376], shortname);
				continue;
			}

			// client requesting new svFlags
			else if (!strncmp((char*)net_packet->data, "SVFL", 4))
			{
				// update svFlags for everyone
				SDLNet_Write32(svFlags, &net_packet->data[4]);
				net_packet->len = 8;

				int c;
				for ( c = 1; c < MAXPLAYERS; c++ )
				{
					if ( client_disconnected[c] )
					{
						continue;
					}
					net_packet->address.host = net_clients[c - 1].host;
					net_packet->address.port = net_clients[c - 1].port;
					sendPacketSafe(net_sock, -1, net_packet, c - 1);
				}
				continue;
			}

			// keepalive
			else if (!strncmp((char*)net_packet->data, "KEEPALIVE", 9))
			{
				client_keepalive[net_packet->data[9]] = ticks;
				continue; // just a keep alive
			}
		}
	}

	// communicating with server
	if ( multiplayer == CLIENT && mode )
	{
		if ( receivedclientnum == false )
		{
#ifdef STEAMWORKS
			CSteamID newSteamID;
#endif

			// trying to connect to the server and get a player number
			// receive the packet:
			bool gotPacket = false;
			if ( directConnect )
			{
				if ( SDLNet_TCP_Recv(net_tcpsock, net_packet->data, 4 + MAXPLAYERS * (3 + 16)) )
				{
					gotPacket = true;
				}
			}
			else
			{
#ifdef STEAMWORKS
				int numpacket;
				for ( numpacket = 0; numpacket < PACKET_LIMIT; numpacket++ )
				{
					uint32_t packetlen = 0;
					if ( !SteamNetworking()->IsP2PPacketAvailable(&packetlen, 0) )
					{
						break;
					}
					packetlen = std::min<int>(packetlen, NET_PACKET_SIZE - 1);
					Uint32 bytesRead = 0;
					if ( !SteamNetworking()->ReadP2PPacket(net_packet->data, packetlen, &bytesRead, &newSteamID, 0) || bytesRead != 4 + MAXPLAYERS * (3 + 16) )
					{
						continue;
					}
					net_packet->len = packetlen;
					if ( packetlen < sizeof(DWORD) )
					{
						continue;
					}

					CSteamID mySteamID = SteamUser()->GetSteamID();
					if ( mySteamID.ConvertToUint64() == newSteamID.ConvertToUint64() )
					{
						continue;
					}
					gotPacket = true;
					break;
				}
#endif
			}

			// parse the packet:
			if ( gotPacket )
			{
				list_FreeAll(&button_l);
				deleteallbuttons = true;
				clientnum = (int)SDLNet_Read32(&net_packet->data[0]);
				if ( clientnum >= MAXPLAYERS || clientnum <= 0 )
				{
					printlog("connection attempt denied by server.\n");
					multiplayer = SINGLE;

					// close current window
					buttonCloseSubwindow(NULL);
					for ( node = button_l.first; node != NULL; node = nextnode )
					{
						nextnode = node->next;
						button = (button_t*)node->element;
						if ( button->focused )
						{
							list_RemoveNode(button->node);
						}
					}

#ifdef STEAMWORKS
					if ( !directConnect )
					{
						if ( currentLobby )
						{
							SteamMatchmaking()->LeaveLobby(*static_cast<CSteamID*>(currentLobby));
							cpp_Free_CSteamID( currentLobby ); //TODO: Bugger this.
							currentLobby = NULL;
						}
					}
#endif

					// create new window
					subwindow = 1;
					subx1 = xres / 2 - 256;
					subx2 = xres / 2 + 256;
					suby1 = yres / 2 - 48;
					suby2 = yres / 2 + 48;
					strcpy(subtext, language[1377]);
					if ( clientnum == MAXPLAYERS )
					{
						strcat(subtext, language[1378]);
					}
					else if ( clientnum == MAXPLAYERS + 1 )
					{
						strcat(subtext, language[1379]);
					}
					else if ( clientnum == MAXPLAYERS + 2 )
					{
						strcat(subtext, language[1380]);
					}
					else if ( clientnum == MAXPLAYERS + 3 )
					{
						strcat(subtext, language[1381]);
					}
					else if ( clientnum == MAXPLAYERS + 4 )
					{
						strcat(subtext, language[1382]);
					}
					else if ( clientnum == MAXPLAYERS + 5 )
					{
						strcat(subtext, language[1383]);
					}
					else
					{
						strcat(subtext, language[1384]);
					}
					clientnum = 0;

					// close button
					button = newButton();
					strcpy(button->label, "x");
					button->x = subx2 - 20;
					button->y = suby1;
					button->sizex = 20;
					button->sizey = 20;
					button->action = &buttonCloseSubwindow;
					button->visible = 1;
					button->focused = 1;
					button->key = SDL_SCANCODE_ESCAPE;
					button->joykey = joyimpulses[INJOY_MENU_CANCEL];

					// okay button
					button = newButton();
					strcpy(button->label, language[732]);
					button->x = subx2 - (subx2 - subx1) / 2 - 28;
					button->y = suby2 - 28;
					button->sizex = 56;
					button->sizey = 20;
					button->action = &buttonCloseSubwindow;
					button->visible = 1;
					button->focused = 1;
					button->key = SDL_SCANCODE_RETURN;
					button->joykey = joyimpulses[INJOY_MENU_NEXT];
				}
				else
				{
					// join game succeeded, advance to lobby
					client_keepalive[0] = ticks;
					receivedclientnum = true;
					printlog("connected to server.\n");
					client_disconnected[clientnum] = false;
					if ( !loadingsavegame )
					{
						stats[clientnum]->appearance = stats[0]->appearance;
					}

					// now set up everybody else
					for ( c = 0; c < MAXPLAYERS; c++ )
					{
						client_disconnected[c] = false;
						client_classes[c] = net_packet->data[4 + c * (3 + 16)]; // class
						stats[c]->sex = static_cast<sex_t>(net_packet->data[5 + c * (3 + 16)]); // sex
						client_disconnected[c] = net_packet->data[6 + c * (3 + 16)]; // connectedness :p
						strcpy(stats[c]->name, (char*)(&net_packet->data[7 + c * (3 + 16)]));  // name
					}

					// request svFlags
					strcpy((char*)net_packet->data, "SVFL");
					net_packet->len = 4;
					net_packet->address.host = net_server.host;
					net_packet->address.port = net_server.port;
					sendPacketSafe(net_sock, -1, net_packet, 0);

					// open lobby window
					lobby_window = true;
					subwindow = 1;
					subx1 = xres / 2 - 400;
					subx2 = xres / 2 + 400;
#ifdef PANDORA
					suby1 = yres / 2 - ((yres==480)?230:290);
					suby2 = yres / 2 + ((yres==480)?230:290);
#else
					suby1 = yres / 2 - 300;
					suby2 = yres / 2 + 300;
#endif

					if ( directConnect )
					{
						strcpy(subtext, language[1385]);
						if ( !broadcast )
						{
							strcat(subtext, last_ip);
						}
						else
						{
							strcat(subtext, "HIDDEN FOR BROADCAST");
						}
					}
					else
					{
						strcpy(subtext, language[1386]);
					}
					strcat(subtext, language[1387]);

					// disconnect button
					button = newButton();
					strcpy(button->label, language[1311]);
					button->sizex = strlen(language[1311]) * 12 + 8;
					button->sizey = 20;
					button->x = subx1 + 4;
					button->y = suby2 - 24;
					button->action = &buttonDisconnect;
					button->visible = 1;
					button->focused = 1;
					button->joykey = joyimpulses[INJOY_MENU_CANCEL];
#ifdef STEAMWORKS
					if ( !directConnect )
					{
						const char* serverNumModsChar = SteamMatchmaking()->GetLobbyData(*static_cast<CSteamID*>(currentLobby), "svNumMods");
						int serverNumModsLoaded = atoi(serverNumModsChar);
						if ( serverNumModsLoaded > 0 )
						{
							// subscribe to server loaded mods button
							button = newButton();
							strcpy(button->label, language[2984]);
							button->sizex = strlen(language[2984]) * 12 + 8;
							button->sizey = 20;
							button->x = subx2 - 4 - button->sizex;
							button->y = suby2 - 24;
							button->action = &buttonGamemodsSubscribeToHostsModFiles;
							button->visible = 1;
							button->focused = 1;

							// mount server mods button
							button = newButton();
							strcpy(button->label, language[2985]);
							button->sizex = strlen(language[2985]) * 12 + 8;
							button->sizey = 20;
							button->x = subx2 - 4 - button->sizex;
							button->y = suby2 - 24;
							button->action = &buttonGamemodsMountHostsModFiles;
							button->visible = 0;
							button->focused = 1;

							g_SteamWorkshop->CreateQuerySubscribedItems(k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_All, k_EUserUGCListSortOrder_LastUpdatedDesc);
							g_SteamWorkshop->subscribedCallStatus = 0;
						}
					}
#endif // STEAMWORKS
				}
			}
		}
		else if ( multiplayer == CLIENT )
		{
#ifdef STEAMWORKS
			CSteamID newSteamID;
#endif
			int numpacket;
			for ( numpacket = 0; numpacket < PACKET_LIMIT; numpacket++ )
			{
				if ( directConnect )
				{
					if ( !SDLNet_UDP_Recv(net_sock, net_packet) )
					{
						break;
					}
				}
				else
				{
#ifdef STEAMWORKS
					uint32_t packetlen = 0;
					if ( !SteamNetworking()->IsP2PPacketAvailable(&packetlen, 0) )
					{
						break;
					}
					packetlen = std::min<int>(packetlen, NET_PACKET_SIZE - 1);
					Uint32 bytesRead = 0;
					if ( !SteamNetworking()->ReadP2PPacket(net_packet->data, packetlen, &bytesRead, &newSteamID, 0) ) //TODO: Sometimes if a host closes a lobby, it can crash here for a client.
					{
						continue;
					}
					net_packet->len = packetlen;
					if ( packetlen < sizeof(DWORD) )
					{
						continue;    //TODO: Again, figure out why this is happening.
					}

					CSteamID mySteamID = SteamUser()->GetSteamID();
					if (mySteamID.ConvertToUint64() == newSteamID.ConvertToUint64())
					{
						continue;
					}
#endif
				}

				if ( handleSafePacket() )
				{
					continue;
				}

				// game start
				if (!strncmp((char*)net_packet->data, "BARONY_GAME_START", 17))
				{
					svFlags = SDLNet_Read32(&net_packet->data[17]);
					uniqueGameKey = SDLNet_Read32(&net_packet->data[21]);
					buttonCloseSubwindow(NULL);
					numplayers = MAXPLAYERS;
					introstage = 3;
					fadeout = true;
					continue;
				}

				// new player
				else if (!strncmp((char*)net_packet->data, "NEWPLAYER", 9))
				{
					client_disconnected[net_packet->data[9]] = false;
					client_classes[net_packet->data[9]] = net_packet->data[10];
					stats[net_packet->data[9]]->sex = static_cast<sex_t>(net_packet->data[11]);
					strcpy(stats[net_packet->data[9]]->name, (char*)(&net_packet->data[12]));

					char shortname[11] = { 0 };
					strncpy(shortname, stats[net_packet->data[9]]->name, 10);
					newString(&lobbyChatboxMessages, 0xFFFFFFFF, language[1388], shortname);
					continue;
				}

				// player disconnect
				else if (!strncmp((char*)net_packet->data, "PLAYERDISCONNECT", 16))
				{
					client_disconnected[net_packet->data[16]] = true;
					if ( net_packet->data[16] == 0 )
					{
						// close lobby window
						buttonCloseSubwindow(NULL);
						for ( node = button_l.first; node != NULL; node = nextnode )
						{
							nextnode = node->next;
							button = (button_t*)node->element;
							if ( button->focused )
							{
								list_RemoveNode(button->node);
							}
						}

						// create new window
						subwindow = 1;
						subx1 = xres / 2 - 256;
						subx2 = xres / 2 + 256;
						suby1 = yres / 2 - 40;
						suby2 = yres / 2 + 40;
						strcpy(subtext, language[1126]);

						// close button
						button = newButton();
						strcpy(button->label, "x");
						button->x = subx2 - 20;
						button->y = suby1;
						button->sizex = 20;
						button->sizey = 20;
						button->action = &buttonCloseSubwindow;
						button->visible = 1;
						button->focused = 1;
						button->joykey = joyimpulses[INJOY_MENU_CANCEL];

						// okay button
						button = newButton();
						strcpy(button->label, language[732]);
						button->x = subx2 - (subx2 - subx1) / 2 - 20;
						button->y = suby2 - 24;
						button->sizex = 56;
						button->sizey = 20;
						button->action = &buttonCloseSubwindow;
						button->visible = 1;
						button->focused = 1;
						button->joykey = joyimpulses[INJOY_MENU_NEXT];

						// reset multiplayer status
						multiplayer = SINGLE;
						stats[0]->sex = stats[clientnum]->sex;
						client_classes[0] = client_classes[clientnum];
						strcpy(stats[0]->name, stats[clientnum]->name);
						clientnum = 0;
						client_disconnected[0] = false;
						for ( c = 1; c < MAXPLAYERS; c++ )
						{
							client_disconnected[c] = true;
						}

						// close any existing net interfaces
						closeNetworkInterfaces();

#ifdef STEAMWORKS
						if ( !directConnect )
						{
							if ( currentLobby )
							{
								SteamMatchmaking()->LeaveLobby(*static_cast<CSteamID*>(currentLobby));
								cpp_Free_CSteamID(currentLobby); //TODO: Bugger this.
								currentLobby = NULL;
							}
						}
#endif
					}
					else
					{
						char shortname[11] = { 0 };
						strncpy(shortname, stats[net_packet->data[16]]->name, 10);
						newString(&lobbyChatboxMessages, 0xFFFFFFFF, language[1376], shortname);
					}
					continue;
				}

				// got a chat message
				else if (!strncmp((char*)net_packet->data, "CMSG", 4))
				{
					newString(&lobbyChatboxMessages, 0xFFFFFFFF, (char*)(&net_packet->data[4]));
					playSound(238, 64);
					continue;
				}

				// update svFlags
				else if (!strncmp((char*)net_packet->data, "SVFL", 4))
				{
					svFlags = SDLNet_Read32(&net_packet->data[4]);
					continue;
				}

				// keepalive
				else if (!strncmp((char*)net_packet->data, "KEEPALIVE", 9))
				{
					client_keepalive[0] = ticks;
					continue; // just a keep alive
				}
			}
		}
	}
	if ( multiplayer == SINGLE )
	{
		receivedclientnum = false;
	}

	// lobby window
	if ( lobby_window )
	{

		int hovering_selection = -1; //0 to NUM_SERVER_FLAGS used for the server flags settings, e.g. are traps, cheats, minotaur, etc enabled.
		SDL_Rect tooltip_box;

		// player info text
		for ( c = 0; c < MAXPLAYERS; ++c )
		{
			if ( client_disconnected[c] )
			{
				continue;
			}
			string charDisplayName = "";
			charDisplayName = stats[c]->name;

#ifdef STEAMWORKS
			if ( !directConnect && c != clientnum )
			{
				//printlog("\n\n/* ********* *\nc = %d", c);
				int remoteIDIndex = c;
				if ( multiplayer == SERVER && c != 0 ) //Skip the server, because that would be undefined behavior (array index of -1). //TODO: if c > clientnum instead?
				{
					remoteIDIndex--;
				}

				if ( remoteIDIndex >= 0 && steamIDRemote[remoteIDIndex] )
				{
					//printlog("remoteIDIndex = %d. Name = \"%s\"", remoteIDIndex, SteamFriends()->GetFriendPersonaName(*static_cast<CSteamID* >(steamIDRemote[remoteIDIndex])));
					charDisplayName += " (";
					charDisplayName += SteamFriends()->GetFriendPersonaName(*static_cast<CSteamID* >(steamIDRemote[remoteIDIndex]));
					charDisplayName += ")";
				}
				/*else
				{
					printlog("remoteIDIndex = %d. No name b/c remote ID is NULL", remoteIDIndex);
				}*/
			}
#endif

			if ( stats[c]->sex )
			{
				ttfPrintTextFormatted(ttf12, subx1 + 8, suby1 + 80 + 60 * c, "%d:  %s\n    %s\n    %s", c + 1, charDisplayName.c_str(), language[1322], playerClassLangEntry(client_classes[c]));
			}
			else
			{
				ttfPrintTextFormatted(ttf12, subx1 + 8, suby1 + 80 + 60 * c, "%d:  %s\n    %s\n    %s", c + 1, charDisplayName.c_str(), language[1321], playerClassLangEntry(client_classes[c]));
			}
		}

		// select gui element w/ mouse
		if ( mousestatus[SDL_BUTTON_LEFT] )
		{
			if ( mouseInBounds(subx1 + 16, subx2 - 16, suby2 - 48, suby2 - 32) )
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;

				// chatbox
				inputstr = lobbyChatbox;
				inputlen = LOBBY_CHATBOX_LENGTH - 1;
				cursorflash = ticks;
			}
			else if ( mouseInBounds(xres / 2, subx2 - 32, suby1 + 56, suby1 + 68) && multiplayer == SERVER )
			{
				mousestatus[SDL_BUTTON_LEFT] = 0;

				// lobby name
#ifdef STEAMWORKS
				inputstr = currentLobbyName;
				inputlen = 31;
#endif
				cursorflash = ticks;
			}

			// server flags
			int i;
			if ( multiplayer == SERVER )
			{
				for ( i = 0; i < NUM_SERVER_FLAGS; i++ )
				{
					if ( mouseInBounds(xres / 2 + 8 + 6, xres / 2 + 8 + 30, suby1 + 80 + i * 16, suby1 + 92 + i * 16) )
					{
						mousestatus[SDL_BUTTON_LEFT] = 0;

						// toggle flag
						svFlags ^= power(2, i);

						// update client flags
						strcpy((char*)net_packet->data, "SVFL");
						SDLNet_Write32(svFlags, &net_packet->data[4]);
						net_packet->len = 8;

						int c;
						for ( c = 1; c < MAXPLAYERS; c++ )
						{
							if ( client_disconnected[c] )
							{
								continue;
							}
							net_packet->address.host = net_clients[c - 1].host;
							net_packet->address.port = net_clients[c - 1].port;
							sendPacketSafe(net_sock, -1, net_packet, c - 1);
						}

						// update lobby data
#ifdef STEAMWORKS
						if ( !directConnect )
						{
							char svFlagsChar[16];
							snprintf(svFlagsChar, 15, "%d", svFlags);
							SteamMatchmaking()->SetLobbyData(*static_cast<CSteamID*>(currentLobby), "svFlags", svFlagsChar);
						}
#endif
					}
				}
			}

			// switch lobby type
#ifdef STEAMWORKS
			if ( !directConnect )
			{
				if ( multiplayer == SERVER )
				{
					for ( i = 0; i < 2; i++ )
					{
						if ( mouseInBounds(xres / 2 + 8 + 6, xres / 2 + 8 + 30, suby1 + 256 + i * 16, suby1 + 268 + i * 16) )
						{
							mousestatus[SDL_BUTTON_LEFT] = 0;
							switch ( i )
							{
								default:
									currentLobbyType = k_ELobbyTypePrivate;
									break;
								case 1:
									currentLobbyType = k_ELobbyTypePublic;
									break;
								/*case 2:
									currentLobbyType = k_ELobbyTypeFriendsOnly;
									// deprecated by steam, doesn't return in getLobbyList.
									break;*/
							}
							SteamMatchmaking()->SetLobbyType(*static_cast<CSteamID*>(currentLobby), currentLobbyType);
						}
					}
				}
			}
#endif
		}

		// switch textboxes with TAB
		if ( keystatus[SDL_SCANCODE_TAB] )
		{
			keystatus[SDL_SCANCODE_TAB] = 0;
#ifdef STEAMWORKS
			if ( inputstr == currentLobbyName )
			{
				inputstr = lobbyChatbox;
				inputlen = LOBBY_CHATBOX_LENGTH - 1;
			}
			else
			{
				inputstr = currentLobbyName;
				inputlen = 31;
			}
#endif
		}

		// server flag elements
		int i;
		for ( i = 0; i < NUM_SERVER_FLAGS; i++ )
		{
			char flagStringBuffer[256] = "";
			if ( i < 5 )
			{
				strncpy(flagStringBuffer, language[153 + i], 255);
			}
			else
			{
				strncpy(flagStringBuffer, language[2917 - 5 + i], 255);
			}
			if ( svFlags & power(2, i) )
			{
				ttfPrintTextFormatted(ttf12, xres / 2 + 8, suby1 + 80 + 16 * i, "[x] %s", flagStringBuffer);
			}
			else
			{
				ttfPrintTextFormatted(ttf12, xres / 2 + 8, suby1 + 80 + 16 * i, "[ ] %s", flagStringBuffer);
			}
			if (mouseInBounds((xres / 2) + 8 + 6, (xres / 2) + 8 + 30, suby1 + 80 + (i * 16), suby1 + 92 + (i * 16)))   //So many gosh dang magic numbers ._.
			{
				if ( i < 5 )
				{
					strncpy(flagStringBuffer, language[1942 + i], 255);
				}
				else
				{
					strncpy(flagStringBuffer, language[2921 - 5 + i], 255);
				}
				if (strlen(flagStringBuffer) > 0)   //Don't bother drawing a tooltip if the file doesn't say anything.
				{
					hovering_selection = i;
#ifndef STEAMWORKS
					if ( hovering_selection == 0 )
					{
						hovering_selection = -1; // don't show cheats tooltip about disabling achievements.
					}
#endif // STEAMWORKS
					tooltip_box.x = mousex + 16;
					tooltip_box.y = mousey + 8;
					tooltip_box.w = strlen(flagStringBuffer) * TTF12_WIDTH + 8; //MORE MAGIC NUMBERS. HNNGH. I can guess what they all do, but dang.
					tooltip_box.h = TTF12_HEIGHT + 8;
				}
			}
		}

		// lobby type elements
#ifdef STEAMWORKS
		if ( !directConnect )
		{
			if ( multiplayer == SERVER )
			{
				for ( i = 0; i < 2; i++ )
				{
					if ( (i == 0 && currentLobbyType == k_ELobbyTypePrivate)
						|| (i == 1 && currentLobbyType == k_ELobbyTypePublic) )
					{
						ttfPrintTextFormatted(ttf12, xres / 2 + 8, suby1 + 256 + 16 * i, "[o] %s", language[250 + i]);
					}
					else
					{
						ttfPrintTextFormatted(ttf12, xres / 2 + 8, suby1 + 256 + 16 * i, "[ ] %s", language[250 + i]);
					}
				}
			}
		}
#endif

#ifdef STEAMWORKS
		if ( !directConnect )
		{
			// server name
			drawDepressed(xres / 2, suby1 + 56, xres / 2 + 388, suby1 + 72);
			ttfPrintTextFormatted(ttf12, xres / 2 + 2, suby1 + 58, "%s", currentLobbyName);
			if ( inputstr == currentLobbyName )
			{
				if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
				{
					int x;
					TTF_SizeUTF8(ttf12, currentLobbyName, &x, NULL);
					ttfPrintTextFormatted(ttf12, xres / 2 + 2 + x, suby1 + 58, "_");
				}
			}

			// update server name
			if ( currentLobby )
			{
				const char* lobbyName = SteamMatchmaking()->GetLobbyData( *static_cast<CSteamID*>(currentLobby), "name");
				if ( lobbyName )
				{
					if ( strcmp(lobbyName, currentLobbyName) )
					{
						if ( multiplayer == CLIENT )
						{
							// update the lobby name on our end
							snprintf( currentLobbyName, 31, "%s", lobbyName );
						}
						else if ( multiplayer == SERVER )
						{
							// update the backend's copy of the lobby name
							SteamMatchmaking()->SetLobbyData(*static_cast<CSteamID*>(currentLobby), "name", currentLobbyName);
						}
					}
				}
			}
		}
#endif

		// chatbox gui elements
		drawDepressed(subx1 + 16, suby2 - 256, subx2 - 16, suby2 - 48);
		drawDepressed(subx1 + 16, suby2 - 48, subx2 - 16, suby2 - 32);

		// draw chatbox main text
		int y = suby2 - 50;
		for ( c = 0; c < 20; c++ )
		{
			node_t* node = list_Node(&lobbyChatboxMessages, list_Size(&lobbyChatboxMessages) - c - 1);
			if ( node )
			{
				string_t* str = (string_t*)node->element;
				y -= str->lines * 12;
				if ( y < suby2 - 254 )   // there were some tall messages and we're out of space
				{
					break;
				}
				ttfPrintTextFormatted(ttf12, subx1 + 18, y, str->data);
			}
			else
			{
				break;
			}
		}
		while ( list_Size(&lobbyChatboxMessages) > 20 )
		{
			// if there are too many messages to fit the chatbox, just cull them
			list_RemoveNode(lobbyChatboxMessages.first);
		}

		// handle chatbox text entry
		if ( !SDL_IsTextInputActive() )
		{
			// this is the default text entry box in this window.
			inputstr = lobbyChatbox;
			inputlen = LOBBY_CHATBOX_LENGTH - 1;
			SDL_StartTextInput();
		}
		if ( keystatus[SDL_SCANCODE_RETURN] && strlen(lobbyChatbox) > 0 )
		{
			keystatus[SDL_SCANCODE_RETURN] = 0;
			if ( multiplayer != CLIENT )
			{
				playSound(238, 64);
			}

			char shortname[11] = {0};
			strncpy(shortname, stats[clientnum]->name, 10);

			char msg[LOBBY_CHATBOX_LENGTH + 32] = { 0 };
			snprintf(msg, LOBBY_CHATBOX_LENGTH, "%s: %s", shortname, lobbyChatbox);
			if ( strlen(lobbyChatbox) > LOBBY_CHATBOX_LENGTH - strlen(shortname) - 2 )
			{
				msg[strlen(msg)] = '\n';
				int i;
				for ( i = 0; i < strlen(shortname) + 2; i++ )
				{
					snprintf((char*)(msg + strlen(msg)), (LOBBY_CHATBOX_LENGTH + 31) - strlen(msg), " ");
				}
				snprintf((char*)(msg + strlen(msg)), (LOBBY_CHATBOX_LENGTH + 31) - strlen(msg), "%s", (char*)(lobbyChatbox + LOBBY_CHATBOX_LENGTH - strlen(shortname) - 2));
			}
			if ( multiplayer != CLIENT )
			{
				newString(&lobbyChatboxMessages, 0xFFFFFFFF, msg);  // servers print their messages right away
			}
			strcpy(lobbyChatbox, "");

			// send the message
			strcpy((char*)net_packet->data, "CMSG");
			strcat((char*)(net_packet->data), msg);
			net_packet->len = 4 + strlen(msg) + 1;
			net_packet->data[net_packet->len - 1] = 0;
			if ( multiplayer == CLIENT )
			{
				net_packet->address.host = net_server.host;
				net_packet->address.port = net_server.port;
				sendPacketSafe(net_sock, -1, net_packet, 0);
			}
			else if ( multiplayer == SERVER )
			{
				int i;
				for ( i = 1; i < MAXPLAYERS; i++ )
				{
					if ( client_disconnected[i] )
					{
						continue;
					}
					net_packet->address.host = net_clients[i - 1].host;
					net_packet->address.port = net_clients[i - 1].port;
					sendPacketSafe(net_sock, -1, net_packet, i - 1);
				}
			}
		}

		// draw chatbox entry text and cursor
		ttfPrintTextFormatted(ttf12, subx1 + 18, suby2 - 46, ">%s", lobbyChatbox);
		if ( inputstr == lobbyChatbox )
		{
			if ( (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
			{
				int x;
				TTF_SizeUTF8(ttf12, lobbyChatbox, &x, NULL);
				ttfPrintTextFormatted(ttf12, subx1 + 18 + x + TTF12_WIDTH, suby2 - 46, "_");
			}
		}

		if (hovering_selection > -1)
		{
			drawTooltip(&tooltip_box);
			char flagStringBuffer[256] = "";
			if ( hovering_selection < 5 )
			{
				strncpy(flagStringBuffer, language[1942 + hovering_selection], 255);
			}
			else
			{
				strncpy(flagStringBuffer, language[2921 - 5 + hovering_selection], 255);
			}
			if (hovering_selection < NUM_SERVER_FLAGS)
			{
				ttfPrintTextFormatted(ttf12, tooltip_box.x + 4, tooltip_box.y + 4, flagStringBuffer);
			}
		}
#ifdef STEAMWORKS
		// draw server workshop mod list
		if ( !directConnect && currentLobby )
		{
			const char* serverNumModsChar = SteamMatchmaking()->GetLobbyData(*static_cast<CSteamID*>(currentLobby), "svNumMods");
			int serverNumModsLoaded = atoi(serverNumModsChar);
			if ( serverNumModsLoaded > 0 )
			{
				char tagName[32];
				std::vector<std::string> serverFileIdsLoaded;
				std::string modList = "Clients can click '";
				modList.append(language[2984]).append("' and \n'").append(language[2985]);
				modList.append("' to automatically subscribe and \n");
				modList.append("mount workshop items loaded in the host's lobby.\n\n");
				modList.append("All clients should be running the same mod load order\n");
				modList.append("to prevent any crashes or undefined behavior.\n\n");
				modList.append("Game client may need to be closed to install and detect\nnew subscriptions due to Workshop limitations.\n");
				int numToolboxLines = 9;
				bool itemNeedsSubscribing = false;
				bool itemNeedsMounting = false;
				Uint32 modsStatusColor = uint32ColorBaronyBlue(*mainsurface);
				bool modListOutOfOrder = false;
				for ( int lines = 0; lines < serverNumModsLoaded; ++lines )
				{
					snprintf(tagName, 32, "svMod%d", lines);
					const char* serverModFileID = SteamMatchmaking()->GetLobbyData(*static_cast<CSteamID*>(currentLobby), tagName);
					if ( strcmp(serverModFileID, "") )
					{
						if ( gamemodsCheckIfSubscribedAndDownloadedFileID(atoi(serverModFileID)) == false )
						{
							modList.append("Workshop item not subscribed or installed: ");
							modList.append(serverModFileID);
							modList.append("\n");
							itemNeedsSubscribing = true;
							++numToolboxLines;
						}
						else if ( gamemodsCheckFileIDInLoadedPaths(atoi(serverModFileID)) == false )
						{
							modList.append("Workshop item downloaded but not loaded: ");
							modList.append(serverModFileID);
							modList.append("\n");
							itemNeedsMounting = true;
							++numToolboxLines;
						}
						serverFileIdsLoaded.push_back(serverModFileID);
					}
				}
				if ( itemNeedsSubscribing )
				{
					for ( node = button_l.first; node != NULL; node = nextnode )
					{
						nextnode = node->next;
						button = (button_t*)node->element;
						if ( button->action == &buttonGamemodsMountHostsModFiles )
						{
							button->visible = 0;
						}
						if ( button->action == &buttonGamemodsSubscribeToHostsModFiles )
						{
							button->visible = 1;
						}
					}
				}
				else
				{
					if ( itemNeedsMounting )
					{
						modsStatusColor = uint32ColorOrange(*mainsurface);
						for ( node = button_l.first; node != NULL; node = nextnode )
						{
							nextnode = node->next;
							button = (button_t*)node->element;
							if ( button->action == &buttonGamemodsMountHostsModFiles )
							{
								button->visible = 1;
							}
							if ( button->action == &buttonGamemodsSubscribeToHostsModFiles )
							{
								button->visible = 0;
							}
						}
					}
					else if ( gamemodsIsClientLoadOrderMatchingHost(serverFileIdsLoaded)
						&& serverFileIdsLoaded.size() == gamemods_workshopLoadedFileIDMap.size() )
					{
						modsStatusColor = uint32ColorGreen(*mainsurface);
						for ( node = button_l.first; node != NULL; node = nextnode )
						{
							nextnode = node->next;
							button = (button_t*)node->element;
							if ( button->action == &buttonGamemodsMountHostsModFiles )
							{
								button->visible = 0;
							}
							if ( button->action == &buttonGamemodsSubscribeToHostsModFiles )
							{
								button->visible = 0;
							}
						}
					}
					else
					{
						modsStatusColor = uint32ColorOrange(*mainsurface);
						modListOutOfOrder = true;
						for ( node = button_l.first; node != NULL; node = nextnode )
						{
							nextnode = node->next;
							button = (button_t*)node->element;
							if ( button->action == &buttonGamemodsMountHostsModFiles )
							{
								button->visible = 1;
							}
							if ( button->action == &buttonGamemodsSubscribeToHostsModFiles )
							{
								button->visible = 0;
							}
						}
					}
				}
				ttfPrintTextFormattedColor(ttf12, xres / 2 + 8, suby1 + 304, modsStatusColor, "%2d mod(s) loaded by host (?)", serverNumModsLoaded);
				std::string modStatusString;
				if ( itemNeedsSubscribing )
				{
					modStatusString = "Your client is missing mods in subscriptions";
				}
				else if ( itemNeedsMounting )
				{
					modStatusString = "Your client is missing mods in load order";
				}
				else if ( modListOutOfOrder )
				{
					modStatusString = "Your client mod list is out of order";
				}
				else
				{
					modStatusString = "Your client has complete mod list";
				}

				int lineStartListLoadedMods = numToolboxLines;
				numToolboxLines += serverNumModsLoaded + 3;
				std::string clientModString = "Your client mod list:\n";
				for ( std::vector<std::pair<std::string, uint64>>::iterator it = gamemods_workshopLoadedFileIDMap.begin(); 
					it != gamemods_workshopLoadedFileIDMap.end(); ++it )
				{
					clientModString.append(std::to_string(it->second));
					clientModString.append("\n");
				}
				std::string serverModString = "Server mod list:\n";
				for ( std::vector<std::string>::iterator it = serverFileIdsLoaded.begin(); it != serverFileIdsLoaded.end(); ++it )
				{
					serverModString.append(*it);
					serverModString.append("\n");
				}

				ttfPrintTextFormattedColor(ttf12, xres / 2 + 8, suby1 + 320, modsStatusColor, "%s", modStatusString.c_str());
				if ( mouseInBounds(xres / 2 + 8, xres / 2 + 8 + 31 * TTF12_WIDTH, suby1 + 304, suby1 + 320 + TTF12_HEIGHT) )
				{
					tooltip_box.w = 60 * TTF12_WIDTH + 8;
					tooltip_box.x = mousex - 16 - tooltip_box.w;
					tooltip_box.y = mousey + 8;
					tooltip_box.h = numToolboxLines * TTF12_HEIGHT + 8;
					drawTooltip(&tooltip_box);
					ttfPrintTextFormatted(ttf12, tooltip_box.x + 4, tooltip_box.y + 8, "%s", modList.c_str());
					ttfPrintTextFormattedColor(ttf12, tooltip_box.x + 4, tooltip_box.y + lineStartListLoadedMods * TTF12_HEIGHT + 16,
						modsStatusColor, "%s", clientModString.c_str());
					ttfPrintTextFormattedColor(ttf12, tooltip_box.x + 4 + 24 * TTF12_WIDTH, tooltip_box.y + lineStartListLoadedMods * TTF12_HEIGHT + 16, 
						modsStatusColor, "%s", serverModString.c_str());
				}
				if ( multiplayer == CLIENT && itemNeedsMounting )
				{
					if ( g_SteamWorkshop->subscribedCallStatus == 1 )
					{
						ttfPrintTextFormattedColor(ttf12, subx2 - 64 * TTF12_WIDTH, suby2 - 4 - TTF12_HEIGHT, uint32ColorOrange(*mainsurface), 
							"retrieving data...");
					}
					else if ( g_SteamWorkshop->subscribedCallStatus == 2 )
					{
						ttfPrintTextFormattedColor(ttf12, subx2 - 64 * TTF12_WIDTH, suby2 - 4 - TTF12_HEIGHT, uint32ColorOrange(*mainsurface),
							"please retry mount operation.");
					}
					else
					{
						ttfPrintTextFormattedColor(ttf12, subx2 - 64 * TTF12_WIDTH, suby2 - 4 - TTF12_HEIGHT, uint32ColorOrange(*mainsurface),
							"press mount button.");
					}
				}
			}
		}
#endif // STEAMWORKS

		// handle keepalive timeouts (lobby)
		if ( multiplayer == SERVER )
		{
			int i;
			for ( i = 1; i < MAXPLAYERS; i++ )
			{
				if ( client_disconnected[i] )
				{
					continue;
				}
				if ( ticks - client_keepalive[i] > TICKS_PER_SECOND * 30 )
				{
					client_disconnected[i] = true;
					strncpy((char*)(net_packet->data), "PLAYERDISCONNECT", 16);
					net_packet->data[16] = i;
					net_packet->len = 17;
					for ( c = 1; c < MAXPLAYERS; c++ )
					{
						if ( client_disconnected[c] )
						{
							continue;
						}
						net_packet->address.host = net_clients[c - 1].host;
						net_packet->address.port = net_clients[c - 1].port;
						sendPacketSafe(net_sock, -1, net_packet, c - 1);
					}
					char shortname[11] = { 0 };
					strncpy(shortname, stats[i]->name, 10);
					newString(&lobbyChatboxMessages, 0xFFFFFFFF, language[1376], shortname);
					continue;
				}
			}
		}
		else if ( multiplayer == CLIENT )
		{
			if ( ticks - client_keepalive[0] > TICKS_PER_SECOND * 30 )
			{
				buttonDisconnect(NULL);
				openFailedConnectionWindow(3); // lost connection to server box
			}
		}

		// send keepalive messages every second
		if ( ticks % (TICKS_PER_SECOND * 1) == 0 && multiplayer != SINGLE )
		{
			strcpy((char*)net_packet->data, "KEEPALIVE");
			net_packet->data[9] = clientnum;
			net_packet->len = 10;
			if ( multiplayer == CLIENT )
			{
				net_packet->address.host = net_server.host;
				net_packet->address.port = net_server.port;
				sendPacketSafe(net_sock, -1, net_packet, 0);
			}
			else if ( multiplayer == SERVER )
			{
				int i;
				for ( i = 1; i < MAXPLAYERS; i++ )
				{
					if ( client_disconnected[i] )
					{
						continue;
					}
					net_packet->address.host = net_clients[i - 1].host;
					net_packet->address.port = net_clients[i - 1].port;
					sendPacketSafe(net_sock, -1, net_packet, i - 1);
				}
			}
		}
	}

	// statistics window
	if ( score_window )
	{
		// draw button label... shamelessly hacked together from "multiplayer scores toggle button" initialisation...
		int toggleText_x = subx2 - 44 - strlen("show multiplayer") * 12;
		int toggleText_y = suby1 + 4 ;
		int w = 0;
		int h = 0;
		list_t* scoresPtr = &topscores;
		if ( !scoreDisplayMultiplayer )
		{
			TTF_SizeUTF8(ttf12, "show multiplayer", &w, &h);
			ttfPrintText(ttf12, toggleText_x + (strlen("show multiplayer") * 12 + 8 - w) / 2, toggleText_y + (20 - h) / 2 + 3, "show multiplayer");
		}
		else
		{
			scoresPtr = &topscoresMultiplayer;
			TTF_SizeUTF8(ttf12, "show solo", &w, &h);
			ttfPrintText(ttf12, toggleText_x + (strlen("show multiplayer") * 12 + 8 - w) / 2, toggleText_y + (20 - h) / 2 + 3, "show solo");
		}
		if ( !list_Size(scoresPtr) )
		{
#define NOSCORESSTR language[1389]
			ttfPrintTextFormatted(ttf16, xres / 2 - strlen(NOSCORESSTR) * 9, yres / 2 - 9, NOSCORESSTR);
		}
		else
		{
			if ( scoreDisplayMultiplayer )
			{
				ttfPrintTextFormatted(ttf16, subx1 + 8, suby1 + 8, "%s - %d / %d", language[2958], score_window, list_Size(&topscoresMultiplayer));
			}
			else
			{
				ttfPrintTextFormatted(ttf16, subx1 + 8, suby1 + 8, "%s - %d / %d", language[1390], score_window, list_Size(&topscores));
			}

			// draw character window
			if (players[clientnum] != nullptr && players[clientnum]->entity != nullptr)
			{
				camera_charsheet.x = players[clientnum]->entity->x / 16.0 + 1.118 * cos(camera_charsheet_offsetyaw); // + 1
				camera_charsheet.y = players[clientnum]->entity->y / 16.0 + 1.118 * sin(camera_charsheet_offsetyaw); // -.5
				camera_charsheet.z = players[clientnum]->entity->z * 2;
				camera_charsheet.ang = atan2(players[clientnum]->entity->y / 16.0 - camera_charsheet.y, players[clientnum]->entity->x / 16.0 - camera_charsheet.x);
				camera_charsheet.vang = PI / 24;
				camera_charsheet.winw = 400;
				camera_charsheet.winy = suby1 + 32;
				camera_charsheet.winh = suby2 - 96 - camera_charsheet.winy;
				camera_charsheet.winx = subx1 + 32;
				pos.x = camera_charsheet.winx;
				pos.y = camera_charsheet.winy;
				pos.w = camera_charsheet.winw;
				pos.h = camera_charsheet.winh;
				drawRect(&pos, 0, 255);
				b = players[clientnum]->entity->flags[BRIGHT];
				players[clientnum]->entity->flags[BRIGHT] = true;
				if ( !players[clientnum]->entity->flags[INVISIBLE] )
				{
					real_t ofov = fov;
					fov = 50;
					glDrawVoxel(&camera_charsheet, players[clientnum]->entity, REALCOLORS);
					fov = ofov;
				}
				players[clientnum]->entity->flags[BRIGHT] = b;
				c = 0;
				for ( node = players[clientnum]->entity->children.first; node != NULL; node = node->next )
				{
					if ( c == 0 )
					{
						c++;
					}
					entity = (Entity*) node->element;
					if ( !entity->flags[INVISIBLE] )
					{
						b = entity->flags[BRIGHT];
						entity->flags[BRIGHT] = true;
						real_t ofov = fov;
						fov = 50;
						glDrawVoxel(&camera_charsheet, entity, REALCOLORS);
						fov = ofov;
						entity->flags[BRIGHT] = b;
					}
					c++;
				}
				SDL_Rect rotateBtn;
				rotateBtn.w = 24;
				rotateBtn.h = 24;
				rotateBtn.x = camera_charsheet.winx + camera_charsheet.winw - rotateBtn.w;
				rotateBtn.y = camera_charsheet.winy + camera_charsheet.winh - rotateBtn.h;
				drawWindow(rotateBtn.x, rotateBtn.y, rotateBtn.x + rotateBtn.w, rotateBtn.y + rotateBtn.h);
				if ( mouseInBounds(rotateBtn.x, rotateBtn.x + rotateBtn.w, rotateBtn.y, rotateBtn.y + rotateBtn.h) )
				{
					if ( mousestatus[SDL_BUTTON_LEFT] )
					{
						camera_charsheet_offsetyaw += 0.05;
						if ( camera_charsheet_offsetyaw > 2 * PI )
						{
							camera_charsheet_offsetyaw -= 2 * PI;
						}
						drawDepressed(rotateBtn.x, rotateBtn.y, rotateBtn.x + rotateBtn.w, rotateBtn.y + rotateBtn.h);
					}
				}
				ttfPrintText(ttf12, rotateBtn.x + 6, rotateBtn.y + 6, ">");

				rotateBtn.x = camera_charsheet.winx + camera_charsheet.winw - rotateBtn.w * 2 - 4;
				rotateBtn.y = camera_charsheet.winy + camera_charsheet.winh - rotateBtn.h;
				drawWindow(rotateBtn.x, rotateBtn.y, rotateBtn.x + rotateBtn.w, rotateBtn.y + rotateBtn.h);
				if ( mouseInBounds(rotateBtn.x, rotateBtn.x + rotateBtn.w, rotateBtn.y, rotateBtn.y + rotateBtn.h) )
				{
					if ( mousestatus[SDL_BUTTON_LEFT] )
					{
						camera_charsheet_offsetyaw -= 0.05;
						if ( camera_charsheet_offsetyaw < 0.f )
						{
							camera_charsheet_offsetyaw += 2 * PI;
						}
						drawDepressed(rotateBtn.x, rotateBtn.y, rotateBtn.x + rotateBtn.w, rotateBtn.y + rotateBtn.h);
					}
				}
				ttfPrintText(ttf12, rotateBtn.x + 4, rotateBtn.y + 6, "<");
			}

			// print name and class
			if ( victory )
			{
				ttfPrintTextFormatted(ttf16, subx1 + 448, suby1 + 40, language[1391]);
				ttfPrintTextFormatted(ttf16, subx1 + 448, suby1 + 56, "%s", stats[clientnum]->name);
				if ( victory == 1 )
				{
					ttfPrintTextFormatted(ttf16, subx1 + 448, suby1 + 72, language[1392]);
				}
				else if ( victory == 2 )
				{
					ttfPrintTextFormatted(ttf16, subx1 + 448, suby1 + 72, language[1393]);
				}
				else if ( victory == 3 )
				{
					ttfPrintTextFormatted(ttf16, subx1 + 448, suby1 + 72, language[2911]);
				}
			}
			else
			{
				ttfPrintTextFormatted(ttf16, subx1 + 448, suby1 + 40, language[1394]);
				ttfPrintTextFormatted(ttf16, subx1 + 448, suby1 + 56, "%s", stats[clientnum]->name);

				char classname[32];
				strcpy(classname, playerClassLangEntry(client_classes[0]));
				classname[0] -= 32;
				ttfPrintTextFormatted(ttf16, subx1 + 448, suby1 + 72, language[1395], classname);
			}

			// print total score
			node = list_Node(scoresPtr, score_window - 1);
			if ( node )
			{
				score_t* score = (score_t*)node->element;
				ttfPrintTextFormatted(ttf16, subx1 + 448, suby1 + 104, language[1404], totalScore(score));
			}

			// print character stats
			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 128, language[359], stats[clientnum]->LVL, playerClassLangEntry(client_classes[clientnum]));
			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 140, language[1396], stats[clientnum]->EXP);
			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 152, language[1397], stats[clientnum]->GOLD);
			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 164, language[361], currentlevel);

			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 188, language[1398], statGetSTR(stats[clientnum]), stats[clientnum]->STR);
			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 200, language[1399], statGetDEX(stats[clientnum]), stats[clientnum]->DEX);
			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 212, language[1400], statGetCON(stats[clientnum]), stats[clientnum]->CON);
			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 224, language[1401], statGetINT(stats[clientnum]), stats[clientnum]->INT);
			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 236, language[1402], statGetPER(stats[clientnum]), stats[clientnum]->PER);
			ttfPrintTextFormatted(ttf12, subx1 + 456, suby1 + 248, language[1403], statGetCHR(stats[clientnum]), stats[clientnum]->CHR);

			// time
			Uint32 sec = (completionTime / TICKS_PER_SECOND) % 60;
			Uint32 min = ((completionTime / TICKS_PER_SECOND) / 60) % 60;
			Uint32 hour = ((completionTime / TICKS_PER_SECOND) / 60) / 60;
			ttfPrintTextFormatted(ttf12, subx1 + 32, suby2 - 80, "%s: %02d:%02d:%02d. %s:", language[1405], hour, min, sec, language[1406]);
			if ( !conductPenniless && !conductFoodless && !conductVegetarian && !conductIlliterate && !conductGameChallenges[CONDUCT_HARDCORE]
				&& !conductGameChallenges[CONDUCT_CHEATS_ENABLED]
				&& !conductGameChallenges[CONDUCT_MODDED]
				&& !conductGameChallenges[CONDUCT_BRAWLER]
				&& !conductGameChallenges[CONDUCT_BLESSED_BOOTS_SPEED]
				&& !conductGameChallenges[CONDUCT_BOOTS_SPEED] )
			{
				ttfPrintText(ttf12, subx1 + 32, suby2 - 64, language[1407]);
			}
			else
			{
				int b = 0;
				strcpy(tempstr, " ");
				if ( conductPenniless )
				{
					strcat(tempstr, language[1408]);
					++b;
				}
				if ( conductFoodless )
				{
					if ( b > 0 )
					{
						strcat(tempstr, ", ");
					}
					strcat(tempstr, language[1409]);
					++b;
				}
				if ( conductVegetarian )
				{
					if ( b > 0 )
					{
						strcat(tempstr, ", ");
					}
					strcat(tempstr, language[1410]);
					++b;
				}
				if ( conductIlliterate )
				{
					if ( b > 0 )
					{
						strcat(tempstr, ", ");
					}
					strcat(tempstr, language[1411]);
					++b;
				}
				for ( int c = 0; c < NUM_CONDUCT_CHALLENGES; ++c )
				{
					if ( conductGameChallenges[c] != 0 )
					{
						if ( b > 0 )
						{
							strcat(tempstr, ", ");
						}
						if ( b > 0 && b % 4 == 0 )
						{
							strcat(tempstr, "\n ");
						}
						strcat(tempstr, language[2925 + c]);
						++b;
					}
				}
				/*	if ( b > 0 )
				{
					tempstr[strlen(tempstr) - 2] = 0;
				}*/
				ttfPrintTextFormatted(ttf12, subx1 + 20, suby2 - 64, tempstr);
			}

			// kills
			int x = 0, y = 0;
			ttfPrintText(ttf12, subx1 + 456, suby1 + 272, language[1412]);
			bool nokills = true;
			for ( x = 0; x < NUMMONSTERS; x++ )
			{
				if ( kills[x] )
				{
					nokills = false;
					if ( kills[x] > 1 )
					{
						if ( x < KOBOLD )
						{
							ttfPrintTextFormatted(ttf12, subx1 + 456 + (y / 14) * 180, suby1 + 296 + (y % 14) * 12, "%d %s", kills[x], language[111 + x]);
						}
						else
						{
							ttfPrintTextFormatted(ttf12, subx1 + 456 + (y / 14) * 180, suby1 + 296 + (y % 14) * 12, "%d %s", kills[x], language[2050 + (x - KOBOLD)]);
						}
					}
					else
					{
						if ( x < KOBOLD )
						{
							ttfPrintTextFormatted(ttf12, subx1 + 456 + (y / 14) * 180, suby1 + 296 + (y % 14) * 12, "%d %s", kills[x], language[90 + x]);
						}
						else
						{
							ttfPrintTextFormatted(ttf12, subx1 + 456 + (y / 14) * 180, suby1 + 296 + (y % 14) * 12, "%d %s", kills[x], language[2000 + (x - KOBOLD)]);
						}
					}
					y++;
				}
			}
			if ( nokills )
			{
				ttfPrintText(ttf12, subx1 + 456, suby1 + 296, language[1413]);
			}
		}
	}
	else
	{
		scoreDisplayMultiplayer = false;
	}

	if ( gamemods_window != 0 )
	{
		int filenameMaxLength = 24;
		int filename_padx = subx1 + 16;
		int filename_pady = suby1 + 32;
		int numFileEntries = 10;

		int filename_padx2 = filename_padx + filenameMaxLength * TTF12_WIDTH + 8;
		int filename_pady2 = filename_pady + numFileEntries * TTF12_HEIGHT + 8;
#ifdef STEAMWORKS
		if ( gamemods_window == 1 || gamemods_window == 2 || gamemods_window == 5 )
		{
			if ( !currentDirectoryFiles.empty() )
			{
				int lineNumber = 0;
				std::string line;
				std::list<std::string>::const_iterator it = currentDirectoryFiles.begin();
				std::advance(it, gamemods_window_scroll);

				drawWindow(filename_padx, filename_pady - 2,
					filename_padx2, filename_pady2);

				SDL_Rect pos;
				pos.x = filename_padx;
				pos.y = filename_pady - 2 + std::max(gamemods_window_fileSelect - 1, 0) * TTF12_HEIGHT;
				pos.w = filenameMaxLength * TTF12_WIDTH + 8;
				pos.h = TTF12_HEIGHT;
				if ( gamemods_window_fileSelect != 0 )
				{
					drawRect(&pos, SDL_MapRGB(mainsurface->format, 64, 64, 64), 255);
				}

				for ( ; it != currentDirectoryFiles.end() && lineNumber < numFileEntries; ++it )
				{
					line = *it;
					line = line.substr(0, filenameMaxLength);
					ttfPrintTextFormatted(ttf12, filename_padx, filename_pady + lineNumber * TTF12_HEIGHT, "%s", line.c_str());
					++lineNumber;
				}
				int entriesToScroll = std::max(static_cast<int>((currentDirectoryFiles.size() / numFileEntries) - 1), 0);
				entriesToScroll = entriesToScroll * numFileEntries + (currentDirectoryFiles.size() % numFileEntries);

				if ( mouseInBounds(filename_padx - 4, filename_padx2, 
					filename_pady, filename_pady2) && currentDirectoryFiles.size() > numFileEntries )
				{
					if ( mousestatus[SDL_BUTTON_WHEELUP] )
					{
						gamemods_window_scroll = std::max(gamemods_window_scroll - 1, 0);
						mousestatus[SDL_BUTTON_WHEELUP] = 0;
					}
					if ( mousestatus[SDL_BUTTON_WHEELDOWN] )
					{
						gamemods_window_scroll = std::min(gamemods_window_scroll + 1, entriesToScroll);
						mousestatus[SDL_BUTTON_WHEELDOWN] = 0;
					}
				}
				for ( int i = 1; i <= numFileEntries; ++i )
				{
					if ( mouseInBounds(filename_padx - 4, filename_padx2,
						filename_pady - 2, filename_pady - 2 + i * TTF12_HEIGHT) )
					{
						if ( mousestatus[SDL_BUTTON_LEFT] )
						{
							gamemods_window_fileSelect = i;
							mousestatus[SDL_BUTTON_LEFT] = 0;
						}
					}
				}
				if ( !directoryToUpload.empty() )
				{
					ttfPrintTextFormatted(ttf12, filename_padx, filename_pady2 + TTF12_HEIGHT, "folder to upload: %s", directoryToUpload.c_str());
				}
			}
		}
		if ( gamemods_window == 2 || gamemods_window == 5 )
		{
			numFileEntries = 20;
			filename_padx = subx2 - (filenameMaxLength * TTF12_WIDTH + 16);
			filename_padx2 = subx2 - 16;
			filename_pady = filename_pady2 + 2 * TTF12_HEIGHT;
			filename_pady2 = filename_pady + numFileEntries * TTF12_HEIGHT + 2;
			if ( !directoryFilesListToUpload.empty() )
			{
				ttfPrintTextFormatted(ttf12, filename_padx, filename_pady - TTF12_HEIGHT, "file preview in folder:");
				drawWindow(filename_padx, filename_pady - 2,
					filename_padx2, filename_pady2);
				int lineNumber = 0;
				std::string line;
				for ( std::list<std::string>::const_iterator it = directoryFilesListToUpload.begin(); it != directoryFilesListToUpload.end() && lineNumber < 20; ++it )
				{
					line = *it;
					if ( line.size() >= filenameMaxLength )
					{
						line = line.substr(0, filenameMaxLength - 3);
						line.append("..");
					}
					else
					{
						line = line.substr(0, filenameMaxLength);
					}
					ttfPrintTextFormatted(ttf12, filename_padx, filename_pady + lineNumber * TTF12_HEIGHT, "%s", line.c_str());
					++lineNumber;
				}
			}

			int status_padx = subx1 + 16;
			int status_pady = filename_pady;
			if ( gamemods_uploadStatus != 0 && g_SteamWorkshop )
			{
				status_pady += 3 * TTF12_HEIGHT;
				if ( gamemods_window == 2 )
				{
					if ( g_SteamWorkshop->SubmitItemUpdateResult.m_eResult == 0
						&& gamemods_uploadStatus < 5 )
					{
						switch ( g_SteamWorkshop->createItemResult.m_eResult )
						{
							case 0:
								ttfPrintTextFormatted(ttf12, status_padx, status_pady, "creating item...");
								break;
							case k_EResultOK:
								if ( gamemods_uploadStatus < 2 )
								{
									for ( node = button_l.first; node != NULL; node = nextnode )
									{
										nextnode = node->next;
										button = (button_t*)node->element;
										if ( button->action == &buttonGamemodsPrepareWorkshopItemUpload )
										{
											button->visible = false;
										}
									}
									gamemods_uploadStatus = 2;
									g_SteamWorkshop->StartItemUpdate();
								}
								else
								{
									if ( g_SteamWorkshop->UGCUpdateHandle == 0 )
									{
										ttfPrintTextFormattedColor(ttf12, status_padx, status_pady, uint32ColorOrange(*mainsurface), "item created! awaiting file handle...");
									}
									else
									{
										if ( gamemods_uploadStatus == 2 )
										{
											gamemods_uploadStatus = 3;
											// set item fields button
											button = newButton();
											strcpy(button->label, "set item fields");
											button->x = subx1 + 16;
											button->y = suby1 + TTF12_HEIGHT * 34;
											button->sizex = 16 * TTF12_WIDTH + 8;
											button->sizey = 32;
											button->action = &buttonGamemodsSetWorkshopItemFields;
											button->visible = 1;
											button->focused = 1;
											gamemods_currentEditField = 0;
										}
										ttfPrintTextFormattedColor(ttf12, status_padx, status_pady, uint32ColorGreen(*mainsurface), "item and file handle create success!");
									}
								}
								break;
							default:
								// error in createItem!
								ttfPrintTextFormatted(ttf12, status_padx, status_pady, "error in creating item!");
								break;
						}
						status_pady += 2 * TTF12_HEIGHT;
						if ( gamemods_uploadStatus >= 3 && g_SteamWorkshop->SubmitItemUpdateResult.m_eResult == 0 )
						{
							for ( int fields = 0; fields < 2; ++fields )
							{
								status_pady += TTF12_HEIGHT;
								drawDepressed(status_padx, status_pady - 4, status_padx + 32 * TTF12_WIDTH, status_pady + TTF12_HEIGHT);
								switch ( fields )
								{
									case 0:
										ttfPrintText(ttf12, status_padx + 8, status_pady - TTF12_HEIGHT, "Enter a title:");
										if ( gamemods_uploadStatus == 3 && gamemods_workshopSetPropertyReturn[0] )
										{
											ttfPrintTextColor(ttf12, status_padx + 20 * TTF12_WIDTH, status_pady - TTF12_HEIGHT, uint32ColorGreen(*mainsurface), true, "success set");
										}
										ttfPrintText(ttf12, status_padx + 8, status_pady, gamemods_uploadTitle);
										break;
									case 1:
										ttfPrintText(ttf12, status_padx + 8, status_pady - TTF12_HEIGHT, "Enter description:");
										if ( gamemods_uploadStatus == 3 && gamemods_workshopSetPropertyReturn[1] )
										{

										}
										ttfPrintText(ttf12, status_padx + 8, status_pady, gamemods_uploadDescription);
										break;
									default:
										break;
								}
								if ( gamemods_uploadStatus == 4 )
								{
									if ( gamemods_workshopSetPropertyReturn[fields] )
									{
										ttfPrintTextColor(ttf12, status_padx + 20 * TTF12_WIDTH, status_pady - TTF12_HEIGHT, uint32ColorGreen(*mainsurface), true, "success set");
									}
									else
									{
										ttfPrintTextColor(ttf12, status_padx + 20 * TTF12_WIDTH, status_pady - TTF12_HEIGHT, uint32ColorRed(*mainsurface), true, "error!");
									}
								}

								if ( mouseInBounds(status_padx, status_padx + 32 * TTF12_WIDTH, status_pady - 4, status_pady + TTF12_HEIGHT) )
								{
									if ( mousestatus[SDL_BUTTON_LEFT] )
									{
										switch ( fields )
										{
											case 0:
												inputstr = gamemods_uploadTitle;
												gamemods_currentEditField = 0;
												break;
											case 1:
												inputstr = gamemods_uploadDescription;
												gamemods_currentEditField = 1;
												break;
											default:
												break;
										}
										mousestatus[SDL_BUTTON_LEFT] = 0;
									}
								}

								if ( gamemods_uploadStatus == 3 && !SDL_IsTextInputActive() )
								{
									inputstr = gamemods_uploadTitle;
									SDL_StartTextInput();
								}
								inputlen = 30;
								if ( SDL_IsTextInputActive() && gamemods_currentEditField == fields
									&& (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
								{
									int x;
									TTF_SizeUTF8(ttf12, inputstr, &x, NULL);
									ttfPrintText(ttf12, status_padx + x + 8, status_pady, "_");
								}
								status_pady += 2 * TTF12_HEIGHT;
							}
							if ( gamemods_uploadStatus >= 4 )
							{
								if ( gamemods_workshopSetPropertyReturn[2] )
								{
									ttfPrintTextColor(ttf12, status_padx, status_pady, uint32ColorGreen(*mainsurface), true, "folder path success set");
								}
								else
								{
									ttfPrintTextColor(ttf12, status_padx, status_pady, uint32ColorRed(*mainsurface), true, "error in folder path!");
								}
							}

							status_pady += 2 * TTF12_HEIGHT;
							// set some workshop item tags
							ttfPrintText(ttf12, status_padx, status_pady, "Select workshop tags");
							int tag_padx = status_padx;
							int tag_pady = status_pady + TTF12_HEIGHT;
							int tag_padx1 = tag_padx + 20 * TTF12_WIDTH;
							gamemodsDrawWorkshopItemTagToggle("dungeons", tag_padx, tag_pady);
							gamemodsDrawWorkshopItemTagToggle("textures", tag_padx1, tag_pady);
							tag_pady += TTF12_HEIGHT + 4;
							gamemodsDrawWorkshopItemTagToggle("models", tag_padx, tag_pady);
							gamemodsDrawWorkshopItemTagToggle("gameplay", tag_padx1, tag_pady);
							tag_pady += TTF12_HEIGHT + 4;
							gamemodsDrawWorkshopItemTagToggle("audio", tag_padx, tag_pady);
							gamemodsDrawWorkshopItemTagToggle("translations", tag_padx1, tag_pady);
							tag_pady += TTF12_HEIGHT + 4;
							gamemodsDrawWorkshopItemTagToggle("misc", tag_padx, tag_pady);
						}
					}
				}
				else if ( gamemods_window == 5 && g_SteamWorkshop->m_myWorkshopItemToModify.m_nPublishedFileId != 0 && gamemods_uploadStatus < 5 )
				{
					std::string line = g_SteamWorkshop->m_myWorkshopItemToModify.m_rgchTitle;
					if ( line.size() > filenameMaxLength )
					{
						line = line.substr(0, filenameMaxLength - 2);
						line.append("..");
					}
					status_pady += 2 * TTF12_HEIGHT;
					ttfPrintTextFormattedColor(ttf12, status_padx + 8, status_pady, uint32ColorBaronyBlue(*mainsurface), "Title:");
					status_pady += TTF12_HEIGHT;
					ttfPrintTextFormatted(ttf12, status_padx + 8, status_pady, "%s", line.c_str());

					line = g_SteamWorkshop->m_myWorkshopItemToModify.m_rgchDescription;
					if ( line.size() > filenameMaxLength )
					{
						line = line.substr(0, filenameMaxLength - 2);
						line.append("..");
					}
					status_pady += TTF12_HEIGHT;
					ttfPrintTextFormattedColor(ttf12, status_padx + 8, status_pady, uint32ColorBaronyBlue(*mainsurface), "Description:");
					status_pady += TTF12_HEIGHT;
					ttfPrintTextFormatted(ttf12, status_padx + 8, status_pady, "%s", line.c_str());

					status_pady += 2 * TTF12_HEIGHT;
					// set some workshop item tags
					ttfPrintText(ttf12, status_padx, status_pady, "Modify workshop tags");
					int tag_padx = status_padx;
					int tag_pady = status_pady + TTF12_HEIGHT;
					int tag_padx1 = tag_padx + 20 * TTF12_WIDTH;
					gamemodsDrawWorkshopItemTagToggle("dungeons", tag_padx, tag_pady);
					gamemodsDrawWorkshopItemTagToggle("textures", tag_padx1, tag_pady);
					tag_pady += TTF12_HEIGHT + 4;
					gamemodsDrawWorkshopItemTagToggle("models", tag_padx, tag_pady);
					gamemodsDrawWorkshopItemTagToggle("gameplay", tag_padx1, tag_pady);
					tag_pady += TTF12_HEIGHT + 4;
					gamemodsDrawWorkshopItemTagToggle("audio", tag_padx, tag_pady);
					gamemodsDrawWorkshopItemTagToggle("translations", tag_padx1, tag_pady);
					tag_pady += TTF12_HEIGHT + 4;
					gamemodsDrawWorkshopItemTagToggle("misc", tag_padx, tag_pady);

					status_pady += 6 * TTF12_HEIGHT;
					if ( directoryFilesListToUpload.empty() )
					{
						ttfPrintTextFormattedColor(ttf12, status_padx, status_pady, uint32ColorGreen(*mainsurface), "Only Workshop tags will be updated.");
					}
					else
					{
						ttfPrintTextFormattedColor(ttf12, status_padx, status_pady, uint32ColorOrange(*mainsurface), "Workshop file contents will be updated.");
					}
				}
				status_pady += 5 * TTF12_HEIGHT;
				if ( gamemods_uploadStatus >= 5 )
				{
					uint64 bytesProc;
					uint64 bytesTotal;
					int status = SteamUGC()->GetItemUpdateProgress(g_SteamWorkshop->UGCUpdateHandle, &bytesProc, &bytesTotal);
					if ( g_SteamWorkshop->SubmitItemUpdateResult.m_eResult != 0 )
					{
						for ( node = button_l.first; node != NULL; node = nextnode )
						{
							nextnode = node->next;
							button = (button_t*)node->element;
							if ( button->action == &buttonGamemodsPrepareWorkshopItemUpload
								|| button->action == &buttonGamemodsStartUploadItem
								|| button->action == &buttonGamemodsSetWorkshopItemFields
								|| button->action == &buttonGamemodsSelectDirectoryForUpload
								|| button->action == &buttonGamemodsModifyExistingWorkshopItemFields
								|| button->action == &buttonGamemodsCancelModifyFileContents )
							{
								button->visible = false;
							}
						}
						if ( g_SteamWorkshop->SubmitItemUpdateResult.m_eResult == k_EResultOK )
						{
							if ( g_SteamWorkshop->uploadSuccessTicks == 0 )
							{
								g_SteamWorkshop->uploadSuccessTicks = ticks;
							}
							else
							{
								if ( ticks - g_SteamWorkshop->uploadSuccessTicks > TICKS_PER_SECOND * 5 )
								{
									//cleanup the interface.
									buttonCloseSubwindow(NULL);
									list_FreeAll(&button_l);
									deleteallbuttons = true;
									gamemodsSubscribedItemsInit();
								}
							}
							ttfPrintTextFormattedColor(ttf12, status_padx, status_pady, uint32ColorGreen(*mainsurface), "successfully uploaded!");
							ttfPrintTextFormattedColor(ttf12, status_padx, status_pady + TTF12_HEIGHT, uint32ColorGreen(*mainsurface), "reloading window in %d...!", 5 - ((ticks - g_SteamWorkshop->uploadSuccessTicks) / TICKS_PER_SECOND));
						}
						else
						{
							ttfPrintTextFormattedColor(ttf12, status_padx, status_pady, uint32ColorOrange(*mainsurface), "error! %d", g_SteamWorkshop->SubmitItemUpdateResult.m_eResult);
							ttfPrintTextFormattedColor(ttf12, status_padx, status_pady + TTF12_HEIGHT, uint32ColorOrange(*mainsurface), "close the window and try again.");
						}
					}
					else
					{
						ttfPrintTextFormattedColor(ttf12, status_padx, status_pady, uint32ColorOrange(*mainsurface), "uploading... status %d", status);
						ttfPrintTextFormattedColor(ttf12, status_padx, status_pady + TTF12_HEIGHT, uint32ColorOrange(*mainsurface), "bytes processed: %d", bytesProc);
					}
				}
			}
		}
		if ( gamemods_window == 3 || gamemods_window == 4 )
		{
			numFileEntries = 8;
			filenameMaxLength = 48;
			filename_padx = subx1 + 16;
			filename_pady = suby1 + 32;
			filename_padx2 = subx2 - 16 - 40;
			filename_pady2 = filename_pady + numFileEntries * TTF12_HEIGHT + 8;
			int filename_rowHeight = 2 * TTF12_HEIGHT + 8;


			if ( gamemods_subscribedItemsStatus == 0 )
			{
				if ( g_SteamWorkshop->SteamUGCQueryCompleted.m_eResult == k_EResultOK )
				{
					gamemods_subscribedItemsStatus = 1;
				}
			}
			else
			{
				filename_pady += 1 * TTF12_HEIGHT;


				filename_pady += 2 * TTF12_HEIGHT;
				if ( gamemods_window == 3 )
				{
					ttfPrintTextFormattedColor(ttf12, filename_padx, filename_pady, uint32ColorGreen(*mainsurface), "successfully retrieved subscribed items!");
				}
				else
				{
					ttfPrintTextFormattedColor(ttf12, filename_padx, filename_pady, uint32ColorGreen(*mainsurface), "successfully retrieved my workshop items!");
				}

				std::string modInfoStr = "current loaded mods (hover for info): ";
				SDL_Rect tooltip; // we will draw the tooltip after drawing the other elements of the display window.
				bool drawModLoadOrder = false;
				int drawExtendedInformationForMod = -1; // value of 0 or greater will draw.
				int maxDescriptionLines = 10;

				tooltip.x = omousex - 256;
				tooltip.y = omousey + 16;
				tooltip.w = 32 + TTF12_WIDTH * 64;
				tooltip.h = (gamemods_mountedFilepaths.size() + 1) * TTF12_HEIGHT + 8;

				if ( gamemods_mountedFilepaths.size() > 0 && gamemods_window == 3 )
				{
					ttfPrintTextFormatted(ttf12, filename_padx2 - modInfoStr.length() * TTF12_WIDTH - 16, filename_pady, "%s %2d", modInfoStr.c_str(), gamemods_mountedFilepaths.size());
					if ( mouseInBounds(filename_padx2 - modInfoStr.length() * TTF12_WIDTH - 16, filename_padx2, filename_pady, filename_pady + TTF12_HEIGHT) )
					{
						drawModLoadOrder = true;
					}
					else
					{
						drawModLoadOrder = false;
					}
				}

				filename_pady += 2 * TTF12_HEIGHT;

				// do slider
				SDL_Rect slider;
				slider.x = filename_padx2 + 8;
				slider.y = filename_pady - 8;
				slider.h = suby2 - (filename_pady + 20);
				slider.w = 32;

				int numSubscribedItemsReturned = g_SteamWorkshop->SteamUGCQueryCompleted.m_unNumResultsReturned;
				int entriesToScroll = std::max(static_cast<int>((numSubscribedItemsReturned / numFileEntries) - 1), 0);
				entriesToScroll = entriesToScroll * numFileEntries + (numSubscribedItemsReturned % numFileEntries);

				// handle slider movement.
				if ( numSubscribedItemsReturned > numFileEntries )
				{
					drawRect(&slider, SDL_MapRGB(mainsurface->format, 64, 64, 64), 255);
					if ( mouseInBounds(filename_padx, slider.x + slider.w,
						slider.y, slider.y + slider.h) )
					{
						if ( mousestatus[SDL_BUTTON_WHEELUP] )
						{
							gamemods_window_scroll = std::max(gamemods_window_scroll - 1, 0);
							mousestatus[SDL_BUTTON_WHEELUP] = 0;
						}
						if ( mousestatus[SDL_BUTTON_WHEELDOWN] )
						{
							gamemods_window_scroll = std::min(gamemods_window_scroll + 1, entriesToScroll);
							mousestatus[SDL_BUTTON_WHEELDOWN] = 0;
						}
					}
				
					if ( keystatus[SDL_SCANCODE_UP] )
					{
						gamemods_window_scroll = std::max(gamemods_window_scroll - 1, 0);
						keystatus[SDL_SCANCODE_UP] = 0;
					}
					if ( keystatus[SDL_SCANCODE_DOWN] )
					{
						gamemods_window_scroll = std::min(gamemods_window_scroll + 1, entriesToScroll);
						keystatus[SDL_SCANCODE_DOWN] = 0;
					}
					slider.h *= (1 / static_cast<real_t>(entriesToScroll + 1));
					slider.y += slider.h * gamemods_window_scroll;
					if ( gamemods_window_scroll == entriesToScroll ) // reached end.
					{
						slider.y += (suby2 - 28) - (slider.y + slider.h); // bottom of slider is (suby2 - 28), so move the y level to imitate hitting the bottom in case of rounding error.
					}
					drawWindowFancy(slider.x, slider.y, slider.x + slider.w, slider.y + slider.h); // draw shortened list relative slider.
				}

				// draw last message results
				if ( ticks - g_SteamWorkshop->LastActionResult.creationTick < TICKS_PER_SECOND * 5 )
				{
					ttfPrintTextFormattedColor(ttf12, filename_padx + 8, suby2 - TTF12_HEIGHT - 4, uint32ColorOrange(*mainsurface), "%s returned status %d", 
						g_SteamWorkshop->LastActionResult.actionMsg.c_str(), static_cast<int>(g_SteamWorkshop->LastActionResult.lastResult));
				}

				// draw the content
				for ( int i = gamemods_window_scroll; i < numSubscribedItemsReturned && i < numFileEntries + gamemods_window_scroll; ++i )
				{
					filename_padx = subx1 + 16;
					SteamUGCDetails_t itemDetails = g_SteamWorkshop->m_subscribedItemListDetails[i];
					char fullpath[PATH_MAX];
					if ( itemDetails.m_eResult == k_EResultOK )
					{
						drawWindowFancy(filename_padx, filename_pady - 8, filename_padx2, filename_pady + filename_rowHeight);
						SDL_Rect highlightEntry;
						highlightEntry.x = filename_padx;
						highlightEntry.y = filename_pady - 8;
						highlightEntry.w = filename_padx2 - filename_padx;
						highlightEntry.h = filename_rowHeight + 8;
						drawRect(&highlightEntry, SDL_MapRGB(mainsurface->format, 128, 128, 128), 64);

						bool itemDownloaded = SteamUGC()->GetItemInstallInfo(itemDetails.m_nPublishedFileId, NULL, fullpath, PATH_MAX, NULL);
						bool pathIsMounted = gamemodsIsPathInMountedFiles(fullpath);

						if ( pathIsMounted && gamemods_window == 3 )
						{
							SDL_Rect pos;
							pos.x = filename_padx + 2;
							pos.y = filename_pady - 6;
							pos.w = filename_padx2 - filename_padx - 4;
							pos.h = filename_rowHeight + 4;
							drawRect(&pos, uint32ColorGreen(*mainsurface), 64);
						}

						std::string line = itemDetails.m_rgchTitle;
						if ( line.length() >= filenameMaxLength )
						{
							line = line.substr(0, 46);
							line.append("..");
						}
						ttfPrintTextFormatted(ttf12, filename_padx + 8, filename_pady, "Title: %s", line.c_str());
						line = itemDetails.m_rgchDescription;
						if ( line.length() >= filenameMaxLength )
						{
							line = line.substr(0, 46);
							line.append("..");
						}
						ttfPrintTextFormatted(ttf12, filename_padx + 8, filename_pady + TTF12_HEIGHT, "Desc: %s", line.c_str());

						// if hovering over title or description, provide more info...
						if ( mouseInBounds(filename_padx + 8, filename_padx + 8 + 52 * TTF12_WIDTH, filename_pady + TTF12_HEIGHT, filename_pady + 2 * TTF12_HEIGHT) )
						{
							line = itemDetails.m_rgchDescription;
							tooltip.h = (6 + std::min(static_cast<int>((line.size() / 64)), maxDescriptionLines)) * TTF12_HEIGHT + 8;
							drawExtendedInformationForMod = i;
						}

						filename_padx = filename_padx2 - (12 * TTF12_WIDTH + 16) * 2;
						// download button
						if ( gamemods_window == 3 )
						{
							if ( !itemDownloaded )
							{
								if ( gamemodsDrawClickableButton(filename_padx, filename_pady, 12 * TTF12_WIDTH + 8, TTF12_HEIGHT, uint32ColorBaronyBlue(*mainsurface), " Download ", 0) )
								{
									SteamUGC()->DownloadItem(itemDetails.m_nPublishedFileId, true);
								}
							}
							filename_padx += (12 * TTF12_WIDTH + 16);
							// mount button
							if ( itemDownloaded && !pathIsMounted )
							{
								if ( gamemodsDrawClickableButton(filename_padx, filename_pady, 12 * TTF12_WIDTH + 8, TTF12_HEIGHT, 0, " Load Item ", 0) )
								{
									if ( PHYSFS_mount(fullpath, NULL, 0) )
									{
										gamemods_mountedFilepaths.push_back(std::make_pair(fullpath, itemDetails.m_rgchTitle));
										gamemods_workshopLoadedFileIDMap.push_back(std::make_pair(itemDetails.m_rgchTitle, itemDetails.m_nPublishedFileId));
										gamemods_modelsListRequiresReload = true;
										gamemods_soundListRequiresReload = true;
									}
								}
							}
							filename_padx -= (12 * TTF12_WIDTH + 16);
						}
						if ( gamemods_window == 4 )
						{
							filename_padx += (12 * TTF12_WIDTH + 16);
							// edit content button
							if ( gamemodsDrawClickableButton(filename_padx, filename_pady + filename_rowHeight / 4, 12 * TTF12_WIDTH + 8, TTF12_HEIGHT, uint32ColorBaronyBlue(*mainsurface), "  Update  ", 0) )
							{
								buttonGamemodsOpenModifyExistingWindow(nullptr);
								gamemods_window = 5;
								gamemods_uploadStatus = 1;
								g_SteamWorkshop->m_myWorkshopItemToModify = itemDetails;

								// grab the current item tags and store them for modification.
								std::string workshopItemTagString = g_SteamWorkshop->m_myWorkshopItemToModify.m_rgchTags;
								std::size_t found = workshopItemTagString.find(",");
								g_SteamWorkshop->workshopItemTags.clear();
								while ( found != std::string::npos )
								{
									std::string line = workshopItemTagString.substr(0, found);
									workshopItemTagString = workshopItemTagString.substr(found + 1); // skip the "," character.
									g_SteamWorkshop->workshopItemTags.push_back(line); // store the comma separated value.
									found = workshopItemTagString.find(",");
								}
								// add the final string.
								g_SteamWorkshop->workshopItemTags.push_back(workshopItemTagString); 
							}
						}
						filename_pady += filename_rowHeight / 2;
						if ( gamemods_window == 3 )
						{
							// unsubscribe button
							if ( gamemodsDrawClickableButton(filename_padx, filename_pady, 12 * TTF12_WIDTH + 8, TTF12_HEIGHT, uint32ColorRed(*mainsurface), "Unsubscribe", 0) )
							{
								if ( pathIsMounted )
								{
									if ( PHYSFS_unmount(fullpath) )
									{
										if ( gamemodsRemovePathFromMountedFiles(fullpath) )
										{
											printlog("[%s] is removed from the search path.\n", fullpath);
											gamemods_modelsListRequiresReload = true;
											gamemods_soundListRequiresReload = true;
										}
									}
								}
								g_SteamWorkshop->UnsubscribeItemFileID(itemDetails.m_nPublishedFileId);
								gamemods_window_scroll = 0;
							}
							filename_padx += (12 * TTF12_WIDTH + 16);
							// unmount button
							if ( itemDownloaded && pathIsMounted )
							{
								if ( gamemodsDrawClickableButton(filename_padx, filename_pady, 12 * TTF12_WIDTH + 8, TTF12_HEIGHT, 0, "Unload Item", 0) )
								{
									if ( PHYSFS_unmount(fullpath) )
									{
										if ( gamemodsRemovePathFromMountedFiles(fullpath) )
										{
											printlog("[%s] is removed from the search path.\n", fullpath);
											gamemods_modelsListRequiresReload = true;
											gamemods_soundListRequiresReload = true;
										}
									}
								}
							}
						}
					}
					filename_pady += filename_rowHeight;
				}

				// draw the tooltip we initialised earlier.
				if ( drawModLoadOrder )
				{
					drawTooltip(&tooltip);
					int numLoadedModLine = 1;
					ttfPrintTextFormattedColor(ttf12, tooltip.x + 4, tooltip.y + 4, uint32ColorBaronyBlue(*mainsurface), 
						"Current load list: (first is lowest priority)");
					for ( std::vector<std::pair<std::string, std::string>>::iterator it = gamemods_mountedFilepaths.begin(); it != gamemods_mountedFilepaths.end(); ++it )
					{
						std::pair<std::string, std::string> line = *it;
						modInfoStr = line.second;
						if ( modInfoStr.length() > 64 )
						{
							modInfoStr = modInfoStr.substr(0, 64 - 2).append("..");
						}
						ttfPrintTextFormatted(ttf12, tooltip.x + 4, tooltip.y + 4 + numLoadedModLine * TTF12_HEIGHT, "%2d) %s", numLoadedModLine, modInfoStr.c_str());
						++numLoadedModLine;
					}
				}
				else if ( drawExtendedInformationForMod >= 0 )
				{
					int tooltip_pady = 8;
					SteamUGCDetails_t itemDetails = g_SteamWorkshop->m_subscribedItemListDetails[drawExtendedInformationForMod];
					// draw the information.
					drawTooltip(&tooltip);
					std::string line = itemDetails.m_rgchTitle;
					if ( line.length() >= 64 )
					{
						line = line.substr(0, 62);
						line.append("..");
					}
					ttfPrintTextFormattedColor(ttf12, tooltip.x + 8, tooltip.y + tooltip_pady, uint32ColorBaronyBlue(*mainsurface), "%s", line.c_str());
					tooltip_pady += 2 * TTF12_HEIGHT;

					line = itemDetails.m_rgchDescription;
					line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
					line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
					std::string subString;
					while ( line.length() >= 62 )
					{
						if ( maxDescriptionLines <= 0 )
						{
							break;
						}
						subString = line.substr(0, 62);
						line = line.substr(62);
						ttfPrintTextFormatted(ttf12, tooltip.x + 8, tooltip.y + tooltip_pady, "  %s", subString.c_str());
						tooltip_pady += TTF12_HEIGHT;
						--maxDescriptionLines;
					}
					subString = line.substr(0, 62);
					ttfPrintTextFormatted(ttf12, tooltip.x + 8, tooltip.y + tooltip_pady, "  %s", subString.c_str());
					tooltip_pady += TTF12_HEIGHT;

					ttfPrintTextFormattedColor(ttf12, tooltip.x + 8, tooltip.y + tooltip_pady, uint32ColorBaronyBlue(*mainsurface), "tags:");
					tooltip_pady += TTF12_HEIGHT;
					line = itemDetails.m_rgchTags;

					maxDescriptionLines = 3;
					int tooltip_padx = 0;
					if ( !line.empty() )
					{
						subString = line;
						ttfPrintTextFormatted(ttf12, tooltip.x + 8, tooltip.y + tooltip_pady, "%s", subString.c_str());
					}
				}
			}
		}
#endif //STEAMWORKS

		if ( gamemods_window == 6 )
		{
			// create blank file structure for a mod.
			filename_padx = subx1 + 16;
			filename_pady = suby1 + 32;
			ttfPrintTextFormatted(ttf12, filename_padx, filename_pady, "Enter base folder name for template directory");
			filename_pady += TTF12_HEIGHT + 8;
			drawDepressed(filename_padx - 4, filename_pady - 4, filename_padx + 32 * TTF12_WIDTH + 8, filename_pady + TTF12_HEIGHT);
			ttfPrintTextFormatted(ttf12, filename_padx, filename_pady, "%s", gamemods_newBlankDirectory);
			if ( !SDL_IsTextInputActive() )
			{
				inputstr = gamemods_newBlankDirectory;
				SDL_StartTextInput();
			}
			inputlen = 30;
			if ( SDL_IsTextInputActive()
				&& (ticks - cursorflash) % TICKS_PER_SECOND < TICKS_PER_SECOND / 2 )
			{
				int x;
				TTF_SizeUTF8(ttf12, inputstr, &x, NULL);
				ttfPrintText(ttf12, filename_padx + x, filename_pady, "_");
			}
			filename_pady += TTF12_HEIGHT + 8;
			if ( strcmp(gamemods_newBlankDirectoryOldName, gamemods_newBlankDirectory) == 0 )
			{
				if ( gamemods_newBlankDirectoryStatus == -1 )
				{
					ttfPrintTextFormattedColor(ttf12, filename_padx, filename_pady, uint32ColorRed(*mainsurface), "Error: could not create directory %s/, already exists in mods/ folder", gamemods_newBlankDirectory);
				}
				else if ( gamemods_newBlankDirectoryStatus == 1 )
				{
					ttfPrintTextFormattedColor(ttf12, filename_padx, filename_pady, uint32ColorGreen(*mainsurface), "Successfully created directory %s/ in mods/ folder", gamemods_newBlankDirectory);
				}
			}
		}
		if ( gamemods_window == 7 )
		{
			numFileEntries = 8;
			filenameMaxLength = 48;
			filename_padx = subx1 + 16;
			filename_pady = suby1 + 32;
			filename_padx2 = subx2 - 16 - 40;
			filename_pady2 = filename_pady + numFileEntries * TTF12_HEIGHT + 8;
			int filename_rowHeight = 2 * TTF12_HEIGHT + 8;
			filename_pady += 3 * TTF12_HEIGHT;
			int numLocalFolders = std::max(static_cast<int>(gamemods_localModFoldernames.size() - 2), 0);
			if ( numLocalFolders > 0 )
			{
				ttfPrintTextFormattedColor(ttf12, filename_padx, filename_pady, uint32ColorGreen(*mainsurface), "successfully retrieved local items!");
			}
			else
			{
				ttfPrintTextFormattedColor(ttf12, filename_padx, filename_pady, uint32ColorOrange(*mainsurface), "no folders found!");
				ttfPrintTextFormattedColor(ttf12, filename_padx, filename_pady + TTF12_HEIGHT + 8, uint32ColorOrange(*mainsurface), "to get started create a new folder, or copy shared custom content to the mods/ folder");
			}

			std::string modInfoStr = "current loaded mods (hover for info): ";
			SDL_Rect tooltip; // we will draw the tooltip after drawing the other elements of the display window.
			bool drawModLoadOrder = false;
			int drawExtendedInformationForMod = -1; // value of 0 or greater will draw.
			int maxDescriptionLines = 10;

			tooltip.x = omousex - 256;
			tooltip.y = omousey + 16;
			tooltip.w = 32 + TTF12_WIDTH * 64;
			tooltip.h = (gamemods_mountedFilepaths.size() + 1) * TTF12_HEIGHT + 8;

			if ( gamemods_mountedFilepaths.size() > 0 )
			{
				ttfPrintTextFormatted(ttf12, filename_padx2 - modInfoStr.length() * TTF12_WIDTH - 16, filename_pady, "%s %2d", modInfoStr.c_str(), gamemods_mountedFilepaths.size());
				if ( mouseInBounds(filename_padx2 - modInfoStr.length() * TTF12_WIDTH - 16, filename_padx2, filename_pady, filename_pady + TTF12_HEIGHT) )
				{
					drawModLoadOrder = true;
				}
				else
				{
					drawModLoadOrder = false;
				}
			}

			filename_pady += 2 * TTF12_HEIGHT;

			// do slider
			SDL_Rect slider;
			slider.x = filename_padx2 + 8;
			slider.y = filename_pady - 8;
			slider.h = suby2 - (filename_pady + 20);
			slider.w = 32;

			int entriesToScroll = std::max(static_cast<int>((numLocalFolders / numFileEntries) - 1), 0);
			entriesToScroll = entriesToScroll * numFileEntries + (numLocalFolders % numFileEntries);

			// handle slider movement.
			if ( numLocalFolders > numFileEntries )
			{
				drawRect(&slider, SDL_MapRGB(mainsurface->format, 64, 64, 64), 255);
				if ( mouseInBounds(filename_padx, slider.x + slider.w,
					slider.y, slider.y + slider.h) )
				{
					if ( mousestatus[SDL_BUTTON_WHEELUP] )
					{
						gamemods_window_scroll = std::max(gamemods_window_scroll - 1, 0);
						mousestatus[SDL_BUTTON_WHEELUP] = 0;
					}
					if ( mousestatus[SDL_BUTTON_WHEELDOWN] )
					{
						gamemods_window_scroll = std::min(gamemods_window_scroll + 1, entriesToScroll);
						mousestatus[SDL_BUTTON_WHEELDOWN] = 0;
					}
				}

				if ( keystatus[SDL_SCANCODE_UP] )
				{
					gamemods_window_scroll = std::max(gamemods_window_scroll - 1, 0);
					keystatus[SDL_SCANCODE_UP] = 0;
				}
				if ( keystatus[SDL_SCANCODE_DOWN] )
				{
					gamemods_window_scroll = std::min(gamemods_window_scroll + 1, entriesToScroll);
					keystatus[SDL_SCANCODE_DOWN] = 0;
				}
				slider.h *= (1 / static_cast<real_t>(entriesToScroll + 1));
				slider.y += slider.h * gamemods_window_scroll;
				if ( gamemods_window_scroll == entriesToScroll ) // reached end.
				{
					slider.y += (suby2 - 28) - (slider.y + slider.h); // bottom of slider is (suby2 - 28), so move the y level to imitate hitting the bottom in case of rounding error.
				}
				drawWindowFancy(slider.x, slider.y, slider.x + slider.w, slider.y + slider.h); // draw shortened list relative slider.
			}

			// draw the content
			for ( int i = gamemods_window_scroll; i < numLocalFolders && i < numFileEntries + gamemods_window_scroll; ++i )
			{
				filename_padx = subx1 + 16;

				std::list<std::string>::iterator it = gamemods_localModFoldernames.begin();
				std::advance(it, 2); // skip the "." and ".." directories.
				std::advance(it, i);
				std::string folderName = *it;

				drawWindowFancy(filename_padx, filename_pady - 8, filename_padx2, filename_pady + filename_rowHeight);
				SDL_Rect highlightEntry;
				highlightEntry.x = filename_padx;
				highlightEntry.y = filename_pady - 8;
				highlightEntry.w = filename_padx2 - filename_padx;
				highlightEntry.h = filename_rowHeight + 8;
				drawRect(&highlightEntry, SDL_MapRGB(mainsurface->format, 128, 128, 128), 64);

				std::string path = "./mods/" + folderName;
				bool pathIsMounted = gamemodsIsPathInMountedFiles(path);

				if ( pathIsMounted )
				{
					SDL_Rect pos;
					pos.x = filename_padx + 2;
					pos.y = filename_pady - 6;
					pos.w = filename_padx2 - filename_padx - 4;
					pos.h = filename_rowHeight + 4;
					drawRect(&pos, uint32ColorGreen(*mainsurface), 64);
				}

				if ( folderName.length() >= filenameMaxLength )
				{
					folderName = folderName.substr(0, 46);
					folderName.append("..");
				}
				ttfPrintTextFormatted(ttf12, filename_padx + 8, filename_pady + TTF12_HEIGHT / 2, "Folder Name: %s", folderName.c_str());

				filename_padx = filename_padx2 - (12 * TTF12_WIDTH + 16);
				// mount button
				if ( !pathIsMounted )
				{
					if ( gamemodsDrawClickableButton(filename_padx, filename_pady, 12 * TTF12_WIDTH + 8, TTF12_HEIGHT, 0, " Load Item ", 0) )
					{
						if ( PHYSFS_mount(path.c_str(), NULL, 0) )
						{
							gamemods_mountedFilepaths.push_back(std::make_pair(path, folderName));
							printlog("[PhysFS]: [%s] is in the search path.\n", path.c_str());
							gamemods_modelsListRequiresReload = true;
							gamemods_soundListRequiresReload = true;
						}
					}
				}
				filename_pady += filename_rowHeight / 2;
				// unmount button
				if ( pathIsMounted )
				{
					if ( gamemodsDrawClickableButton(filename_padx, filename_pady, 12 * TTF12_WIDTH + 8, TTF12_HEIGHT, 0, "Unload Item", 0) )
					{
						if ( PHYSFS_unmount(path.c_str()) )
						{
							if ( gamemodsRemovePathFromMountedFiles(path) )
							{
								printlog("[PhysFS]: [%s] is removed from the search path.\n", path.c_str());
								gamemods_modelsListRequiresReload = true;
								gamemods_soundListRequiresReload = true;
							}
						}
					}
				}
				filename_pady += filename_rowHeight;
			}

			// draw the tooltip we initialised earlier.
			if ( drawModLoadOrder )
			{
				drawTooltip(&tooltip);
				int numLoadedModLine = 1;
				ttfPrintTextFormattedColor(ttf12, tooltip.x + 4, tooltip.y + 4, uint32ColorBaronyBlue(*mainsurface),
					"Current load list: (first is lowest priority)");
				for ( std::vector<std::pair<std::string, std::string>>::iterator it = gamemods_mountedFilepaths.begin(); it != gamemods_mountedFilepaths.end(); ++it )
				{
					std::pair<std::string, std::string> line = *it;
					modInfoStr = line.second;
					if ( modInfoStr.length() > 64 )
					{
						modInfoStr = modInfoStr.substr(0, 64 - 2).append("..");
					}
					ttfPrintTextFormatted(ttf12, tooltip.x + 4, tooltip.y + 4 + numLoadedModLine * TTF12_HEIGHT, "%2d) %s", numLoadedModLine, modInfoStr.c_str());
					++numLoadedModLine;
				}
			}
		}
	}

	// handle fade actions
	if ( fadefinished )
	{
		if ( introstage == 2 )   // quit game
		{
			introstage = 0;
			mainloop = 0;
		}
		else if ( introstage == 3 )     // new game
		{
			introstage = 1;
			fadefinished = false;
			fadeout = false;
			gamePaused = false;
			multiplayerselect = 0;
			intro = true; //Fix items auto-adding to the hotbar on game restart.

			if ( !mode )
			{
				// restarting game, make a highscore
				saveScore();
				deleteSaveGame(multiplayer);
				loadingsavegame = 0;
			}
			camera_charsheet_offsetyaw = (330) * PI / 180; // reset player camera view.

			// undo shopkeeper grudge
			swornenemies[SHOPKEEPER][HUMAN] = false;
			monsterally[SHOPKEEPER][HUMAN] = true;

			// setup game //TODO: Move into a function startGameStuff() or something.
			entity_uids = 1;
			loading = true;
			darkmap = false;
			selected_spell = NULL;
			shootmode = true;
			currentlevel = startfloor;
			secretlevel = false;
			victory = 0;
			completionTime = 0;

			setDefaultPlayerConducts(); // penniless, foodless etc.

			list_FreeAll(&damageIndicators);
			for ( c = 0; c < NUMMONSTERS; c++ )
			{
				kills[c] = 0;
			}

			// disable cheats
			noclip = false;
			godmode = false;
			buddhamode = false;
			everybodyfriendly = false;

#ifdef STEAMWORKS
			if ( !directConnect )
			{
				if ( currentLobby )
				{
					// once the game is started, the lobby is no longer needed.
					// when all steam users have left the lobby,
					// the lobby is destroyed automatically on the backend.

					SteamMatchmaking()->LeaveLobby(*static_cast<CSteamID*>(currentLobby));
					cpp_Free_CSteamID(currentLobby); //TODO: Bugger this.
					currentLobby = NULL;
				}
			}
#endif

			// load dungeon
			if ( multiplayer != CLIENT )
			{
				// stop all sounds
#ifdef USE_FMOD
				if ( sound_group )
				{
					FMOD_ChannelGroup_Stop(sound_group);
				}
#elif defined USE_OPENAL
				if ( sound_group )
				{
					OPENAL_ChannelGroup_Stop(sound_group);
				}
#endif

				// generate a unique game key (used to identify compatible save games)
				prng_seed_time();
				if ( multiplayer == SINGLE )
				{
					uniqueGameKey = prng_get_uint();
					if ( !uniqueGameKey )
					{
						uniqueGameKey++;
					}
				}

				// reset class loadout
				if ( !loadingsavegame )
				{
					stats[0]->clearStats();
					initClass(0);
					mapseed = 0;
				}
				else
				{
					loadGame(0);
				}

				// hack to fix these things from breaking everything...
				hudarm = NULL;
				hudweapon = NULL;
				magicLeftHand = NULL;
				magicRightHand = NULL;

				for ( node = map.entities->first; node != nullptr; node = node->next )
				{
					entity = (Entity*)node->element;
					entity->flags[NOUPDATE] = true;
				}
				lastEntityUIDs = entity_uids;
				numplayers = 0;
				if ( loadingmap == false )
				{
					physfsLoadMapFile(currentlevel, mapseed, false);
				}
				else
				{
					if ( genmap == false )
					{
						std::string fullMapName = physfsFormatMapName(maptoload);
						loadMap(fullMapName.c_str(), &map, map.entities, map.creatures);
					}
					else
					{
						generateDungeon(maptoload, mapseed);
					}
				}
				assignActions(&map);
				generatePathMaps();

				if ( loadingsavegame )
				{
					list_t* followers = loadGameFollowers();
					if ( followers )
					{
						int c;
						for ( c = 0; c < MAXPLAYERS; c++ )
						{
							node_t* tempNode = list_Node(followers, c);
							if ( tempNode )
							{
								list_t* tempFollowers = (list_t*)tempNode->element;
								if (players[c] && players[c]->entity && !client_disconnected[c])
								{
									node_t* node;
									for ( node = tempFollowers->first; node != NULL; node = node->next )
									{
										Stat* tempStats = (Stat*)node->element;
										Entity* monster = summonMonster(tempStats->type, players[c]->entity->x, players[c]->entity->y);
										if ( monster )
										{
											monster->skill[3] = 1; // to mark this monster partially initialized
											list_RemoveNode(monster->children.last);

											node_t* newNode = list_AddNodeLast(&monster->children);
											newNode->element = tempStats->copyStats();
											//newNode->deconstructor = &tempStats->~Stat;
											newNode->size = sizeof(tempStats);

											Stat* monsterStats = (Stat*)newNode->element;
											monsterStats->leader_uid = players[c]->entity->getUID();
											if ( !monsterally[HUMAN][monsterStats->type] )
											{
												monster->flags[USERFLAG2] = true;
											}

											newNode = list_AddNodeLast(&stats[c]->FOLLOWERS);
											newNode->deconstructor = &defaultDeconstructor;
											Uint32* myuid = (Uint32*) malloc(sizeof(Uint32));
											newNode->element = myuid;
											*myuid = monster->getUID();

											if ( c > 0 && multiplayer == SERVER )
											{
												strcpy((char*)net_packet->data, "LEAD");
												SDLNet_Write32((Uint32)monster->getUID(), &net_packet->data[4]);
												net_packet->address.host = net_clients[c - 1].host;
												net_packet->address.port = net_clients[c - 1].port;
												net_packet->len = 8;
												sendPacketSafe(net_sock, -1, net_packet, c - 1);
											}
										}
									}
								}
							}
						}
						list_FreeAll(followers);
						free(followers);
					}
				}

				if ( multiplayer == SINGLE )
				{
					saveGame();
				}
			}
			else
			{
				// hack to fix these things from breaking everything...
				hudarm = NULL;
				hudweapon = NULL;
				magicLeftHand = NULL;
				magicRightHand = NULL;

				client_disconnected[0] = false;

				// initialize class
				if ( !loadingsavegame )
				{
					stats[clientnum]->clearStats();
					initClass(clientnum);
					mapseed = 0;
				}
				else
				{
					loadGame(clientnum);
				}

				// stop all sounds
#ifdef USE_FMOD
				if ( sound_group )
				{
					FMOD_ChannelGroup_Stop(sound_group);
				}
#elif defined USE_OPENAL
				if ( sound_group )
				{
					OPENAL_ChannelGroup_Stop(sound_group);
				}
#endif
				// load next level
				entity_uids = 1;
				lastEntityUIDs = entity_uids;
				numplayers = 0;
				if ( loadingmap == false )
				{
					physfsLoadMapFile(currentlevel, mapseed, false);
				}
				else
				{
					if ( genmap == false )
					{
						std::string fullMapName = physfsFormatMapName(maptoload);
						loadMap(fullMapName.c_str(), &map, map.entities, map.creatures);
					}
					else
					{
						generateDungeon(maptoload, rand());
					}
				}
				assignActions(&map);
				generatePathMaps();
				for ( node = map.entities->first; node != nullptr; node = nextnode )
				{
					nextnode = node->next;
					Entity* entity = (Entity*)node->element;
					if ( entity->flags[NOUPDATE] )
					{
						list_RemoveNode(entity->mynode);    // we're anticipating this entity data from server
					}
				}

				printlog("Done.\n");
			}

			// spice of life achievement
			usedClass[client_classes[clientnum]] = true;
			bool usedAllClasses = true;
			for ( c = 0; c < NUMCLASSES; c++ )
				if ( !usedClass[c] )
				{
					usedAllClasses = false;
				}
			if ( usedAllClasses )
			{
				steamAchievement("BARONY_ACH_SPICE_OF_LIFE");
			}

			// delete game data clutter
			list_FreeAll(&messages);
			list_FreeAll(&command_history);
			list_FreeAll(&safePacketsSent);
			for ( c = 0; c < MAXPLAYERS; c++ )
			{
				list_FreeAll(&safePacketsReceived[c]);
			}
			deleteAllNotificationMessages();
			if ( !loadingsavegame ) // don't delete the followers we just created!
			{
				for (c = 0; c < MAXPLAYERS; c++)
				{
					list_FreeAll(&stats[c]->FOLLOWERS);
				}
			}

			if ( loadingsavegame && multiplayer != CLIENT )
			{
				loadingsavegame = 0;
			}

			list_FreeAll(&removedEntities);
			list_FreeAll(&chestInv);

			// make some messages
			startMessages();

			// kick off the main loop!
			pauseGame(1, 0);
			loading = false;
			intro = false;
		}
		else if ( introstage == 4 )     // credits
		{
			fadefinished = false;
			fadeout = false;
			if ( creditstage == 0 && victory == 3 )
			{
#ifdef MUSIC
			playmusic(citadelmusic[0], true, false, false);
#endif
			}
			creditstage++;
			if ( creditstage >= 15 )
			{
#ifdef MUSIC
				playmusic(intromusic[2], true, false, false);
#endif
				introstage = 1;
				credittime = 0;
				creditstage = 0;
				movie = false;
			}
			else
			{
				credittime = 0;
				movie = true;
			}
		}
		else if ( introstage == 5 )     // end game
		{
			// in greater numbers achievement
			if ( victory )
			{
				int k = 0;
				for ( c = 0; c < MAXPLAYERS; c++ )
				{
					if (players[c] && players[c]->entity)
					{
						k++;
					}
				}
				if ( k >= 2 )
				{
					steamAchievement("BARONY_ACH_IN_GREATER_NUMBERS");
				}
			}

			// make a highscore!
			saveScore();

			// pick a new subtitle :)
			subtitleCurrent = rand() % NUMSUBTITLES;
			subtitleVisible = true;

			for ( c = 0; c < NUMMONSTERS; c++ )
			{
				kills[c] = 0;
			}

			// stop all sounds
#ifdef USE_FMOD
			if ( sound_group )
			{
				FMOD_ChannelGroup_Stop(sound_group);
			}
#elif defined USE_OPENAL
			if ( sound_group )
			{
				OPENAL_ChannelGroup_Stop(sound_group);
			}
#endif

			// send disconnect messages
			if (multiplayer == CLIENT)
			{
				strcpy((char*)net_packet->data, "DISCONNECT");
				net_packet->data[10] = clientnum;
				net_packet->address.host = net_server.host;
				net_packet->address.port = net_server.port;
				net_packet->len = 11;
				sendPacketSafe(net_sock, -1, net_packet, 0);
				printlog("disconnected from server.\n");
			}
			else if (multiplayer == SERVER)
			{
				for (x = 1; x < MAXPLAYERS; x++)
				{
					if ( client_disconnected[x] == true )
					{
						continue;
					}
					strcpy((char*)net_packet->data, "DISCONNECT");
					net_packet->data[10] = clientnum;
					net_packet->address.host = net_clients[x - 1].host;
					net_packet->address.port = net_clients[x - 1].port;
					net_packet->len = 11;
					sendPacketSafe(net_sock, -1, net_packet, x - 1);
					client_disconnected[x] = true;
				}
			}

			// clean up shopInv
			if ( multiplayer == CLIENT )
			{
				if ( shopInv )
				{
					list_FreeAll(shopInv);
					free(shopInv);
					shopInv = NULL;
				}
			}

			// delete save game
			if ( !savethisgame )
			{
				deleteSaveGame(multiplayer);
			}
			else
			{
				savethisgame = false;
			}

			if ( victory )
			{
				// conduct achievements
				if ( (victory == 1 && currentlevel >= 20)
					|| (victory == 2 && currentlevel >= 24)
					|| (victory == 3 && currentlevel >= 35) )
				{
					if ( conductPenniless )
					{
						steamAchievement("BARONY_ACH_PENNILESS_CONDUCT");
					}
					if ( conductFoodless )
					{
						steamAchievement("BARONY_ACH_FOODLESS_CONDUCT");
					}
					if ( conductVegetarian )
					{
						steamAchievement("BARONY_ACH_VEGETARIAN_CONDUCT");
					}
					if ( conductIlliterate )
					{
						steamAchievement("BARONY_ACH_ILLITERATE_CONDUCT");
					}

					if ( completionTime < 20 * 60 * TICKS_PER_SECOND )
					{
						steamAchievement("BARONY_ACH_BOOTS_OF_SPEED");
						conductGameChallenges[CONDUCT_BOOTS_SPEED] = 1;
					}
				}

				if ( victory == 1 )
				{
					if ( currentlevel >= 20 )
					{
						if ( conductGameChallenges[CONDUCT_HARDCORE] )
						{
							steamAchievement("BARONY_ACH_HARDCORE");
						}
					}
				}
				else if ( victory == 2 )
				{
					if ( currentlevel >= 24 )
					{
						if ( conductGameChallenges[CONDUCT_HARDCORE] )
						{
							steamAchievement("BARONY_ACH_HARDCORE");
						}
					}
				}
				else if ( victory == 3 )
				{
					if ( currentlevel >= 35 )
					{
						if ( conductGameChallenges[CONDUCT_BRAWLER] )
						{
							steamAchievement("BARONY_ACH_BRAWLER");
						}
						if ( conductGameChallenges[CONDUCT_BLESSED_BOOTS_SPEED] )
						{
							steamAchievement("BARONY_ACH_PLUS_BOOTS_OF_SPEED");
						}
						if ( conductGameChallenges[CONDUCT_HARDCORE] )
						{
							steamAchievement("BARONY_ACH_POST_HARDCORE");
						}
					}
				}
			}

			// reset game
			darkmap = false;
			appraisal_timer = 0;
			appraisal_item = 0;
			multiplayer = 0;
			shootmode = true;
			currentlevel = 0;
			secretlevel = false;
			clientnum = 0;
			introstage = 1;
			intro = true;
			selected_spell = NULL; //So you don't start off with a spell when the game restarts.
			client_classes[0] = 0;
			spellcastingAnimationManager_deactivate(&cast_animation);
			SDL_StopTextInput();

			// delete game data clutter
			list_FreeAll(&messages);
			list_FreeAll(&command_history);
			list_FreeAll(&safePacketsSent);
			for ( c = 0; c < MAXPLAYERS; c++ )
			{
				list_FreeAll(&safePacketsReceived[c]);
			}
			deleteAllNotificationMessages();
			for (c = 0; c < MAXPLAYERS; c++)
			{
				stats[c]->freePlayerEquipment();
				list_FreeAll(&stats[c]->inventory);
				list_FreeAll(&stats[c]->FOLLOWERS);
			}
			list_FreeAll(&removedEntities);
			list_FreeAll(&chestInv);

			// default player stats
			for ( c = 0; c < MAXPLAYERS; c++ )
			{
				if ( c > 0 )
				{
					client_disconnected[c] = true;
				}
				else
				{
					client_disconnected[c] = false;
				}
				players[c]->entity = nullptr; //TODO: PLAYERSWAP VERIFY. Need to do anything else?
				stats[c]->sex = static_cast<sex_t>(0);
				stats[c]->appearance = 0;
				strcpy(stats[c]->name, "");
				stats[c]->type = HUMAN;
				stats[c]->clearStats();
				entitiesToDelete[c].first = NULL;
				entitiesToDelete[c].last = NULL;
				if ( c == 0 )
				{
					initClass(c);
				}
			}

			// hack to fix these things from breaking everything...
			hudarm = NULL;
			hudweapon = NULL;
			magicLeftHand = NULL;
			magicRightHand = NULL;

			// load menu level
			int menuMapType = 0;
			if ( victory == 3 )
			{
				menuMapType = loadMainMenuMap(true, true);
			}
			else
			{
				switch ( rand() % 2 )
				{
					case 0:
						menuMapType = loadMainMenuMap(true, false);
						break;
					case 1:
						menuMapType = loadMainMenuMap(false, false);
						break;
					default:
						break;
				}
			}
			camera.vang = 0;
			numplayers = 0;
			assignActions(&map);
			generatePathMaps();
			gamePaused = false;
			if ( !victory )
			{
				fadefinished = false;
				fadeout = false;
#ifdef MUSIC
				if ( victory != 3 && menuMapType )
				{
					playmusic(intromusic[2], true, false, false);
				}
				else
				{
					playmusic(intromusic[rand() % 2], true, false, false);
				}
#endif
			}
			else
			{
				if ( victory == 1 )
				{
					introstage = 7;
				}
				else if ( victory == 2 )
				{
					introstage = 8;
				}
				else if ( victory == 3 )
				{
					introstage = 10;
				}
			}

			// finish handling invite
#ifdef STEAMWORKS
			if ( stillConnectingToLobby )
			{
				processLobbyInvite();
			}
#endif
		}
		else if ( introstage == 6 )     // introduction cutscene
		{
			fadefinished = false;
			fadeout = false;
			intromoviestage++;
			if ( intromoviestage >= 9 )
			{
#ifdef MUSIC
				playmusic(intromusic[1], true, false, false);
#endif
				introstage = 1;
				intromovietime = 0;
				intromoviestage = 0;
				int c;
				for ( c = 0; c < 30; c++ )
				{
					intromoviealpha[c] = 0;
				}
				movie = false;
			}
			else
			{
				intromovietime = 0;
				movie = true;
			}
		}
		else if ( introstage == 7 )     // win game sequence (herx)
		{
#ifdef MUSIC
			if ( firstendmoviestage == 0 )
			{
				playmusic(endgamemusic, true, true, false);
			}
#endif
			firstendmoviestage++;
			if ( firstendmoviestage >= 5 )
			{
				introstage = 4;
				firstendmovietime = 0;
				firstendmoviestage = 0;
				int c;
				for ( c = 0; c < 30; c++ )
				{
					firstendmoviealpha[c] = 0;
				}
				fadeout = true;
			}
			else
			{
				fadefinished = false;
				fadeout = false;
				firstendmovietime = 0;
				movie = true;
			}
		}
		else if ( introstage == 8 )     // win game sequence (devil)
		{
#ifdef MUSIC
			if ( secondendmoviestage == 0 )
			{
				playmusic(endgamemusic, true, true, false);
			}
#endif
			secondendmoviestage++;
			if ( secondendmoviestage >= 5 )
			{
				introstage = 4;
				secondendmovietime = 0;
				secondendmoviestage = 0;
				int c;
				for ( c = 0; c < 30; c++ )
				{
					secondendmoviealpha[c] = 0;
				}
				fadeout = true;
			}
			else
			{
				fadefinished = false;
				fadeout = false;
				secondendmovietime = 0;
				movie = true;
			}
		}
		else if ( introstage == 9 )     // mid game sequence
		{
#ifdef MUSIC
			if ( thirdendmoviestage == 0 )
			{
				playmusic(endgamemusic, true, true, false);
			}
#endif
			thirdendmoviestage++;
			if ( thirdendmoviestage >= thirdEndNumLines )
			{
				int c;
				for ( c = 0; c < 30; c++ )
				{
					thirdendmoviealpha[c] = 0;
				}
				fadefinished = false;
				fadeout = false;
				if ( multiplayer != CLIENT )
				{
					movie = false; // allow normal pause screen.
					thirdendmoviestage = 0;
					thirdendmovietime = 0;
					introstage = 1; // return to normal game functionality
					skipLevelsOnLoad = 5;
					loadnextlevel = true; // load the next level.
					pauseGame(1, false); // unpause game
				}
			}
			else
			{
				fadefinished = false;
				fadeout = false;
				thirdendmovietime = 0;
				movie = true;
			}
		}
		else if ( introstage == 10 )     // expansion end game sequence
		{
#ifdef MUSIC
			if ( fourthendmoviestage == 0 )
			{
				playmusic(endgamemusic, true, true, false);
			}
#endif
			fourthendmoviestage++;
			if ( fourthendmoviestage >= fourthEndNumLines )
			{
				int c;
				for ( c = 0; c < 30; c++ )
				{
					fourthendmoviealpha[c] = 0;
				}
				introstage = 4;
				fourthendmovietime = 0;
				fourthendmoviestage = 0;
				fadeout = true;
			}
			else
			{
				fadefinished = false;
				fadeout = false;
				fourthendmovietime = 0;
				movie = true;
			}
		}
	}

	// credits sequence
	if ( creditstage > 0 )
	{
		if ( (credittime >= 300 && (creditstage <= 11 || creditstage > 13)) || (credittime >= 180 && creditstage == 12) ||
		        (credittime >= 480 && creditstage == 13) || mousestatus[SDL_BUTTON_LEFT] || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
		{
			mousestatus[SDL_BUTTON_LEFT] = 0;
			if ( rebindaction == -1 )
			{
				*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
			}
			introstage = 4;
			fadeout = true;
		}

		// stages
		Uint32 colorBlue = SDL_MapRGBA(mainsurface->format, 0, 92, 255, 255);
		if ( creditstage == 1 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[56]), yres / 2 - 9 - 18, colorBlue, language[56]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE02), yres / 2 - 9 + 18, CREDITSLINE02);
		}
		else if ( creditstage == 2 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[57]), yres / 2 - 9 - 18, colorBlue, language[57]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE04), yres / 2 - 9 + 18, CREDITSLINE04);
		}
		else if ( creditstage == 3 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[58]), yres / 2 - 9 - 18, colorBlue, language[58]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE06), yres / 2 - 9, CREDITSLINE06);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE40), yres / 2 - 9 + 18, CREDITSLINE40);
		}
		else if ( creditstage == 4 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[59]), yres / 2 - 9 - 18, colorBlue, language[59]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE39), yres / 2 + 9, CREDITSLINE39);
		}
		else if ( creditstage == 5 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[60]), yres / 2 - 9 - 18, colorBlue, language[60]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE11), yres / 2 - 9, CREDITSLINE11);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE08), yres / 2 - 9 + 18, CREDITSLINE08);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE09), yres / 2 + 9 + 18 * 1, CREDITSLINE09);
		}
		else if ( creditstage == 6 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[61]), yres / 2 - 9 - 18, colorBlue, language[61]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE13), yres / 2 - 9 + 18, CREDITSLINE13);
		}
		else if ( creditstage == 7 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[62]), yres / 2 - 9 - 18 * 4, colorBlue, language[62]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE15), yres / 2 - 9 - 18 * 2, CREDITSLINE15);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE16), yres / 2 - 9 - 18 * 1, CREDITSLINE16);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE17), yres / 2 - 9, CREDITSLINE17);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE18), yres / 2 + 9, CREDITSLINE18);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE19), yres / 2 + 9 + 18 * 1, CREDITSLINE19);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE20), yres / 2 + 9 + 18 * 2, CREDITSLINE20);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE21), yres / 2 + 9 + 18 * 3, CREDITSLINE21);
		}
		else if ( creditstage == 8 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[63]), yres / 2 - 9 - 18 * 4, colorBlue, language[63]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE23), yres / 2 - 9 - 18 * 2, CREDITSLINE23);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE24), yres / 2 - 9 - 18 * 1, CREDITSLINE24);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE25), yres / 2 - 9, CREDITSLINE25);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE26), yres / 2 + 9, CREDITSLINE26);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE27), yres / 2 + 9 + 18 * 1, CREDITSLINE27);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE28), yres / 2 + 9 + 18 * 2, CREDITSLINE28);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE29), yres / 2 + 9 + 18 * 3, CREDITSLINE29);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE30), yres / 2 + 9 + 18 * 4, CREDITSLINE30);
		}
		else if ( creditstage == 9 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[2585]), yres / 2 - 9 - 18, colorBlue, language[2585]);
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[2586]), yres / 2 - 9, colorBlue, language[2586]);
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[2587]), yres / 2 - 9 + 18, colorBlue, language[2587]);
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[2588]), yres / 2 + 9 + 18, colorBlue, language[2588]);
		}
		else if ( creditstage == 10 )
		{
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[64]), yres / 2 - 9 - 18, colorBlue, language[64]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[65]), yres / 2 - 9 + 18, language[65]);
		}
		else if ( creditstage == 11 )
		{
			// logo
			src.x = 0;
			src.y = 0;
			src.w = logo_bmp->w;
			src.h = logo_bmp->h;
			dest.x = xres / 2 - (logo_bmp->w + title_bmp->w) / 2 - 16;
			dest.y = yres / 2 - logo_bmp->h / 2;
			dest.w = xres;
			dest.h = yres;
			drawImage(logo_bmp, &src, &dest);
			// title
			src.x = 0;
			src.y = 0;
			src.w = title_bmp->w;
			src.h = title_bmp->h;
			dest.x = xres / 2 - (logo_bmp->w + title_bmp->w) / 2 + logo_bmp->w + 16;
			dest.y = yres / 2 - title_bmp->h / 2;
			dest.w = xres;
			dest.h = yres;
			drawImage(title_bmp, &src, &dest);
			// text
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[66]), yres / 2 + 96, language[66]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[67]), yres / 2 + 116, language[67]);
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[68]), yres / 2 + 136, language[68]);
			ttfPrintTextFormattedColor(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(language[69]), yres / 2 + 156, colorBlue, language[69]);
		}
		else if ( creditstage == 13 )
		{
			ttfPrintTextFormatted(ttf16, xres / 2 - (TTF16_WIDTH / 2)*strlen(CREDITSLINE37), yres / 2 - 9, CREDITSLINE37);
			//ttfPrintTextFormattedColor(ttf16,xres/2-(TTF16_WIDTH/2)*strlen(CREDITSLINE37),yres/2+9,colorBlue,CREDITSLINE38);
		}
	}

	// intro sequence
	if ( intromoviestage > 0 )
	{
		SDL_Rect pos;
		pos.x = 0;
		pos.y = 0;
		pos.w = xres;
		pos.h = (((real_t)xres) / backdrop_cursed_bmp->w) * backdrop_cursed_bmp->h;
		drawImageScaled(backdrop_cursed_bmp, NULL, &pos);

		if ( intromovietime >= 600 || mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_ESCAPE] ||
		        keystatus[SDL_SCANCODE_SPACE] || keystatus[SDL_SCANCODE_RETURN] || (intromovietime >= 120 && intromoviestage == 1) || (*inputPressed(joyimpulses[INJOY_MENU_NEXT]) && rebindaction == -1) )
		{
			intromovietime = 0;
			mousestatus[SDL_BUTTON_LEFT] = 0;
			if ( rebindaction == -1 )
			{
				*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
			}
			if ( intromoviestage != 9 )
			{
				intromoviestage++;
			}
			else
			{
				introstage = 6;
				fadeout = true;
			}
		}

		if ( intromoviestage >= 1 )
		{
			intromoviealpha[8] = std::min(intromoviealpha[8] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, intromoviealpha[8]), 255) << 24;
			ttfPrintTextColor(ttf16, 16, yres - 32, color, true, language[1414]);
		}
		if ( intromoviestage >= 2 )
		{
			intromoviealpha[0] = std::min(intromoviealpha[0] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, intromoviealpha[0]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1415]);
		}
		if ( intromoviestage >= 3 )
		{
			intromoviealpha[1] = std::min(intromoviealpha[1] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, intromoviealpha[1]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1416]);
		}
		if ( intromoviestage >= 4 )
		{
			intromoviealpha[2] = std::min(intromoviealpha[2] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, intromoviealpha[2]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1417]);
		}
		if ( intromoviestage >= 5 )
		{
			intromoviealpha[3] = std::min(intromoviealpha[3] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, intromoviealpha[3]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1418]);
		}
		if ( intromoviestage >= 6 )
		{
			intromoviealpha[4] = std::min(intromoviealpha[4] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, intromoviealpha[4]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1419]);
		}
		if ( intromoviestage >= 7 )
		{
			intromoviealpha[5] = std::min(intromoviealpha[5] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, intromoviealpha[5]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1420]);
		}
		if ( intromoviestage >= 8 )
		{
			intromoviealpha[6] = std::min(intromoviealpha[6] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, intromoviealpha[6]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1421]);
		}
		if ( intromoviestage == 9 )
		{
			intromoviealpha[7] = std::min(intromoviealpha[7] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, intromoviealpha[7]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1422]);
		}
	}

	// first end sequence (defeating herx)
	if ( firstendmoviestage > 0 )
	{
		SDL_Rect pos;
		pos.x = 0;
		pos.y = 0;
		pos.w = xres;
		pos.h = (((real_t)xres) / backdrop_minotaur_bmp->w) * backdrop_minotaur_bmp->h;
		drawImageScaled(backdrop_minotaur_bmp, NULL, &pos);

		if ( firstendmovietime >= 600 || mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_ESCAPE] ||
		        keystatus[SDL_SCANCODE_SPACE] || keystatus[SDL_SCANCODE_RETURN] || (firstendmovietime >= 120 && firstendmoviestage == 1) )
		{
			firstendmovietime = 0;
			mousestatus[SDL_BUTTON_LEFT] = 0;
			if ( firstendmoviestage != 5 )
			{
				firstendmoviestage++;
			}
			else
			{
				introstage = 7;
				fadeout = true;
			}
		}

		if ( firstendmoviestage >= 1 )
		{
			firstendmoviealpha[8] = std::min(firstendmoviealpha[8] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, firstendmoviealpha[8]), 255) << 24;
			ttfPrintTextColor(ttf16, 16, yres - 32, color, true, language[1414]);
		}
		if ( firstendmoviestage >= 2 )
		{
			firstendmoviealpha[0] = std::min(firstendmoviealpha[0] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, firstendmoviealpha[0]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1423]);
		}
		if ( firstendmoviestage >= 3 )
		{
			firstendmoviealpha[1] = std::min(firstendmoviealpha[1] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, firstendmoviealpha[1]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1424]);
		}
		if ( firstendmoviestage >= 4 )
		{
			firstendmoviealpha[2] = std::min(firstendmoviealpha[2] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, firstendmoviealpha[2]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1425]);
		}
		if ( firstendmoviestage == 5 )
		{
			firstendmoviealpha[3] = std::min(firstendmoviealpha[3] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, firstendmoviealpha[3]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1426]);
		}
	}

	// second end sequence (defeating the devil)
	if ( secondendmoviestage > 0 )
	{
		SDL_Rect pos;
		pos.x = 0;
		pos.y = 0;
		pos.w = xres;
		pos.h = (((real_t)xres) / backdrop_minotaur_bmp->w) * backdrop_minotaur_bmp->h;
		drawImageScaled(backdrop_minotaur_bmp, NULL, &pos);

		if ( secondendmovietime >= 600 || mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_ESCAPE] ||
		        keystatus[SDL_SCANCODE_SPACE] || keystatus[SDL_SCANCODE_RETURN] || (secondendmovietime >= 120 && secondendmoviestage == 1) )
		{
			secondendmovietime = 0;
			mousestatus[SDL_BUTTON_LEFT] = 0;
			if ( secondendmoviestage != 7 )
			{
				secondendmoviestage++;
			}
			else
			{
				introstage = 8;
				fadeout = true;
			}
		}

		if ( secondendmoviestage >= 1 )
		{
			secondendmoviealpha[8] = std::min(secondendmoviealpha[8] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, secondendmoviealpha[8]), 255) << 24;
			ttfPrintTextColor(ttf16, 16, yres - 32, color, true, language[1414]);
		}
		if ( secondendmoviestage >= 2 )
		{
			secondendmoviealpha[0] = std::min(secondendmoviealpha[0] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, secondendmoviealpha[0]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 22, color, true, language[1427]);
		}
		if ( secondendmoviestage >= 3 )
		{
			secondendmoviealpha[1] = std::min(secondendmoviealpha[1] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, secondendmoviealpha[1]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1428]);
		}
		if ( secondendmoviestage >= 4 )
		{
			secondendmoviealpha[2] = std::min(secondendmoviealpha[2] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, secondendmoviealpha[2]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1429]);
		}
		if ( secondendmoviestage >= 5 )
		{
			secondendmoviealpha[3] = std::min(secondendmoviealpha[3] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, secondendmoviealpha[3]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1430]);
		}
		if ( secondendmoviestage >= 6 )
		{
			secondendmoviealpha[4] = std::min(secondendmoviealpha[4] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, secondendmoviealpha[4]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1431]);
		}
		if ( secondendmoviestage == 7 )
		{
			secondendmoviealpha[5] = std::min(secondendmoviealpha[5] + 2, 255);
			Uint32 color = 0x00FFFFFF;
			color += std::min(std::max(0, secondendmoviealpha[5]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[1432]);
		}
	}

	// third end movie stage
	if ( thirdendmoviestage > 0 )
	{
		SDL_Rect pos;
		pos.x = 0;
		pos.y = 0;
		pos.w = xres;
		pos.h = (((real_t)xres) / backdrop_minotaur_bmp->w) * backdrop_minotaur_bmp->h;
		drawRect(&pos, 0, 255);
		drawImageScaled(backdrop_minotaur_bmp, NULL, &pos);

		if ( thirdendmovietime >= 600 || mousestatus[SDL_BUTTON_LEFT] || keystatus[SDL_SCANCODE_ESCAPE] ||
			keystatus[SDL_SCANCODE_SPACE] || keystatus[SDL_SCANCODE_RETURN] || (thirdendmovietime >= 120 && thirdendmoviestage == 1) )
		{
			thirdendmovietime = 0;
			mousestatus[SDL_BUTTON_LEFT] = 0;
			if ( thirdendmoviestage < thirdEndNumLines )
			{
				thirdendmoviestage++;
			}
			else if ( thirdendmoviestage == thirdEndNumLines )
			{
				if ( multiplayer != CLIENT )
				{
					fadeout = true;
					++thirdendmoviestage;
				}
			}
		}
		Uint32 color = 0x00FFFFFF;
		if ( thirdendmoviestage >= 1 )
		{
			thirdendmoviealpha[8] = std::min(thirdendmoviealpha[8] + 2, 255);
			color = 0x00FFFFFF;
			color += std::min(std::max(0, thirdendmoviealpha[8]), 255) << 24;
			ttfPrintTextColor(ttf16, 16, yres - 32, color, true, language[2606]);
		}
		if ( thirdendmoviestage >= 2 )
		{
			thirdendmoviealpha[0] = std::min(thirdendmoviealpha[0] + 2, 255);
			color = 0x00FFFFFF;
			color += std::min(std::max(0, thirdendmoviealpha[0]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2600]);
		}
		if ( thirdendmoviestage >= 3 )
		{
			thirdendmoviealpha[1] = std::min(thirdendmoviealpha[1] + 2, 255);
			color = 0x00FFFFFF;
			color += std::min(std::max(0, thirdendmoviealpha[1]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2601]);
		}
		if ( thirdendmoviestage >= 4 )
		{
			thirdendmoviealpha[2] = std::min(thirdendmoviealpha[2] + 2, 255);
			color = 0x00FFFFFF;
			color += std::min(std::max(0, thirdendmoviealpha[2]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2602]);
		}
		if ( thirdendmoviestage >= 5 )
		{
			thirdendmoviealpha[3] = std::min(thirdendmoviealpha[3] + 2, 255);
			color = 0x00FFFFFF;
			color += std::min(std::max(0, thirdendmoviealpha[3]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2603]);
		}
		if ( thirdendmoviestage >= 6 )
		{
			thirdendmoviealpha[4] = std::min(thirdendmoviealpha[4] + 2, 255);
			color = 0x00FFFFFF;
			color += std::min(std::max(0, thirdendmoviealpha[4]), 255) << 24;
			if ( multiplayer == CLIENT )
			{
				ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2605]);
			}
			else
			{
				ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2604]);
			}
		}
	}
	// fourth (expansion) end movie stage
	if ( fourthendmoviestage > 0 )
	{
		SDL_Rect pos;
		pos.x = 0;
		pos.y = 0;
		pos.w = xres;
		pos.h = (((real_t)xres) / backdrop_blessed_bmp->w) * backdrop_blessed_bmp->h;
		drawRect(&pos, 0, 255);
		drawImageScaled(backdrop_blessed_bmp, NULL, &pos);

		if ( fourthendmovietime >= 600 
			|| (mousestatus[SDL_BUTTON_LEFT] 
				&& fourthendmoviestage < 10 
				&& fourthendmoviestage != 10 
				&& fourthendmoviestage != 5
				&& fourthendmoviestage != 1)
			|| (fourthendmovietime >= 120 && fourthendmoviestage == 1)
			|| (fourthendmovietime >= 60 && fourthendmoviestage == 5)
			|| (fourthendmovietime >= 240 && fourthendmoviestage == 10)
			|| (fourthendmovietime >= 200 && fourthendmoviestage == 11)
			|| (fourthendmovietime >= 60 && fourthendmoviestage == 12)
			|| (fourthendmovietime >= 400 && fourthendmoviestage == 13)
			)
		{
			fourthendmovietime = 0;
			mousestatus[SDL_BUTTON_LEFT] = 0;
			if ( fourthendmoviestage < fourthEndNumLines )
			{
				fourthendmoviestage++;
			}
			else if ( fourthendmoviestage == fourthEndNumLines )
			{
				fadeout = true;
				introstage = 10;
			}
		}
		Uint32 color = 0x00FFFFFF;
		if ( fourthendmoviestage >= 1 )
		{
			fourthendmoviealpha[8] = std::min(fourthendmoviealpha[8] + 2, 255);
			color = 0x00FFFFFF;
			color += std::min(std::max(0, fourthendmoviealpha[8]), 255) << 24;
			ttfPrintTextColor(ttf16, 16, yres - 32, color, true, language[2606]);
		}
		if ( fourthendmoviestage >= 2 )
		{
			if ( fourthendmoviestage < 5 )
			{
				fourthendmoviealpha[0] = std::min(fourthendmoviealpha[0] + 2, 255);
			}
			color = 0x00FFFFFF;
			color += std::min(std::max(0, fourthendmoviealpha[0]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2607]);
		}
		if ( fourthendmoviestage >= 3 )
		{
			if ( fourthendmoviestage < 5 )
			{
				fourthendmoviealpha[1] = std::min(fourthendmoviealpha[1] + 2, 255);
			}
			color = 0x00FFFFFF;
			color += std::min(std::max(0, fourthendmoviealpha[1]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2608]);
		}
		if ( fourthendmoviestage >= 4 )
		{
			if ( fourthendmoviestage < 5 )
			{
				fourthendmoviealpha[2] = std::min(fourthendmoviealpha[2] + 2, 255);
			}
			color = 0x00FFFFFF;
			color += std::min(std::max(0, fourthendmoviealpha[2]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2609]);
		}
		if ( fourthendmoviestage >= 5 )
		{
			fourthendmoviealpha[0] = std::max(fourthendmoviealpha[2] - 2, 0);
			fourthendmoviealpha[1] = std::max(fourthendmoviealpha[2] - 2, 0);
			fourthendmoviealpha[2] = std::max(fourthendmoviealpha[2] - 2, 0);
		}
		if ( fourthendmoviestage >= 6 )
		{
			if ( fourthendmoviestage < 10 )
			{
				fourthendmoviealpha[3] = std::min(fourthendmoviealpha[3] + 2, 255);
			}
			color = 0x00FFFFFF;
			color += std::min(std::max(0, fourthendmoviealpha[3]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2610]);
		}
		if ( fourthendmoviestage >= 7 )
		{
			if ( fourthendmoviestage < 10 )
			{
				fourthendmoviealpha[4] = std::min(fourthendmoviealpha[4] + 2, 255);
			}
			color = 0x00FFFFFF;
			color += std::min(std::max(0, fourthendmoviealpha[4]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2611]);
		}
		if ( fourthendmoviestage >= 8 )
		{
			if ( fourthendmoviestage < 10 )
			{
				fourthendmoviealpha[5] = std::min(fourthendmoviealpha[5] + 2, 255);
			}
			color = 0x00FFFFFF;
			color += std::min(std::max(0, fourthendmoviealpha[5]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2612]);
		}
		if ( fourthendmoviestage >= 9 )
		{
			if ( fourthendmoviestage < 10 )
			{
				fourthendmoviealpha[6] = std::min(fourthendmoviealpha[6] + 2, 255);
			}
			color = 0x00FFFFFF;
			color += std::min(std::max(0, fourthendmoviealpha[6]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres - 960) / 2, 16 + (yres - 600) / 2, color, true, language[2613]);
		}
		if ( fourthendmoviestage >= 10 )
		{
			fourthendmoviealpha[3] = std::max(fourthendmoviealpha[3] - 2, 0);
			fourthendmoviealpha[4] = std::max(fourthendmoviealpha[4] - 2, 0);
			fourthendmoviealpha[5] = std::max(fourthendmoviealpha[5] - 2, 0);
			fourthendmoviealpha[6] = std::max(fourthendmoviealpha[6] - 2, 0);
		}
		if ( fourthendmoviestage >= 11 )
		{
			fourthendmoviealpha[7] = std::min(fourthendmoviealpha[7] + 2, 255);
			color = 0x00FFFFFF;
			color += std::min(std::max(0, fourthendmoviealpha[7]), 255) << 24;
			ttfPrintTextColor(ttf16, 16 + (xres/ 2) - 256, (yres / 2) - 64, color, true, language[2614]);
			if ( fourthendmovietime % 50 == 0 )
			{
				steamAchievement("BARONY_ACH_ALWAYS_WAITING");
			}
		}
		if ( fourthendmoviestage >= 13 )
		{
			fadealpha = std::min(fadealpha + 2, 255);
		}
	}
}

/*-------------------------------------------------------------------------------

	button functions

	this section contains numerous button functions for the game

-------------------------------------------------------------------------------*/

// opens the gameover window
void openGameoverWindow()
{
	node_t* node;

	subwindow = 1;
	subx1 = xres / 2 - 288;
	subx2 = xres / 2 + 288;
	suby1 = yres / 2 - 160;
	suby2 = yres / 2 + 160;
	button_t* button;

	// calculate player score
	char scorenum[16];
	score_t* score = scoreConstructor();
	Uint32 total = totalScore(score);
	snprintf(scorenum, 16, "%d\n\n", total);
	scoreDeconstructor((void*)score);

	bool madetop = false;
	list_t* scoresPtr = &topscores;
	if ( score->conductGameChallenges[CONDUCT_MULTIPLAYER] )
	{
		scoresPtr = &topscoresMultiplayer;
	}
	if ( !list_Size(scoresPtr) )
	{
		madetop = true;
	}
	else if ( list_Size(scoresPtr) < MAXTOPSCORES )
	{
		madetop = true;
	}
	else if ( totalScore((score_t*)scoresPtr->last->element) < total )
	{
		madetop = true;
	}

	shootmode = false;
	if ( multiplayer == SINGLE )
	{
		strcpy(subtext, language[1133]);

		strcat(subtext, language[1134]);

		strcat(subtext, language[1135]);
		strcat(subtext, scorenum);

		if ( madetop )
		{
			strcat(subtext, language[1136]);
		}
		else
		{
			strcat(subtext, language[1137]);
		}

		// identify all inventory items
		for ( node = stats[clientnum]->inventory.first; node != NULL; node = node->next )
		{
			Item* item = (Item*)node->element;
			item->identified = true;
		}

		// Restart
		button = newButton();
		strcpy(button->label, language[1138]);
		button->x = subx2 - strlen(language[1138]) * 12 - 16;
		button->y = suby2 - 28;
		button->sizex = strlen(language[1138]) * 12 + 8;
		button->sizey = 20;
		button->action = &buttonStartSingleplayer;
		button->visible = 1;
		button->focused = 1;
		button->joykey = joyimpulses[INJOY_MENU_NEXT];

		// Return to Main Menu
		button = newButton();
		strcpy(button->label, language[1139]);
		button->x = subx1 + 8;
		button->y = suby2 - 28;
		button->sizex = strlen(language[1139]) * 12 + 8;
		button->sizey = 20;
		button->action = &buttonEndGameConfirm;
		button->visible = 1;
		button->focused = 1;
		button->joykey = joyimpulses[INJOY_MENU_CANCEL];
	}
	else
	{
		strcpy(subtext, language[1140]);

		bool survivingPlayer = false;
		int c;
		for (c = 0; c < MAXPLAYERS; c++)
		{
			if (!client_disconnected[c] && players[c]->entity)
			{
				survivingPlayer = true;
				break;
			}
		}
		if ( survivingPlayer )
		{
			strcat(subtext, language[1141]);
		}
		else
		{
			strcat(subtext, language[1142]);
		}

		strcat(subtext, language[1143]);
		strcat(subtext, scorenum);

		strcat(subtext, "\n\n");

		// Okay
		button = newButton();
		strcpy(button->label, language[1144]);
		button->sizex = strlen(language[1144]) * 12 + 8;
		button->sizey = 20;
		button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
		button->y = suby2 - 28;
		button->action = &buttonCloseSubwindow;
		button->visible = 1;
		button->focused = 1;
		button->joykey = joyimpulses[INJOY_MENU_NEXT];
	}

	// death hints
	if ( currentlevel / LENGTH_OF_LEVEL_REGION < 1 )
	{
		strcat(subtext, language[1145 + rand() % 15]);
	}

	// close button
	button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];
}

// get
void getResolutionList()
{
	// for now just use the resolution modes on the first
	// display.
	int numdisplays = SDL_GetNumVideoDisplays();
	int nummodes = SDL_GetNumDisplayModes(0);
	int im;
	int c;

	printlog("display count: %d.\n", numdisplays);
	printlog("display mode count: %d.\n", nummodes);

	for (im = 0; im < nummodes; im++)
	{
		SDL_DisplayMode mode;
		SDL_GetDisplayMode(0, im, &mode);
		// resolutions below 960x600 are not supported
		if ( mode.w >= 960 && mode.h >= 600 )
		{
			resolution res(mode.w, mode.h);
			resolutions.push_back(res);
		}
	}

	// Sort by total number of pixels
	resolutions.sort([](resolution a, resolution b) {
		return std::get<0>(a) * std::get<1>(a) > std::get<0>(b) * std::get<1>(b);
	});
	resolutions.unique();
}

// sets up the settings window
void openSettingsWindow()
{
	button_t* button;
	int c;

	getResolutionList();

	// set the "settings" variables
	settings_xres = xres;
	settings_yres = yres;
	settings_fov = fov;
	settings_smoothlighting = smoothlighting;
	settings_fullscreen = fullscreen;
	settings_shaking = shaking;
	settings_bobbing = bobbing;
	settings_spawn_blood = spawn_blood;
	settings_light_flicker = flickerLights;
	settings_colorblind = colorblind;
	settings_gamma = vidgamma;
	settings_fps = fpsLimit;
	settings_sfxvolume = sfxvolume;
	settings_musvolume = musvolume;
	for (c = 0; c < NUMIMPULSES; c++)
	{
		settings_impulses[c] = impulses[c];
	}
	for (c = 0; c < NUM_JOY_IMPULSES; c++)
	{
		settings_joyimpulses[c] = joyimpulses[c];
	}
	settings_reversemouse = reversemouse;
	settings_smoothmouse = smoothmouse;
	settings_mousespeed = mousespeed;
	settings_broadcast = broadcast;
	settings_nohud = nohud;
	settings_auto_hotbar_new_items = auto_hotbar_new_items;
	for ( c = 0; c < NUM_HOTBAR_CATEGORIES; ++c )
	{
		settings_auto_hotbar_categories[c] = auto_hotbar_categories[c];
	}
	for ( c = 0; c < NUM_AUTOSORT_CATEGORIES; ++c )
	{
		settings_autosort_inventory_categories[c] = autosort_inventory_categories[c];
	}
	settings_hotbar_numkey_quick_add = hotbar_numkey_quick_add;
	settings_disable_messages = disable_messages;
	settings_right_click_protect = right_click_protect;
	settings_auto_appraise_new_items = auto_appraise_new_items;
	settings_lock_right_sidebar = lock_right_sidebar;
	settings_show_game_timer_always = show_game_timer_always;

	settings_gamepad_leftx_invert = gamepad_leftx_invert;
	settings_gamepad_lefty_invert = gamepad_lefty_invert;
	settings_gamepad_rightx_invert = gamepad_rightx_invert;
	settings_gamepad_righty_invert = gamepad_righty_invert;
	settings_gamepad_menux_invert = gamepad_menux_invert;
	settings_gamepad_menuy_invert = gamepad_menuy_invert;

	settings_gamepad_deadzone = gamepad_deadzone;
	settings_gamepad_rightx_sensitivity = gamepad_rightx_sensitivity;
	settings_gamepad_righty_sensitivity = gamepad_righty_sensitivity;
	settings_gamepad_menux_sensitivity = gamepad_menux_sensitivity;
	settings_gamepad_menuy_sensitivity = gamepad_menuy_sensitivity;

	// create settings window
	settings_window = true;
	subwindow = 1;
	//subx1 = xres/2-256;
	subx1 = xres / 2 - 448;
	//subx2 = xres/2+256;
	subx2 = xres / 2 + 448;
	//suby1 = yres/2-192;
	//suby2 = yres/2+192;
#ifdef PANDORA
	suby1 = yres / 2 - ((yres==480)?210:278);
	suby2 = yres / 2 + ((yres==480)?210:278);
#else
	suby1 = yres / 2 - 288;
	suby2 = yres / 2 + 288;
#endif
	strcpy(subtext, language[1306]);

	// close button
	button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSettingsSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// cancel button
	button = newButton();
	strcpy(button->label, language[1316]);
	button->x = subx1 + 8;
	button->y = suby2 - 28;
	button->sizex = strlen(language[1316]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;

	// ok button
	button = newButton();
	strcpy(button->label, language[1433]);
	button->x = subx2 - strlen(language[1433]) * 12 - 16;
	button->y = suby2 - 28;
	button->sizex = strlen(language[1433]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonSettingsOK;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;
	button->joykey = joyimpulses[INJOY_MENU_NEXT];

	// accept button
	button = newButton();
	strcpy(button->label, language[1317]);
	button->x = subx2 - strlen(language[1317]) * 12 - 16 - strlen(language[1317]) * 12 - 16;
	button->y = suby2 - 28;
	button->sizex = strlen(language[1317]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonSettingsAccept;
	button->visible = 1;
	button->focused = 1;

	int tabx_so_far = subx1 + 16;

	//TODO: Select tab based off of dpad left & right.
	//TODO: Maybe golden highlighting & stuff.

	// video tab
	button = newButton();
	strcpy(button->label, language[1434]);
	button->x = tabx_so_far;
	button->y = suby1 + 24;
	button->sizex = strlen(language[1434]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonVideoTab;
	button->visible = 1;
	button->focused = 1;
	button_video_tab = button;

	tabx_so_far += strlen(language[1434]) * 12 + 8;

	// audio tab
	button = newButton();
	strcpy(button->label, language[1435]);
	button->x = tabx_so_far;
	button->y = suby1 + 24;
	button->sizex = strlen(language[1435]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonAudioTab;
	button->visible = 1;
	button->focused = 1;
	button_audio_tab = button;

	tabx_so_far += strlen(language[1435]) * 12 + 8;

	// keyboard tab
	button = newButton();
	strcpy(button->label, language[1436]);
	button->x = tabx_so_far;
	button->y = suby1 + 24;
	button->sizex = strlen(language[1436]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonKeyboardTab;
	button->visible = 1;
	button->focused = 1;
	button_keyboard_tab = button;

	tabx_so_far += strlen(language[1436]) * 12 + 8;

	// mouse tab
	button = newButton();
	strcpy(button->label, language[1437]);
	button->x = tabx_so_far;
	button->y = suby1 + 24;
	button->sizex = strlen(language[1437]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonMouseTab;
	button->visible = 1;
	button->focused = 1;
	button_mouse_tab = button;

	tabx_so_far += strlen(language[1437]) * 12 + 8;

	//Gamepad bindings tab.
	button = newButton();
	strcpy(button->label, language[1947]);
	button->x = tabx_so_far;
	button->y = suby1 + 24;
	button->sizex = strlen(language[1947]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonGamepadBindingsTab;
	button->visible = 1;
	button->focused = 1;
	button_gamepad_bindings_tab = button;

	tabx_so_far += strlen(language[1947]) * 12 + 8;

	//Gamepad settings tab.
	button = newButton();
	strcpy(button->label, language[2400]);
	button->x = tabx_so_far;
	button->y = suby1 + 24;
	button->sizex = strlen(language[2400]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonGamepadSettingsTab;
	button->visible = 1;
	button->focused = 1;
	button_gamepad_settings_tab = button;

	tabx_so_far += strlen(language[2400]) * 12 + 8;

	// misc tab
	button = newButton();
	strcpy(button->label, language[1438]);
	button->x =  tabx_so_far;
	button->y = suby1 + 24;
	button->sizex = strlen(language[1438]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonMiscTab;
	button->visible = 1;
	button->focused = 1;
	button_misc_tab = button;

	//Initialize resolution confirmation window related variables.
	resolutionChanged = false;
	resolutionConfirmationTimer = 0;

	changeSettingsTab(settings_tab);
}

void openSteamLobbyWaitWindow(button_t* my);

// "failed to connect" message
void openFailedConnectionWindow(int mode)
{
	button_t* button;

	// close current window
	buttonCloseSubwindow(NULL);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	// create new window
	subwindow = 1;
	subx1 = xres / 2 - 256;
	subx2 = xres / 2 + 256;
	suby1 = yres / 2 - 64;
	suby2 = yres / 2 + 64;
	if ( directConnect )
	{
		if ( mode == CLIENT )
		{
			strcpy(subtext, language[1439]);
			strcat(subtext, SDLNet_GetError());
		}
		else if ( mode == SERVER )
		{
			strcpy(subtext, language[1440]);
			strcat(subtext, SDLNet_GetError());
		}
		else
		{
			strcpy(subtext, language[1443]);
		}
	}
	else
	{
		if ( mode == CLIENT )
		{
			strcpy(subtext, language[1441]);
		}
		else if ( mode == SERVER )
		{
			strcpy(subtext, language[1442]);
		}
		else
		{
			strcpy(subtext, language[1443]);
		}
	}

	// close button
	button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// okay button
	button = newButton();
	strcpy(button->label, language[732]);
	button->x = subx2 - (subx2 - subx1) / 2 - strlen(language[732]) * 6;
	button->y = suby2 - 24;
	button->sizex = strlen(language[732]) * 12 + 8;
	button->sizey = 20;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;
	button->joykey = joyimpulses[INJOY_MENU_NEXT];

	if ( directConnect )
	{
		if ( mode == CLIENT )
		{
			button->action = &buttonJoinMultiplayer;
		}
		else if ( mode == SERVER )
		{
			button->action = &buttonHostMultiplayer;
		}
		else
		{
			button->action = &buttonCloseSubwindow;
		}
	}
	else
	{
		if ( mode == CLIENT )
		{
			button->action = &openSteamLobbyWaitWindow;
		}
		else if ( mode == SERVER )
		{
			button->action = &buttonCloseSubwindow;
		}
		else
		{
			button->action = &buttonCloseSubwindow;
		}
	}

	multiplayer = SINGLE;
	clientnum = 0;
}

// opens the wait window for steam lobby (getting lobby list, etc.)
void openSteamLobbyWaitWindow(button_t* my)
{
	button_t* button;

	// close current window
	buttonCloseSubwindow(NULL);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	// create new window
	subwindow = 1;
#ifdef STEAMWORKS
	requestingLobbies = true;
#endif
	subx1 = xres / 2 - 256;
	subx2 = xres / 2 + 256;
	suby1 = yres / 2 - 64;
	suby2 = yres / 2 + 64;
	strcpy(subtext, language[1444]);
#ifdef STEAMWORKS
	//c_SteamMatchmaking_RequestLobbyList();
	//SteamMatchmaking()->RequestLobbyList(); //TODO: Is this sufficient for it to work?
	cpp_SteamMatchmaking_RequestLobbyList();
#endif

	// close button
	button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// cancel button
	button = newButton();
	strcpy(button->label, language[1316]);
	button->sizex = strlen(language[1316]) * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 28;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
}

// opens the lobby browser window (steam client only)
void openSteamLobbyBrowserWindow(button_t* my)
{
	button_t* button;

	// close current window
	buttonCloseSubwindow(NULL);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	// create new window
	subwindow = 1;
	subx1 = xres / 2 - 280;
	subx2 = xres / 2 + 280;
	suby1 = yres / 2 - 192;
	suby2 = yres / 2 + 192;
	strcpy(subtext, language[1334]);

	// setup lobby browser
#ifdef STEAMWORKS //TODO: Should this whole function be ifdeffed?
	selectedSteamLobby = 0;
#endif
	slidery = 0;
	oslidery = 0;

	// close button
	button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// join button
	button = newButton();
	strcpy(button->label, language[1445]);
	button->x = subx1 + 8;
	button->y = suby2 - 56;
	button->sizex = strlen(language[1445]) * 12 + 8;
	button->sizey = 20;
#ifdef STEAMWORKS
	button->action = &buttonSteamLobbyBrowserJoinGame;
#endif
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;
	button->joykey = joyimpulses[INJOY_MENU_NEXT];

	// refresh button
	button = newButton();
	strcpy(button->label, language[1446]);
	button->x = subx1 + 8;
	button->y = suby2 - 28;
	button->sizex = strlen(language[1446]) * 12 + 8;
	button->sizey = 20;
#ifdef STEAMWORKS
	button->action = &buttonSteamLobbyBrowserRefresh;
#endif
	button->visible = 1;
	button->focused = 1;
	button->joykey = joyimpulses[INJOY_MENU_REFRESH_LOBBY]; //"y" refreshes
}

// steam lobby browser join game
void buttonSteamLobbyBrowserJoinGame(button_t* my)
{
#ifndef STEAMWORKS
	return;
#else

	button_t* button;
	int lobbyIndex = std::min(std::max(0, selectedSteamLobby), MAX_STEAM_LOBBIES - 1);
	if ( lobbyIDs[lobbyIndex] )
	{
		// close current window
		int temp1 = connectingToLobby;
		int temp2 = connectingToLobbyWindow;
		//buttonCloseSubwindow(my);
		list_FreeAll(&button_l);
		deleteallbuttons = true;
		connectingToLobby = temp1;
		connectingToLobbyWindow = temp2;

		// create new window
		subwindow = 1;
		subx1 = xres / 2 - 256;
		subx2 = xres / 2 + 256;
		suby1 = yres / 2 - 64;
		suby2 = yres / 2 + 64;
		strcpy(subtext, language[1447]);

		// close button
		button = newButton();
		strcpy(button->label, "x");
		button->x = subx2 - 20;
		button->y = suby1;
		button->sizex = 20;
		button->sizey = 20;
		button->action = &openSteamLobbyWaitWindow;
		button->visible = 1;
		button->focused = 1;
		button->key = SDL_SCANCODE_ESCAPE;
		button->joykey = joyimpulses[INJOY_MENU_CANCEL];

		// cancel button
		button = newButton();
		strcpy(button->label, language[1316]);
		button->sizex = strlen(language[1316]) * 12 + 8;
		button->sizey = 20;
		button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
		button->y = suby2 - 28;
		button->action = &openSteamLobbyWaitWindow;
		button->visible = 1;
		button->focused = 1;

		connectingToLobby = true;
		connectingToLobbyWindow = true;
		strncpy( currentLobbyName, lobbyText[lobbyIndex], 31 );
		cpp_SteamMatchmaking_JoinLobby(*static_cast<CSteamID* >(lobbyIDs[lobbyIndex]));
	}
#endif
}

// steam lobby browser refresh
void buttonSteamLobbyBrowserRefresh(button_t* my)
{
#ifndef STEAMWORKS
	return;
#else
	openSteamLobbyWaitWindow(my);
#endif
}

// quit game button
void buttonQuitConfirm(button_t* my)
{
	subwindow = 0;
	introstage = 2; // prepares to quit the whole game
	fadeout = true;
}

// quit game button (no save)
void buttonQuitNoSaveConfirm(button_t* my)
{
	buttonQuitConfirm(my);
	deleteSaveGame(multiplayer);

	// make a highscore!
	saveScore();
}

// end game button
bool savethisgame = false;
void buttonEndGameConfirm(button_t* my)
{
	savethisgame = false;
	subwindow = 0;
	introstage = 5; // prepares to end the current game (throws to main menu)
	fadeout = true;
	//Edge case for freeing channeled spells on a client.
	if (multiplayer == CLIENT)
	{
		list_FreeAll(&channeledSpells[clientnum]);
	}
	if ( !intro )
	{
		pauseGame(2, false);
	}
}

void buttonEndGameConfirmSave(button_t* my)
{
	subwindow = 0;
	introstage = 5; // prepares to end the current game (throws to main menu)
	fadeout = true;
	savethisgame = true;
	if ( !intro )
	{
		pauseGame(2, false);
	}
}

// generic close window button
void buttonCloseSubwindow(button_t* my)
{
	int c;
	for ( c = 0; c < 512; c++ )
	{
		keystatus[c] = 0;
	}
	if ( !subwindow )
	{
		return;
	}
	loadGameSaveShowRectangle = 0;
	singleplayerSavegameExists = false; // clear this value when closing window, user could delete savegame
	multiplayerSavegameExists = false;  // clear this value when closing window, user could delete savegame
	directoryPath = "";
	gamemodsWindowClearVariables();
	if ( score_window )
	{
		// reset class loadout
		stats[0]->sex = static_cast<sex_t>(0);
		stats[0]->appearance = 0;
		strcpy(stats[0]->name, "");
		stats[0]->type = HUMAN;
		client_classes[0] = 0;
		stats[0]->clearStats();
		initClass(0);
	}
	rebindkey = -1;
#ifdef STEAMWORKS
	requestingLobbies = false;
#endif
	score_window = 0;
	gamemods_window = 0;
	lobby_window = false;
	settings_window = false;
	connect_window = 0;
#ifdef STEAMWORKS
	if ( charcreation_step )
	{
		if ( lobbyToConnectTo )
		{
			// cancel lobby invitation acceptance
			cpp_Free_CSteamID(lobbyToConnectTo); //TODO: Bugger this.
			lobbyToConnectTo = NULL;
		}
	}
	connectingToLobbyWindow = false;
	connectingToLobby = false;
#endif
	charcreation_step = 0;
	subwindow = 0;
	if ( SDL_IsTextInputActive() )
	{
		SDL_StopTextInput();
	}
	playSound(138, 64);
}

void buttonCloseSettingsSubwindow(button_t* my)
{
	if ( rebindkey != -1 || rebindaction != -1 )
	{
		//Do not close settings subwindow if rebinding a key/gamepad button/whatever.
		return;
	}

	buttonCloseSubwindow(my);
}

void buttonCloseAndEndGameConfirm(button_t* my)
{
	//Edge case for freeing channeled spells on a client.
	if (multiplayer == CLIENT)
	{
		list_FreeAll(&channeledSpells[clientnum]);
	}
	buttonCloseSubwindow(my);
	buttonEndGameConfirmSave(my);
}

Uint32 charcreation_ticks = 0;

// move player forward through creation dialogue
void buttonContinue(button_t* my)
{
	button_t* button;

	if ( ticks - charcreation_ticks < TICKS_PER_SECOND / 10 )
	{
		return;
	}
	charcreation_ticks = ticks;
	if ( charcreation_step == 4 && !strcmp(stats[0]->name, "") )
	{
		return;
	}

	charcreation_step++;
	if ( charcreation_step == 4 )
	{
		inputstr = stats[0]->name;
		SDL_StartTextInput();
	}
	else if ( charcreation_step == 5 )
	{
		singleplayerSavegameExists = saveGameExists(true); // load the savegames and see if they exist, once off operation.
		multiplayerSavegameExists = saveGameExists(false); // load the savegames and see if they exist, once off operation.
		if ( SDL_IsTextInputActive() )
		{
			lastname = (string)stats[0]->name;
			SDL_StopTextInput();
		}
#ifdef STEAMWORKS
		if ( lobbyToConnectTo )
		{
			charcreation_step = 0;

			// close current window
			int temp1 = connectingToLobby;
			int temp2 = connectingToLobbyWindow;
			//buttonCloseSubwindow(my);
			list_FreeAll(&button_l);
			deleteallbuttons = true;
			connectingToLobby = temp1;
			connectingToLobbyWindow = temp2;

			// create new window
			subwindow = 1;
			subx1 = xres / 2 - 256;
			subx2 = xres / 2 + 256;
			suby1 = yres / 2 - 64;
			suby2 = yres / 2 + 64;
			strcpy(subtext, language[1447]);

			// close button
			button = newButton();
			strcpy(button->label, "x");
			button->x = subx2 - 20;
			button->y = suby1;
			button->sizex = 20;
			button->sizey = 20;
			button->action = &openSteamLobbyWaitWindow;
			button->visible = 1;
			button->focused = 1;
			button->key = SDL_SCANCODE_ESCAPE;
			button->joykey = joyimpulses[INJOY_MENU_CANCEL];

			// cancel button
			button = newButton();
			strcpy(button->label, language[1316]);
			button->sizex = strlen(language[1316]) * 12 + 8;
			button->sizey = 20;
			button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
			button->y = suby2 - 28;
			button->action = &openSteamLobbyWaitWindow;
			button->visible = 1;
			button->focused = 1;

			connectingToLobby = true;
			connectingToLobbyWindow = true;
			strncpy( currentLobbyName, "", 31 );
			cpp_SteamMatchmaking_JoinLobby(*static_cast<CSteamID*>(lobbyToConnectTo));
			cpp_Free_CSteamID(lobbyToConnectTo); //TODO: Bugger this.
			lobbyToConnectTo = NULL;
		}
#endif
	}
	else if ( charcreation_step == 6 )
	{
		// store this character into previous character.
		lastCreatedCharacterSex = stats[0]->sex;
		lastCreatedCharacterClass = client_classes[0];
		lastCreatedCharacterAppearance = stats[0]->appearance;
		
		if ( multiplayerselect == SINGLE )
		{
			buttonStartSingleplayer(my);
		}
		else if ( multiplayerselect == SERVER )
		{
#ifdef STEAMWORKS
			directConnect = false;
#else
			directConnect = true;
#endif
			buttonHostMultiplayer(my);
		}
		else if ( multiplayerselect == CLIENT )
		{
#ifndef STEAMWORKS
			directConnect = true;
			buttonJoinMultiplayer(my);
#else
			directConnect = false;
			openSteamLobbyWaitWindow(my);
#endif
		}
		else if ( multiplayerselect == DIRECTSERVER )
		{
			directConnect = true;
			buttonHostMultiplayer(my);
		}
		else if ( multiplayerselect == DIRECTCLIENT )
		{
			directConnect = true;
			buttonJoinMultiplayer(my);
		}
	}
}

// move player backward through creation dialogue
void buttonBack(button_t* my)
{
	charcreation_step--;
	if (charcreation_step < 4)
	{
		playing_random_char = false;
	}
	if (charcreation_step == 3)
	{
		// If we've backed out, save what name was input for later
		lastname = (string)inputstr;
		SDL_StopTextInput();
	}
	else if ( charcreation_step == 0 )
	{
		buttonCloseSubwindow(my);
	}
}

// start a singleplayer game
void buttonStartSingleplayer(button_t* my)
{
	buttonCloseSubwindow(my);
	multiplayer = SINGLE;
	numplayers = 0;
	introstage = 3;
	fadeout = true;
	if ( !intro )
	{
		pauseGame(2, false);
	}
}

// host a multiplayer game
void buttonHostMultiplayer(button_t* my)
{
	button_t* button;

	// refresh keepalive
	int c;
	for ( c = 0; c < MAXPLAYERS; c++ )
	{
		client_keepalive[c] = ticks;
	}

	if ( !directConnect )
	{
		snprintf(portnumber_char, 6, "%d", DEFAULT_PORT);
		buttonHostLobby(my);
	}
	else
	{
		// close current window
		buttonCloseSubwindow(my);
		list_FreeAll(&button_l);
		deleteallbuttons = true;

		// open port window
		connect_window = SERVER;
		subwindow = 1;
		subx1 = xres / 2 - 128;
		subx2 = xres / 2 + 128;
		suby1 = yres / 2 - 56;
		suby2 = yres / 2 + 56;
		strcpy(subtext, language[1448]);

		// close button
		button = newButton();
		strcpy(button->label, "x");
		button->x = subx2 - 20;
		button->y = suby1;
		button->sizex = 20;
		button->sizey = 20;
		button->action = &buttonCloseSubwindow;
		button->visible = 1;
		button->focused = 1;
		button->key = SDL_SCANCODE_ESCAPE;
		button->joykey = joyimpulses[INJOY_MENU_CANCEL];

		// host button
		button = newButton();
		strcpy(button->label, language[1449]);
		button->sizex = strlen(language[1449]) * 12 + 8;
		button->sizey = 20;
		button->x = subx2 - button->sizex - 4;
		button->y = suby2 - 24;
		button->action = &buttonHostLobby;
		button->visible = 1;
		button->focused = 1;
		button->key = SDL_SCANCODE_RETURN;
		button->joykey = joyimpulses[INJOY_MENU_NEXT];

		// cancel button
		button = newButton();
		strcpy(button->label, language[1316]);
		button->sizex = strlen(language[1316]) * 12 + 8;
		button->sizey = 20;
		button->x = subx1 + 4;
		button->y = suby2 - 24;
		button->action = &buttonCloseSubwindow;
		button->visible = 1;
		button->focused = 1;
		strcpy(portnumber_char, last_port); //Copy the last used port.
	}
}

// join a multiplayer game
void buttonJoinMultiplayer(button_t* my)
{
	button_t* button;

	// close current window
	buttonCloseSubwindow(my);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	// open port window
	connect_window = CLIENT;
	subwindow = 1;
	subx1 = xres / 2 - 210;
	subx2 = xres / 2 + 210;
	suby1 = yres / 2 - 56;
	suby2 = yres / 2 + 56;
	strcpy(subtext, language[1450]);

	// close button
	button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// join button
	button = newButton();
	strcpy(button->label, language[1451]);
	button->sizex = strlen(language[1451]) * 12 + 8;
	button->sizey = 20;
	button->x = subx2 - button->sizex - 4;
	button->y = suby2 - 24;
	button->action = &buttonJoinLobby;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;
	button->joykey = joyimpulses[INJOY_MENU_NEXT];

	// cancel button
	button = newButton();
	strcpy(button->label, language[1316]);
	button->x = subx1 + 4;
	button->y = suby2 - 24;
	button->sizex = strlen(language[1316]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;

	strcpy(connectaddress, last_ip); //Copy the last used IP.
}

// starts a lobby as host
void buttonHostLobby(button_t* my)
{
	button_t* button;
	char *portnumbererr;
	int c;

	// close current window
	buttonCloseSubwindow(my);
	list_FreeAll(&button_l);
	deleteallbuttons = true;
	portnumber = (Uint16)strtol(portnumber_char, &portnumbererr, 10); // get the port number from the text field
	list_FreeAll(&lobbyChatboxMessages);

	if ( *portnumbererr != '\0' || portnumber < 1024 )
	{
		printlog("warning: invalid port number: %d\n", portnumber);
		openFailedConnectionWindow(SERVER);
		return;
	}

	newString(&lobbyChatboxMessages, 0xFFFFFFFF, language[1452]);
	if ( loadingsavegame )
	{
		newString(&lobbyChatboxMessages, 0xFFFFFFFF, language[1453]);
	}

	// close any existing net interfaces
	closeNetworkInterfaces();

	if ( !directConnect )
	{
#ifdef STEAMWORKS
		for ( c = 0; c < MAXPLAYERS; c++ )
		{
			if ( steamIDRemote[c] )
			{
				cpp_Free_CSteamID( steamIDRemote[c] ); //TODO: Bugger this.
				steamIDRemote[c] = NULL;
			}
		}
		currentLobbyType = k_ELobbyTypePrivate;
		cpp_SteamMatchmaking_CreateLobby(currentLobbyType, 4);
#endif
	}
	else
	{
		// resolve host's address
		if (SDLNet_ResolveHost(&net_server, NULL, portnumber) == -1)
		{
			printlog( "warning: resolving host at localhost:%d has failed.\n", portnumber);
			openFailedConnectionWindow(SERVER);
			return;
		}

		// open sockets
		if (!(net_sock = SDLNet_UDP_Open(portnumber)))
		{
			printlog( "warning: SDLNet_UDP_open has failed: %s\n", SDLNet_GetError());
			openFailedConnectionWindow(SERVER);
			return;
		}
		if (!(net_tcpsock = SDLNet_TCP_Open(&net_server)))
		{
			printlog( "warning: SDLNet_TCP_open has failed: %s\n", SDLNet_GetError());
			openFailedConnectionWindow(SERVER);
			return;
		}
		tcpset = SDLNet_AllocSocketSet(4);
		SDLNet_TCP_AddSocket(tcpset, net_tcpsock);
	}

	// allocate data for client connections
	net_clients = (IPaddress*) malloc(sizeof(IPaddress) * MAXPLAYERS);
	net_tcpclients = (TCPsocket*) malloc(sizeof(TCPsocket) * MAXPLAYERS);
	for ( c = 0; c < MAXPLAYERS; c++ )
	{
		net_tcpclients[c] = NULL;
	}

	// allocate packet data
	if (!(net_packet = SDLNet_AllocPacket(NET_PACKET_SIZE)))
	{
		printlog( "warning: packet allocation failed: %s\n", SDLNet_GetError());
		openFailedConnectionWindow(SERVER);
		return;
	}

	if ( directConnect )
	{
		printlog( "server initialized successfully.\n");
	}
	else
	{
		printlog( "steam lobby opened successfully.\n");
	}

	// open lobby window
	multiplayer = SERVER;
	lobby_window = true;
	subwindow = 1;
	subx1 = xres / 2 - 400;
	subx2 = xres / 2 + 400;
#ifdef PANDORA
	suby1 = yres / 2 - ((yres==480)?230:290);
	suby2 = yres / 2 + ((yres==480)?230:290);
#else
	suby1 = yres / 2 - 300;
	suby2 = yres / 2 + 300;
#endif
	if ( directConnect )
	{
		strcpy(subtext, language[1454]);
		strcat(subtext, portnumber_char);
		strcat(subtext, language[1456]);
	}
	else
	{
		strcpy(subtext, language[1455]);
		strcat(subtext, language[1456]);
	}

	// start game button
	button = newButton();
	strcpy(button->label, language[1457]);
	button->sizex = strlen(language[1457]) * 12 + 8;
	button->sizey = 20;
	button->x = subx2 - button->sizex - 4;
	button->y = suby2 - 24;
	button->action = &buttonStartServer;
	button->visible = 1;
	button->focused = 1;
	button->joykey = joyimpulses[INJOY_MENU_NEXT];

	// disconnect button
	button = newButton();
	strcpy(button->label, language[1311]);
	button->sizex = strlen(language[1311]) * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + 4;
	button->y = suby2 - 24;
	button->action = &buttonDisconnect;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];
	c = button->x + button->sizex + 4;

	// invite friends button
	if ( !directConnect )
	{
#ifdef STEAMWORKS
		button = newButton();
		strcpy(button->label, language[1458]);
		button->sizex = strlen(language[1458]) * 12 + 8;
		button->sizey = 20;
		button->x = c;
		button->y = suby2 - 24;
		button->action = &buttonInviteFriends;
		button->visible = 1;
		button->focused = 1;
#endif
	}

	if ( loadingsavegame )
	{
		loadGame(clientnum);
	}

	strcpy(last_port, portnumber_char);
	saveConfig("default.cfg");
}

// joins a lobby as client
void buttonJoinLobby(button_t* my)
{
	button_t* button;
	int c;

	// refresh keepalive
	client_keepalive[0] = ticks;

	// close current window
#ifdef STEAMWORKS
	int temp1 = connectingToLobby;
	int temp2 = connectingToLobbyWindow;
#endif
	if ( directConnect )
	{
		buttonCloseSubwindow(my);
	}
	list_FreeAll(&button_l);
	deleteallbuttons = true;
#ifdef STEAMWORKS
	connectingToLobby = temp1;
	connectingToLobbyWindow = temp2;
#endif

	multiplayer = CLIENT;
	if ( loadingsavegame )
	{
		loadGame(getSaveGameClientnum(false));
	}

	// open wait window
	list_FreeAll(&lobbyChatboxMessages);
	newString(&lobbyChatboxMessages, 0xFFFFFFFF, language[1452]);
	subwindow = 1;
	subx1 = xres / 2 - 256;
	subx2 = xres / 2 + 256;
	suby1 = yres / 2 - 64;
	suby2 = yres / 2 + 64;
	strcpy(subtext, language[1459]);

	// close button
	button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &openSteamLobbyWaitWindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// cancel button
	button = newButton();
	strcpy(button->label, language[1316]);
	button->sizex = strlen(language[1316]) * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 28;
	button->action = &openSteamLobbyWaitWindow;
	button->visible = 1;
	button->focused = 1;

	if ( directConnect )
	{
		for ( c = 0; c < sizeof(connectaddress); c++ )
		{
			if ( connectaddress[c] == ':' )
			{
				break;
			}
		}		
		char *portnumbererr;
		strncpy(address, connectaddress, c); // get the address from the text field
		portnumber = (Uint16)strtol(&connectaddress[c + 1], &portnumbererr, 10); // get the port number from the text field
		if ( *portnumbererr != '\0' || portnumber < 1024 )
		{
			printlog("warning: invalid port number %d.\n", portnumber);
			openFailedConnectionWindow(CLIENT);
			return;
		}
		strcpy(last_ip, connectaddress);
		saveConfig("default.cfg");
	}

	// close any existing net interfaces
	closeNetworkInterfaces();

	if ( directConnect )
	{
		// resolve host's address
		printlog("resolving host's address at %s:%d...\n", address, portnumber);
		if (SDLNet_ResolveHost(&net_server, address, portnumber) == -1)
		{
			printlog( "warning: resolving host at %s:%d has failed.\n", address, portnumber);
			openFailedConnectionWindow(CLIENT);
			return;
		}

		// open sockets
		printlog("opening TCP and UDP sockets...\n");
		if (!(net_sock = SDLNet_UDP_Open(0)))
		{
			printlog( "warning: SDLNet_UDP_open has failed.\n");
			openFailedConnectionWindow(CLIENT);
			return;
		}
		if (!(net_tcpsock = SDLNet_TCP_Open(&net_server)))
		{
			printlog( "warning: SDLNet_TCP_open has failed.\n");
			openFailedConnectionWindow(CLIENT);
			return;
		}
		tcpset = SDLNet_AllocSocketSet(4);
		SDLNet_TCP_AddSocket(tcpset, net_tcpsock);
	}

	// allocate packet data
	if (!(net_packet = SDLNet_AllocPacket(NET_PACKET_SIZE)))
	{
		printlog( "warning: packet allocation failed.\n");
		openFailedConnectionWindow(CLIENT);
		return;
	}

	if ( directConnect )
	{
		printlog( "successfully contacted server at %s:%d.\n", address, portnumber);
	}

	printlog( "submitting join request...\n");

	// send join request
	strcpy((char*)net_packet->data, "BARONY_JOIN_REQUEST");
	if ( loadingsavegame )
	{
		strncpy((char*)net_packet->data + 19, stats[getSaveGameClientnum(false)]->name, 22);
		SDLNet_Write32((Uint32)client_classes[getSaveGameClientnum(false)], &net_packet->data[42]);
		SDLNet_Write32((Uint32)stats[getSaveGameClientnum(false)]->sex, &net_packet->data[46]);
		SDLNet_Write32((Uint32)stats[getSaveGameClientnum(false)]->appearance, &net_packet->data[50]);
		strcpy((char*)net_packet->data + 54, VERSION);
		net_packet->data[62] = 0;
		net_packet->data[63] = getSaveGameClientnum(false);
	}
	else
	{
		strncpy((char*)net_packet->data + 19, stats[0]->name, 22);
		SDLNet_Write32((Uint32)client_classes[0], &net_packet->data[42]);
		SDLNet_Write32((Uint32)stats[0]->sex, &net_packet->data[46]);
		SDLNet_Write32((Uint32)stats[0]->appearance, &net_packet->data[50]);
		strcpy((char*)net_packet->data + 54, VERSION);
		net_packet->data[62] = 0;
		net_packet->data[63] = 0;
	}
	if ( loadingsavegame )
	{
		// send over the map seed being used
		SDLNet_Write32(getSaveGameMapSeed(false), &net_packet->data[64]);
	}
	else
	{
		SDLNet_Write32(0, &net_packet->data[64]);
	}
	SDLNet_Write32(loadingsavegame, &net_packet->data[68]); // send unique game key
	net_packet->address.host = net_server.host;
	net_packet->address.port = net_server.port;
	net_packet->len = 72;
	if ( !directConnect )
	{
#ifdef STEAMWORKS
		sendPacket(net_sock, -1, net_packet, 0);
		SDL_Delay(5);
		sendPacket(net_sock, -1, net_packet, 0);
		SDL_Delay(5);
		sendPacket(net_sock, -1, net_packet, 0);
		SDL_Delay(5);
		sendPacket(net_sock, -1, net_packet, 0);
		SDL_Delay(5);
		sendPacket(net_sock, -1, net_packet, 0);
		SDL_Delay(5);
#endif
	}
	else
	{
		sendPacket(net_sock, -1, net_packet, 0);
	}
}

// starts the game as server
void buttonStartServer(button_t* my)
{
	int c;

	// close window
	buttonCloseSubwindow(my);

	multiplayer = SERVER;
	intro = true;
	introstage = 3;
	numplayers = 0;
	fadeout = true;

	// send the ok to start
	for ( c = 1; c < MAXPLAYERS; c++ )
	{
		if ( !client_disconnected[c] )
		{
			if ( !loadingsavegame || !intro )
			{
				stats[c]->clearStats();
				initClass(c);
			}
			else
			{
				loadGame(c);
			}
		}
	}
	uniqueGameKey = prng_get_uint();
	if ( !uniqueGameKey )
	{
		uniqueGameKey++;
	}
	for ( c = 1; c < MAXPLAYERS; c++ )
	{
		if ( client_disconnected[c] )
		{
			continue;
		}
		strcpy((char*)net_packet->data, "BARONY_GAME_START");
		SDLNet_Write32(svFlags, &net_packet->data[17]);
		SDLNet_Write32(uniqueGameKey, &net_packet->data[21]);
		net_packet->address.host = net_clients[c - 1].host;
		net_packet->address.port = net_clients[c - 1].port;
		net_packet->len = 25;
		sendPacketSafe(net_sock, -1, net_packet, c - 1);
	}
}

// opens the steam dialog to invite friends
#ifdef STEAMWORKS
void buttonInviteFriends(button_t* my)
{
	if (SteamUser()->BLoggedOn())
	{
		SteamFriends()->ActivateGameOverlayInviteDialog(*static_cast<CSteamID*>(currentLobby));
	}
	return;
}
#endif

// disconnects from whatever lobby the game is connected to
void buttonDisconnect(button_t* my)
{
	int c;

	if ( multiplayer == SERVER )
	{
		// send disconnect message to clients
		for ( c = 1; c < MAXPLAYERS; c++ )
		{
			if ( client_disconnected[c] )
			{
				continue;
			}
			strcpy((char*)net_packet->data, "PLAYERDISCONNECT");
			net_packet->data[16] = clientnum;
			net_packet->address.host = net_clients[c - 1].host;
			net_packet->address.port = net_clients[c - 1].port;
			net_packet->len = 17;
			sendPacketSafe(net_sock, -1, net_packet, c - 1);
		}
	}
	else
	{
		// send disconnect message to server
		strcpy((char*)net_packet->data, "PLAYERDISCONNECT");
		net_packet->data[16] = clientnum;
		net_packet->address.host = net_server.host;
		net_packet->address.port = net_server.port;
		net_packet->len = 17;
		sendPacketSafe(net_sock, -1, net_packet, 0);
	}

	// reset multiplayer status
	multiplayer = SINGLE;
	stats[0]->sex = stats[clientnum]->sex;
	client_classes[0] = client_classes[clientnum];
	strcpy(stats[0]->name, stats[clientnum]->name);
	clientnum = 0;
	client_disconnected[0] = false;
	for ( c = 1; c < MAXPLAYERS; c++ )
	{
		client_disconnected[c] = true;
	}

	// close any existing net interfaces
	closeNetworkInterfaces();
#ifdef STEAMWORKS
	if ( currentLobby )
	{
		SteamMatchmaking()->LeaveLobby(*static_cast<CSteamID*>(currentLobby));
		cpp_Free_CSteamID(currentLobby); //TODO: Bugger this.
		currentLobby = NULL;
	}
#endif

	// close lobby window
	buttonCloseSubwindow(my);
}

// open the video tab in the settings window
void buttonVideoTab(button_t* my)
{
	changeSettingsTab(SETTINGS_VIDEO_TAB);
}

// open the audio tab in the settings window
void buttonAudioTab(button_t* my)
{
	changeSettingsTab(SETTINGS_AUDIO_TAB);
}

// open the keyboard tab in the settings window
void buttonKeyboardTab(button_t* my)
{
	changeSettingsTab(SETTINGS_KEYBOARD_TAB);
}

// open the mouse tab in the settings window
void buttonMouseTab(button_t* my)
{
	changeSettingsTab(SETTINGS_MOUSE_TAB);
}

//Open the gamepad bindings tab in the settings window
void buttonGamepadBindingsTab(button_t* my)
{
	changeSettingsTab(SETTINGS_GAMEPAD_BINDINGS_TAB);
}

//Open the general gamepad settings tab in the settings window
void buttonGamepadSettingsTab(button_t* my)
{
	changeSettingsTab(SETTINGS_GAMEPAD_SETTINGS_TAB);
}

// open the misc tab in the settings window
void buttonMiscTab(button_t* my)
{
	changeSettingsTab(SETTINGS_MISC_TAB);
}

void applySettings()
{
	int c;

	// set video options
	fov = settings_fov;
	smoothlighting = settings_smoothlighting;
	oldFullscreen = fullscreen;
	fullscreen = settings_fullscreen;
	shaking = settings_shaking;
	bobbing = settings_bobbing;
	spawn_blood = settings_spawn_blood;
	flickerLights = settings_light_flicker;
	colorblind = settings_colorblind;
	oldGamma = vidgamma;
	vidgamma = settings_gamma;
	fpsLimit = settings_fps;
	oldXres = xres;
	oldYres = yres;
	xres = settings_xres;
	yres = settings_yres;
	camera.winx = 0;
	camera.winy = 0;
	camera.winw = std::min(camera.winw, xres);
	camera.winh = std::min(camera.winh, yres);
	if(xres!=oldXres || yres!=oldYres || oldFullscreen!=fullscreen || oldGamma!=vidgamma)
	{
		if ( !changeVideoMode() )
		{
			printlog("critical error! Attempting to abort safely...\n");
			mainloop = 0;
		}
		if ( zbuffer != NULL )
		{
			free(zbuffer);
		}
		zbuffer = (real_t*) malloc(sizeof(real_t) * xres * yres);
		if ( clickmap != NULL )
		{
			free(clickmap);
		}
		clickmap = (Entity**) malloc(sizeof(Entity*)*xres * yres);
	}
	// set audio options
	sfxvolume = settings_sfxvolume;
	musvolume = settings_musvolume;

#ifdef USE_FMOD
	FMOD_ChannelGroup_SetVolume(music_group, musvolume / 128.f);
	FMOD_ChannelGroup_SetVolume(sound_group, sfxvolume / 128.f);
#elif defined USE_OPENAL
	OPENAL_ChannelGroup_SetVolume(music_group, musvolume / 128.f);
	OPENAL_ChannelGroup_SetVolume(sound_group, sfxvolume / 128.f);
#endif

	// set keyboard options
	for (c = 0; c < NUMIMPULSES; c++)
	{
		impulses[c] = settings_impulses[c];
	}
	for (c = 0; c < NUM_JOY_IMPULSES; c++)
	{
		joyimpulses[c] = settings_joyimpulses[c];
	}

	// set mouse options
	reversemouse = settings_reversemouse;
	smoothmouse = settings_smoothmouse;
	mousespeed = settings_mousespeed;

	// set misc options
	broadcast = settings_broadcast;
	nohud = settings_nohud;

	auto_hotbar_new_items = settings_auto_hotbar_new_items;
	for ( c = 0; c < NUM_HOTBAR_CATEGORIES; ++c )
	{
		auto_hotbar_categories[c] = settings_auto_hotbar_categories[c];
	}
	for ( c = 0; c < NUM_AUTOSORT_CATEGORIES; ++c )
	{
		autosort_inventory_categories[c] = settings_autosort_inventory_categories[c];
	}
	hotbar_numkey_quick_add = settings_hotbar_numkey_quick_add;
	disable_messages = settings_disable_messages;
	right_click_protect = settings_right_click_protect;
	auto_appraise_new_items = settings_auto_appraise_new_items;
	lock_right_sidebar = settings_lock_right_sidebar;
	show_game_timer_always = settings_show_game_timer_always;

	gamepad_leftx_invert = settings_gamepad_leftx_invert;
	gamepad_lefty_invert = settings_gamepad_lefty_invert;
	gamepad_rightx_invert = settings_gamepad_rightx_invert;
	gamepad_righty_invert = settings_gamepad_righty_invert;
	gamepad_menux_invert = settings_gamepad_menux_invert;
	gamepad_menuy_invert = settings_gamepad_menuy_invert;


	gamepad_deadzone = settings_gamepad_deadzone;
	gamepad_rightx_sensitivity = settings_gamepad_rightx_sensitivity;
	gamepad_righty_sensitivity = settings_gamepad_righty_sensitivity;
	gamepad_menux_sensitivity = settings_gamepad_menux_sensitivity;
	gamepad_menuy_sensitivity = settings_gamepad_menuy_sensitivity;

	saveConfig("default.cfg");
}

void openConfirmResolutionWindow()
{
	mousestatus[SDL_BUTTON_LEFT] = 0;
	keystatus[SDL_SCANCODE_RETURN] = 0;
	*inputPressed(joyimpulses[INJOY_MENU_NEXT]) = 0;
	playSound(139, 64);

	//Create confirmation window
	subwindow = 1;
	subx1 = xres / 2 - 128;
	subx2 = xres / 2 + 128;
	suby1 = yres / 2 - 40;
	suby2 = yres / 2 + 40;
	strcpy(subtext, "Testing resolution.\nWill revert in 10 seconds.");

	//Accept button
	button_t* button = newButton();
	strcpy(button->label, "Accept");
	button->x = subx1 + 8;
	button->y = suby2 - 28;
	button->sizex = strlen("Accept") * 12 + 8;
	button->sizey = 20;
	button->action = &buttonAcceptResolution;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;
	button->joykey = joyimpulses[INJOY_MENU_NEXT];

	//Revert button
	button = newButton();
	strcpy(button->label, "Revert");
	button->x = subx2 - strlen("Revert") * 12 - 16;
	button->y = suby2 - 28;
	button->sizex = strlen("Revert") * 12 + 8;
	button->sizey = 20;
	button->action = &buttonRevertResolution;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];
	revertResolutionButton = button;

	resolutionConfirmationTimer = SDL_GetTicks();
	confirmResolutionWindow = true;
}

void buttonAcceptResolution(button_t* my)
{
	confirmResolutionWindow = false;
	buttonCloseSubwindow(my);
	list_FreeAll(&button_l);
	deleteallbuttons = true;
	revertResolutionButton = nullptr;

	applySettings();
}

void buttonRevertResolution(button_t* my)
{
	revertResolution();

	confirmResolutionWindow = false;
	buttonCloseSubwindow(my);
	list_FreeAll(&button_l);
	deleteallbuttons = true;
	revertResolutionButton = nullptr;
}

void revertResolution()
{
	settings_xres = oldXres;
	settings_yres = oldYres;

	applySettings();
}

// settings accept button
void buttonSettingsAccept(button_t* my)
{
	applySettings();

	if ( resolutionChanged )
	{
		buttonCloseSettingsSubwindow(my);
		resolutionChanged = false;
		list_FreeAll(&button_l);
		deleteallbuttons = true;
		openConfirmResolutionWindow();
	}
	else
	{
		// we need to reposition the settings window now.
		buttonCloseSubwindow(my);
		list_FreeAll(&button_l);
		deleteallbuttons = true;
		openSettingsWindow();
	}
}

// settings okay button
void buttonSettingsOK(button_t* my)
{
	buttonSettingsAccept(my);
	if ( !confirmResolutionWindow )
	{
		buttonCloseSubwindow(my);
	}
}

// next score button (statistics window)
void buttonScoreNext(button_t* my)
{
	if ( scoreDisplayMultiplayer )
	{
		score_window = std::min<int>(score_window + 1, std::max<Uint32>(1, list_Size(&topscoresMultiplayer)));
	}
	else
	{
		score_window = std::min<int>(score_window + 1, std::max<Uint32>(1, list_Size(&topscores)));
	}
	loadScore(score_window - 1);
	camera_charsheet_offsetyaw = (330) * PI / 180;
}

// previous score button (statistics window)
void buttonScorePrev(button_t* my)
{
	score_window = std::max(score_window - 1, 1);
	loadScore(score_window - 1);
	camera_charsheet_offsetyaw = (330) * PI / 180;
}

void buttonScoreToggle(button_t* my)
{
	score_window = 1;
	camera_charsheet_offsetyaw = (330) * PI / 180;
	scoreDisplayMultiplayer = !scoreDisplayMultiplayer;
	loadScore(score_window - 1);
}

void buttonOpenScoresWindow(button_t* my)
{
	// create statistics window
	clientnum = 0;
	subwindow = 1;
	score_window = 1;
	camera_charsheet_offsetyaw = (330) * PI / 180;
	loadScore(0);
	subx1 = xres / 2 - 400;
	subx2 = xres / 2 + 400;
#ifdef PANDORA
	suby1 = yres / 2 - ((yres == 480) ? 200 : 240);
	suby2 = yres / 2 + ((yres == 480) ? 200 : 240);
#else
	suby1 = yres / 2 - 240;
	suby2 = yres / 2 + 240;
#endif
	strcpy(subtext, "");

	// close button
	button_t* button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1 + 4;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// next button
	button = newButton();
	strcpy(button->label, ">");
	button->sizex = strlen(">") * 12 + 8;
	button->sizey = 20;
	button->x = subx2 - button->sizex - 4;
	button->y = suby2 - 24;
	button->action = &buttonScoreNext;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RIGHT;
	button->joykey = joyimpulses[INJOY_DPAD_RIGHT];

	// previous button
	button = newButton();
	strcpy(button->label, "<");
	button->sizex = strlen("<") * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + 4;
	button->y = suby2 - 24;
	button->action = &buttonScorePrev;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_LEFT;
	button->joykey = joyimpulses[INJOY_DPAD_LEFT];

	// multiplayer scores toggle button
	button = newButton();
	strcpy(button->label, "");
	button->sizex = strlen("show multiplayer") * 12 + 8;
	button->sizey = 20;
	button->x = subx2 - 44 - strlen("show multiplayer") * 12;
	button->y = suby1 + 4;
	button->action = &buttonScoreToggle;
	button->visible = 1;
	button->focused = 1;

	// delete single score button
	button = newButton();
	strcpy(button->label, "delete score");
	button->sizex = strlen("delete score") * 12 + 8;
	button->sizey = 20;
	button->x = subx2 - 44 - (strlen("delete score") + strlen("show multiplayer") + 1) * 12;
	button->y = suby1 + 4;
	button->action = &buttonDeleteScoreWindow;
	button->visible = 1;
	button->focused = 1;
}

void buttonDeleteCurrentScore(button_t* my)
{
	node_t* node = nullptr;
	if ( scoreDisplayMultiplayer )
	{
		node = list_Node(&topscoresMultiplayer, score_window_to_delete - 1);
		if ( node )
		{
			list_RemoveNode(node);
			score_window_to_delete = std::max(score_window_to_delete - 1, 1);
		}
	}
	else
	{
		node = list_Node(&topscores, score_window_to_delete - 1);
		if ( node )
		{
			list_RemoveNode(node);
			score_window_to_delete = std::max(score_window_to_delete - 1, 1);
		}
	}
}

// handles slider
void doSlider(int x, int y, int dots, int minvalue, int maxvalue, int increment, int* var, SDL_Surface* slider_font, int slider_font_char_width)
{
	int c;

	// build bar
	strcpy(tempstr, "| ");
	for ( c = 0; c < dots; c++ )
	{
		strcat(tempstr, ". ");
	}
	strcat(tempstr, "| %d");
	printTextFormatted(slider_font, x, y, tempstr, *var);

	// control
	int range = maxvalue - minvalue;
	int sliderLength = ((strlen(tempstr) - 4) * (slider_font->w / slider_font_char_width));
	if ( mousestatus[SDL_BUTTON_LEFT] )
	{
		if ( omousex >= x && omousex < x + sliderLength + (slider_font->w / slider_font_char_width) )
		{
			if ( omousey >= y - (slider_font->h / slider_font_char_width) / 2 && omousey < y + ((slider_font->h / slider_font_char_width) / 2) * 3 )
			{
				*var = ((real_t)(mousex - x - (slider_font->w / slider_font_char_width) / 2) / sliderLength) * range + minvalue;
				if ( increment )
				{
					*var += increment / 2;
					*var /= increment;
					*var *= increment;
				}
				*var = std::min(std::max(minvalue, *var), maxvalue);
			}
		}
	}

	// draw slider
	int sliderx = x + (slider_font->w / slider_font_char_width) / 2;
	sliderx += (((real_t)(*var) - minvalue) / range) * sliderLength;
	drawWindowFancy( sliderx - (slider_font->w / slider_font_char_width) / 2, y - (slider_font->h / slider_font_char_width) / 2, sliderx + (slider_font->w / slider_font_char_width) / 2, y + ((slider_font->h / slider_font_char_width) / 2) * 3);
}

// handles slider (float)
void doSliderF(int x, int y, int dots, real_t minvalue, real_t maxvalue, real_t increment, real_t* var)
{
	int c;

	// build bar
	strcpy(tempstr, "| ");
	for ( c = 0; c < dots; c++ )
	{
		strcat(tempstr, ". ");
	}
	strcat(tempstr, "| %.3f");
	printTextFormatted(SLIDERFONT, x, y, tempstr, *var);

	// control
	real_t range = maxvalue - minvalue;
	int sliderLength = ((strlen(tempstr) - 6) * (SLIDERFONT->w / 16));
	if ( mousestatus[SDL_BUTTON_LEFT] )
	{
		if ( omousex >= x && omousex < x + sliderLength + (SLIDERFONT->w / 16) )
		{
			if ( omousey >= y - (SLIDERFONT->h / 16) / 2 && omousey < y + ((SLIDERFONT->h / 16) / 2) * 3 )
			{
				*var = ((real_t)(mousex - x - (SLIDERFONT->w / 16) / 2) / sliderLength) * range + minvalue;
				if ( increment )
				{
					*var += increment / 2;
					*var /= increment;
					*var = floor(*var);
					*var *= increment;
				}
				*var = fmin(fmax(minvalue, *var), maxvalue);
			}
		}
	}

	// draw slider
	int sliderx = x + (SLIDERFONT->w / 16) / 2;
	sliderx += (((*var) - minvalue) / range) * sliderLength;
	drawWindowFancy( sliderx - (SLIDERFONT->w / 16) / 2, y - (SLIDERFONT->h / 16) / 2, sliderx + (SLIDERFONT->w / 16) / 2, y + ((SLIDERFONT->h / 16) / 2) * 3);
}

void openLoadGameWindow(button_t* my)
{
	button_t* button;

	// close current window
	buttonCloseSubwindow(NULL);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	// create confirmation window
	subwindow = 1;
	subx1 = xres / 2 - 256;
	subx2 = xres / 2 + 256;
	suby1 = yres / 2 - 128;
	suby2 = yres / 2 + 128;
	strcpy(subtext, language[1460]);
	bool singleplayerSave = saveGameExists(true);
	bool multiplayerSave = saveGameExists(false);

	char saveGameName[1024] = "";
	if ( singleplayerSave && multiplayerSave )
	{
		strncpy(saveGameName, getSaveGameName(true), 1024);
		strcat(subtext, saveGameName);
		strcat(subtext, "\n\n");
		strncpy(saveGameName, getSaveGameName(false), 1024);
		loadGameSaveShowRectangle = 2;

		suby1 = yres / 2 - 152;
		suby2 = yres / 2 + 152;
	}
	else if ( singleplayerSave )
	{
		strncpy(saveGameName, getSaveGameName(true), 1024);
		loadGameSaveShowRectangle = 1;
	}
	else if ( multiplayerSave )
	{
		strncpy(saveGameName, getSaveGameName(false), 1024);
		loadGameSaveShowRectangle = 1;
	}
	strcat(subtext, saveGameName);
	strcat(subtext, language[1461]);

	if ( gamemods_numCurrentModsLoaded >= 0 )
	{
		suby1 -= 24;
		suby2 += 24;
	}

	// close button
	button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// yes solo button
	button = newButton();
	if ( multiplayerSave && !singleplayerSave )
	{
		strcpy(button->label, language[2959]);
		button->action = &buttonLoadMultiplayerGame;
	}
	else
	{
		strcpy(button->label, language[1462]);
		button->action = &buttonLoadSingleplayerGame;
	}
	button->sizex = strlen(language[2959]) * 9 + 16;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 52;
	if ( singleplayerSave && multiplayerSave )
	{
		button->x -= 124;
	}
	else
	{
		button->sizex = strlen(language[1463]) * 12 + 8; // resize to be wider
		button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2; // resize to match new width
	}
	button->visible = 1;
	button->focused = 1;
	button->joykey = joyimpulses[INJOY_MENU_NEXT]; //load save game yes => "a" button

	// yes multiplayer button
	if ( singleplayerSave && multiplayerSave )
	{
		button = newButton();
		strcpy(button->label, language[2959]);
		button->sizex = strlen(language[2959]) * 9 + 16;
		button->sizey = 20;
		button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2 + 124;
		button->y = suby2 - 52;
		button->action = &buttonLoadMultiplayerGame;
		button->visible = 1;
		button->focused = 1;
		//button->joykey = joyimpulses[INJOY_MENU_NEXT]; //load save game yes => "a" button
	}

	// no button
	button = newButton();
	strcpy(button->label, language[1463]);
	button->sizex = strlen(language[1463]) * 10 + 8;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 28;
	button->action = &buttonOpenCharacterCreationWindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;
	button->joykey = joyimpulses[INJOY_MENU_DONT_LOAD_SAVE]; //load save game no => "y" button

	// delete savegame button
	if ( singleplayerSave || multiplayerSave )
	{
		button = newButton();
		strcpy(button->label, language[2961]);
		button->sizex = strlen(language[2961]) * 12 + 8;
		button->sizey = 20;
		button->x = subx2 - button->sizex - 8;
		button->y = suby1 + TTF12_HEIGHT * 2 + 4;
		if ( singleplayerSave && multiplayerSave)
		{
			button->action = &buttonDeleteSavedSoloGame; // showing 2 entries, single player delete
		}
		if ( singleplayerSave && !multiplayerSave ) // showing 1 entry, single player delete
		{
			button->action = &buttonDeleteSavedSoloGame;
		}
		if ( !singleplayerSave && multiplayerSave ) // showing 1 entry, multi player delete
		{
			button->action = &buttonDeleteSavedMultiplayerGame;
		}
		button->visible = 1;
		button->focused = 1;
	}
	if ( singleplayerSave && multiplayerSave )
	{
		button = newButton();
		strcpy(button->label, language[2961]);
		button->sizex = strlen(language[2961]) * 12 + 8;
		button->sizey = 20;
		button->x = subx2 - button->sizex - 8;
		button->y = suby1 + TTF12_HEIGHT * 5 + 6;
		button->action = &buttonDeleteSavedMultiplayerGame;
		button->visible = 1;
		button->focused = 1;
	}
}

void buttonDeleteSavedSoloGame(button_t* my)
{
	// close current window
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	loadGameSaveShowRectangle = 1;

	// create confirmation window
	subwindow = 1;
	subx1 = xres / 2 - 288;
	subx2 = xres / 2 + 288;
	suby1 = yres / 2 - 80;
	suby2 = yres / 2 + 80;
	char saveGameName[1024];
	strcpy(subtext, language[2963]);
	strncpy(saveGameName, getSaveGameName(true), 1024);
	strcat(subtext, saveGameName);
	// close button
	button_t* button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// delete button
	button = newButton();
	strcpy(button->label, language[2961]);
	button->sizex = strlen(language[2961]) * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 56;
	button->action = &buttonConfirmDeleteSoloFile;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;
	//button->joykey = joyimpulses[INJOY_MENU_DONT_LOAD_SAVE]; //load save game no => "y" button

	// close button
	button = newButton();
	strcpy(button->label, language[2962]);
	button->sizex = strlen(language[2962]) * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 28;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
}

void buttonDeleteSavedMultiplayerGame(button_t* my)
{
	// close current window
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	loadGameSaveShowRectangle = 1;

	// create confirmation window
	subwindow = 1;
	subx1 = xres / 2 - 288;
	subx2 = xres / 2 + 288;
	suby1 = yres / 2 - 80;
	suby2 = yres / 2 + 80;
	char saveGameName[1024];
	strcpy(subtext, language[2964]);
	strncpy(saveGameName, getSaveGameName(false), 1024);
	strcat(subtext, saveGameName);
	// close button
	button_t* button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// delete button
	button = newButton();
	strcpy(button->label, language[2961]);
	button->sizex = strlen(language[2961]) * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 56;
	button->action = &buttonConfirmDeleteMultiplayerFile;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;
	//button->joykey = joyimpulses[INJOY_MENU_DONT_LOAD_SAVE]; //load save game no => "y" button

	// close button
	button = newButton();
	strcpy(button->label, language[2962]);
	button->sizex = strlen(language[2962]) * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 28;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
}

void buttonConfirmDeleteSoloFile(button_t* my)
{
	// close current window
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;
	loadGameSaveShowRectangle = 0;
	deleteSaveGame(SINGLE);
	if ( saveGameExists(false) ) // check for multiplayer game to load up
	{
		openLoadGameWindow(nullptr);
	}
	playSound(153, 96);
}

void buttonConfirmDeleteMultiplayerFile(button_t* my)
{
	// close current window
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;
	loadGameSaveShowRectangle = 0;
	deleteSaveGame(CLIENT);
	if ( saveGameExists(true) ) // check for singleplayer game to load up
	{
		openLoadGameWindow(nullptr);
	}
	playSound(153, 96);
}


void buttonDeleteScoreCancel(button_t* my)
{
	// close current window
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	buttonOpenScoresWindow(nullptr);
	score_window = score_window_to_delete;
	score_window_to_delete = 0;

	loadScore(score_window - 1);
}

void buttonDeleteScoreConfirm(button_t* my)
{
	buttonDeleteCurrentScore(nullptr);
	buttonDeleteScoreCancel(nullptr);
}

void buttonDeleteScoreWindow(button_t* my)
{
	score_window_to_delete = score_window;

	// close current window
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	// create confirmation window
	subwindow = 1;
	subx1 = xres / 2 - 244;
	subx2 = xres / 2 + 244;
	suby1 = yres / 2 - 60;
	suby2 = yres / 2 + 60;
	strcpy(subtext, language[3002]);

	// close button
	button_t* button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonDeleteScoreCancel;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// delete button
	button = newButton();
	strcpy(button->label, language[3001]);
	button->sizex = strlen(language[3001]) * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 56;
	button->action = &buttonDeleteScoreConfirm;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;

	// close button
	button = newButton();
	strcpy(button->label, language[2962]);
	button->sizex = strlen(language[2962]) * 12 + 8;
	button->sizey = 20;
	button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
	button->y = suby2 - 28;
	button->action = &buttonDeleteScoreCancel;
	button->visible = 1;
	button->focused = 1;
}

void buttonOpenCharacterCreationWindow(button_t* my)
{
	button_t* button;

	playing_random_char = false;
	loadingsavegame = 0;
	loadGameSaveShowRectangle = 0;
	// reset class loadout
	clientnum = 0;
	stats[0]->sex = static_cast<sex_t>(0);
	stats[0]->appearance = 0;
	strcpy(stats[0]->name, "");
	stats[0]->type = HUMAN;
	client_classes[0] = 0;
	stats[0]->clearStats();
	initClass(0);

	// close current window
	if ( subwindow )
	{
		buttonCloseSubwindow(NULL);
		list_FreeAll(&button_l);
		deleteallbuttons = true;
	}

	// create character creation window
	charcreation_step = 1;
	camera_charsheet_offsetyaw = (330) * PI / 180;
	subwindow = 1;
	subx1 = xres / 2 - 400;
	subx2 = xres / 2 + 400;
	suby1 = yres / 2 - 240;
	suby2 = yres / 2 + 240;
	strcpy(subtext, "");

	// close button
	button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->joykey = joyimpulses[INJOY_PAUSE_MENU];

	if ( lastCreatedCharacterClass >= 0
		&& lastCreatedCharacterAppearance >= 0 
		&& lastCreatedCharacterSex >= 0 )
	{
		button_t* replayCharacterBtn = newButton();
		strcpy(replayCharacterBtn->label, language[3000]);
		replayCharacterBtn->sizex = strlen(language[3000]) * 12 + 8;
		replayCharacterBtn->sizey = 20;
		replayCharacterBtn->x = button->x - (replayCharacterBtn->sizex + 4); // take position of button attributes above.
		replayCharacterBtn->y = button->y;
		replayCharacterBtn->action = &buttonReplayLastCharacter;
		replayCharacterBtn->visible = 1;
		replayCharacterBtn->focused = 1;
	}

	// Continue ...
	button = newButton();
	strcpy(button->label, language[1464]);
	button->sizex = strlen(language[1464]) * 12 + 8;
	button->sizey = 20;
	button->x = subx2 - button->sizex - 4;
	button->y = suby2 - 24;
	button->action = &buttonContinue;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_RETURN;
	button->joykey = joyimpulses[INJOY_MENU_NEXT];

	// Back ...
	button = newButton();
	strcpy(button->label, language[1465]);
	button->x = subx1 + 4;
	button->y = suby2 - 24;
	button->sizex = strlen(language[1465]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonBack;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];
	int button_back_x = button->x;
	int button_back_width = button->sizex;

	// Random Character ...
	button = newButton();
	strcpy(button->label, language[1466]);
	button->x = button_back_x + button_back_width + 4;
	button->y = suby2 - 24;
	button->sizex = strlen(language[1466]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonRandomCharacter;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_R; //NOTE: This might cause the character to randomly R when you're typing a name. So far, exactly one user has reported something like this happening exactly once in the entirety of existence.
	button->joykey = joyimpulses[INJOY_MENU_RANDOM_CHAR]; //random character => "y" button

	//Random Name.
	button = newButton();
	strcpy(button->label, language[2498]);
	button->x = button_back_x + button_back_width + 4;
	button->y = suby2 - 24;
	button->sizex = strlen(language[2498]) * 12 + 8;
	button->sizey = 20;
	button->action = &buttonRandomName;
	button->visible = 1;
	button->focused = 1;
	button->joykey = joyimpulses[INJOY_MENU_RANDOM_NAME];
}

void buttonLoadSingleplayerGame(button_t* button)
{
	loadGameSaveShowRectangle = 0;
	loadingsavegame = getSaveGameUniqueGameKey(true);
	int mul = getSaveGameType(true);

	if ( mul == DIRECTSERVER )
	{
		directConnect = true;
		buttonHostMultiplayer(button);
	}
	else if ( mul == DIRECTCLIENT )
	{
		directConnect = true;
		buttonJoinMultiplayer(button);
	}
	else if ( mul == SINGLE )
	{
		buttonStartSingleplayer(button);
	}
	else
	{
		directConnect = false;
#ifdef STEAMWORKS
		if ( mul == SERVER )
		{
			buttonHostMultiplayer(button);
		}
		else if ( mul == CLIENT )
		{
			if ( !lobbyToConnectTo )
			{
				openSteamLobbyBrowserWindow(button);
			}
			else
			{
				// close current window
				int temp1 = connectingToLobby;
				int temp2 = connectingToLobbyWindow;
				//buttonCloseSubwindow(button);
				list_FreeAll(&button_l);
				deleteallbuttons = true;
				connectingToLobby = temp1;
				connectingToLobbyWindow = temp2;

				// create new window
				subwindow = 1;
				subx1 = xres / 2 - 256;
				subx2 = xres / 2 + 256;
				suby1 = yres / 2 - 64;
				suby2 = yres / 2 + 64;
				strcpy(subtext, language[1467]);

				// close button
				button = newButton();
				strcpy(button->label, "x");
				button->x = subx2 - 20;
				button->y = suby1;
				button->sizex = 20;
				button->sizey = 20;
				button->action = &openSteamLobbyWaitWindow;
				button->visible = 1;
				button->focused = 1;
				button->key = SDL_SCANCODE_ESCAPE;
				button->joykey = joyimpulses[INJOY_MENU_CANCEL];

				// cancel button
				button = newButton();
				strcpy(button->label, language[1316]);
				button->sizex = strlen(language[1316]) * 12 + 8;
				button->sizey = 20;
				button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
				button->y = suby2 - 28;
				button->action = &openSteamLobbyWaitWindow;
				button->visible = 1;
				button->focused = 1;

				connectingToLobby = true;
				connectingToLobbyWindow = true;
				strncpy( currentLobbyName, "", 31 );
				cpp_SteamMatchmaking_JoinLobby(*static_cast<CSteamID* >(lobbyToConnectTo));
				cpp_Free_CSteamID(lobbyToConnectTo);
				lobbyToConnectTo = NULL;
			}
		}
		else
		{
			buttonStartSingleplayer(button);
		}
#endif
	}
}

void buttonLoadMultiplayerGame(button_t* button)
{
	loadGameSaveShowRectangle = 0;
	loadingsavegame = getSaveGameUniqueGameKey(false);
	int mul = getSaveGameType(false);

	if ( mul == DIRECTSERVER )
	{
		directConnect = true;
		buttonHostMultiplayer(button);
	}
	else if ( mul == DIRECTCLIENT )
	{
		directConnect = true;
		buttonJoinMultiplayer(button);
	}
	else if ( mul == SINGLE )
	{
		buttonStartSingleplayer(button);
	}
	else
	{
		directConnect = false;
#ifdef STEAMWORKS
		if ( mul == SERVER )
		{
			buttonHostMultiplayer(button);
		}
		else if ( mul == CLIENT )
		{
			if ( !lobbyToConnectTo )
			{
				openSteamLobbyBrowserWindow(button);
			}
			else
			{
				// close current window
				int temp1 = connectingToLobby;
				int temp2 = connectingToLobbyWindow;
				//buttonCloseSubwindow(button);
				list_FreeAll(&button_l);
				deleteallbuttons = true;
				connectingToLobby = temp1;
				connectingToLobbyWindow = temp2;

				// create new window
				subwindow = 1;
				subx1 = xres / 2 - 256;
				subx2 = xres / 2 + 256;
				suby1 = yres / 2 - 64;
				suby2 = yres / 2 + 64;
				strcpy(subtext, language[1467]);

				// close button
				button = newButton();
				strcpy(button->label, "x");
				button->x = subx2 - 20;
				button->y = suby1;
				button->sizex = 20;
				button->sizey = 20;
				button->action = &openSteamLobbyWaitWindow;
				button->visible = 1;
				button->focused = 1;
				button->key = SDL_SCANCODE_ESCAPE;
				button->joykey = joyimpulses[INJOY_MENU_CANCEL];

				// cancel button
				button = newButton();
				strcpy(button->label, language[1316]);
				button->sizex = strlen(language[1316]) * 12 + 8;
				button->sizey = 20;
				button->x = subx1 + (subx2 - subx1) / 2 - button->sizex / 2;
				button->y = suby2 - 28;
				button->action = &openSteamLobbyWaitWindow;
				button->visible = 1;
				button->focused = 1;

				connectingToLobby = true;
				connectingToLobbyWindow = true;
				strncpy(currentLobbyName, "", 31);
				cpp_SteamMatchmaking_JoinLobby(*static_cast<CSteamID* >(lobbyToConnectTo));
				cpp_Free_CSteamID(lobbyToConnectTo);
				lobbyToConnectTo = NULL;
			}
		}
		else
		{
			buttonStartSingleplayer(button);
		}
#endif
	}
}

void buttonRandomCharacter(button_t* my)
{
	playing_random_char = true;
	charcreation_step = 4;
	camera_charsheet_offsetyaw = (330) * PI / 180;
	stats[0]->sex = static_cast<sex_t>(rand() % 2);
	client_classes[0] = rand() % NUMCLASSES;
	stats[0]->clearStats();
	initClass(0);
	stats[0]->appearance = rand() % NUMAPPEARANCES;
}

void buttonReplayLastCharacter(button_t* my)
{
	if ( lastCreatedCharacterClass >= 0 )
	{
		playing_random_char = false;
		charcreation_step = 5;
		camera_charsheet_offsetyaw = (330) * PI / 180;
		stats[0]->sex = static_cast<sex_t>(lastCreatedCharacterSex);
		client_classes[0] = lastCreatedCharacterClass;
		stats[0]->clearStats();
		initClass(0);
		stats[0]->appearance = lastCreatedCharacterAppearance;
		strcpy(stats[0]->name, lastname.c_str());
	}
}

void buttonRandomName(button_t* my)
{
	if ( !SDL_IsTextInputActive() || charcreation_step != 4 )
	{
		return;
	}

	std::vector<std::string> *names;

	if ( stats[0]->sex == MALE )
	{
		names = &randomPlayerNamesMale;
	}
	else
	{
		names = &randomPlayerNamesFemale;
	}

	if ( !names->size() )
	{
		printlog("Warning: Random Name: Need names to pick from!");
		return;
	}
	std::string name;
	try
	{
		name = randomEntryFromVector(*names);
	}
	catch ( const char* e )
	{
		printlog("Error: Random Name: \"%s\"", e);
		return;
	}
	catch ( ... )
	{
		printlog("Error: Failed to choose random name.");
		return;
	}

	strncpy(inputstr, name.c_str(), std::min<size_t>(name.length(), inputlen));
	inputstr[std::min<size_t>(name.length(), inputlen)] = '\0';
}

void buttonGamemodsOpenDirectory(button_t* my)
{
	if ( gamemods_window_fileSelect != 0 && !currentDirectoryFiles.empty() )
	{
		std::list<std::string>::const_iterator it = currentDirectoryFiles.begin();
		std::advance(it, std::max(gamemods_window_scroll + gamemods_window_fileSelect - 1, 0));
		std::string directoryName = *it;

		if ( directoryName.compare("..") == 0 || directoryName.compare(".") == 0 )
		{
			directoryPath = directoryName;
			directoryPath.append(PHYSFS_getDirSeparator());
		}
		else
		{
			directoryPath.append(directoryName);
			directoryPath.append(PHYSFS_getDirSeparator());
		}
		gamemods_window_fileSelect = 0;
		gamemods_window_scroll = 0;
		currentDirectoryFiles = directoryContents(directoryPath.c_str(), true, false);
	}
}

void buttonGamemodsPrevDirectory(button_t* my)
{
	gamemods_window_fileSelect = 0;
	gamemods_window_scroll = 0;
	directoryPath.append("..");
	directoryPath.append(PHYSFS_getDirSeparator());
	currentDirectoryFiles = directoryContents(directoryPath.c_str(), true, false);
}


void writeLevelsTxt(std::string modFolder)
{
	std::string path = BASE_DATA_DIR;
	path.append("mods/").append(modFolder);
	if ( access(path.c_str(), F_OK) == 0 )
	{
		std::string writeFile = modFolder + "/maps/levels.txt";
		PHYSFS_File *physfp = PHYSFS_openWrite(writeFile.c_str());
		if ( physfp != NULL )
		{
			PHYSFS_writeBytes(physfp, "map: start\n", 11);
			PHYSFS_writeBytes(physfp, "gen: mine\n", 10);
			PHYSFS_writeBytes(physfp, "gen: mine\n", 10);
			PHYSFS_writeBytes(physfp, "gen: mine\n", 10);
			PHYSFS_writeBytes(physfp, "gen: mine\n", 10);
			PHYSFS_writeBytes(physfp, "map: minetoswamp\n", 17);			PHYSFS_writeBytes(physfp, "gen: swamp\n", 11);			PHYSFS_writeBytes(physfp, "gen: swamp\n", 11);			PHYSFS_writeBytes(physfp, "gen: swamp\n", 11);			PHYSFS_writeBytes(physfp, "gen: swamp\n", 11);			PHYSFS_writeBytes(physfp, "map: swamptolabyrinth\n", 22);			PHYSFS_writeBytes(physfp, "gen: labyrinth\n", 15);			PHYSFS_writeBytes(physfp, "gen: labyrinth\n", 15);			PHYSFS_writeBytes(physfp, "gen: labyrinth\n", 15);			PHYSFS_writeBytes(physfp, "gen: labyrinth\n", 15);			PHYSFS_writeBytes(physfp, "map: labyrinthtoruins\n", 22);			PHYSFS_writeBytes(physfp, "gen: ruins\n", 11);			PHYSFS_writeBytes(physfp, "gen: ruins\n", 11);			PHYSFS_writeBytes(physfp, "gen: ruins\n", 11);			PHYSFS_writeBytes(physfp, "gen: ruins\n", 11);			PHYSFS_writeBytes(physfp, "map: boss\n", 10);			PHYSFS_writeBytes(physfp, "gen: hell\n", 10);			PHYSFS_writeBytes(physfp, "gen: hell\n", 10);			PHYSFS_writeBytes(physfp, "gen: hell\n", 10);			PHYSFS_writeBytes(physfp, "map: hellboss\n", 14);			PHYSFS_writeBytes(physfp, "map: hamlet\n", 12);			PHYSFS_writeBytes(physfp, "gen: caves\n", 11);			PHYSFS_writeBytes(physfp, "gen: caves\n", 11);			PHYSFS_writeBytes(physfp, "gen: caves\n", 11);			PHYSFS_writeBytes(physfp, "gen: caves\n", 11);			PHYSFS_writeBytes(physfp, "map: cavestocitadel\n", 20);			PHYSFS_writeBytes(physfp, "gen: citadel\n", 13);			PHYSFS_writeBytes(physfp, "gen: citadel\n", 13);			PHYSFS_writeBytes(physfp, "gen: citadel\n", 13);			PHYSFS_writeBytes(physfp, "gen: citadel\n", 13);			PHYSFS_writeBytes(physfp, "map: sanctum", 12);
			PHYSFS_close(physfp);
		}
		else
		{
			printlog("[PhysFS]: Failed to open %s/maps/levels.txt for writing.", path.c_str());
		}
	}
	else
	{
		printlog("[PhysFS]: Failed to write levels.txt in %s", path.c_str());
	}
}

void buttonGamemodsCreateModDirectory(button_t* my)
{
	std::string baseDir = PHYSFS_getBaseDir();
	baseDir.append("mods").append(PHYSFS_getDirSeparator()).append(gamemods_newBlankDirectory);

	if ( access(baseDir.c_str(), F_OK) == 0 )
	{
		// folder already exists!
		gamemods_newBlankDirectoryStatus = -1;
	}
	else
	{
		if ( PHYSFS_mkdir(gamemods_newBlankDirectory) )
		{
			gamemods_newBlankDirectoryStatus = 1;
			std::string dir = gamemods_newBlankDirectory;
			std::string folder = "/books";
			PHYSFS_mkdir((dir + folder).c_str());
			folder = "/editor";
			PHYSFS_mkdir((dir + folder).c_str());

			folder = "/images";
			PHYSFS_mkdir((dir + folder).c_str());
			std::string subfolder = "/sprites";
			PHYSFS_mkdir((dir + folder + subfolder).c_str());
			subfolder = "/system";
			PHYSFS_mkdir((dir + folder + subfolder).c_str());
			subfolder = "/tiles";
			PHYSFS_mkdir((dir + folder + subfolder).c_str());

			folder = "/items";
			PHYSFS_mkdir((dir + folder).c_str());
			subfolder = "/images";
			PHYSFS_mkdir((dir + folder + subfolder).c_str());

			folder = "/lang";
			PHYSFS_mkdir((dir + folder).c_str());
			folder = "/maps";
			PHYSFS_mkdir((dir + folder).c_str());
			writeLevelsTxt(gamemods_newBlankDirectory);

			folder = "/models";
			PHYSFS_mkdir((dir + folder).c_str());
			subfolder = "/creatures";
			PHYSFS_mkdir((dir + folder + subfolder).c_str());
			subfolder = "/decorations";
			PHYSFS_mkdir((dir + folder + subfolder).c_str());
			subfolder = "/doors";
			PHYSFS_mkdir((dir + folder + subfolder).c_str());
			subfolder = "/items";
			PHYSFS_mkdir((dir + folder + subfolder).c_str());
			subfolder = "/particles";
			PHYSFS_mkdir((dir + folder + subfolder).c_str());

			folder = "/music";
			PHYSFS_mkdir((dir + folder).c_str());
			folder = "/sound";
			PHYSFS_mkdir((dir + folder).c_str());
		}
	}
	strcpy(gamemods_newBlankDirectoryOldName, gamemods_newBlankDirectory);
}

void buttonGamemodsCreateNewModTemplate(button_t* my)
{
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;

	gamemods_window = 6;

	// create window
	subwindow = 1;
	subx1 = xres / 2 - 400;
	subx2 = xres / 2 + 400;
	suby1 = yres / 2 - 70;
	suby2 = yres / 2 + 70;
	strcpy(subtext, "Create new blank mod template");

	// close button
	button_t* button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// create button
	button = newButton();
	strcpy(button->label, "create");
	button->x = subx2 - (strlen(button->label) * TTF12_WIDTH + 8);
	button->y = suby2 - TTF12_HEIGHT - 8;
	button->sizex = strlen(button->label) * TTF12_WIDTH + 8;
	button->sizey = 20;
	button->action = &buttonGamemodsCreateModDirectory;
	button->visible = 1;
	button->focused = 1;
}


void buttonGamemodsBaseDirectory(button_t* my)
{
	gamemods_window_fileSelect = 0;
	gamemods_window_scroll = 0;
	directoryPath = datadir;
	directoryToUpload = directoryPath;
	currentDirectoryFiles = directoryContents(directoryPath.c_str(), true, false);
}

#ifdef STEAMWORKS
void buttonGamemodsSelectDirectoryForUpload(button_t* my)
{
	if ( !currentDirectoryFiles.empty() )
	{
		std::list<std::string>::const_iterator it = currentDirectoryFiles.begin();
		std::advance(it, std::max(gamemods_window_scroll + gamemods_window_fileSelect - 1, 0));
		std::string directoryName = *it;

		if ( directoryName.compare("..") == 0 || directoryName.compare(".") == 0 )
		{
			directoryToUpload = directoryName;
			directoryToUpload.append(PHYSFS_getDirSeparator());
		}
		else
		{
			directoryToUpload = directoryPath;
			directoryToUpload.append(directoryName);
			directoryToUpload.append(PHYSFS_getDirSeparator());
		}
	}
	if ( gamemods_window != 5 )
	{
		if ( g_SteamWorkshop )
		{
			g_SteamWorkshop->createItemResult = {};
		}
		gamemods_uploadStatus = 0;
		gamemods_window = 2;
	}
	directoryFilesListToUpload = directoryContents(directoryToUpload.c_str(), true, true);
}

void buttonGamemodsPrepareWorkshopItemUpload(button_t* my)
{
	if ( SteamUser()->BLoggedOn() && g_SteamWorkshop )
	{
		g_SteamWorkshop->CreateItem();
		gamemods_uploadStatus = 1;
	}
}

void buttonGamemodsCancelModifyFileContents(button_t* my)
{
	directoryFilesListToUpload.clear();
}

void buttonGamemodsPrepareWorkshopItemUpdate(button_t* my)
{
	if ( SteamUser()->BLoggedOn() && g_SteamWorkshop )
	{
		g_SteamWorkshop->CreateItem();
		gamemods_uploadStatus = 1;
	}
}

void buttonGamemodsSetWorkshopItemFields(button_t* my)
{
	if ( SteamUser()->BLoggedOn() && g_SteamWorkshop )
	{
		bool itemTagSetSuccess = false;
		if ( g_SteamWorkshop->UGCUpdateHandle != 0 )
		{
			if ( !strcmp(gamemods_uploadTitle, "") )
			{
				strcpy(gamemods_uploadTitle, "Title");
			}
			gamemods_workshopSetPropertyReturn[0] = SteamUGC()->SetItemTitle(g_SteamWorkshop->UGCUpdateHandle, gamemods_uploadTitle);
			if ( !strcmp(gamemods_uploadDescription, "") )
			{
				strcpy(gamemods_uploadDescription, "Description");
			}
			gamemods_workshopSetPropertyReturn[1] = SteamUGC()->SetItemDescription(g_SteamWorkshop->UGCUpdateHandle, gamemods_uploadDescription);
#ifdef WINDOWS
			char pathbuffer[PATH_MAX];
			GetFullPathName(directoryToUpload.c_str(), PATH_MAX, pathbuffer, NULL);
			std::string fullpath = pathbuffer;
#else
			char pathbuffer[PATH_MAX];
			realpath(directoryToUpload.c_str(), pathbuffer);
			std::string fullpath = pathbuffer;
#endif
			if ( access(fullpath.c_str(), F_OK) == 0 )
			{
				gamemods_workshopSetPropertyReturn[2] = SteamUGC()->SetItemContent(g_SteamWorkshop->UGCUpdateHandle, fullpath.c_str());
				// set preview image.
				bool imagePreviewFound = false;
				std::string imgPath = fullpath;
				imgPath.append("preview.jpg");
				if ( !imagePreviewFound && access((imgPath).c_str(), F_OK) == 0 )
				{
					imagePreviewFound = SteamUGC()->SetItemPreview(g_SteamWorkshop->UGCUpdateHandle, imgPath.c_str());
				}
				imgPath = fullpath;
				imgPath.append("preview.png");
				if ( !imagePreviewFound && access((imgPath).c_str(), F_OK) == 0 )
				{
					imagePreviewFound = SteamUGC()->SetItemPreview(g_SteamWorkshop->UGCUpdateHandle, imgPath.c_str());
				}
				imgPath = fullpath;
				imgPath.append("preview.jpg");
				if ( !imagePreviewFound && access((imgPath).c_str(), F_OK) == 0 )
				{
					imagePreviewFound = SteamUGC()->SetItemPreview(g_SteamWorkshop->UGCUpdateHandle, imgPath.c_str());
				}
				if ( !imagePreviewFound )
				{
					printlog("Failed to upload image for workshop item!");
				}
			}

			// some mumbo jumbo to work with the steam API needing const char[][]
			SteamParamStringArray_t SteamParamStringArray;
			SteamParamStringArray.m_nNumStrings = g_SteamWorkshop->workshopItemTags.size() + 1;

			// construct new char[][]
			char **tagArray = new char*[gamemods_maxTags];
			int i = 0;
			for ( i = 0; i < gamemods_maxTags; ++i )
			{
				tagArray[i] = new char[32];
			}

			// copy all the items into this new char[][].
			std::string line;
			i = 0;
			for ( std::list<std::string>::iterator it = g_SteamWorkshop->workshopItemTags.begin(); it != g_SteamWorkshop->workshopItemTags.end(); ++it )
			{
				line = *it;
				strcpy(tagArray[i], line.c_str());
				++i;
			}
			strcpy(tagArray[i], VERSION); // copy the version number as a tag.

			// set the tags in the API call.
			SteamParamStringArray.m_ppStrings = const_cast<const char**>(tagArray);
			itemTagSetSuccess = SteamUGC()->SetItemTags(g_SteamWorkshop->UGCUpdateHandle, &SteamParamStringArray);

			// delete the allocated char[][]
			for ( i = 0; i < gamemods_maxTags; ++i )
			{
				delete[] tagArray[i];
			}
			delete[] tagArray;
		}
		gamemods_uploadStatus = 4;
		if ( itemTagSetSuccess && gamemods_workshopSetPropertyReturn[0] && gamemods_workshopSetPropertyReturn[1] && gamemods_workshopSetPropertyReturn[2] )
		{
			my->visible = false;
			// set item fields button
			button_t* button = newButton();
			strcpy(button->label, "upload!");
			button->x = subx1 + 16;
			button->y = suby1 + TTF12_HEIGHT * 34;
			button->sizex = 16 * TTF12_WIDTH + 8;
			button->sizey = 32;
			button->action = &buttonGamemodsStartUploadItem;
			button->visible = 1;
			button->focused = 1;
			gamemods_currentEditField = 0;
		}
	}
}

void buttonGamemodsModifyExistingWorkshopItemFields(button_t* my)
{
	if ( SteamUser()->BLoggedOn() && g_SteamWorkshop && g_SteamWorkshop->m_myWorkshopItemToModify.m_nPublishedFileId != 0 )
	{
		g_SteamWorkshop->StartItemExistingUpdate(g_SteamWorkshop->m_myWorkshopItemToModify.m_nPublishedFileId);
		if ( g_SteamWorkshop->UGCUpdateHandle != 0 )
		{
			bool itemTagSetSuccess = false;
			bool itemContentSetSuccess = false;
			if ( !directoryFilesListToUpload.empty() )
			{
#ifdef WINDOWS
				char pathbuffer[PATH_MAX];
				GetFullPathName(directoryToUpload.c_str(), PATH_MAX, pathbuffer, NULL);
				std::string fullpath = pathbuffer;
#else
				char pathbuffer[PATH_MAX];
				realpath(directoryToUpload.c_str(), pathbuffer);
				std::string fullpath = pathbuffer;
#endif
				if ( access(fullpath.c_str(), F_OK) == 0 )
				{
					itemContentSetSuccess = SteamUGC()->SetItemContent(g_SteamWorkshop->UGCUpdateHandle, fullpath.c_str());
					// set preview image.
					bool imagePreviewFound = false;
					std::string imgPath = fullpath;
					imgPath.append("preview.jpg");
					if ( !imagePreviewFound && access((imgPath).c_str(), F_OK) == 0 )
					{
						imagePreviewFound = SteamUGC()->SetItemPreview(g_SteamWorkshop->UGCUpdateHandle, imgPath.c_str());
					}
					imgPath = fullpath;
					imgPath.append("preview.png");
					if ( !imagePreviewFound && access((imgPath).c_str(), F_OK) == 0 )
					{
						imagePreviewFound = SteamUGC()->SetItemPreview(g_SteamWorkshop->UGCUpdateHandle, imgPath.c_str());
					}
					imgPath = fullpath;
					imgPath.append("preview.jpg");
					if ( !imagePreviewFound && access((imgPath).c_str(), F_OK) == 0 )
					{
						imagePreviewFound = SteamUGC()->SetItemPreview(g_SteamWorkshop->UGCUpdateHandle, imgPath.c_str());
					}
					if ( !imagePreviewFound )
					{
						printlog("Failed to upload image for workshop item!");
					}
				}
			}

			// some mumbo jumbo to work with the steam API needing const char[][]
			SteamParamStringArray_t SteamParamStringArray;
			SteamParamStringArray.m_nNumStrings = g_SteamWorkshop->workshopItemTags.size() + 1;

			// construct new char[][]
			char **tagArray = new char*[gamemods_maxTags];
			int i = 0;
			for ( i = 0; i < gamemods_maxTags; ++i )
			{
				tagArray[i] = new char[32];
			}

			// copy all the items into this new char[][].
			std::string line;
			i = 0;
			for ( std::list<std::string>::iterator it = g_SteamWorkshop->workshopItemTags.begin(); it != g_SteamWorkshop->workshopItemTags.end(); ++it )
			{
				line = *it;
				strcpy(tagArray[i], line.c_str());
				++i;
			}
			strcpy(tagArray[i], VERSION); // copy the version number as a tag.

			// set the tags in the API call.
			SteamParamStringArray.m_ppStrings = const_cast<const char**>(tagArray);
			itemTagSetSuccess = SteamUGC()->SetItemTags(g_SteamWorkshop->UGCUpdateHandle, &SteamParamStringArray);

			// delete the allocated char[][]
			for ( i = 0; i < gamemods_maxTags; ++i )
			{
				delete[] tagArray[i];
			}
			delete[] tagArray;

			if ( itemTagSetSuccess && (directoryFilesListToUpload.empty() || (!directoryFilesListToUpload.empty() && itemContentSetSuccess)) )
			{
				my->visible = false;
				// set item fields button
				button_t* button = newButton();
				strcpy(button->label, "upload!");
				button->x = subx1 + 16;
				button->y = suby1 + TTF12_HEIGHT * 34;
				button->sizex = 16 * TTF12_WIDTH + 8;
				button->sizey = 32;
				button->action = &buttonGamemodsStartUploadItem;
				button->visible = 1;
				button->focused = 1;
				gamemods_currentEditField = 0;
			}
		}
	}
}

void buttonGamemodsStartUploadItem(button_t* my)
{
	if ( SteamUser()->BLoggedOn() && g_SteamWorkshop && g_SteamWorkshop->UGCUpdateHandle != 0 )
	{
		if ( gamemods_window == 5 )
		{
			g_SteamWorkshop->SubmitItemUpdate("Item updated.");
		}
		else
		{
			g_SteamWorkshop->SubmitItemUpdate("First upload.");
		}
		gamemods_uploadStatus = 5;
		my->visible = false;
	}
}

void gamemodsWindowUploadInit(bool creatingNewItem)
{
	gamemods_window = 1;
	currentDirectoryFiles = directoryContents(datadir, true, false);
	directoryToUpload = datadir;

	// create window
	subwindow = 1;
	subx1 = xres / 2 - 320;
	subx2 = xres / 2 + 320;
	suby1 = yres / 2 - 300;
	suby2 = yres / 2 + 300;
	strcpy(subtext, "Upload to workshop");

	// close button
	button_t* button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// subscribed items window button
	button = newButton();
	strcpy(button->label, "view workshop items");
	button->x = subx2 - 40 - strlen(button->label) * TTF12_WIDTH;
	button->y = suby1;
	button->sizex = strlen(button->label) * TTF12_WIDTH + 16;
	button->sizey = 20;
	button->action = &buttonGamemodsOpenSubscribedWindow;
	button->visible = 1;
	button->focused = 1;

	// previous directory button
	button = newButton();
	strcpy(button->label, "home directory");
	button->x = subx1 + 250;
	button->y = suby1 + 32;
	button->sizex = strlen("home directory") * 12 + 8;
	button->sizey = 20;
	button->action = &buttonGamemodsBaseDirectory;
	button->visible = 1;
	button->focused = 1;


	// open directory button
	button = newButton();
	strcpy(button->label, "open");
	button->x = subx1 + 250;
	button->y = suby1 + 56;
	button->sizex = strlen("home directory") * 12 + 8;
	button->sizey = 20;
	button->action = &buttonGamemodsOpenDirectory;
	button->visible = 1;
	button->focused = 1;

	// previous directory button
	button = newButton();
	strcpy(button->label, "previous folder");
	button->x = subx1 + 250;
	button->y = suby1 + 80;
	button->sizex = strlen("home directory") * 12 + 8;
	button->sizey = 20;
	button->action = &buttonGamemodsPrevDirectory;
	button->visible = 1;
	button->focused = 1;

	// previous directory button
	button = newButton();
	strcpy(button->label, "new mod folder");
	button->x = subx1 + 250;
	button->y = suby1 + 128;
	button->sizex = strlen("new mod folder") * 12 + 8;
	button->sizey = 20;
	button->action = &buttonGamemodsCreateNewModTemplate;
	button->visible = 1;
	button->focused = 1;

	// select directory button
	button = newButton();
	strcpy(button->label, "select folder to upload");
	button->x = subx1 + 16;
	button->y = suby1 + 14 * TTF12_HEIGHT + 8;
	button->sizex = 24 * TTF12_WIDTH + 8;
	button->sizey = 32;
	button->action = &buttonGamemodsSelectDirectoryForUpload;
	button->visible = 1;
	button->focused = 1;

	// prepare directory button
	button_t* button2 = newButton();
	if ( creatingNewItem )
	{
		strcpy(button2->label, "prepare");
		button2->action = &buttonGamemodsPrepareWorkshopItemUpload;
	}
	else
	{
		strcpy(button2->label, "deselect folder");
		button2->action = &buttonGamemodsCancelModifyFileContents;
	}
	button2->x = button->x + button->sizex + 4;
	button2->y = button->y;
	button2->sizex = 16 * TTF12_WIDTH + 8;
	button2->sizey = 32;
	button2->visible = 1;
	button2->focused = 1;

	if ( !creatingNewItem )
	{
		// modify item fields button
		button = newButton();
		strcpy(button->label, "modify tags/content");
		button->x = subx1 + 16;
		button->y = suby1 + TTF12_HEIGHT * 34;
		button->sizex = 22 * TTF12_WIDTH + 8;
		button->sizey = 32;
		button->action = &buttonGamemodsModifyExistingWorkshopItemFields;
		button->visible = 1;
		button->focused = 1;
	}
}

void gamemodsSubscribedItemsInit()
{
	gamemods_window = 3;
	currentDirectoryFiles = directoryContents(datadir, true, false);
	directoryToUpload = datadir;

	gamemodsMountAllExistingPaths();

	// create confirmation window
	subwindow = 1;
	subx1 = xres / 2 - 420;
	subx2 = xres / 2 + 420;
	suby1 = yres / 2 - 300;
	suby2 = yres / 2 + 300;
	strcpy(subtext, "Workshop items");

	// close button
	button_t* button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// upload window button
	button = newButton();
	strcpy(button->label, "upload workshop content");
	button->x = subx2 - 40 - strlen(button->label) * TTF12_WIDTH;
	button->y = suby1;
	button->sizex = strlen(button->label) * TTF12_WIDTH + 16;
	button->sizey = 20;
	button->action = &buttonGamemodsOpenUploadWindow;
	button->visible = 1;
	button->focused = 1;

	// fetch subscribed items button
	button = newButton();
	strcpy(button->label, "get subscribed item list");
	button->x = subx1 + 16;
	button->y = suby1 + 2 * TTF12_HEIGHT + 8;
	button->sizex = 25 * TTF12_WIDTH + 8;
	button->sizey = 32;
	button->action = &buttonGamemodsGetSubscribedItems;
	button->visible = 1;
	button->focused = 1;

	// fetch my workshop items
	button_t* button2 = newButton();
	strcpy(button2->label, "my workshop items");
	button2->x = button->x + button->sizex + 16;
	button2->y = suby1 + 2 * TTF12_HEIGHT + 8;
	button2->sizex = 25 * TTF12_WIDTH + 8;
	button2->sizey = 32;
	button2->action = &buttonGamemodsGetMyWorkshopItems;
	button2->visible = 1;
	button2->focused = 1;

	// start modded game
	button = newButton();
	strcpy(button->label, "start modded game");
	button->sizex = 25 * TTF12_WIDTH + 8;
	button->sizey = 32;
	button->x = subx2 - (button->sizex + 16);
	button->y = suby1 + 2 * TTF12_HEIGHT + 8;
	button->action = &buttonGamemodsStartModdedGame;
	button->visible = 1;
	button->focused = 1;
}

void buttonGamemodsOpenUploadWindow(button_t* my)
{
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;
	gamemodsWindowUploadInit(true);
}

void buttonGamemodsOpenModifyExistingWindow(button_t* my)
{
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;
	gamemodsWindowUploadInit(false);
}

void buttonGamemodsOpenSubscribedWindow(button_t* my)
{
	buttonCloseSubwindow(nullptr);
	list_FreeAll(&button_l);
	deleteallbuttons = true;
	gamemodsSubscribedItemsInit();
}

void buttonGamemodsGetSubscribedItems(button_t* my)
{
	if ( g_SteamWorkshop )
	{
		g_SteamWorkshop->CreateQuerySubscribedItems(k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_All, k_EUserUGCListSortOrder_LastUpdatedDesc);
		gamemods_window_scroll = 0;
		gamemods_window = 3;
	}
}

void buttonGamemodsGetMyWorkshopItems(button_t* my)
{
	if ( g_SteamWorkshop )
	{
		g_SteamWorkshop->CreateQuerySubscribedItems(k_EUserUGCList_Published, k_EUGCMatchingUGCType_All, k_EUserUGCListSortOrder_LastUpdatedDesc);
		gamemods_window_scroll = 0;
		gamemods_window = 4;
	}
}

void gamemodsDrawWorkshopItemTagToggle(std::string tagname, int x, int y)
{
	if ( !g_SteamWorkshop )
	{
		return;
	}
	std::string printText = tagname;
	std::string line;
	bool foundTag = false;
	std::list<std::string>::iterator it;
	if ( !g_SteamWorkshop->workshopItemTags.empty() )
	{
		for ( it = g_SteamWorkshop->workshopItemTags.begin(); it != g_SteamWorkshop->workshopItemTags.end(); ++it )
		{
			line = *it;
			std::size_t found = line.find_first_of(' '); // trim any trailing spaces.
			if ( found != std::string::npos )
			{
				line = line.substr(0, found);
			}
			if ( line.compare(tagname) == 0 )
			{
				foundTag = true;
				break;
			}
		}
	}
	while ( printText.length() < 12 )
	{
		printText.append(" ");
	}
	if ( foundTag )
	{
		printText.append(": [x]");
	}
	else
	{
		printText.append(": [ ]");
	}
	if ( mouseInBounds(x, x + printText.size() * TTF12_WIDTH, y, y + TTF12_HEIGHT) )
	{
		ttfPrintTextColor(ttf12, x, y, SDL_MapRGBA(mainsurface->format, 128, 128, 128, 255), true, printText.c_str());
		if ( mousestatus[SDL_BUTTON_LEFT] )
		{
			playSound(139, 64);
			if ( foundTag )
			{
				g_SteamWorkshop->workshopItemTags.erase(it);
			}
			else
			{
				g_SteamWorkshop->workshopItemTags.push_back(tagname);
			}
			mousestatus[SDL_BUTTON_LEFT] = 0;
		}
	}
	else
	{
		ttfPrintText(ttf12, x, y, printText.c_str());
	}
}

bool gamemodsCheckIfSubscribedAndDownloadedFileID(uint64 fileID)
{
	if ( directConnect || !currentLobby )
	{
		return false;
	}

	uint64 itemState = SteamUGC()->GetItemState(fileID);
	if ( (itemState & k_EItemStateSubscribed) && (itemState & k_EItemStateInstalled) )
	{
		return true; // client has downloaded and subscribed to content.
	}

	return false; // client does not have item subscribed or downloaded.
}

bool gamemodsCheckFileIDInLoadedPaths(uint64 fileID)
{
	if ( directConnect || !currentLobby )
	{
		return false;
	}

	bool found = false;
	for ( std::vector<std::pair<std::string, uint64>>::iterator it = gamemods_workshopLoadedFileIDMap.begin();
		it != gamemods_workshopLoadedFileIDMap.end(); ++it )
	{
		if ( it->second == fileID )
		{
			return true; // client has fileID in mod load path.
		}
	}

	return false; // client does not have fileID in mod load path.
}

void buttonGamemodsSubscribeToHostsModFiles(button_t* my)
{
	if ( !directConnect && currentLobby && g_SteamWorkshop )
	{
		const char* serverNumModsChar = SteamMatchmaking()->GetLobbyData(*static_cast<CSteamID*>(currentLobby), "svNumMods");
		int serverNumModsLoaded = atoi(serverNumModsChar);
		if ( serverNumModsLoaded > 0 )
		{
			char tagName[32];
			char fullpath[PATH_MAX];
			std::vector<uint64> fileIdsToDownload;
			for ( int lines = 0; lines < serverNumModsLoaded; ++lines )
			{
				snprintf(tagName, 32, "svMod%d", lines);
				const char* serverModFileID = SteamMatchmaking()->GetLobbyData(*static_cast<CSteamID*>(currentLobby), tagName);
				if ( strcmp(serverModFileID, "") )
				{
					if ( gamemodsCheckIfSubscribedAndDownloadedFileID(atoi(serverModFileID)) == false )
					{
						SteamUGC()->SubscribeItem(atoi(serverModFileID));
					}
					fileIdsToDownload.push_back(atoi(serverModFileID));
				}
			}
			for ( std::vector<uint64>::iterator it = fileIdsToDownload.begin(); it != fileIdsToDownload.end(); ++it )
			{
				SteamUGC()->DownloadItem(*it, true); // download all the newly subscribed items.
				// hopefully enough time elapses for this to complete
			}
			g_SteamWorkshop->CreateQuerySubscribedItems(k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_All, k_EUserUGCListSortOrder_LastUpdatedDesc);
		}
	}
}

void buttonGamemodsMountHostsModFiles(button_t* my)
{
	if ( !directConnect && currentLobby && g_SteamWorkshop )
	{
		const char* serverNumModsChar = SteamMatchmaking()->GetLobbyData(*static_cast<CSteamID*>(currentLobby), "svNumMods");
		int serverNumModsLoaded = atoi(serverNumModsChar);
		if ( serverNumModsLoaded > 0 )
		{
			char tagName[32];
			char fullpath[PATH_MAX];
			// prepare to mount only the hosts workshop files.
			gamemodsClearAllMountedPaths();
			gamemods_mountedFilepaths.clear();
			gamemods_workshopLoadedFileIDMap.clear();
			for ( int lines = 0; lines < serverNumModsLoaded; ++lines )
			{
				snprintf(tagName, 32, "svMod%d", lines);
				const char* serverModFileID = SteamMatchmaking()->GetLobbyData(*static_cast<CSteamID*>(currentLobby), tagName);
				if ( strcmp(serverModFileID, "") )
				{
					if ( gamemodsCheckFileIDInLoadedPaths(atoi(serverModFileID)) == false )
					{
						if ( SteamUGC()->GetItemInstallInfo(atoi(serverModFileID), NULL, fullpath, PATH_MAX, NULL) )
						{
							for ( int i = 0; i < g_SteamWorkshop->numSubcribedItemResults; ++i )
							{
								if ( g_SteamWorkshop->m_subscribedItemListDetails[i].m_nPublishedFileId == atoi(serverModFileID) )
								{
									gamemods_mountedFilepaths.push_back(std::make_pair(fullpath, g_SteamWorkshop->m_subscribedItemListDetails[i].m_rgchTitle));
									gamemods_workshopLoadedFileIDMap.push_back(std::make_pair(g_SteamWorkshop->m_subscribedItemListDetails[i].m_rgchTitle, 
										g_SteamWorkshop->m_subscribedItemListDetails[i].m_nPublishedFileId));
									break;
								}
							}
						}
					}
				}
			}
			g_SteamWorkshop->CreateQuerySubscribedItems(k_EUserUGCList_Subscribed, k_EUGCMatchingUGCType_All, k_EUserUGCListSortOrder_LastUpdatedDesc);
			gamemodsMountAllExistingPaths(); // mount all the new filepaths, update gamemods_numCurrentModsLoaded.
		}
	}
}

bool gamemodsIsClientLoadOrderMatchingHost(std::vector<std::string> serverModList)
{
	std::vector<std::pair<std::string, uint64>>::iterator found = gamemods_workshopLoadedFileIDMap.begin();
	std::vector<std::pair<std::string, uint64>>::iterator previousFound = gamemods_workshopLoadedFileIDMap.begin();
	std::vector<std::string>::iterator itServerList;
	if ( serverModList.empty() || (serverModList.size() > gamemods_mountedFilepaths.size()) )
	{
		return false;
	}

	for ( itServerList = serverModList.begin(); itServerList != serverModList.end(); ++itServerList )
	{
		for ( found = previousFound; found != gamemods_workshopLoadedFileIDMap.end(); ++found )
		{
			if ( std::to_string(found->second) == *itServerList )
			{
				break;
			}
		}
		if ( found != gamemods_workshopLoadedFileIDMap.end() )
		{
			// look for the server's modID in my loaded paths.
			// check the distance along the vector our found result is.
			// if the distance is negative, then our mod order is out of sync with the server's mod list
			// and requires rearranging.
			if ( std::distance(previousFound, found) < 0 )
			{
				return false;
			}
			previousFound = found;
		}
		else
		{
			// server's mod doesn't exist in our filepath, so our mod lists are not in sync.
			return false;
		}
	}
	return true;
}

#endif //STEAMWORKS

bool gamemodsDrawClickableButton(int padx, int pady, int padw, int padh, Uint32 btnColor, std::string btnText, int action)
{
	bool clicked = false;
	if ( mouseInBounds(padx, padx + padw, pady - 4, pady + padh) )
	{
		drawDepressed(padx, pady - 4, padx + padw, pady + padh);
		if ( mousestatus[SDL_BUTTON_LEFT] )
		{
			playSound(139, 64);
			mousestatus[SDL_BUTTON_LEFT] = 0;
			clicked = true;
		}
	}
	else
	{
		drawWindow(padx, pady - 4, padx + padw, pady + padh);
	}
	SDL_Rect pos;
	pos.x = padx;
	pos.y = pady - 4;
	pos.w = padw;
	pos.h = padh + 4;
	drawRect(&pos, btnColor, 64);
	ttfPrintTextFormatted(ttf12, padx + 8, pady, "%s", btnText.c_str());
	return clicked;
}

bool gamemodsRemovePathFromMountedFiles(std::string findStr)
{
	std::vector<std::pair<std::string, std::string>>::iterator it;
	std::pair<std::string, std::string> line;
	for ( it = gamemods_mountedFilepaths.begin(); it != gamemods_mountedFilepaths.end(); ++it )
	{
		line = *it;
		if ( line.first.compare(findStr) == 0 )
		{
			// found entry, remove from list.
#ifdef STEAMWORKS
			for ( std::vector<std::pair<std::string, uint64>>::iterator itId = gamemods_workshopLoadedFileIDMap.begin();
				itId != gamemods_workshopLoadedFileIDMap.end(); ++itId )
			{
				if ( itId->first.compare(line.second) == 0 )
				{
					gamemods_workshopLoadedFileIDMap.erase(itId);
					break;
				}
			}
#endif // STEAMWORKS
			gamemods_mountedFilepaths.erase(it);
			return true;
		}
	}
	return false;
}

bool gamemodsIsPathInMountedFiles(std::string findStr)
{
	std::vector<std::pair<std::string, std::string>>::iterator it;
	std::pair<std::string, std::string> line;
	for ( it = gamemods_mountedFilepaths.begin(); it != gamemods_mountedFilepaths.end(); ++it )
	{
		line = *it;
		if ( line.first.compare(findStr) == 0 )
		{
			// found entry
			return true;
		}
	}
	return false;
}

void buttonGamemodsGetLocalMods(button_t* my)
{
	gamemods_window_scroll = 0;
	gamemods_window = 7;
	gamemods_localModFoldernames.clear();
	gamemods_localModFoldernames = directoryContents("./mods/", true, false);
}

void buttonGamemodsStartModdedGame(button_t* my)
{
	gamemods_numCurrentModsLoaded = gamemods_mountedFilepaths.size();
	if ( gamemods_numCurrentModsLoaded > 0 )
	{
		steamAchievement("BARONY_ACH_LOCAL_CUSTOMS");
	}

	if ( physfsIsMapLevelListModded() )
	{
		gamemods_disableSteamAchievements = true;
	}
	else
	{
		gamemods_disableSteamAchievements = false;
	}

	int w, h;

	if ( !gamemods_modelsListRequiresReload && gamemods_modelsListLastStartedUnmodded )
	{
		if ( physfsSearchModelsToUpdate() || !gamemods_modelsListModifiedIndexes.empty() )
		{
			gamemods_modelsListRequiresReload = true;
		}
		gamemods_modelsListLastStartedUnmodded = false;
	}
	if ( !gamemods_soundListRequiresReload && gamemods_soundsListLastStartedUnmodded )
	{
		if ( physfsSearchSoundsToUpdate() )
		{
			gamemods_soundListRequiresReload = true;
		}
		gamemods_soundsListLastStartedUnmodded = false;
	}

	// process any new model files encountered in the mod load list.
	int modelsIndexUpdateStart = 1;
	int modelsIndexUpdateEnd = nummodels;
	if ( gamemods_modelsListRequiresReload )
	{
		if ( physfsSearchModelsToUpdate() || !gamemods_modelsListModifiedIndexes.empty() )
		{
			// print a loading message
			drawClearBuffers();
			TTF_SizeUTF8(ttf16, language[2989], &w, &h);
			ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[2989]);
			GO_SwapBuffers(screen);
			physfsModelIndexUpdate(modelsIndexUpdateStart, modelsIndexUpdateEnd, true);
			generatePolyModels(modelsIndexUpdateStart, modelsIndexUpdateEnd, false);
		}
		gamemods_modelsListRequiresReload = false;
	}
	if ( gamemods_soundListRequiresReload )
	{
		if ( physfsSearchSoundsToUpdate() )
		{
			// print a loading message
			drawClearBuffers();
			TTF_SizeUTF8(ttf16, language[2987], &w, &h);
			ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[2987]);
			GO_SwapBuffers(screen);
			physfsReloadSounds(true);
		}
		gamemods_soundListRequiresReload = false;
	}

	if ( physfsSearchTilesToUpdate() )
	{
		// print a loading message
		drawClearBuffers();
		TTF_SizeUTF8(ttf16, language[3003], &w, &h);
		ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[3003]);
		GO_SwapBuffers(screen);
		physfsReloadTiles(false);
		gamemods_tileListRequireReloadUnmodded = true;
	}

	if ( physfsSearchBooksToUpdate() )
	{
		// print a loading message
		drawClearBuffers();
		TTF_SizeUTF8(ttf16, language[2991], &w, &h);
		ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[2991]);
		GO_SwapBuffers(screen);
		physfsReloadBooks();
		gamemods_booksRequireReloadUnmodded = true;
	}

	if ( physfsSearchMusicToUpdate() )
	{
		// print a loading message
		drawClearBuffers();
		TTF_SizeUTF8(ttf16, language[2993], &w, &h);
		ttfPrintText(ttf16, (xres - w) / 2, (yres - h) / 2, language[2993]);
		GO_SwapBuffers(screen);
		bool reloadIntroMusic = false;
		physfsReloadMusic(reloadIntroMusic);
		if ( reloadIntroMusic )
		{
			playmusic(intromusic[rand() % NUMINTROMUSIC], false, true, true);
		}
		gamemods_musicRequireReloadUnmodded = true;
	}

	// look for a save game
	if ( saveGameExists(true) || saveGameExists(false) )
	{
		openLoadGameWindow(NULL);
	}
	else
	{
		buttonOpenCharacterCreationWindow(NULL);
	}
}

void gamemodsCustomContentInit()
{

	gamemods_window = 3;
	currentDirectoryFiles = directoryContents(datadir, true, false);
	directoryToUpload = datadir;

	gamemodsMountAllExistingPaths();

	// create confirmation window
	subwindow = 1;
	subx1 = xres / 2 - 420;
	subx2 = xres / 2 + 420;
	suby1 = yres / 2 - 300;
	suby2 = yres / 2 + 300;
	strcpy(subtext, "Custom Content");

	// close button
	button_t* button = newButton();
	strcpy(button->label, "x");
	button->x = subx2 - 20;
	button->y = suby1;
	button->sizex = 20;
	button->sizey = 20;
	button->action = &buttonCloseSubwindow;
	button->visible = 1;
	button->focused = 1;
	button->key = SDL_SCANCODE_ESCAPE;
	button->joykey = joyimpulses[INJOY_MENU_CANCEL];

	// fetch local mods button
	button = newButton();
	strcpy(button->label, "local mods");
	button->x = subx1 + 16;
	button->y = suby1 + 2 * TTF12_HEIGHT + 8;
	button->sizex = 25 * TTF12_WIDTH + 8;
	button->sizey = 32;
	button->action = &buttonGamemodsGetLocalMods;
	button->visible = 1;
	button->focused = 1;

	// fetch my workshop items
	button_t* button2 = newButton();
	strcpy(button2->label, "new mod folder");
	button2->x = button->x + button->sizex + 16;
	button2->y = suby1 + 2 * TTF12_HEIGHT + 8;
	button2->sizex = 25 * TTF12_WIDTH + 8;
	button2->sizey = 32;
	button2->action = &buttonGamemodsCreateNewModTemplate;
	button2->visible = 1;
	button2->focused = 1;

	// start modded game
	button = newButton();
	strcpy(button->label, "start modded game");
	button->sizex = 25 * TTF12_WIDTH + 8;
	button->sizey = 32;
	button->x = subx2 - (button->sizex + 16);
	button->y = suby1 + 2 * TTF12_HEIGHT + 8;
	button->action = &buttonGamemodsStartModdedGame;
	button->visible = 1;
	button->focused = 1;
}

bool gamemodsClearAllMountedPaths()
{
	bool success = true;
	char **i;
	for ( i = PHYSFS_getSearchPath(); *i != NULL; i++ )
	{
		std::string line = *i;
		if ( line.compare("./") != 0 ) // don't unmount the base ./ directory
		{
			if ( PHYSFS_unmount(*i) == NULL )
			{
				success = false;
				printlog("[%s] unsuccessfully removed from the search path.\n", line.c_str());
			}
			else
			{
				printlog("[%s] is removed from the search path.\n", line.c_str());
			}
		}
	}
	gamemods_numCurrentModsLoaded = -1;
	PHYSFS_freeList(*i);
	return success;
}

bool gamemodsMountAllExistingPaths()
{
	bool success = true;
	std::vector<std::pair<std::string, std::string>>::iterator it;
	for ( it = gamemods_mountedFilepaths.begin(); it != gamemods_mountedFilepaths.end(); ++it )
	{
		std::pair<std::string, std::string> itpair = *it;
		if ( PHYSFS_mount(itpair.first.c_str(), NULL, 0) )
		{
			printlog("[%s] is in the search path.\n", itpair.first.c_str());
		}
		else
		{
			printlog("[%s] unsuccessfully added to search path.\n", itpair.first.c_str());
			success = false;
		}
	}
	gamemods_numCurrentModsLoaded = gamemods_mountedFilepaths.size();
	return success;
}

void gamemodsWindowClearVariables()
{
#ifdef STEAMWORKS
	if ( g_SteamWorkshop )
	{
		g_SteamWorkshop->createItemResult = {};
		g_SteamWorkshop->UGCUpdateHandle = {};
		g_SteamWorkshop->SubmitItemUpdateResult = {};
		SteamUGC()->ReleaseQueryUGCRequest(g_SteamWorkshop->UGCQueryHandle);
		g_SteamWorkshop->UGCQueryHandle = {};
		g_SteamWorkshop->SteamUGCQueryCompleted = {};
		g_SteamWorkshop->UnsubscribePublishedFileResult = {};
		g_SteamWorkshop->LastActionResult.creationTick = 0;
		g_SteamWorkshop->LastActionResult.actionMsg = "";
		g_SteamWorkshop->LastActionResult.lastResult = static_cast<EResult>(0);
		g_SteamWorkshop->workshopItemTags.clear();
		for ( int i = 0; i < 50; ++i )
		{
			g_SteamWorkshop->m_subscribedItemListDetails[i] = {};
		}
		g_SteamWorkshop->uploadSuccessTicks = 0;
		g_SteamWorkshop->m_myWorkshopItemToModify = {};
	}
#endif // STEAMWORKS
	directoryToUpload.clear();
	directoryPath.clear();
	directoryFilesListToUpload.clear();
	gamemods_window_scroll = 0;
	gamemods_uploadStatus = 0;
	strcpy(gamemods_uploadTitle, "Title");
	strcpy(gamemods_uploadDescription, "Description");
	strcpy(gamemods_newBlankDirectory, "");
	strcpy(gamemods_newBlankDirectoryOldName, "");
	gamemods_newBlankDirectoryStatus = 0;
	gamemods_currentEditField = 0;
	gamemods_workshopSetPropertyReturn[0] = false;
	gamemods_workshopSetPropertyReturn[1] = false;
	gamemods_workshopSetPropertyReturn[2] = false;
	gamemods_subscribedItemsStatus = 0;
}