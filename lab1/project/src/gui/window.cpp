#include <cmath>
#include "imgui.h"
#include "window.h"
#include "polynomial.h"

const int STRING_BUFFER_LENGTH = 65536, REGISTER_NUMBER = 65536;

enum Operations { AC, ADD, SUB, MUL };
enum InputMode { BUFFER, REGIND };
enum StatusMsg { NOMSG, READREG, SAVEREG, INVALIDREG };
enum BufferState { 
	INPUT_EMPTY, INPUT_COF_SYMB, INPUT_COF_INT, INPUT_COF_DOT,
	INPUT_COF_DECIMAL, INPUT_X, INPUT_DEG_BEFSYMB, INPUT_DEG_SYMB, 
	INPUT_DEG_NUM };

char str_buffer[STRING_BUFFER_LENGTH];
Polynomial polynomial_registers[REGISTER_NUMBER];
Polynomial& REG_LAST = polynomial_registers[0];
Operations cur_opr = AC;
bool init_display = true, ans_empty = true;
InputMode input_mode = BUFFER;
StatusMsg status_msg = NOMSG;
BufferState current_buffer_state = INPUT_EMPTY;

int invalid_register(int reg)
{
	return reg < 0 || reg > REGISTER_NUMBER;
}

ImFont* font_smaller;

void init_gui()
{

    ImGuiIO& io = ImGui::GetIO(); 

    font_smaller = io.Fonts->AddFontFromFileTTF(
		"../assets/Hack-Regular.ttf", 
		24.0f, nullptr, 
		io.Fonts->GetGlyphRangesChineseSimplifiedCommon()
	);
    IM_ASSERT(font_smaller != nullptr);
}

float draw_polynomial_width(char* const format_str, int hide_width = 0)
{
    float width = 0;
    bool deg_flag = false, sgn_flag = false;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    int len = strlen(format_str);
    for(int i = 0; i < len; i++)
	{
        if(format_str[i] == '^')
        {
            deg_flag = true;
            continue;
        }
        if(deg_flag && sgn_flag 
            && (format_str[i] == '+' || format_str[i] == '-'))
        {
            deg_flag = false;
            sgn_flag = false;
        }
        if(deg_flag && (format_str[i] == '-'
            || (format_str[i] >= '0' || format_str[i] <= '9')))
        {
            sgn_flag = true;
        }

        if(deg_flag) ImGui::PushFont(font_smaller);
        if(i >= hide_width)
            width += ImGui::CalcTextSize(format_str + i, 
                                        format_str + i + 1).x;
        if(deg_flag) ImGui::PopFont();
	}
    ImGui::PopStyleVar();
    return width;
}

// Assert that the format of the string is correct.
void draw_polynomial(char* const format_str, int hide_width = 0)
{
    bool sameline_flag = false, deg_flag = false, 
         sgn_flag = false;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    int len = strlen(format_str);
    for(int i = 0; i < len; i++)
	{
        if(format_str[i] == '^')
        {
            deg_flag = true;
            continue;
        }
        if(deg_flag && sgn_flag 
            && (format_str[i] == '+' || format_str[i] == '-'))
        {
            deg_flag = false;
            sgn_flag = false;
        }
        if(deg_flag && (format_str[i] == '-'
            || (format_str[i] >= '0' || format_str[i] <= '9')))
        {
            sgn_flag = true;
        }

        if(sameline_flag) ImGui::SameLine();
        if(deg_flag) ImGui::PushFont(font_smaller);
        if(i >= hide_width)
        {
            ImGui::Text("%c", format_str[i]);
            sameline_flag = true;
        }
        if(deg_flag) ImGui::PopFont();
	}
    ImGui::PopStyleVar();
}

float draw_polynomial_width(Polynomial poly)
{
    char* format_str = new char[STRING_BUFFER_LENGTH];
    polynomial_tostring(poly, format_str);
    return draw_polynomial_width(format_str);
    delete format_str;
}

void draw_polynomial(Polynomial poly)
{
    char* format_str = new char[STRING_BUFFER_LENGTH];
    polynomial_tostring(poly, format_str);
    draw_polynomial(format_str);
    delete format_str;
}

void format_str_sort(char* format_str)
{
    Polynomial poly;
    poly.data_head = NULL;
    polynomial_build_from_string(&poly, format_str);
    polynomial_tostring(poly, format_str);
    polynomial_delete(&poly);
}

BufferState buffer_state(char* const buffer = str_buffer)
{
    int len = strlen(buffer);

	BufferState state = INPUT_EMPTY;
    for(int i = 0; i < len; i++)
	{
		if((state == INPUT_EMPTY || state == INPUT_DEG_NUM || state == INPUT_X
			|| state == INPUT_COF_INT || state == INPUT_COF_DECIMAL)
				&& (buffer[i] == '+' || buffer[i] == '-'))
		{
			state = INPUT_COF_SYMB;
		}
		if((state == INPUT_EMPTY || state == INPUT_COF_SYMB)
			&& (buffer[i] >= '0' && buffer[i] <= '9'))
		{
			state = INPUT_COF_INT;
		}
		if(state == INPUT_COF_INT && buffer[i] == '.')
		{
			state = INPUT_COF_DOT;
		}
		if(state == INPUT_COF_DOT && (buffer[i] >= '0' && buffer[i] <= '9'))
		{
			state = INPUT_COF_DECIMAL;
		}
		if((state == INPUT_EMPTY || state == INPUT_COF_SYMB
					|| state == INPUT_COF_INT || state == INPUT_COF_DECIMAL) 
				&& buffer[i] == 'x')
		{
			state = INPUT_X;
		}
		if(state == INPUT_X && buffer[i] == '^')
		{
			state = INPUT_DEG_BEFSYMB;
		}
		if(state == INPUT_DEG_BEFSYMB 
				&& (buffer[i] == '-' || buffer[i] == '+'))
		{
			state = INPUT_DEG_SYMB;
		}
		if((state == INPUT_DEG_BEFSYMB || state == INPUT_DEG_SYMB)
					&& (buffer[i] >= '0' && buffer[i] <= '9'))
		{
			state = INPUT_DEG_NUM;
		}
	}
	return state;
}

void event_input(char c)
{
	init_display = false;

    int buffer_len = strlen(str_buffer);
    if(status_msg == INVALIDREG) status_msg = NOMSG;

    if(c == 'D')
    {
        if(buffer_len > 0)
            str_buffer[--buffer_len] = '\0';
        while(buffer_len > 0 && str_buffer[buffer_len - 1] == '^')
            str_buffer[--buffer_len] = '\0';
        return;
    }
	if(buffer_len == STRING_BUFFER_LENGTH - 2)
	{
		return;
	}
    if(input_mode == BUFFER)
    {
		if(current_buffer_state == INPUT_COF_DOT && !(c >= '0' && c <= '9'))
		{
			// Remove the '.'
			str_buffer[--buffer_len] = '\0';
		}
		if(c == '^')
		{
			if(current_buffer_state == INPUT_DEG_BEFSYMB)
			{
				str_buffer[--buffer_len] = '\0';
				return;
			}
			if(current_buffer_state != INPUT_X)
				return;
		}
		if(c >= '0' && c <= '9')
		{
			if(current_buffer_state == INPUT_X)
				str_buffer[buffer_len++] = '^';
		}
		if(c == 'x')
		{
			if(current_buffer_state != INPUT_EMPTY
				&& current_buffer_state != INPUT_COF_INT
			 	&& current_buffer_state != INPUT_COF_DOT
			 	&& current_buffer_state != INPUT_COF_DECIMAL
			 	&& current_buffer_state != INPUT_COF_SYMB)
				return;
		}
		if(c == '.')
		{
			if(current_buffer_state == INPUT_COF_SYMB 
					|| current_buffer_state == INPUT_EMPTY)
			{
				str_buffer[buffer_len++] = '0';
				str_buffer[buffer_len] = '\0';
			}
			else if(current_buffer_state != INPUT_COF_INT)
				return;
		}
		if(c == '+')
		{
			if(current_buffer_state == INPUT_COF_SYMB
					|| current_buffer_state == INPUT_DEG_SYMB)
			{
				if(str_buffer[buffer_len - 1] == '-')
				{
					if(current_buffer_state == INPUT_DEG_SYMB)
					{
						str_buffer[--buffer_len] = '\0';
					}
					else
					{
						str_buffer[buffer_len - 1] = '+';
					}
					return;
				}
				if(str_buffer[buffer_len - 1] == '+')
				{
					return;
				}
			}
			if(current_buffer_state == INPUT_DEG_BEFSYMB)
			{
				str_buffer[buffer_len - 1] = '+';
				return;
			}
		}
		if(c == '-')
		{
			if(current_buffer_state == INPUT_COF_SYMB
					|| current_buffer_state == INPUT_DEG_SYMB)
			{
				if(str_buffer[buffer_len - 1] == '-')
				{
					if (current_buffer_state == INPUT_DEG_SYMB)
					{
						str_buffer[--buffer_len] = '\0';
					}
					return;
				}
				else if(str_buffer[buffer_len - 1] == '+')
				{
					str_buffer[buffer_len - 1] = '-';
					return;
				}
			}
		}
        str_buffer[buffer_len++] = c;
        str_buffer[buffer_len] = '\0';
    }
    else if(input_mode == REGIND)
    {
        if(c >= '0' && c <= '9' && !(buffer_len == 0 && c == '0'))
        {
            str_buffer[buffer_len++] = c;
            str_buffer[buffer_len] = '\0';
        }
    }
}

void event_ac()
{
	init_display = false;
    cur_opr = AC;
    input_mode = BUFFER;
    status_msg = NOMSG;
    str_buffer[0] = '\0';
    ans_empty = true;
}

void event_ans()
{
	init_display = false;
	if(input_mode != BUFFER) return;
    if(status_msg == INVALIDREG) status_msg = NOMSG;
	str_buffer[0] = '\0';
	polynomial_tostring(REG_LAST, str_buffer);
}

void event_equal()
{
	init_display = false;
    if(status_msg == INVALIDREG) status_msg = NOMSG;
    ans_empty = false;
    if(str_buffer[0] == '\0')
        return;
    Polynomial poly, poly_new;
    poly.data_head = NULL;
    poly_new.data_head = NULL;
    polynomial_build_from_string(&poly, str_buffer);

    switch(cur_opr)
    {
        case AC:
            polynomial_copy(&poly_new, &poly);
            break;
        case ADD:
			poly_new = polynomial_add(REG_LAST, poly);
            break;
        case SUB:
			poly_new = polynomial_subtract(REG_LAST, poly);
            break;
        case MUL:
			poly_new = polynomial_multiply(REG_LAST, poly);
            break;
    }

    cur_opr = AC;

    polynomial_delete(&poly);
    polynomial_delete(&REG_LAST);
    REG_LAST = poly_new;
    str_buffer[0] = '\0';
}

void event_ok()
{
	init_display = false;
    if(str_buffer[0] == '\0')
    {
        status_msg = NOMSG;
        input_mode = BUFFER;
        str_buffer[0] = '\0';
        return;
    }
    int reg = atoi(str_buffer);
    if(invalid_register(reg))
    {
        status_msg = INVALIDREG;
        str_buffer[0] = '\0';
    }
    else
    {
        if(status_msg == READREG)
        {
            polynomial_tostring(polynomial_registers[reg], str_buffer);
        }
        else if(status_msg == SAVEREG)
        {
            polynomial_copy(&polynomial_registers[reg], &REG_LAST);
            str_buffer[0] = '\0';
        }
        status_msg = NOMSG;
    }
    input_mode = BUFFER;
}

void event_setopr(Operations opr)
{
	init_display = false;
	if(input_mode != BUFFER) return;
    if(status_msg == INVALIDREG) status_msg = NOMSG;
    if(ans_empty) 
        event_equal();
    if(cur_opr != opr)
        cur_opr = opr;
    else 
        cur_opr = AC;
}

void event_deri()
{
	init_display = false;
	if(input_mode != BUFFER) return;
    if(status_msg == INVALIDREG) status_msg = NOMSG;
    if(str_buffer[0] != '\0')
    {
        cur_opr = AC;
        event_equal();
    }
    Polynomial poly = polynomial_derivation(REG_LAST);
    polynomial_delete(&REG_LAST);
    REG_LAST = poly;
    cur_opr = AC;
    str_buffer[0] = '\0';
}

void event_save()
{
	init_display = false;
    if(status_msg == SAVEREG)
    {
        status_msg = NOMSG;
        input_mode = BUFFER;
        str_buffer[0] = '\0';
        return;
    }
    else if(status_msg != READREG)
    {
        if(str_buffer[0] != '\0' && !(cur_opr == AC && !ans_empty))
        {
            cur_opr = AC;
            event_equal();
        }
        str_buffer[0] = '\0';
    }
    input_mode = REGIND;
    status_msg = SAVEREG;
}

void event_reg()
{
	init_display = false;
    if(status_msg == READREG)
    {
        status_msg = NOMSG;
        input_mode = BUFFER;
        str_buffer[0] = '\0';
        return;
    }
    else if(status_msg != SAVEREG)
    {
        str_buffer[0] = '\0';
    }
    input_mode = REGIND;
    status_msg = READREG;
}

void draw_gui()
{
    ImGuiIO& io = ImGui::GetIO(); 
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin("Polynomial Calc", nullptr, 
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove 
			| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings 
			| ImGuiWindowFlags_NoBringToFrontOnFocus);  

    int buffer_len = strlen(str_buffer);
	current_buffer_state = buffer_state();

    ImGui::SetCursorPos(ImVec2(0, 0));
    float screen_padding = 16;
    float line1_y = 24;
    float line2_y = line1_y + 56;

    ImGui::SetCursorPos(ImVec2(screen_padding, screen_padding));
	if(init_display)
	{
		ImGui::PushFont(font_smaller);
		ImGui::Text("Polynomial Calculator  by rubatotree in USTC DS Lab1");
		ImGui::PopFont();
	}
    if(!ans_empty)
    {
        char* format_str = new char[STRING_BUFFER_LENGTH];
        polynomial_tostring(REG_LAST, format_str);

        int hide_width = 0;
        float max_width = ImGui::GetWindowWidth() - 2 * screen_padding - 32;
        float buffer_width = draw_polynomial_width(str_buffer);

        float ans_width = draw_polynomial_width(REG_LAST);
        while(ans_width >= max_width)
        {
            hide_width++;
            ans_width = draw_polynomial_width(format_str, hide_width);
        }
        float ans_cursor_x = 
                    ImGui::GetWindowWidth() - screen_padding - ans_width;

        ImGui::SetCursorPosY(line1_y);
        if(hide_width > 0)
        {
            ImGui::SetCursorPosX(ans_cursor_x - 40);
            ImGui::PushFont(font_smaller);
            ImGui::Text("...");
            ImGui::PopFont();
            ImGui::SameLine();
        }
        ImGui::SetCursorPosX(ans_cursor_x);
        draw_polynomial(format_str, hide_width);

        delete format_str;
    }
    else
    {
        ImGui::Text(" ");
    }

    ImGui::SetCursorPosX(screen_padding);
    ImGui::SetCursorPosY(line2_y);
    switch(cur_opr)
    {
        case ADD:
            ImGui::Text("+");
            ImGui::SameLine();
            break;
        case SUB:
            ImGui::Text("-");
            ImGui::SameLine();
            break;
        case MUL:
            ImGui::Text("*");
            ImGui::SameLine();
            break;
    }

    if(input_mode == BUFFER)
    {
        int hide_width = 0;
        float max_width = ImGui::GetWindowWidth() - 2 * screen_padding - 64;
        float buffer_width = draw_polynomial_width(str_buffer);

        while(buffer_width >= max_width)
        {
            hide_width++;
            buffer_width = draw_polynomial_width(str_buffer, hide_width);
        }

        float buffer_cursor_x = 
                    ImGui::GetWindowWidth() - screen_padding - buffer_width;
        ImGui::SetCursorPosY(line2_y);
        if(hide_width > 0)
        {
            ImGui::SetCursorPosX(buffer_cursor_x - 40);
            ImGui::PushFont(font_smaller);
            ImGui::Text("...");
            ImGui::PopFont();
            ImGui::SameLine();
        }
        ImGui::SetCursorPosX(buffer_cursor_x);
        draw_polynomial(str_buffer, hide_width);
    }


    ImVec2 item_spacing = ImVec2(4, 4);
    int button_width = 
        (ImGui::GetWindowWidth() - 2 * 8 - 4 * item_spacing.x) / 5;
    int button_height = 60;

    ImGui::SetCursorPos(ImVec2(8, ImGui::GetWindowHeight() - 8
                        - button_height * 5 - 4 * item_spacing.y - 36));

    ImGui::PushFont(font_smaller);
    switch(status_msg)
    {
        case READREG:
            ImGui::Text("Read from Register #%s", str_buffer);
            break;
        case SAVEREG:
            ImGui::Text("Save to Register   #%s", str_buffer);
            break;
        case INVALIDREG:
            ImGui::Text("Error: Invalid Register");
            break;
        case NOMSG:
            ImGui::Text(" ");
            break;
    }
    ImGui::PopFont();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, item_spacing);
    ImGui::SetCursorPos(ImVec2(8, ImGui::GetWindowHeight() - 8
                        - button_height * 5 - 4 * item_spacing.y));

    if(ImGui::Button("REG", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_R))
    {
        event_reg();
    }
    ImGui::SameLine();
    if(ImGui::Button("SAVE", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_S))
    {
        event_save();
    }
    ImGui::SameLine();
    if(ImGui::Button("ANS", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_A))
    {
		event_ans();
    }
    ImGui::SameLine();
    if(ImGui::Button("DEL", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_Backspace)
		|| ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        event_input('D');
    }
    ImGui::SameLine();
    if(ImGui::Button("AC", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_Escape))
    {
        event_ac();
    }

	ImGui::SetCursorPosX(8);
    if(ImGui::Button("()+()", ImVec2(button_width, button_height))
		|| ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift 
			| ImGuiKey_Equal)
		|| ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_KeypadAdd)
		|| ImGui::IsKeyChordPressed(ImGuiMod_Shift | ImGuiKey_Equal)
				&& buffer_len == 0
		|| ImGui::IsKeyPressed(ImGuiKey_KeypadAdd) && buffer_len == 0)
    {
        event_setopr(ADD);
    }
    ImGui::SameLine();
    if(ImGui::Button("7", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_7)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad7))
    {
        event_input('7');
    }
    ImGui::SameLine();
    if(ImGui::Button("8", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_8)
					&& !ImGui::IsKeyDown(ImGuiMod_Shift)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad8))
    {
        event_input('8');
    }
    ImGui::SameLine();
    if(ImGui::Button("9", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_9)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad9))
    {
        event_input('9');
    }
    ImGui::SameLine();
    if(ImGui::Button("+", ImVec2(button_width, button_height))
		|| ImGui::IsKeyChordPressed(ImGuiMod_Shift | ImGuiKey_Equal)
				&& !(ImGui::IsKeyDown(ImGuiMod_Ctrl) || buffer_len == 0)
		|| ImGui::IsKeyPressed(ImGuiKey_KeypadAdd)
				&& !(ImGui::IsKeyDown(ImGuiMod_Ctrl) || buffer_len == 0))
    {
        event_input('+');
    }

	ImGui::SetCursorPosX(8);
    if(ImGui::Button("()-()", ImVec2(button_width, button_height))
		|| ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Minus)
		|| ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_KeypadSubtract)
		|| ImGui::IsKeyPressed(ImGuiKey_Minus) && buffer_len == 0
		|| ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract) && buffer_len == 0)
    {
        event_setopr(SUB);
    }
    ImGui::SameLine();
    if(ImGui::Button("4", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_4)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad4))
    {
        event_input('4');
    }
    ImGui::SameLine();
    if(ImGui::Button("5", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_5)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad5))
    {
        event_input('5');
    }
    ImGui::SameLine();
    if(ImGui::Button("6", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_6)
					&& !ImGui::IsKeyDown(ImGuiMod_Shift)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad6))
    {
        event_input('6');
    }
    ImGui::SameLine();
    if(ImGui::Button("-", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_Minus)
				&& !(ImGui::IsKeyDown(ImGuiMod_Ctrl) || buffer_len == 0)
		|| ImGui::IsKeyPressed(ImGuiKey_KeypadSubtract)
				&& !(ImGui::IsKeyDown(ImGuiMod_Ctrl) || buffer_len == 0))
    {
        event_input('-');
    }

	ImGui::SetCursorPosX(8);
    if(ImGui::Button("()*()", ImVec2(button_width, button_height))
		|| ImGui::IsKeyChordPressed(ImGuiMod_Shift | ImGuiKey_8)
		|| ImGui::IsKeyPressed(ImGuiKey_KeypadMultiply))
    {
        event_setopr(MUL);
    }
    ImGui::SameLine();
    if(ImGui::Button("1", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_1)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad1))
    {
        event_input('1');
    }
    ImGui::SameLine();
    if(ImGui::Button("2", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_2)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad2))
    {
        event_input('2');
    }
    ImGui::SameLine();
    if(ImGui::Button("3", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_3)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad3))
    {
        event_input('3');
    }
    ImGui::SameLine();
    if(ImGui::Button("^", ImVec2(button_width, button_height))
		|| ImGui::IsKeyChordPressed(ImGuiMod_Shift | ImGuiKey_6))
    {
        event_input('^');
    }

	ImGui::SetCursorPosX(8);
    if(ImGui::Button("()'", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_Apostrophe))
    {
        event_deri();
    }
    ImGui::SameLine();
    if(ImGui::Button("0", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_0)
		|| ImGui::IsKeyPressed(ImGuiKey_Keypad0))
    {
        event_input('0');
    }
    ImGui::SameLine();
    if(ImGui::Button(".", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_Period)
		|| ImGui::IsKeyPressed(ImGuiKey_KeypadDecimal))
    {
        event_input('.');
    }
    ImGui::SameLine();
    if(ImGui::Button("x", ImVec2(button_width, button_height))
		|| ImGui::IsKeyPressed(ImGuiKey_X))
    {
        event_input('x');
    }
    ImGui::SameLine();
    if(input_mode == BUFFER)
    {
        if(ImGui::Button("=", ImVec2(button_width, button_height))
			|| ImGui::IsKeyPressed(ImGuiKey_Equal)
					&& !ImGui::IsKeyDown(ImGuiMod_Shift) 
			|| ImGui::IsKeyPressed(ImGuiKey_KeypadEqual)
			|| ImGui::IsKeyPressed(ImGuiKey_Enter)
			|| ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
        {
            event_equal();
        }
    }
    else if(input_mode == REGIND)
    {
        if(ImGui::Button("OK", ImVec2(button_width, button_height))
			|| ImGui::IsKeyPressed(ImGuiKey_Equal)
					&& !ImGui::IsKeyDown(ImGuiMod_Shift) 
			|| ImGui::IsKeyPressed(ImGuiKey_KeypadEqual)
			|| ImGui::IsKeyPressed(ImGuiKey_Enter)
			|| ImGui::IsKeyPressed(ImGuiKey_KeypadEnter))
        {
            event_ok();
        }
    }
    ImGui::PopStyleVar();

    ImGui::End();
}
