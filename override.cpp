#include "utils.h"
#include "finder.h"
#include "override.h"
#include "path.h"


#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>

static std::unordered_map<std::string, std::vector<std::string>>  otheroverrides;

static std::unordered_map<std::string, std::vector<std::string>>  raceOverrides;


/*
For every race there is a pair (one for sex) of TESRace linking the override for the voices
nullptr means it use the key race (so it doesn't enable an override)
*/
static std::unordered_map<TESRace*, std::pair<std::vector<TESRace*>,std::vector<TESRace*>>> useVoiceOfRaceOverride;

void putRaceOverride(const char* editorID, const char* name){
	std::string namestr = name;
	std::string edid = editorID;
	std::string namestr_lower = "";
	std::string edid_low = "";
	std::transform(namestr.begin(), namestr.end(), std::back_inserter(namestr_lower), ::tolower);
	std::transform(edid.begin(), edid.end(), std::back_inserter(edid_low), ::tolower);
	auto iter = raceOverrides.find(edid_low);
	if(iter != raceOverrides.end()){
		if (std::find(iter->second.begin(), iter->second.end(), namestr_lower) == iter->second.end())
			iter->second.push_back(namestr_lower);
	}
	else{
		auto raceOverrideVector = std::vector<std::string>();
		raceOverrideVector.push_back(namestr_lower);
		raceOverrides[edid_low] = raceOverrideVector;
	}
}

void putRaceVoiceOVerride(TESRace* race, TESRace* overrideMale, TESRace* overrideFemale) {
	auto iter = useVoiceOfRaceOverride.find(race);
	if (iter == useVoiceOfRaceOverride.end()) {
		auto maleOverride = std::vector<TESRace*>();
		auto femaleOverride = std::vector<TESRace*>();
		maleOverride.push_back(overrideMale);
		femaleOverride.push_back(overrideFemale);
		auto pair = std::make_pair(maleOverride, femaleOverride);
		useVoiceOfRaceOverride[race] = pair;
	}
	else {
		auto& pair = iter->second;
		auto& maleOverride = pair.first;
		auto& femaleOverride = pair.second;
		if (std::find(maleOverride.cbegin(), maleOverride.cend(), overrideMale) == maleOverride.cend()) {
			maleOverride.push_back(overrideMale);
		}
		if (std::find(femaleOverride.cbegin(), femaleOverride.cend(), overrideFemale) == femaleOverride.cend()) {
			femaleOverride.push_back(overrideFemale);
		}
	}
}

std::vector<TESRace*> getRaceVoiceOverride(TESRace* race, bool isFemale) {
	auto raceOver = useVoiceOfRaceOverride.find(race);
	if (raceOver == useVoiceOfRaceOverride.end()) return std::vector<TESRace*>();
	return isFemale ? raceOver->second.second : raceOver->second.first; 
}

void InitializeConfigurationOverrides(const char* overrideFile){
	
}

void printMap(){
	std::string print = "";
	for(auto iter = raceOverrides.begin(); iter != raceOverrides.end(); iter++){
		print += iter->first + ":";
		for(auto&& iter1 = iter->second.begin(); iter1 != iter->second.end(); iter1++){
			print += *iter1 + ", ";
		}
		print += "\n";
	}
	_MESSAGE_D("Race name overrides:\n");
	_MESSAGE_D(print.c_str());
	print.clear();
	for (auto iter = useVoiceOfRaceOverride.begin(); iter != useVoiceOfRaceOverride.end(); iter++) {
		print += iter->first->GetEditorName();
		print += ":";
		std::string print_inner = "";
		for (auto&& iter1 = iter->second.first.begin(); iter1 != iter->second.first.end(); iter1++) {
			print_inner += ", ";
			print_inner += (*iter1) ? (*iter1)->GetEditorName() : "<THIS>";
		}
		print += &print_inner[2];
		print += " ||| ";
		print_inner.clear();
		for (auto&& iter1 = iter->second.second.begin(); iter1 != iter->second.second.end(); iter1++) {
			print_inner += ", ";
			print_inner += (*iter1) ? (*iter1)->GetEditorName() : "<THIS>";
		}
		print += &print_inner[2];
		print += "\n";
	}
	_MESSAGE_D("Race overrides:\n");
	_MESSAGE_D(print.c_str());
}

//Will need to test for overhead.
//TODO possible premature optimization: Add a boolean to save if a current name is either equal to the race edid or to a previous name
//This will allow to save the entire order of race names appearing even if equals keeping the substituion operation cheaply, as the number of files checked will be reduced
bool getOverrideFor(std::string& RaceToOverride, const char* input, char* output){
	auto vec_ref = raceOverrides.find(RaceToOverride);
	if (vec_ref == raceOverrides.end()) return false;
	auto& val = vec_ref->second;
	for (std::string& over : reverse_wrapper(val)){
		replacePathComponent(Component::Race, input, over.c_str(), output);
		if (FileExists(output)) return true;
	}
	return false;
}
//Not exactly true. A back override (an override with a name appeared previosuly) will not be saved in the vector, so it will appear as an scripted change.
//NO biggie probably, the proper fix will be probably unfeasible without serious profiling and optimizations
bool IsRaceNameScriptOverride(std::string& edid, std::string&  name) {
	auto&& vec_ref = raceOverrides.find(edid);
	if (vec_ref == raceOverrides.end()) return true; //No override found assumed logic
	if (vec_ref->second.empty()) return true; 
	if (!vec_ref->second.empty() && vec_ref->second.back() != name) {
		return true;  //if name different then last loaded override assume script changed
	}
	return false;
}

void ApplyTransform(std::function<TESRace*(TESRace*)> transform) {
	for (auto& ref : useVoiceOfRaceOverride) {
		auto& pairOver = ref.second;
		std::transform(pairOver.first.cbegin(), pairOver.first.cend(), pairOver.first.begin(), transform);
		std::transform(pairOver.second.cbegin(), pairOver.second.cend(), pairOver.second.begin(), transform);
	}
}
