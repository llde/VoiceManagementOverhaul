#include <PluginAPI.h>

//From REID
struct REIDInteropData
{
	static const UInt32			kBufferSize = 0x200;

	UInt32						FormID;

	bool						HasEditorID;
	char						EditorIDBuffer[kBufferSize];
};
const UInt32 kTESRaceFullNameLoad1 = 0x0052D8E4;
const UInt32 kTESRaceFullNameLoad2 = 0x0052D8F5;

const UInt32 kTESRaceVoiceOverrideJump = 0x0052DA02;
const UInt32 kTESRaceVoiceOverrideNullJump = 0x0052D9D6;
const UInt32 kTESRaceVoiceOverrideDest = 0x0052DD4C;


const UInt32 kTESFullNameLoad = 0x0046C7A0;

const UInt32 kTESForm_SetEditorID_VTBLOffset = 0xD8;
const UInt32 kTESForm_GetEditorID_VTBLOffset = 0xD4;
struct VTBLData
{
	UInt32				Address;
	const char*			Class;
};

const UInt32 VTBLTableSizeREID = 53;
const VTBLData g_VTBLTableREID[VTBLTableSizeREID] =
{
	{ 0x00A52C6C, "TESClass" },
	{ 0x00A53524, "TESFaction" },
	{ 0x00A53654, "TESHair" },
	{ 0x00A53414, "TESEyes" },
	{ 0x00A548FC, "TESRace" },
	{ 0x00A54EFC, "TESSkill" },
	{ 0x00A32B14, "EffectSetting" },
	{ 0x00A49DA4, "Script" },
	{ 0x00A4656C, "TESLandTexture" },
	{ 0x00A33C84, "EnchantmentItem" },
	{ 0x00A34C3C, "SpellItem" },
	{ 0x00A52524, "BirthSign" },
	{ 0x00A43CB4, "TESObjectACTI" },
	{ 0x00A43FB4, "TESObjectAPPA" },
	{ 0x00A441BC, "TESObjectARMO" },
	{ 0x00A44454, "TESObjectBOOK" },
	{ 0x00A44644, "TESObjectCLOT" },
	{ 0x00A44824, "TESObjectCONT" },
	{ 0x00A44A54, "TESObjectDOOR" },
	{ 0x00A33F5C, "IngredientItem" },
	{ 0x00A4319C, "TESObjectLIGH" },
	{ 0x00A44CA4, "TESObjectMISC" },
	{ 0x00A44DEC, "TESObjectSTAT" },
	{ 0x00A4280C, "TESGrass" },
	{ 0x00A44FB4, "TESObjectTREE" },
	{ 0x00A41B1C, "TESFlora" },
	{ 0x00A41D1C, "TESFurniture" },
	{ 0x00A45354, "TESObjectWEAP" },
	{ 0x00A4068C, "TESAmmo" },
	{ 0x00A53DD4, "TESNPC" },
	{ 0x00A5324C, "TESCreature" },
	{ 0x00A42B9C, "TESLevCreature" },
	{ 0x00A456F4, "TESSoulGem" },
	{ 0x00A42A4C, "TESKey" },
	{ 0x00A3217C, "AlchemyItem" },
	{ 0x00A4592C, "TESSubSpace" },
	{ 0x00A45534, "TESSigilStone" },
	{ 0x00A42DC4, "TESLevItem" },
	{ 0x00A47FEC, "TESWeather" },
	{ 0x00A45C9C, "TESClimate" },
	{ 0x00A3FEE4, "TESRegion" },
	{ 0x00A46C44, "TESObjectREFR" },
	{ 0x00A53774, "TESIdleForm" },
	{ 0x00A6743C, "TESPackage" },
	{ 0x00A407F4, "TESCombatStyle" },
	{ 0x00A4997C, "TESLoadScreen" },
	{ 0x00A42F1C, "TESLevSpell" },
	{ 0x00A43DFC, "TESObjectANIO" },
	{ 0x00A47F0C, "TESWaterForm" },
	{ 0x00A419EC, "TESEffectShader" },
	{ 0x00A6FC9C, "Character" },
	{ 0x00A6E074, "Actor" },
	{ 0x00A710F4, "Creature" }
};

const UInt32 VTBLTableSizeNoREID = 2;
const VTBLData g_VTBLTableNoREID[VTBLTableSizeNoREID] = {
	{ 0x00A548FC, "TESRace" },
	{ 0x00A53DD4, "TESNPC" }
};

void ApplyEdidHooks(const OBSEInterface* obse); 
