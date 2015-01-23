#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <winuser.h>

#include "../module_headers/module.h"
#include "../module_headers/control_module.h"
#include "../rcml_compiler/globals.h"

#include "keyboard_control_module.h"

#include "SimpleIni.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

std::map<std::string, regval> axis_names;
std::map<WORD, AxisKey*> axis_keys;
std::map<regval, AxisData*> axis;

/* GLOBALS CONFIG */
int COUNT_AXIS;

inline regval getIniValue(CSimpleIniA *ini, const char *section_name, const char *key_name) {
	const char *tmp = ini->GetValue(section_name, key_name, NULL);
	if (!tmp) {
		printf("Not specified value for \"%s\" in section \"%s\"!\n", key_name, section_name);
		throw;
	}
	return strtol(tmp, NULL, 0);
}

inline const char *copyStrValue(const char *source) {
	char *dest = new char[strlen(source)+1];
	strcpy(dest, source);
	return dest;
}

void KeyboardControlModule::execute(sendAxisState_t sendAxisState) {
	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if (hStdin == INVALID_HANDLE_VALUE) {
		goto exit; //error
	}

	DWORD fdwSaveOldMode;
	if (!GetConsoleMode(hStdin, &fdwSaveOldMode)) {
		goto exit; //error
	}
	DWORD fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
	if (!SetConsoleMode(hStdin, fdwMode)) {
		goto exit; //error
	}

	bool working = true;
	INPUT_RECORD irInBuf[128];
	DWORD cNumRead;
	DWORD i;
	while (working) {
		if (!ReadConsoleInput(hStdin, irInBuf, 128, &cNumRead))  {
			return; //error
		}

		for (i = 0; i < cNumRead; i++) {
			switch (irInBuf[i].EventType) {
				case KEY_EVENT: {
						WORD key_code = irInBuf[i].Event.KeyEvent.wVirtualKeyCode;
						printf("Key event: %d", key_code);
						if (key_code != VK_ESCAPE) {
							if (axis_keys.find(key_code) != axis_keys.end()) {
								AxisKey *ak = axis_keys[key_code];
								regval axis_index = ak->axis_index;
								regval val = irInBuf[i].Event.KeyEvent.bKeyDown ? ak->pressed_value : ak->unpressed_value;
								printf("axis %d val %d \n", axis_index, val);
								(*sendAxisState)(axis_index, val);
							}
						} else {
							working = false;
						}
					}
				default:
					break;
			}
		}
	}

	exit:
	SetConsoleMode(hStdin, fdwSaveOldMode);
}

KeyboardControlModule::KeyboardControlModule() {
	WCHAR DllPath[MAX_PATH] = {0};
	GetModuleFileNameW((HINSTANCE)&__ImageBase, DllPath, _countof(DllPath));

	WCHAR *tmp = wcsrchr(DllPath, L'\\');
	WCHAR ConfigPath[MAX_PATH] = {0};
	
	size_t path_len = tmp - DllPath;

	wcsncpy(ConfigPath, DllPath, path_len);
	wcscat(ConfigPath, L"\\config.ini");

	CSimpleIniA ini;
    ini.SetMultiKey(true);
    ini.LoadFile(ConfigPath);

	CSimpleIniA::TNamesDepend axis_names_ini;
	ini.GetAllKeys("mapped_axis", axis_names_ini);
	
	regval axis_id = 1;
	try {
		for (
			CSimpleIniA::TNamesDepend::const_iterator i = axis_names_ini.begin(); 
			i != axis_names_ini.end(); 
			++i) { 
			
			std::string section_name(i->pItem);
			const char *section_name_c = section_name.c_str();
			
			axis_names[section_name] = axis_id;
			axis[axis_id] = new AxisData();
			axis[axis_id]->axis_index = axis_id;
			axis[axis_id]->name = copyStrValue(section_name_c);
			axis[axis_id]->upper_value = getIniValue(&ini, section_name_c, "upper_value");
			axis[axis_id]->lower_value = getIniValue(&ini, section_name_c, "lower_value");

			CSimpleIniA::TNamesDepend values;
			ini.GetAllValues("mapped_axis", section_name_c, values);

			for(
				CSimpleIniA::TNamesDepend::const_iterator iniValue = values.begin(); 
				iniValue != values.end(); 
				++iniValue) { 
					WORD key_code = strtol(iniValue->pItem, NULL, 0);

					std::string key_name = section_name;
					key_name.append("_");
					key_name.append(iniValue->pItem);
			
					const char *key_name_c = key_name.c_str();

					AxisKey *ak = new AxisKey;
					ak->axis_index = axis_id;
					ak->pressed_value = getIniValue(&ini, key_name_c, "pressed_value");
					ak->unpressed_value = getIniValue(&ini, key_name_c, "unpressed_value");
					axis_keys[key_code] = ak;
			}
			values.clear();
			++axis_id;
		}
	} catch (...) {
		is_error_init = 1;
		return;
	}
	axis_names_ini.clear();
	axis_names.clear();

	COUNT_AXIS = axis.size();
	robot_axis = new AxisData[COUNT_AXIS];
	for (int j = 0; j < COUNT_AXIS; ++j) {
		robot_axis[j].axis_index = axis[j+1]->axis_index;
		robot_axis[j].lower_value = axis[j+1]->lower_value;
		robot_axis[j].upper_value = axis[j+1]->upper_value;
		robot_axis[j].name = axis[j+1]->name;
	}
	
	is_error_init = 0;
}

int KeyboardControlModule::init() {
	return is_error_init ? 1 : 0;
}

AxisData* KeyboardControlModule::getAxis(int *count_axis) {
	if (is_error_init) {
		(*count_axis) = 0;
		return NULL;
	}
	
	(*count_axis) = COUNT_AXIS;
	return robot_axis;
}

void KeyboardControlModule::destroy() {
	delete[] robot_axis;
	
	for(
		std::map<WORD, AxisKey*>::iterator i = axis_keys.begin();
		i != axis_keys.end();
		++i
		) {
		delete i->second;
	}
	axis_keys.clear();
	
	for(
		std::map<regval, AxisData*>::iterator i = axis.begin();
		i != axis.end();
		++i
		) {
		delete[] i->second->name;
		delete i->second;
	}
	axis.clear();

	delete this;
}

__declspec(dllexport) ControlModule* getControlModuleObject() {
	KeyboardControlModule *kcm = new KeyboardControlModule();
	ControlModule *cm = kcm;
	return cm;
}