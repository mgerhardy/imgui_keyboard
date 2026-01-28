#include "imgui_keyboard.h"

namespace ImKeyboard {

#ifndef IMGUI_DISABLE

ImGuiKeyboardStyle::ImGuiKeyboardStyle() {
	KeyUnit = 34.0f;
	SectionGap = 15.0f;
	KeyRounding = 3.0f;
	KeyFaceRounding = 2.0f;
	KeyFaceBorderSize = 2.0f;
	KeyFaceOffset = ImVec2(4.0f, 3.0f);
	KeyLabelOffset = ImVec2(6.0f, 4.0f);
	BoardPadding = 5.0f;

	Colors[ImGuiKeyboardCol_KeyBackground] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);			  // Light gray
	Colors[ImGuiKeyboardCol_KeyBorder] = ImVec4(0.094f, 0.094f, 0.094f, 1.0f);		  // Dark gray
	Colors[ImGuiKeyboardCol_KeyFaceBorder] = ImVec4(0.757f, 0.757f, 0.757f, 1.0f);	  // Medium gray
	Colors[ImGuiKeyboardCol_KeyFace] = ImVec4(0.988f, 0.988f, 0.988f, 1.0f);		  // Near white
	Colors[ImGuiKeyboardCol_KeyLabel] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);			  // Dark text
	Colors[ImGuiKeyboardCol_KeyPressed] = ImVec4(1.0f, 0.0f, 0.0f, 0.5f);			  // Red
	Colors[ImGuiKeyboardCol_KeyHighlighted] = ImVec4(0.0f, 1.0f, 0.0f, 0.5f);		  // Green
	Colors[ImGuiKeyboardCol_KeyPressedHighlighted] = ImVec4(1.0f, 1.0f, 0.0f, 0.5f);  // Yellow
}

struct KeyLayoutData {
	float X, Y;			 // Position in key units (not pixels)
	float Width, Height; // Size in key units (1.0 = standard key)
	const char *Label;
	ImGuiKey Key;
};

struct KeyboardContext {
	ImVector<ImGuiKey> HighlightedKeys;
	ImGuiKeyboardStyle Style;

	KeyboardContext() {
	}
};

static KeyboardContext *GetContext() {
	static KeyboardContext ctx;
	return &ctx;
}

ImGuiKeyboardStyle &GetStyle() {
	return GetContext()->Style;
}

static ImU32 GetColorU32(ImGuiKeyboardCol idx) {
	return ImGui::ColorConvertFloat4ToU32(GetContext()->Style.Colors[idx]);
}

static bool IsKeyHighlighted(ImGuiKey key) {
	KeyboardContext *ctx = GetContext();
	for (int i = 0; i < ctx->HighlightedKeys.Size; i++) {
		if (ctx->HighlightedKeys[i] == key) {
			return true;
		}
	}
	return false;
}

// Numeric keypad layout
static const KeyLayoutData numpad_keys[] = {
	// Row 0 - NumLock, /, *, -
	{0.0f, 0.0f, 1.0f, 1.0f, "Num", ImGuiKey_NumLock},
	{1.0f, 0.0f, 1.0f, 1.0f, "/", ImGuiKey_KeypadDivide},
	{2.0f, 0.0f, 1.0f, 1.0f, "*", ImGuiKey_KeypadMultiply},
	{3.0f, 0.0f, 1.0f, 1.0f, "-", ImGuiKey_KeypadSubtract},
	// Row 1 - 7, 8, 9, + (+ spans 2 rows)
	{0.0f, 1.0f, 1.0f, 1.0f, "7", ImGuiKey_Keypad7},
	{1.0f, 1.0f, 1.0f, 1.0f, "8", ImGuiKey_Keypad8},
	{2.0f, 1.0f, 1.0f, 1.0f, "9", ImGuiKey_Keypad9},
	{3.0f, 1.0f, 1.0f, 2.0f, "+", ImGuiKey_KeypadAdd},
	// Row 2 - 4, 5, 6
	{0.0f, 2.0f, 1.0f, 1.0f, "4", ImGuiKey_Keypad4},
	{1.0f, 2.0f, 1.0f, 1.0f, "5", ImGuiKey_Keypad5},
	{2.0f, 2.0f, 1.0f, 1.0f, "6", ImGuiKey_Keypad6},
	// Row 3 - 1, 2, 3, Enter (Enter spans 2 rows)
	{0.0f, 3.0f, 1.0f, 1.0f, "1", ImGuiKey_Keypad1},
	{1.0f, 3.0f, 1.0f, 1.0f, "2", ImGuiKey_Keypad2},
	{2.0f, 3.0f, 1.0f, 1.0f, "3", ImGuiKey_Keypad3},
	{3.0f, 3.0f, 1.0f, 2.0f, "Ent", ImGuiKey_KeypadEnter},
	// Row 4 - 0 (spans 2 columns), .
	{0.0f, 4.0f, 2.0f, 1.0f, "0", ImGuiKey_Keypad0},
	{2.0f, 4.0f, 1.0f, 1.0f, ".", ImGuiKey_KeypadDecimal},
};

// Function key row (F1-F12 + Esc, Print, etc.)
static const KeyLayoutData function_row_keys[] = {
	{0.0f, 0.0f, 1.0f, 1.0f, "Esc", ImGuiKey_Escape},
	// Gap
	{2.0f, 0.0f, 1.0f, 1.0f, "F1", ImGuiKey_F1},
	{3.0f, 0.0f, 1.0f, 1.0f, "F2", ImGuiKey_F2},
	{4.0f, 0.0f, 1.0f, 1.0f, "F3", ImGuiKey_F3},
	{5.0f, 0.0f, 1.0f, 1.0f, "F4", ImGuiKey_F4},
	// Gap
	{6.5f, 0.0f, 1.0f, 1.0f, "F5", ImGuiKey_F5},
	{7.5f, 0.0f, 1.0f, 1.0f, "F6", ImGuiKey_F6},
	{8.5f, 0.0f, 1.0f, 1.0f, "F7", ImGuiKey_F7},
	{9.5f, 0.0f, 1.0f, 1.0f, "F8", ImGuiKey_F8},
	// Gap
	{11.0f, 0.0f, 1.0f, 1.0f, "F9", ImGuiKey_F9},
	{12.0f, 0.0f, 1.0f, 1.0f, "F10", ImGuiKey_F10},
	{13.0f, 0.0f, 1.0f, 1.0f, "F11", ImGuiKey_F11},
	{14.0f, 0.0f, 1.0f, 1.0f, "F12", ImGuiKey_F12},
	// Print, Scroll, Pause
	{15.25f, 0.0f, 1.0f, 1.0f, "Prt", ImGuiKey_PrintScreen},
	{16.25f, 0.0f, 1.0f, 1.0f, "Scr", ImGuiKey_ScrollLock},
	{17.25f, 0.0f, 1.0f, 1.0f, "Pse", ImGuiKey_Pause},
};

// Navigation cluster (Insert, Delete, Home, End, PageUp, PageDown, Arrows)
static const KeyLayoutData nav_cluster_keys[] = {
	// Row 0 - Insert, Home, PageUp
	{0.0f, 0.0f, 1.0f, 1.0f, "Ins", ImGuiKey_Insert},
	{1.0f, 0.0f, 1.0f, 1.0f, "Hm", ImGuiKey_Home},
	{2.0f, 0.0f, 1.0f, 1.0f, "PgU", ImGuiKey_PageUp},
	// Row 1 - Delete, End, PageDown
	{0.0f, 1.0f, 1.0f, 1.0f, "Del", ImGuiKey_Delete},
	{1.0f, 1.0f, 1.0f, 1.0f, "End", ImGuiKey_End},
	{2.0f, 1.0f, 1.0f, 1.0f, "PgD", ImGuiKey_PageDown},
	// Row 3 - Arrow Up (centered) - aligned with bottom modifier row
	{1.0f, 3.0f, 1.0f, 1.0f, "^", ImGuiKey_UpArrow},
	// Row 4 - Arrow Left, Down, Right
	{0.0f, 4.0f, 1.0f, 1.0f, "<", ImGuiKey_LeftArrow},
	{1.0f, 4.0f, 1.0f, 1.0f, "v", ImGuiKey_DownArrow},
	{2.0f, 4.0f, 1.0f, 1.0f, ">", ImGuiKey_RightArrow},
};

// Main keyboard - Number row
static const KeyLayoutData number_row_keys[] = {
	{0.0f, 0.0f, 1.0f, 1.0f, "`", ImGuiKey_GraveAccent}, {1.0f, 0.0f, 1.0f, 1.0f, "1", ImGuiKey_1},
	{2.0f, 0.0f, 1.0f, 1.0f, "2", ImGuiKey_2},			 {3.0f, 0.0f, 1.0f, 1.0f, "3", ImGuiKey_3},
	{4.0f, 0.0f, 1.0f, 1.0f, "4", ImGuiKey_4},			 {5.0f, 0.0f, 1.0f, 1.0f, "5", ImGuiKey_5},
	{6.0f, 0.0f, 1.0f, 1.0f, "6", ImGuiKey_6},			 {7.0f, 0.0f, 1.0f, 1.0f, "7", ImGuiKey_7},
	{8.0f, 0.0f, 1.0f, 1.0f, "8", ImGuiKey_8},			 {9.0f, 0.0f, 1.0f, 1.0f, "9", ImGuiKey_9},
	{10.0f, 0.0f, 1.0f, 1.0f, "0", ImGuiKey_0},			 {11.0f, 0.0f, 1.0f, 1.0f, "-", ImGuiKey_Minus},
	{12.0f, 0.0f, 1.0f, 1.0f, "=", ImGuiKey_Equal},		 {13.0f, 0.0f, 2.0f, 1.0f, "Back", ImGuiKey_Backspace},
};

// QWERTY letter rows
static const KeyLayoutData qwerty_row1_keys[] = {
	{0.0f, 0.0f, 1.5f, 1.0f, "Tab", ImGuiKey_Tab},
	{1.5f, 0.0f, 1.0f, 1.0f, "Q", ImGuiKey_Q},
	{2.5f, 0.0f, 1.0f, 1.0f, "W", ImGuiKey_W},
	{3.5f, 0.0f, 1.0f, 1.0f, "E", ImGuiKey_E},
	{4.5f, 0.0f, 1.0f, 1.0f, "R", ImGuiKey_R},
	{5.5f, 0.0f, 1.0f, 1.0f, "T", ImGuiKey_T},
	{6.5f, 0.0f, 1.0f, 1.0f, "Y", ImGuiKey_Y},
	{7.5f, 0.0f, 1.0f, 1.0f, "U", ImGuiKey_U},
	{8.5f, 0.0f, 1.0f, 1.0f, "I", ImGuiKey_I},
	{9.5f, 0.0f, 1.0f, 1.0f, "O", ImGuiKey_O},
	{10.5f, 0.0f, 1.0f, 1.0f, "P", ImGuiKey_P},
	{11.5f, 0.0f, 1.0f, 1.0f, "[", ImGuiKey_LeftBracket},
	{12.5f, 0.0f, 1.0f, 1.0f, "]", ImGuiKey_RightBracket},
	{13.5f, 0.0f, 1.5f, 1.0f, "\\", ImGuiKey_Backslash},
};

static const KeyLayoutData qwerty_row2_keys[] = {
	{0.0f, 0.0f, 1.75f, 1.0f, "Caps", ImGuiKey_CapsLock}, {1.75f, 0.0f, 1.0f, 1.0f, "A", ImGuiKey_A},
	{2.75f, 0.0f, 1.0f, 1.0f, "S", ImGuiKey_S},			  {3.75f, 0.0f, 1.0f, 1.0f, "D", ImGuiKey_D},
	{4.75f, 0.0f, 1.0f, 1.0f, "F", ImGuiKey_F},			  {5.75f, 0.0f, 1.0f, 1.0f, "G", ImGuiKey_G},
	{6.75f, 0.0f, 1.0f, 1.0f, "H", ImGuiKey_H},			  {7.75f, 0.0f, 1.0f, 1.0f, "J", ImGuiKey_J},
	{8.75f, 0.0f, 1.0f, 1.0f, "K", ImGuiKey_K},			  {9.75f, 0.0f, 1.0f, 1.0f, "L", ImGuiKey_L},
	{10.75f, 0.0f, 1.0f, 1.0f, ";", ImGuiKey_Semicolon},  {11.75f, 0.0f, 1.0f, 1.0f, "'", ImGuiKey_Apostrophe},
	{12.75f, 0.0f, 2.25f, 1.0f, "Enter", ImGuiKey_Enter},
};

static const KeyLayoutData qwerty_row3_keys[] = {
	{0.0f, 0.0f, 2.25f, 1.0f, "Shift", ImGuiKey_LeftShift},
	{2.25f, 0.0f, 1.0f, 1.0f, "Z", ImGuiKey_Z},
	{3.25f, 0.0f, 1.0f, 1.0f, "X", ImGuiKey_X},
	{4.25f, 0.0f, 1.0f, 1.0f, "C", ImGuiKey_C},
	{5.25f, 0.0f, 1.0f, 1.0f, "V", ImGuiKey_V},
	{6.25f, 0.0f, 1.0f, 1.0f, "B", ImGuiKey_B},
	{7.25f, 0.0f, 1.0f, 1.0f, "N", ImGuiKey_N},
	{8.25f, 0.0f, 1.0f, 1.0f, "M", ImGuiKey_M},
	{9.25f, 0.0f, 1.0f, 1.0f, ",", ImGuiKey_Comma},
	{10.25f, 0.0f, 1.0f, 1.0f, ".", ImGuiKey_Period},
	{11.25f, 0.0f, 1.0f, 1.0f, "/", ImGuiKey_Slash},
	{12.25f, 0.0f, 2.75f, 1.0f, "Shift", ImGuiKey_RightShift},
};

// QWERTZ letter rows (German layout - Y and Z swapped)
static const KeyLayoutData qwertz_row1_keys[] = {
	{0.0f, 0.0f, 1.5f, 1.0f, "Tab", ImGuiKey_Tab},
	{1.5f, 0.0f, 1.0f, 1.0f, "Q", ImGuiKey_Q},
	{2.5f, 0.0f, 1.0f, 1.0f, "W", ImGuiKey_W},
	{3.5f, 0.0f, 1.0f, 1.0f, "E", ImGuiKey_E},
	{4.5f, 0.0f, 1.0f, 1.0f, "R", ImGuiKey_R},
	{5.5f, 0.0f, 1.0f, 1.0f, "T", ImGuiKey_T},
	{6.5f, 0.0f, 1.0f, 1.0f, "Z", ImGuiKey_Z},
	{7.5f, 0.0f, 1.0f, 1.0f, "U", ImGuiKey_U},
	{8.5f, 0.0f, 1.0f, 1.0f, "I", ImGuiKey_I},
	{9.5f, 0.0f, 1.0f, 1.0f, "O", ImGuiKey_O},
	{10.5f, 0.0f, 1.0f, 1.0f, "P", ImGuiKey_P},
	{11.5f, 0.0f, 1.0f, 1.0f, "U:", ImGuiKey_LeftBracket},
	{12.5f, 0.0f, 1.0f, 1.0f, "+", ImGuiKey_RightBracket},
	{13.5f, 0.0f, 1.5f, 1.0f, "#", ImGuiKey_Backslash},
};

static const KeyLayoutData qwertz_row2_keys[] = {
	{0.0f, 0.0f, 1.75f, 1.0f, "Caps", ImGuiKey_CapsLock}, {1.75f, 0.0f, 1.0f, 1.0f, "A", ImGuiKey_A},
	{2.75f, 0.0f, 1.0f, 1.0f, "S", ImGuiKey_S},			  {3.75f, 0.0f, 1.0f, 1.0f, "D", ImGuiKey_D},
	{4.75f, 0.0f, 1.0f, 1.0f, "F", ImGuiKey_F},			  {5.75f, 0.0f, 1.0f, 1.0f, "G", ImGuiKey_G},
	{6.75f, 0.0f, 1.0f, 1.0f, "H", ImGuiKey_H},			  {7.75f, 0.0f, 1.0f, 1.0f, "J", ImGuiKey_J},
	{8.75f, 0.0f, 1.0f, 1.0f, "K", ImGuiKey_K},			  {9.75f, 0.0f, 1.0f, 1.0f, "L", ImGuiKey_L},
	{10.75f, 0.0f, 1.0f, 1.0f, "O:", ImGuiKey_Semicolon}, {11.75f, 0.0f, 1.0f, 1.0f, "A:", ImGuiKey_Apostrophe},
	{12.75f, 0.0f, 2.25f, 1.0f, "Enter", ImGuiKey_Enter},
};

static const KeyLayoutData qwertz_row3_keys[] = {
	{0.0f, 0.0f, 2.25f, 1.0f, "Shift", ImGuiKey_LeftShift},
	{2.25f, 0.0f, 1.0f, 1.0f, "Y", ImGuiKey_Y},
	{3.25f, 0.0f, 1.0f, 1.0f, "X", ImGuiKey_X},
	{4.25f, 0.0f, 1.0f, 1.0f, "C", ImGuiKey_C},
	{5.25f, 0.0f, 1.0f, 1.0f, "V", ImGuiKey_V},
	{6.25f, 0.0f, 1.0f, 1.0f, "B", ImGuiKey_B},
	{7.25f, 0.0f, 1.0f, 1.0f, "N", ImGuiKey_N},
	{8.25f, 0.0f, 1.0f, 1.0f, "M", ImGuiKey_M},
	{9.25f, 0.0f, 1.0f, 1.0f, ",", ImGuiKey_Comma},
	{10.25f, 0.0f, 1.0f, 1.0f, ".", ImGuiKey_Period},
	{11.25f, 0.0f, 1.0f, 1.0f, "-", ImGuiKey_Slash},
	{12.25f, 0.0f, 2.75f, 1.0f, "Shift", ImGuiKey_RightShift},
};

// AZERTY letter rows (French layout)
static const KeyLayoutData azerty_row1_keys[] = {
	{0.0f, 0.0f, 1.5f, 1.0f, "Tab", ImGuiKey_Tab},
	{1.5f, 0.0f, 1.0f, 1.0f, "A", ImGuiKey_A},
	{2.5f, 0.0f, 1.0f, 1.0f, "Z", ImGuiKey_Z},
	{3.5f, 0.0f, 1.0f, 1.0f, "E", ImGuiKey_E},
	{4.5f, 0.0f, 1.0f, 1.0f, "R", ImGuiKey_R},
	{5.5f, 0.0f, 1.0f, 1.0f, "T", ImGuiKey_T},
	{6.5f, 0.0f, 1.0f, 1.0f, "Y", ImGuiKey_Y},
	{7.5f, 0.0f, 1.0f, 1.0f, "U", ImGuiKey_U},
	{8.5f, 0.0f, 1.0f, 1.0f, "I", ImGuiKey_I},
	{9.5f, 0.0f, 1.0f, 1.0f, "O", ImGuiKey_O},
	{10.5f, 0.0f, 1.0f, 1.0f, "P", ImGuiKey_P},
	{11.5f, 0.0f, 1.0f, 1.0f, "^", ImGuiKey_LeftBracket},
	{12.5f, 0.0f, 1.0f, 1.0f, "$", ImGuiKey_RightBracket},
	{13.5f, 0.0f, 1.5f, 1.0f, "*", ImGuiKey_Backslash},
};

static const KeyLayoutData azerty_row2_keys[] = {
	{0.0f, 0.0f, 1.75f, 1.0f, "Caps", ImGuiKey_CapsLock}, {1.75f, 0.0f, 1.0f, 1.0f, "Q", ImGuiKey_Q},
	{2.75f, 0.0f, 1.0f, 1.0f, "S", ImGuiKey_S},			  {3.75f, 0.0f, 1.0f, 1.0f, "D", ImGuiKey_D},
	{4.75f, 0.0f, 1.0f, 1.0f, "F", ImGuiKey_F},			  {5.75f, 0.0f, 1.0f, 1.0f, "G", ImGuiKey_G},
	{6.75f, 0.0f, 1.0f, 1.0f, "H", ImGuiKey_H},			  {7.75f, 0.0f, 1.0f, 1.0f, "J", ImGuiKey_J},
	{8.75f, 0.0f, 1.0f, 1.0f, "K", ImGuiKey_K},			  {9.75f, 0.0f, 1.0f, 1.0f, "L", ImGuiKey_L},
	{10.75f, 0.0f, 1.0f, 1.0f, "M", ImGuiKey_M},		  {11.75f, 0.0f, 1.0f, 1.0f, "u`", ImGuiKey_Apostrophe},
	{12.75f, 0.0f, 2.25f, 1.0f, "Enter", ImGuiKey_Enter},
};

static const KeyLayoutData azerty_row3_keys[] = {
	{0.0f, 0.0f, 2.25f, 1.0f, "Shift", ImGuiKey_LeftShift},
	{2.25f, 0.0f, 1.0f, 1.0f, "W", ImGuiKey_W},
	{3.25f, 0.0f, 1.0f, 1.0f, "X", ImGuiKey_X},
	{4.25f, 0.0f, 1.0f, 1.0f, "C", ImGuiKey_C},
	{5.25f, 0.0f, 1.0f, 1.0f, "V", ImGuiKey_V},
	{6.25f, 0.0f, 1.0f, 1.0f, "B", ImGuiKey_B},
	{7.25f, 0.0f, 1.0f, 1.0f, "N", ImGuiKey_N},
	{8.25f, 0.0f, 1.0f, 1.0f, ",", ImGuiKey_Comma},
	{9.25f, 0.0f, 1.0f, 1.0f, ";", ImGuiKey_Semicolon},
	{10.25f, 0.0f, 1.0f, 1.0f, ":", ImGuiKey_Period},
	{11.25f, 0.0f, 1.0f, 1.0f, "!", ImGuiKey_Slash},
	{12.25f, 0.0f, 2.75f, 1.0f, "Shift", ImGuiKey_RightShift},
};

// Colemak letter rows
static const KeyLayoutData colemak_row1_keys[] = {
	{0.0f, 0.0f, 1.5f, 1.0f, "Tab", ImGuiKey_Tab},
	{1.5f, 0.0f, 1.0f, 1.0f, "Q", ImGuiKey_Q},
	{2.5f, 0.0f, 1.0f, 1.0f, "W", ImGuiKey_W},
	{3.5f, 0.0f, 1.0f, 1.0f, "F", ImGuiKey_F},
	{4.5f, 0.0f, 1.0f, 1.0f, "P", ImGuiKey_P},
	{5.5f, 0.0f, 1.0f, 1.0f, "G", ImGuiKey_G},
	{6.5f, 0.0f, 1.0f, 1.0f, "J", ImGuiKey_J},
	{7.5f, 0.0f, 1.0f, 1.0f, "L", ImGuiKey_L},
	{8.5f, 0.0f, 1.0f, 1.0f, "U", ImGuiKey_U},
	{9.5f, 0.0f, 1.0f, 1.0f, "Y", ImGuiKey_Y},
	{10.5f, 0.0f, 1.0f, 1.0f, ";", ImGuiKey_Semicolon},
	{11.5f, 0.0f, 1.0f, 1.0f, "[", ImGuiKey_LeftBracket},
	{12.5f, 0.0f, 1.0f, 1.0f, "]", ImGuiKey_RightBracket},
	{13.5f, 0.0f, 1.5f, 1.0f, "\\", ImGuiKey_Backslash},
};

static const KeyLayoutData colemak_row2_keys[] = {
	{0.0f, 0.0f, 1.75f, 1.0f, "Bksp", ImGuiKey_Backspace},
	{1.75f, 0.0f, 1.0f, 1.0f, "A", ImGuiKey_A},
	{2.75f, 0.0f, 1.0f, 1.0f, "R", ImGuiKey_R},
	{3.75f, 0.0f, 1.0f, 1.0f, "S", ImGuiKey_S},
	{4.75f, 0.0f, 1.0f, 1.0f, "T", ImGuiKey_T},
	{5.75f, 0.0f, 1.0f, 1.0f, "D", ImGuiKey_D},
	{6.75f, 0.0f, 1.0f, 1.0f, "H", ImGuiKey_H},
	{7.75f, 0.0f, 1.0f, 1.0f, "N", ImGuiKey_N},
	{8.75f, 0.0f, 1.0f, 1.0f, "E", ImGuiKey_E},
	{9.75f, 0.0f, 1.0f, 1.0f, "I", ImGuiKey_I},
	{10.75f, 0.0f, 1.0f, 1.0f, "O", ImGuiKey_O},
	{11.75f, 0.0f, 1.0f, 1.0f, "'", ImGuiKey_Apostrophe},
	{12.75f, 0.0f, 2.25f, 1.0f, "Enter", ImGuiKey_Enter},
};

static const KeyLayoutData colemak_row3_keys[] = {
	{0.0f, 0.0f, 2.25f, 1.0f, "Shift", ImGuiKey_LeftShift},
	{2.25f, 0.0f, 1.0f, 1.0f, "Z", ImGuiKey_Z},
	{3.25f, 0.0f, 1.0f, 1.0f, "X", ImGuiKey_X},
	{4.25f, 0.0f, 1.0f, 1.0f, "C", ImGuiKey_C},
	{5.25f, 0.0f, 1.0f, 1.0f, "V", ImGuiKey_V},
	{6.25f, 0.0f, 1.0f, 1.0f, "B", ImGuiKey_B},
	{7.25f, 0.0f, 1.0f, 1.0f, "K", ImGuiKey_K},
	{8.25f, 0.0f, 1.0f, 1.0f, "M", ImGuiKey_M},
	{9.25f, 0.0f, 1.0f, 1.0f, ",", ImGuiKey_Comma},
	{10.25f, 0.0f, 1.0f, 1.0f, ".", ImGuiKey_Period},
	{11.25f, 0.0f, 1.0f, 1.0f, "/", ImGuiKey_Slash},
	{12.25f, 0.0f, 2.75f, 1.0f, "Shift", ImGuiKey_RightShift},
};

// Dvorak letter rows
static const KeyLayoutData dvorak_row1_keys[] = {
	{0.0f, 0.0f, 1.5f, 1.0f, "Tab", ImGuiKey_Tab},	{1.5f, 0.0f, 1.0f, 1.0f, "'", ImGuiKey_Apostrophe},
	{2.5f, 0.0f, 1.0f, 1.0f, ",", ImGuiKey_Comma},	{3.5f, 0.0f, 1.0f, 1.0f, ".", ImGuiKey_Period},
	{4.5f, 0.0f, 1.0f, 1.0f, "P", ImGuiKey_P},		{5.5f, 0.0f, 1.0f, 1.0f, "Y", ImGuiKey_Y},
	{6.5f, 0.0f, 1.0f, 1.0f, "F", ImGuiKey_F},		{7.5f, 0.0f, 1.0f, 1.0f, "G", ImGuiKey_G},
	{8.5f, 0.0f, 1.0f, 1.0f, "C", ImGuiKey_C},		{9.5f, 0.0f, 1.0f, 1.0f, "R", ImGuiKey_R},
	{10.5f, 0.0f, 1.0f, 1.0f, "L", ImGuiKey_L},		{11.5f, 0.0f, 1.0f, 1.0f, "/", ImGuiKey_Slash},
	{12.5f, 0.0f, 1.0f, 1.0f, "=", ImGuiKey_Equal}, {13.5f, 0.0f, 1.5f, 1.0f, "\\", ImGuiKey_Backslash},
};

static const KeyLayoutData dvorak_row2_keys[] = {
	{0.0f, 0.0f, 1.75f, 1.0f, "Caps", ImGuiKey_CapsLock}, {1.75f, 0.0f, 1.0f, 1.0f, "A", ImGuiKey_A},
	{2.75f, 0.0f, 1.0f, 1.0f, "O", ImGuiKey_O},			  {3.75f, 0.0f, 1.0f, 1.0f, "E", ImGuiKey_E},
	{4.75f, 0.0f, 1.0f, 1.0f, "U", ImGuiKey_U},			  {5.75f, 0.0f, 1.0f, 1.0f, "I", ImGuiKey_I},
	{6.75f, 0.0f, 1.0f, 1.0f, "D", ImGuiKey_D},			  {7.75f, 0.0f, 1.0f, 1.0f, "H", ImGuiKey_H},
	{8.75f, 0.0f, 1.0f, 1.0f, "T", ImGuiKey_T},			  {9.75f, 0.0f, 1.0f, 1.0f, "N", ImGuiKey_N},
	{10.75f, 0.0f, 1.0f, 1.0f, "S", ImGuiKey_S},		  {11.75f, 0.0f, 1.0f, 1.0f, "-", ImGuiKey_Minus},
	{12.75f, 0.0f, 2.25f, 1.0f, "Enter", ImGuiKey_Enter},
};

static const KeyLayoutData dvorak_row3_keys[] = {
	{0.0f, 0.0f, 2.25f, 1.0f, "Shift", ImGuiKey_LeftShift},
	{2.25f, 0.0f, 1.0f, 1.0f, ";", ImGuiKey_Semicolon},
	{3.25f, 0.0f, 1.0f, 1.0f, "Q", ImGuiKey_Q},
	{4.25f, 0.0f, 1.0f, 1.0f, "J", ImGuiKey_J},
	{5.25f, 0.0f, 1.0f, 1.0f, "K", ImGuiKey_K},
	{6.25f, 0.0f, 1.0f, 1.0f, "X", ImGuiKey_X},
	{7.25f, 0.0f, 1.0f, 1.0f, "B", ImGuiKey_B},
	{8.25f, 0.0f, 1.0f, 1.0f, "M", ImGuiKey_M},
	{9.25f, 0.0f, 1.0f, 1.0f, "W", ImGuiKey_W},
	{10.25f, 0.0f, 1.0f, 1.0f, "V", ImGuiKey_V},
	{11.25f, 0.0f, 1.0f, 1.0f, "Z", ImGuiKey_Z},
	{12.25f, 0.0f, 2.75f, 1.0f, "Shift", ImGuiKey_RightShift},
};

// Bottom row (modifiers + spacebar)
static const KeyLayoutData bottom_row_keys[] = {
	{0.0f, 0.0f, 1.25f, 1.0f, "Ctrl", ImGuiKey_LeftCtrl}, {1.25f, 0.0f, 1.25f, 1.0f, "Win", ImGuiKey_LeftSuper},
	{2.5f, 0.0f, 1.25f, 1.0f, "Alt", ImGuiKey_LeftAlt},	  {3.75f, 0.0f, 6.25f, 1.0f, "Space", ImGuiKey_Space},
	{10.0f, 0.0f, 1.25f, 1.0f, "Alt", ImGuiKey_RightAlt}, {11.25f, 0.0f, 1.25f, 1.0f, "Win", ImGuiKey_RightSuper},
	{12.5f, 0.0f, 1.25f, 1.0f, "Menu", ImGuiKey_Menu},	  {13.75f, 0.0f, 1.25f, 1.0f, "Ctrl", ImGuiKey_RightCtrl},
};

static void RenderKey(ImDrawList *draw_list, const ImVec2 &key_min, const ImVec2 &key_size, const char *label,
					  ImGuiKey key, float scale, ImGuiKeyboardFlags flags) {
	const ImGuiKeyboardStyle &style = GetStyle();
	const float key_rounding = style.KeyRounding * scale;
	const float key_face_rounding = style.KeyFaceRounding * scale;
	const ImVec2 key_face_pos(style.KeyFaceOffset.x * scale, style.KeyFaceOffset.y * scale);
	const ImVec2 key_label_pos(style.KeyLabelOffset.x * scale, style.KeyLabelOffset.y * scale);

	ImVec2 key_max = ImVec2(key_min.x + key_size.x, key_min.y + key_size.y);
	ImVec2 key_face_size = ImVec2(key_size.x - style.KeyFaceOffset.x * 2.0f * scale,
								  key_size.y - style.KeyFaceOffset.y * 2.0f * scale);

	// Key background
	draw_list->AddRectFilled(key_min, key_max, GetColorU32(ImGuiKeyboardCol_KeyBackground), key_rounding);
	draw_list->AddRect(key_min, key_max, GetColorU32(ImGuiKeyboardCol_KeyBorder), key_rounding);

	// Key face
	ImVec2 face_min = ImVec2(key_min.x + key_face_pos.x, key_min.y + key_face_pos.y);
	ImVec2 face_max = ImVec2(face_min.x + key_face_size.x, face_min.y + key_face_size.y);
	draw_list->AddRect(face_min, face_max, GetColorU32(ImGuiKeyboardCol_KeyFaceBorder), key_face_rounding,
					   ImDrawFlags_None, style.KeyFaceBorderSize);
	draw_list->AddRectFilled(face_min, face_max, GetColorU32(ImGuiKeyboardCol_KeyFace), key_face_rounding);

	// Label
	ImVec2 label_min = ImVec2(key_min.x + key_label_pos.x, key_min.y + key_label_pos.y);
	draw_list->AddText(label_min, GetColorU32(ImGuiKeyboardCol_KeyLabel), label);

	// Highlight if pressed (red) or explicitly highlighted (green)
	const bool isPressed = (flags & ImGuiKeyboardFlags_ShowPressed) && ImGui::IsKeyDown(key);
	const bool isHighlighted = IsKeyHighlighted(key);
	if (isPressed && isHighlighted) {
		draw_list->AddRectFilled(key_min, key_max, GetColorU32(ImGuiKeyboardCol_KeyPressedHighlighted), key_rounding);
	} else if (isPressed) {
		draw_list->AddRectFilled(key_min, key_max, GetColorU32(ImGuiKeyboardCol_KeyPressed), key_rounding);
	} else if (isHighlighted) {
		draw_list->AddRectFilled(key_min, key_max, GetColorU32(ImGuiKeyboardCol_KeyHighlighted), key_rounding);
	}
}

static void RenderKeyRow(ImDrawList *draw_list, const KeyLayoutData *keys, int key_count, const ImVec2 &start_pos,
						 float key_unit, float scale, ImGuiKeyboardFlags flags) {
	for (int i = 0; i < key_count; i++) {
		const KeyLayoutData *key = &keys[i];
		ImVec2 key_min = ImVec2(start_pos.x + key->X * key_unit, start_pos.y + key->Y * key_unit);
		ImVec2 key_size = ImVec2(key->Width * key_unit - 2.0f * scale, key->Height * key_unit - 2.0f * scale);
		RenderKey(draw_list, key_min, key_size, key->Label, key->Key, scale, flags);
	}
}

void Highlight(ImGuiKey key, bool highlight) {
	KeyboardContext *ctx = GetContext();
	if (highlight) {
		// Add key if not already present
		if (!IsKeyHighlighted(key)) {
			ctx->HighlightedKeys.push_back(key);
		}
	} else {
		// Remove key if present
		for (int i = 0; i < ctx->HighlightedKeys.Size; i++) {
			if (ctx->HighlightedKeys[i] == key) {
				ctx->HighlightedKeys.erase(&ctx->HighlightedKeys[i]);
				break;
			}
		}
	}
}

void ClearHighlights() {
	KeyboardContext *ctx = GetContext();
	ctx->HighlightedKeys.clear();
}

void Keyboard(ImGuiKeyboardLayout layout, ImGuiKeyboardFlags flags) {
	KeyboardContext *ctx = GetContext();
	const ImGuiKeyboardStyle &style = ctx->Style;
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	const float scale = ImGui::GetFontSize() / 13.0f;
	const float key_unit = style.KeyUnit * scale;
	const float section_gap = style.SectionGap * scale;
	const float board_padding = style.BoardPadding * scale;

	ImVec2 board_min = ImGui::GetCursorScreenPos();

	// Calculate board dimensions based on layout
	float board_width, board_height;
	if (layout == ImGuiKeyboardLayout_NumericPad) {
		// Numpad only: 4 keys wide, 5 rows
		board_width = 4.0f * key_unit + board_padding * 2.0f;
		board_height = 5.0f * key_unit + board_padding * 2.0f;
	} else {
		// Full keyboard: main section (15 keys) + nav cluster (3 keys) + numpad (4 keys) + gaps
		board_width = 15.0f * key_unit + section_gap + 3.0f * key_unit + section_gap + 4.0f * key_unit + board_padding * 2.0f;
		board_height = 6.5f * key_unit + board_padding * 2.0f; // Function row + gap + 5 main rows
	}

	ImVec2 board_max = ImVec2(board_min.x + board_width, board_min.y + board_height);
	ImVec2 start_pos = ImVec2(board_min.x + board_padding, board_min.y + board_padding);

	// Reserve space and check visibility
	ImGui::Dummy(ImVec2(board_width, board_height));
	if (!ImGui::IsItemVisible()) {
		return;
	}

	draw_list->PushClipRect(board_min, board_max, true);

	if (layout == ImGuiKeyboardLayout_NumericPad) {
		// Render only numpad
		RenderKeyRow(draw_list, numpad_keys, IM_ARRAYSIZE(numpad_keys), start_pos, key_unit, scale, flags);
	} else {
		// Full keyboard rendering

		// Function row (with gap below)
		ImVec2 func_row_pos = start_pos;
		RenderKeyRow(draw_list, function_row_keys, IM_ARRAYSIZE(function_row_keys), func_row_pos, key_unit, scale,
					 flags);

		// Main keyboard section (offset by function row + gap)
		float main_section_y = start_pos.y + key_unit + 0.5f * key_unit;

		// Number row
		ImVec2 num_row_pos = ImVec2(start_pos.x, main_section_y);
		RenderKeyRow(draw_list, number_row_keys, IM_ARRAYSIZE(number_row_keys), num_row_pos, key_unit, scale, flags);

		// Letter rows - select based on layout
		const KeyLayoutData *row1_keys;
		const KeyLayoutData *row2_keys;
		const KeyLayoutData *row3_keys;
		int row1_count, row2_count, row3_count;

		switch (layout) {
		case ImGuiKeyboardLayout_Qwertz:
			row1_keys = qwertz_row1_keys;
			row1_count = IM_ARRAYSIZE(qwertz_row1_keys);
			row2_keys = qwertz_row2_keys;
			row2_count = IM_ARRAYSIZE(qwertz_row2_keys);
			row3_keys = qwertz_row3_keys;
			row3_count = IM_ARRAYSIZE(qwertz_row3_keys);
			break;
		case ImGuiKeyboardLayout_Azerty:
			row1_keys = azerty_row1_keys;
			row1_count = IM_ARRAYSIZE(azerty_row1_keys);
			row2_keys = azerty_row2_keys;
			row2_count = IM_ARRAYSIZE(azerty_row2_keys);
			row3_keys = azerty_row3_keys;
			row3_count = IM_ARRAYSIZE(azerty_row3_keys);
			break;
		case ImGuiKeyboardLayout_Colemak:
			row1_keys = colemak_row1_keys;
			row1_count = IM_ARRAYSIZE(colemak_row1_keys);
			row2_keys = colemak_row2_keys;
			row2_count = IM_ARRAYSIZE(colemak_row2_keys);
			row3_keys = colemak_row3_keys;
			row3_count = IM_ARRAYSIZE(colemak_row3_keys);
			break;
		case ImGuiKeyboardLayout_Dvorak:
			row1_keys = dvorak_row1_keys;
			row1_count = IM_ARRAYSIZE(dvorak_row1_keys);
			row2_keys = dvorak_row2_keys;
			row2_count = IM_ARRAYSIZE(dvorak_row2_keys);
			row3_keys = dvorak_row3_keys;
			row3_count = IM_ARRAYSIZE(dvorak_row3_keys);
			break;
		default: // QWERTY
			row1_keys = qwerty_row1_keys;
			row1_count = IM_ARRAYSIZE(qwerty_row1_keys);
			row2_keys = qwerty_row2_keys;
			row2_count = IM_ARRAYSIZE(qwerty_row2_keys);
			row3_keys = qwerty_row3_keys;
			row3_count = IM_ARRAYSIZE(qwerty_row3_keys);
			break;
		}

		ImVec2 row1_pos = ImVec2(start_pos.x, main_section_y + key_unit);
		RenderKeyRow(draw_list, row1_keys, row1_count, row1_pos, key_unit, scale, flags);

		ImVec2 row2_pos = ImVec2(start_pos.x, main_section_y + 2.0f * key_unit);
		RenderKeyRow(draw_list, row2_keys, row2_count, row2_pos, key_unit, scale, flags);

		ImVec2 row3_pos = ImVec2(start_pos.x, main_section_y + 3.0f * key_unit);
		RenderKeyRow(draw_list, row3_keys, row3_count, row3_pos, key_unit, scale, flags);

		// Bottom row (modifiers + spacebar)
		ImVec2 bottom_row_pos = ImVec2(start_pos.x, main_section_y + 4.0f * key_unit);
		RenderKeyRow(draw_list, bottom_row_keys, IM_ARRAYSIZE(bottom_row_keys), bottom_row_pos, key_unit, scale, flags);

		// Navigation cluster (Insert/Delete/Home/End/PgUp/PgDn + arrows)
		float nav_x = start_pos.x + 15.0f * key_unit + section_gap;
		ImVec2 nav_pos = ImVec2(nav_x, main_section_y);
		RenderKeyRow(draw_list, nav_cluster_keys, IM_ARRAYSIZE(nav_cluster_keys), nav_pos, key_unit, scale, flags);

		// Numeric keypad
		float numpad_x = nav_x + 3.0f * key_unit + section_gap;
		ImVec2 numpad_pos = ImVec2(numpad_x, main_section_y);
		RenderKeyRow(draw_list, numpad_keys, IM_ARRAYSIZE(numpad_keys), numpad_pos, key_unit, scale, flags);
	}

	draw_list->PopClipRect();
}

#ifndef IMGUI_DISABLE_DEMO_WINDOWS
void KeyboardDemo() {
	static bool showPressed = true;
	static int currentLayout = ImGuiKeyboardLayout_Qwerty;
	static bool highlightWASD = false;
	static bool highlightArrows = false;
	static bool highlightNumpad = false;
	static bool showStyleEditor = false;

	ImGui::Text("Keyboard Widget Demo");
	ImGui::Separator();

	// Layout selection
	ImGui::Text("Layout:");
	ImGui::SameLine();
	const char *layoutNames[] = {"QWERTY", "QWERTZ", "AZERTY", "Colemak", "Dvorak", "Numeric Pad"};
	if (ImGui::BeginCombo("##Layout", layoutNames[currentLayout])) {
		for (int i = 0; i < ImGuiKeyboardLayout_Count; i++) {
			const bool isSelected = (currentLayout == i);
			if (ImGui::Selectable(layoutNames[i], isSelected)) {
				currentLayout = i;
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	// Flags
	ImGui::Text("Options:");
	ImGui::Checkbox("Show Pressed Keys (Red)", &showPressed);

	ImGui::Separator();
	ImGui::Text("Highlight Groups (Green):");

	// WASD highlight toggle
	if (ImGui::Checkbox("Highlight WASD", &highlightWASD)) {
		Highlight(ImGuiKey_W, highlightWASD);
		Highlight(ImGuiKey_A, highlightWASD);
		Highlight(ImGuiKey_S, highlightWASD);
		Highlight(ImGuiKey_D, highlightWASD);
	}

	// Arrow keys highlight toggle
	ImGui::SameLine();
	if (ImGui::Checkbox("Highlight Arrows", &highlightArrows)) {
		Highlight(ImGuiKey_UpArrow, highlightArrows);
		Highlight(ImGuiKey_DownArrow, highlightArrows);
		Highlight(ImGuiKey_LeftArrow, highlightArrows);
		Highlight(ImGuiKey_RightArrow, highlightArrows);
	}

	// Numpad highlight toggle
	ImGui::SameLine();
	if (ImGui::Checkbox("Highlight Numpad", &highlightNumpad)) {
		Highlight(ImGuiKey_Keypad0, highlightNumpad);
		Highlight(ImGuiKey_Keypad1, highlightNumpad);
		Highlight(ImGuiKey_Keypad2, highlightNumpad);
		Highlight(ImGuiKey_Keypad3, highlightNumpad);
		Highlight(ImGuiKey_Keypad4, highlightNumpad);
		Highlight(ImGuiKey_Keypad5, highlightNumpad);
		Highlight(ImGuiKey_Keypad6, highlightNumpad);
		Highlight(ImGuiKey_Keypad7, highlightNumpad);
		Highlight(ImGuiKey_Keypad8, highlightNumpad);
		Highlight(ImGuiKey_Keypad9, highlightNumpad);
	}

	// Individual key highlight
	ImGui::Text("Highlight Individual Key:");
	static int selectedKey = 0;
	static bool keyHighlighted = false;
	const char *keyNames[] = {"Space", "Enter", "Tab", "Escape", "Backspace", "Left Shift", "Left Ctrl", "Left Alt"};
	const ImGuiKey keyValues[] = {ImGuiKey_Space,	  ImGuiKey_Enter,	  ImGuiKey_Tab,		 ImGuiKey_Escape,
								  ImGuiKey_Backspace, ImGuiKey_LeftShift, ImGuiKey_LeftCtrl, ImGuiKey_LeftAlt};
	ImGui::SetNextItemWidth(150);
	if (ImGui::BeginCombo("##Key", keyNames[selectedKey])) {
		for (int i = 0; i < IM_ARRAYSIZE(keyNames); i++) {
			const bool isSelected = (selectedKey == i);
			if (ImGui::Selectable(keyNames[i], isSelected)) {
				// Remove highlight from previous key
				if (keyHighlighted) {
					Highlight(keyValues[selectedKey], false);
				}
				selectedKey = i;
				// Apply highlight to new key if checkbox is checked
				if (keyHighlighted) {
					Highlight(keyValues[selectedKey], true);
				}
			}
			if (isSelected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Highlight##IndividualKey", &keyHighlighted)) {
		Highlight(keyValues[selectedKey], keyHighlighted);
	}

	// Clear all highlights button
	if (ImGui::Button("Clear All Highlights")) {
		ClearHighlights();
		highlightWASD = false;
		highlightArrows = false;
		highlightNumpad = false;
		keyHighlighted = false;
	}

	ImGui::Separator();

	// Style editor toggle
	ImGui::Checkbox("Show Style Editor", &showStyleEditor);
	if (showStyleEditor) {
		ImGuiKeyboardStyle &style = GetStyle();

		if (ImGui::TreeNode("Sizes")) {
			ImGui::SliderFloat("Key Unit", &style.KeyUnit, 20.0f, 60.0f, "%.1f");
			ImGui::SliderFloat("Section Gap", &style.SectionGap, 0.0f, 30.0f, "%.1f");
			ImGui::SliderFloat("Key Rounding", &style.KeyRounding, 0.0f, 10.0f, "%.1f");
			ImGui::SliderFloat("Key Face Rounding", &style.KeyFaceRounding, 0.0f, 10.0f, "%.1f");
			ImGui::SliderFloat("Key Face Border", &style.KeyFaceBorderSize, 0.0f, 5.0f, "%.1f");
			ImGui::SliderFloat2("Key Face Offset", &style.KeyFaceOffset.x, 0.0f, 10.0f, "%.1f");
			ImGui::SliderFloat2("Key Label Offset", &style.KeyLabelOffset.x, 0.0f, 15.0f, "%.1f");
			ImGui::SliderFloat("Board Padding", &style.BoardPadding, 0.0f, 20.0f, "%.1f");

			if (ImGui::Button("Reset Sizes")) {
				ImGuiKeyboardStyle defaultStyle;
				style.KeyUnit = defaultStyle.KeyUnit;
				style.SectionGap = defaultStyle.SectionGap;
				style.KeyRounding = defaultStyle.KeyRounding;
				style.KeyFaceRounding = defaultStyle.KeyFaceRounding;
				style.KeyFaceBorderSize = defaultStyle.KeyFaceBorderSize;
				style.KeyFaceOffset = defaultStyle.KeyFaceOffset;
				style.KeyLabelOffset = defaultStyle.KeyLabelOffset;
				style.BoardPadding = defaultStyle.BoardPadding;
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Colors")) {
			const char *colorNames[] = {"Key Background", "Key Border",		 "Key Face Border",
										"Key Face",		  "Key Label",		 "Key Pressed",
										"Key Highlighted", "Key Pressed+Highlighted"};
			for (int i = 0; i < ImGuiKeyboardCol_COUNT; i++) {
				ImGui::ColorEdit4(colorNames[i], &style.Colors[i].x,
								  ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf);
			}

			if (ImGui::Button("Reset Colors")) {
				ImGuiKeyboardStyle defaultStyle;
				for (int i = 0; i < ImGuiKeyboardCol_COUNT; i++) {
					style.Colors[i] = defaultStyle.Colors[i];
				}
			}
			ImGui::TreePop();
		}
	}

	ImGui::Text("Legend: (Default) Red = Pressed | Green = Highlighted | Yellow = Both");
	ImGui::Separator();

	// Render the keyboard
	ImGuiKeyboardFlags flags = ImGuiKeyboardFlags_None;
	if (showPressed) {
		flags |= ImGuiKeyboardFlags_ShowPressed;
	}
	Keyboard((ImGuiKeyboardLayout)currentLayout, flags);
}
#endif // IMGUI_DISABLE_DEMO_WINDOWS
#endif // IMGUI_DISABLE

} // namespace ImKeyboard
