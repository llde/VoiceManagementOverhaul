#include <StdAfx.h>
#include "obse/PluginAPI.h"
#include "obse/CommandTable.h"
#include "obse_common/SafeWrite.h"
#include "obse/GameData.h"
#include "obse/ParamInfos.h"
#include "obse/Script.h"
#include "obse/GameObjects.h"

#include <string>
#include <vector>

#include "utils.h"
#include "path.h"
#include "override.h"
#include "finder.h"
#include "edid_hook.h"

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
OBSEMessagingInterface*		g_msg;
OBSETasksInterface*			g_Task;
const OBSEInterface*		g_obse;
IDebugLog		gLog("VoiceManagementOverhaul.log");
#define MAX_VOICENAME 256
std::string name = "Voice Management Overhaul";
std::string completeName = "Let's People Speak: " +  name;
//static const char* ConfigurationFile = "Data\\OBSE\\Plugins\\voice_redirector.ini";
//static const char* OverrideFile = "Data\\OBSE\\Plugins\\voice_redirector.over";
static const char* SilentVoiceMp3 = "Data\\Sound\\Voice\\LetPeopleRead.mp3";

static char NewSoundFile[MAX_VOICENAME];
static void __stdcall  HookedCreateSoundString(Actor* actor, BSStringT* path) {
	TESNPC* npc = actor->baseForm->typeID == FormType::kFormType_NPC ? (TESNPC*)actor->baseForm : nullptr;
	char* str = path->m_data;
	if (npc) {
		std::string edid;
		std::string npcName = std::string(npc->GetEditorName()); //TO FIX an issue with REID integration
		MESSAGE_DEBUG("%s: Got '%s'  for '%s'", npcName.c_str(), str, npc->race.race->GetEditorName());
		//Override for NPC specific voice
		memset(NewSoundFile, 0, MAX_VOICENAME);
		replacePathComponent(Component::Race, str, npcName.c_str(), NewSoundFile);
		stripPathComponent(Component::Sex, NewSoundFile); //Strip sex component in specific NPC case. Sex is implicit in the NPC proper.
		if (FileExists(NewSoundFile)) {
			MESSAGE_DEBUG("Sound file for NPC %s at  %s found", npcName.c_str(), NewSoundFile);
			path->Set(NewSoundFile);
			return;
		}
		else {
			MESSAGE_DEBUG("Sound file for NPC %s at  %s not found", npcName.c_str(), NewSoundFile);
		}
		//Continue race override logic
		auto overr = getRaceVoiceOverride(npc->race.race, npc->actorBaseData.IsFemale());
		//	if (overr.empty()) overr.push_back(npc->race); //NO override specified for race
		TESRace* overRace = npc->race.race->voiceRaces[npc->actorBaseData.IsFemale()];
		if (overr.empty() || (!overr.empty() && overr.back() != overRace)) {
			MESSAGE_DEBUG("Race Voice OVerride changed with scripts, pick this before the rest");
			overr.push_back((npc->race.race->voiceRaces[npc->actorBaseData.IsFemale()] ? npc->race.race->voiceRaces[npc->actorBaseData.IsFemale()] : npc->race.race));
		}
		for (auto& override_race : reverse_wrapper(overr)) {
			std::string name;
			if (override_race) {
				edid = override_race->GetEditorName();
				name = override_race->fullName.name.m_data;
			}
			else {
				edid = npc->race.race->GetEditorName();
				name = npc->race.race->fullName.name.m_data;

			}
			std::string edid_lower;
			std::string name_lower;
			std::transform(edid.begin(), edid.end(), std::back_inserter(edid_lower), ::tolower);
			std::transform(name.begin(), name.end(), std::back_inserter(name_lower), ::tolower);

			//Race name can be changed on runtime. Some mods expect to see the audio in the path using the newly setted name
			if (IsRaceNameScriptOverride(edid_lower, name_lower)) {
				MESSAGE_DEBUG("Race name was overrided, try the asked file before");
				if (FileExists(str)) {
					MESSAGE_DEBUG("File exist, Bailout");
					return;
				}
				else {
					MESSAGE_DEBUG("File don't exist. Continue logic");
				}
			}
			memset(NewSoundFile, 0, MAX_VOICENAME);
			replacePathComponent(Component::Race, str, edid_lower.c_str(), NewSoundFile);
			path->Set(NewSoundFile);
			str = path->m_data; //SAFETY: Set could reallocate
			MESSAGE_DEBUG("After Edid override '%s'", NewSoundFile);
			if (!FileExists(NewSoundFile)) { /* Test for path with editor id */
				memset(NewSoundFile, 0, MAX_VOICENAME);
				bool override_found = getOverrideFor(edid_lower, str, NewSoundFile); /*First level override*/
				if (override_found) {
					path->Set(NewSoundFile);
					MESSAGE_DEBUG("Override '%s'", NewSoundFile);
					return;
				}
				MESSAGE_DEBUG("No Voice found for %s", edid_lower.c_str());
			}
			else {
				MESSAGE_DEBUG("Override '%s'", NewSoundFile);
				return;
			}
		}
		path->Set(SilentVoiceMp3);
	}
}

static UInt32 kHookSaveActor = 0x0052E4CA;
static UInt32 kHookSaveActorRetn = 0x0052E4D0;
static Actor* ActorContext = nullptr;

static __declspec(naked) void HookSaveActorContext() {
	__asm {
		push eax 
		mov eax,  [esp + 0x38 + 0x4]
		mov ActorContext, eax
		pop eax
		mov [esp + 0x34 + 0x4], eax
		cmp ebp, ebx 
		jmp [kHookSaveActorRetn]
	}
}
static UInt32 kHookCreateSoundString = 0x0052E64E;
static __declspec(naked) void HookCreateSoundString() {
	__asm {
		push[esp + 0x0 + 0x14]
		push ActorContext
		call HookedCreateSoundString
		mov eax, 1
		retn 0x14
	}
}

//TODO: Add proper configuration
//Races whose english name match the Edid won't be inserted.
static void AddDefaultEnglishRaceNames(void){
	putRaceOverride("HighElf" , "High Elf" );
	putRaceOverride("WoodElf" , "Wood Elf" );
	putRaceOverride("DarkElf" , "Dark Elf" );
	putRaceOverride("GoldenSaint" , "Golden Saint" );
	putRaceOverride("DarkSeducer" , "Dark Seducer" );
}

static UInt32* kBackgroundLoadLip = (UInt32*)0x00B1490C;
static void EventMessageCallback(OBSEMessagingInterface::Message* msg) {
	switch (msg->type) {

	case OBSEMessagingInterface::kMessage_GameInitialized:
		MESSAGE_DEBUG("Fixups Race Voice Overrides From GI");
		*kBackgroundLoadLip = 0;  //Disable LipAsyncTask, force the setting, as jumping cause the subtitle to not appear. 
		AddDefaultEnglishRaceNames();
		//The original task seems to have an issue where redirected hello (except changing only the race field apparently) doesn't play
		ApplyTransform([](TESRace* refID) { return (TESRace*) LookupFormByID((UInt32)refID); });
		printMap();
		break;
	case OBSEMessagingInterface::kMessage_PostPostLoad:
		ApplyEdidHooks(g_obse);
		break;
	default:
		return;
	}
}


static void TaskFunction() {
	static bool DoOnce = 0;
	if (DoOnce == 0) {
		MESSAGE_DEBUG("Fixups Race Voice Overrides From Task");
		*kBackgroundLoadLip = 0;
		AddDefaultEnglishRaceNames();
		ApplyTransform([](TESRace* refID) { return (TESRace*)LookupFormByID((UInt32)refID); });
		printMap();
		DoOnce = 1;
	}
}

extern "C" {

	bool OBSEPlugin_Query(const OBSEInterface * obse, PluginInfo * info)
	{
		_MESSAGE("%s: OBSE calling plugin's Query function. <v1.2.7>", completeName.c_str());

		// fill out the info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = name.c_str();
		info->version = MAKE_OBLIVION_VERSION(1,2,7);
		g_pluginHandle = obse->GetPluginHandle();

		// version checks
		if (!obse->isEditor)
		{
			if (obse->obseVersion < OBSE_VERSION_INTEGER)
			{
				_ERROR("OBSE version too old (got %08X expected at least %08X)", obse->obseVersion, OBSE_VERSION_INTEGER);
				return false;
			}

			if (!(OBSETasks2Interface*)obse->QueryInterface(kInterface_Tasks2)) {
				_MESSAGE("Currrent OBSE version doesn't support GameInitialized messages. Use Task method");
				g_Task = (OBSETasksInterface*)obse->QueryInterface(kInterface_Tasks);
				if (!g_Task) {
					_MESSAGE("[ERROR] Cannot load Task Interface. OBSE version too old. Use At Least xOBSE 22.0 (xOBSE 22.9 or above preferred)");
					return false;
				}
				g_Task->EnqueueTask(TaskFunction);
			}
			g_msg = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
			if (!g_msg) _MESSAGE("[ERROR] Cannot Initialize OBSE Messaging Interface");
			g_msg->RegisterListener(g_pluginHandle, "OBSE", &EventMessageCallback);
		}
		else
		{
			_MESSAGE("Not compatible with editor, exiting.");
			return false;

		}

		return true;
	}

	bool OBSEPlugin_Load(const OBSEInterface * obse)
	{
		_MESSAGE("%s: OBSE calling plugin's Load function.", completeName.c_str());
		g_obse = obse;
		WriteRelJump(kHookCreateSoundString, (UInt32)&HookCreateSoundString);
		WriteRelJump(kHookSaveActor, (UInt32) &HookSaveActorContext);
		_MESSAGE("Hook address for Voice path generation function.");
	//	WriteRelCall(0x0052E5BF, (UInt32)&Asseghimma); //This create the path using the last override (mod list) //Keep the hook address 
	//	WriteRelCall(0x0052E612, (UInt32)&Asseghimma); // This with the first masterfile. 
	//	WriteRelJump(0x005F7489, 0x005F74D4);
	//	ApplyEdidHooks(obse);
		
		return true;
	}

};
