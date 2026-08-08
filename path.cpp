#include "path.h"
/*
* Return the NULL terminated path component in an HEAP allocated way
* nullptr if invalid component
*/
char* getSingleComponent(Component path_component, const char* input) {
	size_t component = 0;
	size_t len = strlen(input);
	const char* new_ptr = nullptr;
	size_t comp_len = 0;
	for (size_t i = 0; i < len; i++) {
		if (input[i] == '\\') {
			component++;
			continue;
		}
		if (path_component == Component::Race && component > 4) break;
		if (path_component == Component::Race && component == 4) {
			if (new_ptr == nullptr) new_ptr = input + i;
			comp_len++;
		}
	}
	if (new_ptr == nullptr || comp_len == 0) return nullptr;
	char* ttt = new char[comp_len + 1];
	memcpy(ttt, new_ptr, comp_len);
	ttt[comp_len] = '\0';
	//_MESSAGE("SIZE %zu", comp_len);
	return ttt;
}

void stripPathComponent(Component path_component, char* input) {
	size_t len = strlen(input);
	char* new_ptr = nullptr;
	size_t pos_start_comp = 0, pos_end_comp = 0;
	size_t component = 0;
	for (size_t i = 0; i < len; i++) {
		if (input[i] == '\\') {
			component++;
			if (path_component == Component::Sex && component == 5 ) {
				pos_start_comp = i;
				new_ptr = input + i;
			}
			if (path_component == Component::Sex && component > 5) {
				pos_end_comp = i;
				break;
			};
		}
	}
	if (new_ptr) {
		size_t newlen = len - (pos_end_comp - pos_start_comp);
		memmove(new_ptr, input + pos_end_comp, newlen);
		memset(input + newlen + 1, 0, len - newlen);
	}
}

bool replacePathComponent(Component path_component, const char* input, const char* repl, char* output) {
	size_t component = 0;
	size_t len = strlen(input);
	const char* new_ptr = nullptr;
	size_t pos_start_comp = 0;
	size_t remain = 0;
	for (size_t i = 0; i < len; i++) {
		if (input[i] == '\\') {
			component++;
			if (path_component == Component::Race && component == 4) {
				pos_start_comp = i;
			}
			if (path_component == Component::Race && component > 4) {
				new_ptr = input + i;
				remain = len - i;
				break;
			};
		}
	}
	if (new_ptr == nullptr) return false;
	memset(output,0, len);
 	memcpy(output, input , pos_start_comp +1);
	memcpy(output + pos_start_comp +1, repl, strlen(repl));
	memcpy(output + pos_start_comp + strlen(repl) +1 ,  new_ptr, remain);
	return true;
}

/*
void appendToPath(char* string, const char* app) {
	strcat(string, app);
}*/

void removeExtension(char* string1) {
	for (size_t i = (strlen(string1) - 1); i >= 0; i--) {
		if (string1[i] == '.') {
			string1[i] = '\0';
			break;
		}
	}
}