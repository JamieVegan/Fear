// for std::mbstowcs
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <algorithm>
#include <string>
#include <map>
#include <any>
#include <windows.h>

// Returning arrays from functions :)
#include <vector>

// File reading
#include <fstream>

// String to WString
#include <cstdio>

// PathFileExists
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

// For lower
#include <algorithm>
#include <cctype>

// For std::stoi
#include <stdlib.h>

// Regex...
#include <regex>

typedef std::string str;
typedef std::wstring wstr;

// Adjustable
const str Caret = ">";
const str NewLines = "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
const int TextSpeed = 8; // Sleep time in miliseconds for each character (This might be inaccurate by around 3 milliseconds)
const int DialogueSleep = 400;

// Technical
bool Running = true;
int CurrentScene = 0;

std::map<str, str> GlobalVariables = {};

wstr BasePath;

// Prints a string to cout.
// If Slow is true, prints each character letter by letter with TextSpeed miliseconds of delay.
// If Dialogue is true, sleeps a bit after the line of dialogue finishes.
void Print(str Text = "", bool Slow = false, bool Dialogue = false) {
	if (Slow) {
		for (int i = 0; i < Text.size(); i++) {
			std::cout << Text[i];
			Sleep(TextSpeed);
		}

		if (Dialogue) {
			Sleep((Text.size() * TextSpeed) + DialogueSleep);
		}
		std::cout << "\n";
		return;
	}

	std::cout << Text << "\n";
	return;
}

// Stolen from somewhere i forgor
str ToLower(str String) {
	std::transform(String.begin(), String.end(), String.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return String;
}

// Prints "> " and waits for the user to enter text.
// Stores the text in the global variable Input.
// Input automatically gets converted to lowercase
str GetInput(std::map<str, str> Options = {}) {
	str Input;
	std::cout << Caret;
	std::getline(std::cin, Input);
	Input = ToLower(Input);

	// Convert an option's number to its string variant
	// Example: 1 -> c (continue)
	if (Options[Input] != "") {
		Input = Options[Input];
	}

	std::cout << "\n";
	return Input;
}

// For debugging
void Wait() {
	str Temp;
	std::cout << Caret << "Press enter to continue";
	std::getline(std::cin, Temp);
	return;
}

// use StrToWString(String).c_str() instead :)
// we love wstrings!!!!!!!!!!
LPCWSTR StrToLPCWSTR(str String) {
}

std::wstring StrToWStr(str String) {
	// Convert string to wstring (https://stackoverflow.com/a/2573845)
	std::wstring WString(String.size(), L' ');
	WString.resize(std::mbstowcs(&WString[0], String.c_str(), String.size()));
	return WString;
}

// Creates file if it doesn't already exist with default text.
void FileCreator9000(str Path, str DefaultText) {
	std::wstring FullPath = BasePath + StrToWStr(Path);

	if (!PathFileExists(FullPath.c_str())) {
		HANDLE FileHandle = CreateFile(FullPath.c_str(), // name of the file
			GENERIC_WRITE, // open for writing
			0, // do not share
			NULL, // default security
			OPEN_ALWAYS, // create new file only
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL); // no attr. template

		WriteFile(FileHandle, DefaultText.c_str(), strlen(DefaultText.c_str()), NULL, NULL);
		CloseHandle(FileHandle);
	}
	return;
}

// Creates file if it doesn't already exist with default text.
void FolderCreator9000(str Path) {
	std::wstring FullPath = BasePath + StrToWStr(Path);

	// Create folder if it doesn't exist
	if (!PathFileExists(FullPath.c_str())) {
		CreateDirectory(FullPath.c_str(), NULL);
	}
	return;
}

str Lower(str String) {
	std::transform(String.begin(), String.end(), String.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return String;
}

// I think I made this one myself but I'm not sure.
std::vector<str> ParseArguments(str String, str Delimiter = " ") {
	std::vector<std::string> List;
	bool InString = false;
	int Start = 0;
	int RemoveAmount = 0;

	for (int i = 0; i < String.size(); i++) {
		if (String[i] == '\"') {
			InString = !InString;
		}
		else if (String[i] == ' ' && !InString) {
			List.push_back(String.substr(Start, i - RemoveAmount));
			Start = i + 1;
			RemoveAmount = Start;
		}
	}
	List.push_back(String.substr(Start, String.size()));

	return List;
}

// https://stackoverflow.com/a/6277674
bool VectorHas(std::vector<str> Vector, str String) {
	if (std::find(Vector.begin(), Vector.end(), String) != Vector.end()) {
		return true;
	}
	return false;
}

// https://stackoverflow.com/a/1798170
str Strip(str String, str whitespace = " ") {
	int StrBegin = String.find_first_not_of(whitespace);

	if (StrBegin == std::string::npos) {
		return "";
	}

	int StrEnd = String.find_last_not_of(whitespace);
	int StrRange = StrEnd - StrBegin + 1;

	return String.substr(StrBegin, StrRange);
}

str ParseVariables(str String, std::map<str, str> VariableMap) {
	for (auto const& [Key, Value] : VariableMap) {
		String = std::regex_replace(String, std::regex("\\$" + Key), Value);
	}
	return String;
}

// KanonScript interpreter
// "GameData/" is automatically appended to the start of the filename.
void RunKanonScript(str Filename) {
	wstr FullPath = BasePath + L"GameData\\" + StrToWStr(Filename);

	std::ifstream File(FullPath);

	// Check if the file is open
	if (!File.is_open()) {
		std::wcerr << L"Failed to open file: " + FullPath;
		wstr ErrorMessage = L"Failed to open file: " + FullPath;
		MessageBox(NULL, ErrorMessage.c_str(), L"Error", 0);
		return;
	}

	// ---------- Interpreter ----------
	str Input;

	int OptionCount = 0;
	int IfCount = 0;
	int LastOptionCall = 0; // Index
	bool FoundIf = false; // This variable name doesn't make sense
	bool ShouldExecute = true;
	std::map<str, str> Options;

	// We need to use getline and a loop for this otherwise it triggers windows defender
	// for reading the entire file at once (I think anyways)
	std::vector<str> Lines;
	str TempLine;
	while (std::getline(File, TempLine)) {
		Lines.push_back(TempLine);
	}

	for (int i = 0; i < Lines.size(); i++) {
		str Line = Lines[i];
		Line = Strip(Line);

		std::vector<str> Arguments = ParseArguments(Line);

		// Strip doublequotes from arguments
		for (str& Argument : Arguments) {
			Argument = Strip(Argument, "\"");
			// TODO: Add global variable adding, removing, and add variable if statements :sob::sob::sob::sob::sob:
			Argument = ParseVariables(Argument, GlobalVariables);
		}

		// Make the first argument lowercase
		// The first argument is always the function, such as "input" or "getinput"
		//Arguments[0] = Lower(Arguments[0]);
		str Function = Lower(Arguments[0]);

		if (ShouldExecute) {
			// Empty line
			if (Line == "") {
				continue;
			}

			// Comment (// or #)
			else if (Line.starts_with("//") || Line.starts_with("#")) {
				continue;
			}

			// Print
			else if (Line.starts_with("\"") && Line.ends_with("\"")) {
				Line = ParseVariables(Line, GlobalVariables);
				Print(Line.substr(1, Line.size() - 2), true, true);
			}

			// Input
			else if (Function == "input") {
				OptionCount = 0;
				IfCount = 0;
				LastOptionCall = i - 2;
				std::cout << "\n";
			}

			// Option
			else if (Function.starts_with("option")) {
				OptionCount++;
				Print(std::to_string(OptionCount) + ". " + Arguments[1], true);
				Options[std::to_string(OptionCount)] = Arguments[2];
			}

			// GetInput
			else if (Function.starts_with("getinput")) {
				Input = GetInput(Options);
				Options.clear(); // Clear for memory
				FoundIf = false;
			}

			// If statement
			else if (Line.ends_with("{")) {
				IfCount++;
				str IfArg = Function;

				// Ensure if doesn't end with {
				if (IfArg.ends_with("{")) {
					IfArg = IfArg.substr(0, IfArg.size() - 1);
				}

				//std::cout << "OptionCount: " << OptionCount << ", IfCount: " << IfCount << "\n";

				if (Input.starts_with(IfArg) && !FoundIf) {
					FoundIf = true;
				}
				else {
					if (IfCount >= OptionCount && !FoundIf) {
						Print("----- Invalid option -----", true);
						i = LastOptionCall;
						continue;
					}
					ShouldExecute = false;
				}
			}

			// System
			else if (Function == "system") {
				system(Arguments[1].c_str());
			}

			// RunScript
			else if (Function == "runscript") {
				RunKanonScript(Arguments[1]);
			}

			// Goto
			else if (Function == "goto") {
				if (isdigit(Arguments[1][0])) {
					i = std::stoi(Arguments[1]) - 2; // Not sure why this needs to be -2 to match up
				}
				else {
					// TODO: Make labels work
				}
			}

			// Return
			else if (Function == "return") {
				return;
			}

			// }
			else if (Line == "}") {
				// This is to avoid it thinking it's unrecognized vvv
				continue;
			}

			// Unrecognized
			else {
				Print("Unrecognized: " + Line);
			}
		}
		else {
			// }
			if (Line.contains("}")) {
				ShouldExecute = true;
				continue;
			}
		}
	}
	
	File.close();
}

int main()
{
	// Get executable path
	TCHAR Buffer[MAX_PATH];
	GetModuleFileName(NULL, Buffer, MAX_PATH);
	std::wstring Filename(Buffer);

	// Remove Fear.exe from the end of path
	// "C:\Path\Fear.exe"
	// to
	// "C:\Path\"
	BasePath = Filename.substr(0, Filename.find_last_of(L"\\") + 1);

	FolderCreator9000("GameData");
	FolderCreator9000("UserData");

	RunKanonScript("NameTest.ks");

	Print("--------- End ---------", true);
	GetInput();

	while (Running) {
		Sleep(TextSpeed);
		Print(NewLines);
		Sleep(TextSpeed);
	}
}
