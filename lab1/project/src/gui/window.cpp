#include <cmath>
#include "imgui.h"
#include "window.h"
#include "polynomial.h"

ImFont* font_smaller;

void draw_polynomial(Polynomial& poly)
{
    bool sameline_flag = false;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	for(PolynomialNode *p = poly.data_head; p != NULL; p = p->next)
	{
		if(p != poly.data_head && p->term.cof > 0)
		{
            if(sameline_flag) ImGui::SameLine();
            ImGui::Text("+");
            sameline_flag = true;
		}
		if(fabs(p->term.cof + 1) < POLYNOMIAL_EPSILON)
		{
            if(sameline_flag) ImGui::SameLine();
            ImGui::Text("-");
            sameline_flag = true;
		}
		else if(fabs(p->term.cof - 1) < POLYNOMIAL_EPSILON)
		{
			if(p->term.deg == 0)
            {
                if(sameline_flag) ImGui::SameLine();
                ImGui::Text("1");
                sameline_flag = true;
            }
		}
		else
		{
            if(sameline_flag) ImGui::SameLine();
			if(fabs(p->term.cof - (double)(int)(p->term.cof)) 
													< POLYNOMIAL_EPSILON)
			{
				ImGui::Text("%d", (int)(p->term.cof));
			}
			else
			{
				ImGui::Text("%.2lf", p->term.cof); 
			}
            sameline_flag = true;
		}
		if(p->term.deg == 1)
		{
            if(sameline_flag) ImGui::SameLine();
			ImGui::Text("x");
            sameline_flag = true;
		}
		else if(p->term.deg != 0)
		{
            if(sameline_flag) ImGui::SameLine();
			ImGui::Text("x");
            ImGui::SameLine();
            ImGui::PushFont(font_smaller);
			ImGui::Text("%d", p->term.deg);
            ImGui::PopFont();
            sameline_flag = true;
		}
	}
	if(poly.data_head == NULL)
	{
		// When the polynomial is Empty
		ImGui::Text("0");
	}
    ImGui::PopStyleVar();
}

void draw_polynomial(char* const format_str)
{
    Polynomial poly;
    poly.data_head = NULL;
    polynomial_build_from_string(&poly, format_str);
    draw_polynomial(poly);
    polynomial_delete(&poly);
}

void init_gui()
{
    ImGuiIO& io = ImGui::GetIO(); 

    font_smaller = io.Fonts->AddFontFromFileTTF(
		"../assets/NotoSansCJKsc-Regular.otf", 
		32.0f, nullptr, 
		io.Fonts->GetGlyphRangesChineseSimplifiedCommon()
	);
    IM_ASSERT(font_smaller != nullptr);

}

void draw_gui()
{
    ImGuiIO& io = ImGui::GetIO(); 
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    ImGui::Begin("多项式计算器", nullptr, 
			ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove 
			| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings 
			| ImGuiWindowFlags_NoBringToFrontOnFocus);                          

    ImGui::Text("你好谢谢小笼包再见");

    draw_polynomial("x^2 + 2x + 1");
    draw_polynomial("x^3 + 2x^2 + 1");
    draw_polynomial("x^-3 +2x^-2 + 1+x");
    draw_polynomial("");

    ImGui::NewLine();

    ImVec2 item_spacing = ImVec2(4, 4);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, item_spacing);
    if(ImGui::Button("REG", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("SET", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("^", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("＋", ImVec2(180, 60)))
    {

    }

    if(ImGui::Button("7", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("8", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("9", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("—", ImVec2(180, 60)))
    {

    }

    if(ImGui::Button("4", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("5", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("6", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("×", ImVec2(180, 60)))
    {

    }

    if(ImGui::Button("1", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("2", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("3", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("d/dx", ImVec2(180, 60)))
    {

    }

    if(ImGui::Button("x", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("0", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button(".", ImVec2(180, 60)))
    {

    }
    ImGui::SameLine();
    if(ImGui::Button("=", ImVec2(180, 60)))
    {

    }
    ImGui::PopStyleVar();

    ImGui::End();
}
