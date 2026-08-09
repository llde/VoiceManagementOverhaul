#include <GameForms.h>
#include <GameData.h>
#include <obse_common/SafeWrite.h>
#include <PluginAPI.h>
#include "edid_hook.h"
#include "constants.h"
#include "override.h"
#include <utils.h>

OBSEMessagingInterface* g_msgIntfc;

REIDInteropData* msg = new REIDInteropData();

const char* __stdcall DispatchREIDMessage(TESForm* form){
	msg->FormID = form->refID;
	bool dis = g_msgIntfc->Dispatch(g_pluginHandle, 'EDID' , msg , sizeof(REIDInteropData) ,"REID");
	if (!dis) _MESSAGE("Dispatch failed");
	if (dis && msg->HasEditorID) {
//		_MESSAGE("EDID found for  %08X %s %s", form, msg->EditorIDBuffer, form->GetFullName() ? form->GetFullName()->name.m_data : "<no name>");
		return msg->EditorIDBuffer;
	}
//	_MESSAGE("Non EDID found for  %08X  %s", form, form->GetFullName() ? form->GetFullName()->name.m_data: "<no name>");
	return "";
}

void __stdcall TESForm_GetEditorIDREID()
{
	__asm
	{
		push	ecx
		call	DispatchREIDMessage
	}
}

std::map<UInt32, const char*>	FormIDReferenceMap;


void __stdcall SetEditorID(TESForm* form, const char* EditorID){
	size_t newBufferSize = strlen(EditorID) + 1;
	char* edid = (char*) FormHeap_Allocate(newBufferSize);
	strcpy_s(edid, newBufferSize ,EditorID);
	const char* old_edid =  FormIDReferenceMap[form->refID]; 
	FormIDReferenceMap[form->refID] = edid;
	if(old_edid) FormHeap_Free((void*)old_edid);
}

const char* __stdcall GetEditorID(TESForm* form){
	const char* old_edid =  FormIDReferenceMap[form->refID]; 
	if(old_edid) return old_edid;
	return "";
}

void __stdcall TESForm_SetEditorID(const char* EditorID)
{
	__asm
	{
		push	EditorID
		push	ecx
		call SetEditorID
	}
}


void __stdcall TESForm_GetEditorID()
{
	__asm
	{
		push	ecx
		call	GetEditorID
	}
}

UInt32 (__cdecl* ResolveFormID)(UInt32* formID, ModEntry::Data* file)  = (UInt32 (__cdecl * )(UInt32*,  ModEntry::Data*))0x0046BB20;

void (__cdecl* TESFullName_Load)(TESFullName*, ModEntry::Data*) = (void (__cdecl*)(TESFullName*, ModEntry::Data*))kTESFullNameLoad;

static TESRace* currentRace;
void __cdecl TESFullNameHook(TESFullName* name, ModEntry::Data* file){
	TESRace* currentForm;
	__asm {
		mov currentForm, ebx
	}
	TESFullName_Load(name, file); /*If FULL records exist but are empty the BSString get a null pointer m_data*/
	if(name->name.m_data){
		putRaceOverride(currentForm->GetEditorName(),  name->name.m_data);
		currentRace = currentForm;
	}
//	_MESSAGE("FullName for  %s : %s", currentForm->GetEditorName(), name->name.m_data);

}
//While maleVoice and femaleVoice are technically forms, at this point they arent resolved so they are integers (values are still not resolve by mod id load order and so correspond to the value in the ESM/ESP)
void __stdcall TESRace_OverrideVoice(TESRace* thisRace, ModEntry::Data* file,  UInt32 maleVoice, UInt32 femaleVoice){
	/*Fixup form id with mod load order id*/
	if(maleVoice) ResolveFormID(&maleVoice, file);
	if(femaleVoice) ResolveFormID(&femaleVoice, file);
	putRaceVoiceOVerride(thisRace, (TESRace*)maleVoice, (TESRace*)femaleVoice);
	currentRace = nullptr;
	/*if(thisRace && thisRace->GetEditorName())
		_MESSAGE("%s %08X  %s  %08X  %08X" , thisRace->GetEditorName(), thisRace->refID, file->name ,maleVoice, femaleVoice  );
	else if(thisRace){
		_MESSAGE("<NULL> %08X  %s  %08X  %08X" , thisRace->refID, file->name ,maleVoice, femaleVoice  );
	}
	else {
		_MESSAGE("NULL this Race in %s", file->name);
	}*/
}

void __declspec(naked) TESRace_OverrideVoiceHook()
{
	__asm
	{
		pushad
		push 	edx
		push	ecx
		mov eax, [ebp + 0x8]
		push eax
		push 	ebx
		call TESRace_OverrideVoice
		popad
		jmp [kTESRaceVoiceOverrideDest]
	}
}
void __declspec(naked) TESRace_OverrideVoiceNullHook()
{
	__asm
	{
		pushad
		push 	eax
		push	eax
		push 	ebx
		call TESRace_OverrideVoice
		popad
		jmp [kTESRaceVoiceOverrideDest]
	}
}
void __stdcall Log(TESRace* nooverride, UInt32 eax) { _MESSAGE("Scream  %s  %u", nooverride->GetEditorName(),eax); }
UInt32 kLoc = 0x0052D9E4;
void __declspec(naked) TESRace_NullOverride() {
	__asm {
	cmp eax, 8
	jz skip
	pushad
	push eax
	push ebx
	call Log
	popad 
	jmp [kTESRaceVoiceOverrideDest]

	skip:
	jmp [kLoc]
	}
}

void __stdcall TESRace_Finalize(UInt32 eax) {
	//Not sure why this is called more then once
	if (currentRace && eax == 1) {
//		_MESSAGE("%s %08X ", currentRace->GetEditorName(), currentRace->refID );
//		_MESSAGE("It's the end of the line %u", eax);
		putRaceVoiceOVerride(currentRace, nullptr, nullptr);
		currentRace = nullptr;
	}
}

static UInt32 kCall = 0x009811E2;
static UInt32 kRetn = 0x0052DD7A;
void __declspec(naked) TESRace_EndHook() {
	__asm {
		pushad
		push eax
		call TESRace_Finalize
		popad
		jmp [kRetn]
	}
}
#define TESFORM_GetEditorIDVanilla 0x004129A0
const unsigned char* ReturnEmptyStringFirstByte = reinterpret_cast<const unsigned char*>(0x004129A0);
void ApplyEdidHooks(const OBSEInterface* obse){
	bool isMessageLoggerLoaded = obse->GetPluginLoaded("MessageLogger");
	if(obse->GetPluginLoaded("REID")){
		_MESSAGE("%d   %c", *ReturnEmptyStringFirstByte, *ReturnEmptyStringFirstByte);
		if (*ReturnEmptyStringFirstByte != 0xB8) {
			_MESSAGE("Detected EditorIdMapper");
		}
		else {
			_MESSAGE("Detected REID, applying Message Interop");
			g_msgIntfc = (OBSEMessagingInterface*)obse->QueryInterface(kInterface_Messaging);
			if (!g_msgIntfc) _MESSAGE("[ERROR] Cannot get Messaging Interface from OBSE");
			for (UInt32 i = 0; i < VTBLTableSizeREID; i++) {
				UInt32 PatchAddressGet = g_VTBLTableREID[i].Address + kTESForm_GetEditorID_VTBLOffset;
				if (*(UInt32*)PatchAddressGet == TESFORM_GetEditorIDVanilla) {
					SafeWrite32(PatchAddressGet, (UInt32)TESForm_GetEditorIDREID);
				}
			}
		}
	}
	else if(isMessageLoggerLoaded){
		_MESSAGE("Detected MessageLogger. Do nothing");
	}
	else{
		_MESSAGE("No EDID plugin found, install own hooks.");
		for (UInt32 i = 0; i < VTBLTableSizeNoREID; i++) {
			UInt32 PatchAddressGet = g_VTBLTableNoREID[i].Address + kTESForm_GetEditorID_VTBLOffset;
			UInt32 PatchAddress = g_VTBLTableNoREID[i].Address + kTESForm_SetEditorID_VTBLOffset;
			SafeWrite32(PatchAddressGet, (UInt32)TESForm_GetEditorID);
			SafeWrite32(PatchAddress, (UInt32)TESForm_SetEditorID);
		}
	}
	WriteRelCall(kTESRaceFullNameLoad1, (UInt32)&TESFullNameHook);
	WriteRelJump(kTESRaceVoiceOverrideJump, (UInt32)&TESRace_OverrideVoiceHook);
	WriteRelJump(kTESRaceVoiceOverrideNullJump, (UInt32)&TESRace_OverrideVoiceNullHook); //These hooks are apparently unused
	WriteRelJump(0x0052D9DE, (UInt32)&TESRace_NullOverride); //These hooks are apparently unused
	WriteRelJump(0x0052DD75, (UInt32)&TESRace_EndHook);
}


