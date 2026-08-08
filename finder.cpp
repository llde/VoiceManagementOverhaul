#include "obse/GameData.h"
#include "finder.h"

bool FileExists(const char* file) {
	return (*g_FileFinder)->FindFile(file, 0, FileFinder::Flags::FindFile_ArchiveAndLooseFile, -1) == FileFinder::kFileStatus_NotFound ? false : true;
	//TODO packed /unpacked results from FindFile?
}