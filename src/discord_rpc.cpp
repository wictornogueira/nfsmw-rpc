#include <ctime>
#include <stdio.h>
#include <windows.h>
#include <discord_rpc.h>

#include <config.h>
#include <utils.h>

using namespace std;
using namespace utils;

static void format_state (char* state, char length) {
  if (*C_CAR_PTR < 0 || *CARS_ADDR_PTR == 0 || *OPT_ADDR_PRT == 0) {
    state[0] = 0;
    return;
  }

  char c_mode = *(char*)(*OPT_ADDR_PRT + 0x12C);
  switch (c_mode) {
    case 1:
    case 4:
    case 33:
      break;
    default:
      state[0] = 0;
      return;
  }

  const int car_addr = (*CARS_ADDR_PTR + (*C_CAR_PTR * 0xD0));
  const char* const car_name = (char*)(car_addr + 0x10);

  const auto car_itr = CAR_TABLE.find(car_name);

  if (car_itr != CAR_TABLE.end()) {
    sprintf_s(state, length, "%s", car_itr->second);
  } else {
    const char* const car_brand = (char*)(car_addr + 0x40);
    sprintf_s(state, length, "%s %s", car_brand, car_name);
  }
}

static void format_details (char* details, char length) {
  if (*OPT_ADDR_PRT == 0) { return; }

  char c_mode = *(char*)(*OPT_ADDR_PRT + 0x12C);
  switch (c_mode) {
    case 4:
      sprintf_s(details, length, "Quick Race");
      break;
    case 1:
      sprintf_s(details, length, "Career");
      break;
    case 33:
      sprintf_s(details, length, "Customization Shop");
      break;
    default:
      sprintf_s(details, length, IN_MENU);
  }
}

static DWORD WINAPI ThreadEntry (LPVOID lpParam) {
  char state[64];
  char details[64];
  
  Discord_Initialize(APP_ID, 0, 0, 0);

  DiscordRichPresence discord_presence;
  memset(&discord_presence, 0, sizeof(discord_presence));

  discord_presence.startTimestamp = time(0);
  discord_presence.largeImageKey = IMG_KEY;
  discord_presence.largeImageText = IMG_TXT;

  discord_presence.state = state;
  discord_presence.details = details;

  details[0] = 0;

  while (1) {
    format_details(details, sizeof(details));
    format_state(state, sizeof(state));

    Discord_UpdatePresence(&discord_presence);
    Discord_RunCallbacks();
    Sleep(UPD_INTVL);
  }
}

extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
  switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(hinstDLL);
      CreateThread(0, 0, ThreadEntry, 0, 0, 0);
      break;
    case DLL_PROCESS_DETACH:
      Discord_Shutdown();
      break;
  }

  return 1;
}
