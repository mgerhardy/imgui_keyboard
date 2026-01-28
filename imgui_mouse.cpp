#include "imgui_mouse.h"

namespace ImMouse {

#ifndef IMGUI_DISABLE

ImGuiMouseStyle::ImGuiMouseStyle() {
	Scale = 1.0f;
	BodyWidth = 60.0f;
	BodyHeight = 100.0f;
	BodyRounding = 30.0f;
	ButtonHeight = 35.0f;
	ButtonGap = 2.0f;
	WheelWidth = 10.0f;
	WheelHeight = 20.0f;

	Colors[ImGuiMouseCol_Background] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	Colors[ImGuiMouseCol_Border] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
	Colors[ImGuiMouseCol_ButtonBackground] = ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
	Colors[ImGuiMouseCol_ButtonBorder] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	Colors[ImGuiMouseCol_ButtonPressed] = ImVec4(1.0f, 0.0f, 0.0f, 0.5f);
	Colors[ImGuiMouseCol_ButtonHighlighted] = ImVec4(0.0f, 1.0f, 0.0f, 0.5f);
	Colors[ImGuiMouseCol_WheelBackground] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	Colors[ImGuiMouseCol_WheelForeground] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
}

struct MouseContext {
	ImVector<int> HighlightedButtons;
	ImGuiMouseStyle Style;
};

static MouseContext *GetContext() {
	static MouseContext ctx;
	return &ctx;
}

ImGuiMouseStyle &GetStyle() {
	return GetContext()->Style;
}

static ImU32 GetColorU32(ImGuiMouseCol idx) {
	const ImGuiMouseStyle &style = GetStyle();
	return ImGui::ColorConvertFloat4ToU32(style.Colors[idx]);
}

static bool IsButtonHighlighted(int button) {
	MouseContext *ctx = GetContext();
	for (int i = 0; i < ctx->HighlightedButtons.Size; i++) {
		if (ctx->HighlightedButtons[i] == button) {
			return true;
		}
	}
	return false;
}

void HighlightButton(int button, bool highlight) {
	MouseContext *ctx = GetContext();
	if (highlight) {
		if (!IsButtonHighlighted(button)) {
			ctx->HighlightedButtons.push_back(button);
		}
	} else {
		for (int i = 0; i < ctx->HighlightedButtons.Size; i++) {
			if (ctx->HighlightedButtons[i] == button) {
				ctx->HighlightedButtons.erase(&ctx->HighlightedButtons[i]);
				break;
			}
		}
	}
}

void ClearHighlights() {
	MouseContext *ctx = GetContext();
	ctx->HighlightedButtons.clear();
}

void Mouse(ImGuiMouseLayout layout, ImGuiMouseFlags flags) {
	const ImGuiMouseStyle &style = GetStyle();
	const float scale = style.Scale * (ImGui::GetFontSize() / 13.0f);

	const float body_width = style.BodyWidth * scale;
	const float body_height = style.BodyHeight * scale;
	const float body_rounding = style.BodyRounding * scale;
	const float button_height = style.ButtonHeight * scale;
	const float button_gap = style.ButtonGap * scale;
	const float wheel_width = style.WheelWidth * scale;
	const float wheel_height = style.WheelHeight * scale;

	ImVec2 canvas_size(body_width + 10.0f * scale, body_height + 10.0f * scale);
	ImVec2 canvas_pos = ImGui::GetCursorScreenPos();

	ImGui::Dummy(canvas_size);
	ImDrawList *draw_list = ImGui::GetWindowDrawList();
	draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true);

	ImVec2 body_pos(canvas_pos.x + 5.0f * scale, canvas_pos.y + 5.0f * scale);

	// Draw mouse body
	draw_list->AddRectFilled(body_pos, ImVec2(body_pos.x + body_width, body_pos.y + body_height),
							 GetColorU32(ImGuiMouseCol_Background), body_rounding);
	draw_list->AddRect(body_pos, ImVec2(body_pos.x + body_width, body_pos.y + body_height),
					   GetColorU32(ImGuiMouseCol_Border), body_rounding, 0, 2.0f * scale);

	// Calculate button layout
	const bool threeButton = (layout == ImGuiMouseLayout_ThreeButton);
	const bool showWheel = (flags & ImGuiMouseFlags_ShowWheel);
	const bool showPressed = (flags & ImGuiMouseFlags_ShowPressed);

	float left_button_width, right_button_width, middle_button_width = 0.0f;
	if (threeButton || showWheel) {
		// Three sections: left button, middle (wheel or button), right button
		left_button_width = (body_width - button_gap * 2.0f - wheel_width) / 2.0f;
		right_button_width = left_button_width;
		middle_button_width = wheel_width;
	} else {
		// Two sections: left button, right button
		left_button_width = (body_width - button_gap) / 2.0f;
		right_button_width = left_button_width;
	}

	// Left button
	ImVec2 left_btn_min(body_pos.x, body_pos.y);
	ImVec2 left_btn_max(body_pos.x + left_button_width, body_pos.y + button_height);
	draw_list->AddRectFilled(left_btn_min, left_btn_max, GetColorU32(ImGuiMouseCol_ButtonBackground), body_rounding,
							 ImDrawFlags_RoundCornersTopLeft);
	draw_list->AddRect(left_btn_min, left_btn_max, GetColorU32(ImGuiMouseCol_ButtonBorder), body_rounding,
					   ImDrawFlags_RoundCornersTopLeft);

	// Left button pressed/highlighted overlay
	bool leftPressed = showPressed && ImGui::IsMouseDown(ImGuiMouseButton_Left);
	bool leftHighlighted = IsButtonHighlighted(ImGuiMouseButton_Left);
	if (leftPressed) {
		draw_list->AddRectFilled(left_btn_min, left_btn_max, GetColorU32(ImGuiMouseCol_ButtonPressed), body_rounding,
								 ImDrawFlags_RoundCornersTopLeft);
	} else if (leftHighlighted) {
		draw_list->AddRectFilled(left_btn_min, left_btn_max, GetColorU32(ImGuiMouseCol_ButtonHighlighted), body_rounding,
								 ImDrawFlags_RoundCornersTopLeft);
	}

	// Right button
	float right_btn_x = body_pos.x + body_width - right_button_width;
	ImVec2 right_btn_min(right_btn_x, body_pos.y);
	ImVec2 right_btn_max(right_btn_x + right_button_width, body_pos.y + button_height);
	draw_list->AddRectFilled(right_btn_min, right_btn_max, GetColorU32(ImGuiMouseCol_ButtonBackground), body_rounding,
							 ImDrawFlags_RoundCornersTopRight);
	draw_list->AddRect(right_btn_min, right_btn_max, GetColorU32(ImGuiMouseCol_ButtonBorder), body_rounding,
					   ImDrawFlags_RoundCornersTopRight);

	// Right button pressed/highlighted overlay
	bool rightPressed = showPressed && ImGui::IsMouseDown(ImGuiMouseButton_Right);
	bool rightHighlighted = IsButtonHighlighted(ImGuiMouseButton_Right);
	if (rightPressed) {
		draw_list->AddRectFilled(right_btn_min, right_btn_max, GetColorU32(ImGuiMouseCol_ButtonPressed), body_rounding,
								 ImDrawFlags_RoundCornersTopRight);
	} else if (rightHighlighted) {
		draw_list->AddRectFilled(right_btn_min, right_btn_max, GetColorU32(ImGuiMouseCol_ButtonHighlighted),
								 body_rounding, ImDrawFlags_RoundCornersTopRight);
	}

	// Middle button or wheel
	if (threeButton || showWheel) {
		float middle_x = body_pos.x + left_button_width + button_gap;
		ImVec2 middle_min(middle_x, body_pos.y + (button_height - wheel_height) / 2.0f);
		ImVec2 middle_max(middle_x + middle_button_width, middle_min.y + wheel_height);

		if (showWheel) {
			// Draw scroll wheel
			draw_list->AddRectFilled(middle_min, middle_max, GetColorU32(ImGuiMouseCol_WheelBackground),
									 wheel_width * 0.3f);
			// Draw wheel notches
			float notch_spacing = wheel_height / 4.0f;
			for (int i = 1; i < 4; i++) {
				float y = middle_min.y + i * notch_spacing;
				draw_list->AddLine(ImVec2(middle_min.x + 2.0f * scale, y), ImVec2(middle_max.x - 2.0f * scale, y),
								   GetColorU32(ImGuiMouseCol_WheelForeground), 1.0f * scale);
			}
		}

		if (threeButton) {
			// Middle button pressed/highlighted overlay
			bool middlePressed = showPressed && ImGui::IsMouseDown(ImGuiMouseButton_Middle);
			bool middleHighlighted = IsButtonHighlighted(ImGuiMouseButton_Middle);
			if (middlePressed) {
				draw_list->AddRectFilled(middle_min, middle_max, GetColorU32(ImGuiMouseCol_ButtonPressed),
										 wheel_width * 0.3f);
			} else if (middleHighlighted) {
				draw_list->AddRectFilled(middle_min, middle_max, GetColorU32(ImGuiMouseCol_ButtonHighlighted),
										 wheel_width * 0.3f);
			}
		}
	}

	draw_list->PopClipRect();
}

#ifndef IMGUI_DISABLE_DEMO_WINDOWS
void MouseDemo() {
	static bool showPressed = true;
	static bool showWheel = true;
	static int currentLayout = ImGuiMouseLayout_ThreeButton;

	ImGui::Text("Mouse Widget Demo");
	ImGui::Separator();

	// Layout selection
	ImGui::Text("Layout:");
	ImGui::SameLine();
	const char *layoutNames[] = {"Two Button", "Three Button"};
	if (ImGui::BeginCombo("##MouseLayout", layoutNames[currentLayout])) {
		for (int i = 0; i < ImGuiMouseLayout_Count; i++) {
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
	ImGui::Checkbox("Show Pressed Buttons", &showPressed);
	ImGui::Checkbox("Show Scroll Wheel", &showWheel);

	ImGui::Separator();

	// Render the mouse
	ImGuiMouseFlags flags = ImGuiMouseFlags_None;
	if (showPressed) {
		flags |= ImGuiMouseFlags_ShowPressed;
	}
	if (showWheel) {
		flags |= ImGuiMouseFlags_ShowWheel;
	}
	Mouse((ImGuiMouseLayout)currentLayout, flags);
}
#endif // IMGUI_DISABLE_DEMO_WINDOWS
#endif // IMGUI_DISABLE

} // namespace ImMouse
