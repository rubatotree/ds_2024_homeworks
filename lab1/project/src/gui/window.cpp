#include <cmath>
#include "imgui.h"
#include "window.h"
#include "polynomial.h"

#define REGISTER_NUMBER 65536

Polynomial polynomial_registers[REGISTER_NUMBER];
#define REG_LAST polynomial_registers[0]
int invalid_register(int reg)
{
	return reg < 0 || reg > REGISTER_NUMBER;
}

const int STRING_BUFFER_LENGTH = 65536;
enum Operations { AC, ADD, SUB, MUL };
enum InputMode { BUFFER, REGIND };
enum StatusMsg { NOMSG, READREG, SAVEREG, INVALIDREG };

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

float draw_polynomial_width(char* const format_str, 
                                            int hide_width = 0)
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

char str_buffer[STRING_BUFFER_LENGTH];
Operations cur_opr = AC;
bool ans_empty = true;
InputMode input_mode = BUFFER;
StatusMsg status_msg = NOMSG;

void event_input(char c)
{
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
    if(input_mode == BUFFER)
    {
        str_buffer[buffer_len] = c;
        str_buffer[buffer_len + 1] = '\0';
    }
    else if(input_mode == REGIND)
    {
        if(c >= '0' && c <= '9' && !(buffer_len == 0 && c == '0'))
        {
            str_buffer[buffer_len] = c;
            str_buffer[buffer_len + 1] = '\0';
        }
    }
}

void event_ac()
{
    cur_opr = AC;
    input_mode = BUFFER;
    status_msg = NOMSG;
    str_buffer[0] = '\0';
    ans_empty = true;
}

void event_equal()
{
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
			poly_new = polynomial_substract(REG_LAST, poly);
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

    ImGui::SetCursorPos(ImVec2(0, 0));
    float screen_padding = 16;
    float line1_y = 24;
    float line2_y = line1_y + 56;
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
        ImGui::Text("");
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
            ImGui::Text("");
            break;
    }
    ImGui::PopFont();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, item_spacing);
    ImGui::SetCursorPos(ImVec2(8, ImGui::GetWindowHeight() - 8
                        - button_height * 5 - 4 * item_spacing.y));

    if(ImGui::Button("REG", ImVec2(button_width, button_height)))
    {
        event_reg();
    }
    ImGui::SameLine();
    if(ImGui::Button("SAVE", ImVec2(button_width, button_height)))
    {
        event_save();
    }
    ImGui::SameLine();
    if(ImGui::Button("ANS", ImVec2(button_width, button_height)))
    {
        str_buffer[0] = '\0';
        polynomial_tostring(REG_LAST, str_buffer);
    }
    ImGui::SameLine();
    if(ImGui::Button("DEL", ImVec2(button_width, button_height)))
    {
        event_input('D');
    }
    ImGui::SameLine();
    if(ImGui::Button("AC", ImVec2(button_width, button_height)))
    {
        event_ac();
    }

    if(ImGui::Button("()+()", ImVec2(button_width, button_height)))
    {
        event_setopr(ADD);
    }
    ImGui::SameLine();
    if(ImGui::Button("7", ImVec2(button_width, button_height)))
    {
        event_input('7');
    }
    ImGui::SameLine();
    if(ImGui::Button("8", ImVec2(button_width, button_height)))
    {
        event_input('8');
    }
    ImGui::SameLine();
    if(ImGui::Button("9", ImVec2(button_width, button_height)))
    {
        event_input('9');
    }
    ImGui::SameLine();
    if(ImGui::Button("+", ImVec2(button_width, button_height)))
    {
        event_input('+');
    }

    if(ImGui::Button("()-()", ImVec2(button_width, button_height)))
    {
        event_setopr(SUB);
    }
    ImGui::SameLine();
    if(ImGui::Button("4", ImVec2(button_width, button_height)))
    {
        event_input('4');
    }
    ImGui::SameLine();
    if(ImGui::Button("5", ImVec2(button_width, button_height)))
    {
        event_input('5');
    }
    ImGui::SameLine();
    if(ImGui::Button("6", ImVec2(button_width, button_height)))
    {
        event_input('6');
    }
    ImGui::SameLine();
    if(ImGui::Button("-", ImVec2(button_width, button_height)))
    {
        event_input('-');
    }

    if(ImGui::Button("()*()", ImVec2(button_width, button_height)))
    {
        event_setopr(MUL);
    }
    ImGui::SameLine();
    if(ImGui::Button("1", ImVec2(button_width, button_height)))
    {
        event_input('1');
    }
    ImGui::SameLine();
    if(ImGui::Button("2", ImVec2(button_width, button_height)))
    {
        event_input('2');
    }
    ImGui::SameLine();
    if(ImGui::Button("3", ImVec2(button_width, button_height)))
    {
        event_input('3');
    }
    ImGui::SameLine();
    if(ImGui::Button("^", ImVec2(button_width, button_height)))
    {
        event_input('^');
    }

    if(ImGui::Button("()'", ImVec2(button_width, button_height)))
    {
        event_deri();
    }
    ImGui::SameLine();
    if(ImGui::Button("0", ImVec2(button_width, button_height)))
    {
        event_input('0');
    }
    ImGui::SameLine();
    if(ImGui::Button(".", ImVec2(button_width, button_height)))
    {
        event_input('.');
    }
    ImGui::SameLine();
    if(ImGui::Button("x", ImVec2(button_width, button_height)))
    {
        event_input('x');
    }
    ImGui::SameLine();
    if(input_mode == BUFFER)
    {
        if(ImGui::Button("=", ImVec2(button_width, button_height)))
        {
            event_equal();
        }
    }
    else if(input_mode == REGIND)
    {
        if(ImGui::Button("OK", ImVec2(button_width, button_height)))
        {
            event_ok();
        }
    }
    ImGui::PopStyleVar();

    ImGui::End();
}
