//
// Copyright 2020 Electronic Arts Inc.
//
// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection

/* $Header$ */
/***********************************************************************************************
 ***             C O N F I D E N T I A L  ---  W E S T W O O D   S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                    File Name : EXPAND.CPP                                                   *
 *                                                                                             *
 *                   Programmer : Joe L. Bostic                                                *
 *                                                                                             *
 *                   Start Date : 11/03/95                                                     *
 *                                                                                             *
 *                  Last Update : November 3, 1995 [JLB]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "function.h"
#include "common/framelimit.h"
#include "common/ini.h"
#ifdef IMGUI
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#endif

#ifdef NEWMENU

bool Expansion_Present(void)
{
    CCFileClass file("EXPAND.DAT");

    return (file.Is_Available());
}

class EListClass : public ListClass
{
public:
    EListClass(int id, int x, int y, int w, int h, TextPrintType flags, void const* up, void const* down)
        : ListClass(id, x, y, w, h, flags, up, down){};

protected:
    virtual void Draw_Entry(int index, int x, int y, int width, int selected);
};

void EListClass::Draw_Entry(int index, int x, int y, int width, int selected)
{
    if (TextFlags & TPF_6PT_GRAD) {
        TextPrintType flags = TextFlags;

        if (selected) {
            flags = flags | TPF_BRIGHT_COLOR;
            LogicPage->Fill_Rect(x, y, x + width - 1, y + LineHeight - 1, CC_GREEN_SHADOW);
        } else {
            if (!(flags & TPF_USE_GRAD_PAL)) {
                flags = flags | TPF_MEDIUM_COLOR;
            }
        }

        Conquer_Clip_Text_Print(List[index] + sizeof(int), x, y, CC_GREEN, TBLACK, flags, width, Tabs);

    } else {
        Conquer_Clip_Text_Print(
            List[index] + sizeof(int), x, y, (selected ? BLUE : WHITE), TBLACK, TextFlags, width, Tabs);
    }
}

// table for dos to latin1 conversion found at the icu site there : https://icu4c-demos.unicode.org/icu-bin/convexp?conv=ibm-850_P100-1995&s=ALL&s=IBM
static char corresp[256/2] = {
    0xc7, 0xfc, 0xe9, 0xe2, 0xe4, 0xe0, 0xe5, 0xe7, 0xea, 0xeb, 0xe8, 0xef, 0xee, 0xec, 0xc4, 0xc5, // 0x80
    0xc9, 0xe6, 0xc6, 0xf4, 0xf6, 0xf2, 0xfb, 0xf9, 0xff, 0xd6, 0xdc, 0xf8, 0xa3, 0xd8, 0xd7, 0x92, // 0x90 last is 0x192 but it's over the limit !
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xa0 not done
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xb0 not done
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xc0 not done
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0xd0 not done
    0xd3, 0xdf, 0xd4, 0xd2, 0xf5, 0xd5, 0xb5, 0xfe, 0xde, 0xda, 0xdb, 0xd9, 0xfd, 0xdd, 0xaf, 0xb4, // 0xe0
    0xad, 0xb1, 0x17, 0xbe, 0xb6, 0xa7, 0xf7, 0xb8, 0xb0, 0xa8, 0xb7, 0xb9, 0xb3, 0xb2, 0xa0, 0xa0 // 0xf0 (2 chars out of bounds)
};

static void conv(unsigned char *s, unsigned char *s2) {
    // Very simple dos to utf8 conversion found at https://stackoverflow.com/questions/4059775/convert-iso-8859-1-strings-to-utf-8-in-c-c
    // The idea is to try to avoid to bring in icu, avoid dependancies as much as possible... !
    int len = strlen((const char *)s);
    int n;
    // 1 conversion dos -> latin1
    for (n=0; n<len; n++)
	if ((s[n] >= 0x80 && s[n] <= 0x9f) || (s[n] >= 0xe0 && s[n] <= 0xef)) {
	    s[n] = corresp[s[n]-0x80];
	} else if (s[n] > 0x9f)
	    printf("char out of bounds %d from string %s\n",s[n],s);

    // 2 latin1 -> utf8
    unsigned char *in = s;
    unsigned char *out = s2;
    while (*in) {
	if (*in < 128) *out++ = *in++;
	else {
	    *out++ = 0xc0 | (*in >> 6);
	    *out++ = (*(in++) & 0x3f) + 0x80;
	}
    }
    *out++ = 0;
}

bool Expansion_Dialog(void)
{
#ifdef IMGUI
    typedef struct {
	int index;
	char *desc;
    } tentry;
    bool okval = false;

    tentry *data = NULL;
    int nb_alloc = 0,n = 0, index;
    INIClass ini;
    extern bool imgui_active;
    imgui_active = true;
    SDL_SetRelativeMouseMode(SDL_FALSE);

    for (index = 19; index < 340; index++) {
        char buffer[128];
        CCFileClass file;

	if (index == 19) {
	    file.Set_Name("SCJ01EA.INI");
	} else {
	    Set_Scenario_Name(buffer, index, SCEN_PLAYER_GDI, SCEN_DIR_EAST, SCEN_VAR_A);
	    strcat(buffer, ".INI");
	    file.Set_Name(buffer);
	}
        if (file.Is_Available()) {
            ini.Clear();
            ini.Load(file);
            ini.Get_String("Basic", "Name", "Funpark", buffer, sizeof(buffer));

	    if (n == nb_alloc) {
		nb_alloc += 10;
		data = (tentry*)realloc(data,sizeof(tentry)*nb_alloc);
		memset(&data[nb_alloc-10],0,sizeof(tentry)*10);
	    }
	    data[n].index = index;
	    unsigned char s[512];
	    snprintf((char*)s,512,"GDI: %s",buffer);
	    unsigned char s2[512];
	    conv(s,s2);
	    data[n++].desc = strdup((const char*)s2);
        }
    }

    for (index = 19; index < 300; index++) {
        char buffer[128];
        CCFileClass file;

	if (index == 19) {
	    file.Set_Name("SCJ01EA.INI");
	} else {
	    Set_Scenario_Name(buffer, index, SCEN_PLAYER_NOD, SCEN_DIR_EAST, SCEN_VAR_A);
	    strcat(buffer, ".INI");
	    file.Set_Name(buffer);
	}
        if (file.Is_Available()) {

            ini.Clear();
            ini.Load(file);
            ini.Get_String("Basic", "Name", "funpark", buffer, sizeof(buffer));

	    if (n == nb_alloc) {
		nb_alloc += 10;
		data = (tentry*)realloc(data,sizeof(tentry)*nb_alloc);
		memset(&data[nb_alloc-10],0,sizeof(tentry)*10);
	    }
	    data[n].index = index;
	    unsigned char s[512],s2[512];
	    snprintf((char*)s,512,"NOD: %s",buffer);
	    conv(s,s2);
	    data[n++].desc = strdup((const char*)s2);
        }
    }

    int ncampaign = n;
    char *gdi[] = {
	"scg01ea",
	"scg02ea",
	"scg03ea",
	"scg04ea",
	"scg04wa",
	"scg04wb",
	"scg05ea",
	"scg05eb",
	"scg05wa",
	"scg05wb",
	"scg06ea",
	"scg07ea",
	"scg08ea",
	"scg08eb",
	"scg09ea",
	"scg10ea",
	"scg10eb",
	"scg11ea",
	"scg12ea",
	"scg12eb",
	"scg13ea",
	"scg13eb",
	"scg14ea",
	"scg15ea",
	"scg15eb",
	"scg15ec",
    };

    // For some unknown reason, westood didn't fill the Name field in the ini of the campaign missions...
    // So I got it mainly from https://cnc.fandom.com/wiki/Category:Missions and partly from https://www.jeuxvideo.com/wikis-soluce-astuces/1235933/solution-complete-par-mission.htm
    char *gdi_name[] = {
	"1 X16-Y42",
	"2 Knock Out that Refinery",
	"3 Air Supremacy",

	"4a Stolen Property (Belarus)",
	"4aw Stolen Property (Poland)",
	"4bw Reinforce Bialystok",

	"5a Restoring Power (West Ukraine)",
	"5aw Restoring Power (East Ukraine)",
	"5b Restoring Power (West Germany)",
	"5bw Restoring Power (East Germany)",

	"6 Havoc",
	"7 Destroy the Airstrip",

	"8a U.N. Sanctions",
	"8b Doctor Mobius",

	"9 Clearing a Path",

	"10a Orcastration (Slovenia)",
	"10b Orcastration (Romania)",

	"11 Code Name Delphi",

	"12a Saving Doctor Mobius (Albania)",
	"12b Saving Doctor Mobius (Romania)",

	"13a Ion Cannon Strike (West Yugoslavia)",
	"13b Ion Cannon Strike (East Yugoslavia)",
	"14 Fish in a Barrel",
	"15a Temple Strike (West Sarajevo)",
	"15b Temple Strike (East Sarajevo)",
	"15c Temple Strike (Center Sarajevo)",
    };

    char *nod[] = {
	"scb01ea",
	"scb02ea",
	"scb02eb",
	"scb03ea",
	"scb03eb",
	"scb04ea",
	"scb04eb",
	"scb05ea",
	"scb06ea",
	"scb06eb",
	"scb06ec",
	"scb07ea",
	"scb07eb",
	"scb07ec",
	"scb08ea",
	"scb08eb",
	"scb09ea",
	"scb10ea",
	"scb10eb",
	"scb11ea",
	"scb11eb",
	"scb12ea",
	"scb13ea",
	"scb13eb",
	"scb13ec",
    };

    char *nod_name[] = {
	"1 Silencing Nikoomba",
	"2a Liberation of Egypt (North)",
	"2b Liberation of Egypt (South)",
	"3a Friends of the Brotherhood (East Sudan)",
	"3b Friends of the Brotherhood (West Sudan)",
	"4a Convoy Interception",
	"4b False Flag Operation",
	"5 Grounded",
	"6a Extract the Detonator (Ivory Coast)",
	"6b Extract the Detonator (Benign)",
	"6c Extract the Detonator (Nigeria)",
	"7a Sick and Dying (Gabon)",
	"7b Sick and Dying (Cameroon)",
	"7c Orca Heist",
	"8a New Construction Options (West Zaire)",
	"8b New Construction Options (East Zaire)",
	"9 No Mercy",
	"10a Doctor Wong",
	"10b Belly of the Beast",
	"11a Ezekiel's Wheel (Nambia)",
	"11b Ezekiel's Wheel (Mozanbique)",
	"12 Steal the Codes",
	"13a Cradle of My Temple (Northern South Africa)",
	"13b Cradle of My Temple (Center South Africa)",
	"13c Cradle of My Temple (Southern South Africa)"
    };

    Force_CD_Available(0);
    for (index = 0; index < 26; index++) {
        char buffer[128];
        CCFileClass file;

	strcpy(buffer,gdi[index]);
	strupr(buffer);
	strcat(buffer,".INI");
	file.Set_Name(buffer);
        if (file.Is_Available()) {
	    if (n == nb_alloc) {
		nb_alloc += 10;
		data = (tentry*)realloc(data,sizeof(tentry)*nb_alloc);
		memset(&data[nb_alloc-10],0,sizeof(tentry)*10);
	    }

	    data[n].index = index;
	    unsigned char s[512];
	    snprintf((char*)s,512,"GDI: %s",gdi_name[index]);
	    data[n++].desc = strdup((const char*)s);
        }
    }

    Force_CD_Available(1);
    for (index = 0; index < 25; index++) {
        char buffer[128];
        CCFileClass file;

	strcpy(buffer,nod[index]);
	strupr(buffer);
	strcat(buffer,".INI");
	file.Set_Name(buffer);
        if (file.Is_Available()) {
	    if (n == nb_alloc) {
		nb_alloc += 10;
		data = (tentry*)realloc(data,sizeof(tentry)*nb_alloc);
		memset(&data[nb_alloc-10],0,sizeof(tentry)*10);
	    }

	    data[n].index = index;
	    unsigned char s[512];
	    snprintf((char*)s,512,"NOD: %s",nod_name[index]);
	    data[n++].desc = strdup((const char*)s);
        }
    }

    bool disp_dlg = true;

    while (disp_dlg) {
	ImGui_ImplSDLRenderer2_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	static int selected = 0, selected2 = 0;
	int last;
	ImGui::SetNextWindowSize(ImVec2(825, 514), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("New Missions", &disp_dlg, 0))
	{
	    // Left
	    {
		ImGui::BeginChild("left pane", ImVec2(311, 0), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeX);
		if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
		{
		    if (ImGui::BeginTabItem("Covert"))
		    {
			for (int i = 0; i < ncampaign; i++)
			{
			    if (ImGui::Selectable(data[i].desc, selected == i)) {
				last = selected = i;
				ScenVar = SCEN_VAR_A;
				if (data[i].desc[0] == 'G') {
				    ScenPlayer = SCEN_PLAYER_GDI;
				    Whom = HOUSE_GOOD;
				} else {
				    ScenPlayer = SCEN_PLAYER_NOD;
				    Whom = HOUSE_BAD;
				}
				ScenDir = SCEN_DIR_EAST;
				Scen.Scenario = data[i].index;
				if (Scen.Scenario == 19) {
				    Scen.Scenario = 1;
				    ScenPlayer = SCEN_PLAYER_JP;
				    // These 2 are required to play the whole campaign, otherwise the game switches to the normal campaign at the end of the mission !
				    Special.IsJurassic = true;
				    AreThingiesEnabled = true;
				} else {
				    Special.IsJurassic = false;
				    AreThingiesEnabled = false;
				}
				Set_Scenario_Name(Scen.ScenarioName, Scen.Scenario, ScenPlayer, ScenDir, ScenVar);
				ini.Clear();
				char buf[50];
				snprintf(buf,50,"%s.INI",Scen.ScenarioName);
				CCFileClass file;
				file.Set_Name(buf);
				ini.Load(file);
				ini.Get_TextBlock("Briefing", Scen.BriefingText, sizeof(Scen.BriefingText));

				/*
				 **	If the briefing text could not be found in the INI file, then search
				 **	the mission.ini file.
				 */
				if (Scen.BriefingText[0] == '\0') {
				    INIClass mini;
				    CCFileClass missionIniFile("MISSION.INI");
				    mini.Load(missionIniFile);
				    mini.Get_TextBlock(Scen.ScenarioName, Scen.BriefingText, sizeof(Scen.BriefingText));
				}
				unsigned char s2[512];
				conv((unsigned char*)Scen.BriefingText,s2);
				strncpy(Scen.BriefingText,(const char*)s2,512);
				Scen.BriefingText[511] = 0;
			    }
			}
			ImGui::EndTabItem();
		    }
		    if (ImGui::BeginTabItem("Campaign"))
		    {
			for (int i = ncampaign; i < n; i++)
			{
			    if (ImGui::Selectable(data[i].desc, selected2 == i)) {
				last = selected2 = i;
				if (data[i].desc[0] == 'G') {
				    ScenPlayer = SCEN_PLAYER_GDI;
				    Whom = HOUSE_GOOD;
				} else {
				    ScenPlayer = SCEN_PLAYER_NOD;
				    Whom = HOUSE_BAD;
				}
				char mis[5];
				strncpy(mis,&data[i].desc[5],4); // get number + direction
				mis[4] = 0;
				char *b = strchr(mis,' ');
				if (b) *b = 0;
				Scen.Scenario = atoi(mis);
				char *dir = &mis[0];
				while (*dir >= '0' && *dir <= '9')
				    dir++;
				if (!strcmp(dir,"a") || dir[0]==0) {
				    ScenDir = SCEN_DIR_EAST;
				    ScenVar = SCEN_VAR_A;
				} else if (!strcmp(dir,"b")) {
				    ScenDir = SCEN_DIR_EAST;
				    ScenVar = SCEN_VAR_B;
				} else if (!strcmp(dir,"aw")) {
				    ScenDir = SCEN_DIR_WEST;
				    ScenVar = SCEN_VAR_A;
				} else if (!strcmp(dir,"bw")) {
				    ScenDir = SCEN_DIR_WEST;
				    ScenVar = SCEN_VAR_B;
				} else if (!strcmp(dir,"c")) {
				    ScenDir = SCEN_DIR_EAST;
				    ScenVar = SCEN_VAR_C;
				} else {
				    printf("unknown direction for %s\n",dir);
				    exit(1);
				}

				Set_Scenario_Name(Scen.ScenarioName, Scen.Scenario, ScenPlayer, ScenDir, ScenVar);
				ini.Clear();
				char buf[50];
				snprintf(buf,50,"%s.INI",Scen.ScenarioName);
				CCFileClass file;
				file.Set_Name(buf);
				ini.Load(file);
				ini.Get_TextBlock("Briefing", Scen.BriefingText, sizeof(Scen.BriefingText));

				/*
				 **	If the briefing text could not be found in the INI file, then search
				 **	the mission.ini file.
				 */
				if (Scen.BriefingText[0] == '\0') {
				    INIClass mini;
				    CCFileClass missionIniFile("MISSION.INI");
				    mini.Load(missionIniFile);
				    mini.Get_TextBlock(Scen.ScenarioName, Scen.BriefingText, sizeof(Scen.BriefingText));
				}
				unsigned char s2[512];
				conv((unsigned char*)Scen.BriefingText,s2);
				strncpy(Scen.BriefingText,(const char*)s2,512);
				Scen.BriefingText[511] = 0;
			    }
			}
			ImGui::EndTabItem();
		    }
		}
		ImGui::EndTabBar();
		ImGui::EndChild();
	    }
	    ImGui::SameLine();

	    {
		ImGui::BeginGroup();
		ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
		ImGui::Text(data[last].desc);
		ImGui::Separator();
		ImGui::TextWrapped(Scen.BriefingText);
		ImGui::EndChild();
		if (ImGui::Button("Go!")) {
		    okval = true;
		    disp_dlg = false;
		}
		ImGui::SameLine();
		ImGui::EndGroup();
	    }
	}
	ImGui::End();

	Frame_Limiter();
	KeyNumType key;
	key = Keyboard->Check();
    }
    imgui_active = false;
    SDL_SetRelativeMouseMode(SDL_TRUE);
    for (int i=0; i<n; i++)
	free(data[i].desc);
    free(data);
    return okval;

#else
    int factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;

    int option_width = 236 * factor;
    int option_height = 162 * factor;
    int option_x = (320 * factor - option_width) / 2;
    int option_y = (200 * factor - option_height) / 2;

    GadgetClass* buttons = NULL;

    void const* up_button;
    void const* down_button;

    if (InMainLoop || factor == 1) {
        up_button = Hires_Retrieve("BTN-UP.SHP");
        down_button = Hires_Retrieve("BTN-DN.SHP");
    } else {
        up_button = Hires_Retrieve("BTN-UP2.SHP");
        down_button = Hires_Retrieve("BTN-DN2.SHP");
    }

    TextButtonClass ok(
        200, TXT_OK, TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 25 * factor, option_y + option_height - 15 * factor);
    TextButtonClass cancel(201,
                           TXT_CANCEL,
                           TPF_6PT_GRAD | TPF_NOSHADOW,
                           option_x + option_width - 50 * factor,
                           option_y + option_height - 15 * factor);
    EListClass list(202,
                    option_x + 10 * factor,
                    option_y + 20 * factor,
                    option_width - 20 * factor,
                    option_height - 40 * factor,
                    TPF_6PT_GRAD | TPF_NOSHADOW,
                    up_button,
                    down_button);

    buttons = &ok;
    cancel.Add(*buttons);
    list.Add(*buttons);

    /*
    **	Add in all the expansion scenarios.
    */
    int index;
    INIClass ini;

    for (index = 19; index < 340; index++) {
        char buffer[128];
        CCFileClass file;

	if (index == 19) {
	    file.Set_Name("SCJ01EA.INI");
	} else {
	    Set_Scenario_Name(buffer, index, SCEN_PLAYER_GDI, SCEN_DIR_EAST, SCEN_VAR_A);
	    strcat(buffer, ".INI");
	    file.Set_Name(buffer);
	}
        if (file.Is_Available()) {
            ini.Clear();
            ini.Load(file);
            ini.Get_String("Basic", "Name", "Funpark", buffer, sizeof(buffer));

            char* data = new char[strlen(buffer) + 1 + sizeof(int) + 25];
            *((int*)&data[0]) = index;
            strcpy(&data[sizeof(int)], "GDI: ");
            strcat(&data[sizeof(int)], buffer);
            list.Add_Item(data);
        }
    }

    for (index = 19; index < 300; index++) {
        char buffer[128];
        CCFileClass file;

	if (index == 19) {
	    file.Set_Name("SCJ01EA.INI");
	} else {
	    Set_Scenario_Name(buffer, index, SCEN_PLAYER_NOD, SCEN_DIR_EAST, SCEN_VAR_A);
	    strcat(buffer, ".INI");
	    file.Set_Name(buffer);
	}
        if (file.Is_Available()) {

            ini.Clear();
            ini.Load(file);
            ini.Get_String("Basic", "Name", "funpark", buffer, sizeof(buffer));
            char* data = new char[strlen(buffer) + 1 + sizeof(int) + 25];
            *((int*)&data[0]) = index;
            strcpy(&data[sizeof(int)], "NOD: ");
            strcat(&data[sizeof(int)], buffer);
            list.Add_Item(data);
        }
    }

    Set_Logic_Page(SeenBuff);
    bool recalc = true;
    bool display = true;
    bool process = true;
    bool okval = true;
    while (process) {

        Call_Back();

        /*
        ** If we have just received input focus again after running in the background then
        ** we need to redraw.
        */
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = true;
        }

        if (display) {
            display = false;

            Hide_Mouse();

            /*
            **	Load the background picture.
            */
            Load_Title_Screen(TitlePicture, &HidPage, Palette);
            Blit_Hid_Page_To_Seen_Buff();

            Dialog_Box(option_x, option_y, option_width, option_height);
            Draw_Caption(TXT_MISSION_DESCRIPTION, option_x, option_y, option_width);
            buttons->Draw_All();
            Show_Mouse();
        }

        KeyNumType input = buttons->Input();
        switch (input) {
        case KN_RETURN:
        case 200 | KN_BUTTON:
            if (list.Current_Item()) {
                if (list.Current_Item()[sizeof(int)] == 'G') {
                    ScenPlayer = SCEN_PLAYER_GDI;
		    Whom = HOUSE_GOOD;
                } else {
                    ScenPlayer = SCEN_PLAYER_NOD;
		    Whom = HOUSE_BAD;
                }
                ScenDir = SCEN_DIR_EAST;
                Scen.Scenario = *(int*)list.Current_Item();
		if (Scen.Scenario == 19) {
		    Scen.Scenario = 1;
		    ScenPlayer = SCEN_PLAYER_JP;
		    // These 2 are required to play the whole campaign, otherwise the game switches to the normal campaign at the end of the mission !
		    Special.IsJurassic = true;
		    AreThingiesEnabled = true;
		} else {
		    Special.IsJurassic = false;
		    AreThingiesEnabled = false;
		}
                okval = true;
                process = false;
                break;
            }

        case KN_ESC:
        case 201 | KN_BUTTON:
            ScenPlayer = SCEN_PLAYER_GDI;
            ScenDir = SCEN_DIR_EAST;
            Whom = HOUSE_GOOD;
            if (list.Current_Item()) {
                Scen.Scenario = *(int*)list.Current_Item();
            }
            process = false;
            okval = false;
            break;

        default:
            break;
        }

        Frame_Limiter();
    }

    /*
    **	Free up the allocations for the text lines in the list box.
    */
    for (index = 0; index < list.Count(); index++) {
        delete[](char*) list.Get_Item(index);
    }

    return (okval);
#endif
}

#ifndef IMGUI
bool Campaign_Dialog(void)
{
    // This gui is crap so this code is shamelessly copyed from the previous function, Expansion_Dialog, just changing the part about filling the list
    int factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;

    int option_width = 236 * factor;
    int option_height = 162 * factor;
    int option_x = (320 * factor - option_width) / 2;
    int option_y = (200 * factor - option_height) / 2;
    Special.IsJurassic = false;
    AreThingiesEnabled = false;

    GadgetClass* buttons = NULL;

    void const* up_button;
    void const* down_button;

    if (InMainLoop || factor == 1) {
        up_button = Hires_Retrieve("BTN-UP.SHP");
        down_button = Hires_Retrieve("BTN-DN.SHP");
    } else {
        up_button = Hires_Retrieve("BTN-UP2.SHP");
        down_button = Hires_Retrieve("BTN-DN2.SHP");
    }

    TextButtonClass ok(
        200, TXT_OK, TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 25 * factor, option_y + option_height - 15 * factor);
    TextButtonClass cancel(201,
                           TXT_CANCEL,
                           TPF_6PT_GRAD | TPF_NOSHADOW,
                           option_x + option_width - 50 * factor,
                           option_y + option_height - 15 * factor);
    EListClass list(202,
                    option_x + 10 * factor,
                    option_y + 20 * factor,
                    option_width - 20 * factor,
                    option_height - 40 * factor,
                    TPF_6PT_GRAD | TPF_NOSHADOW,
                    up_button,
                    down_button);

    buttons = &ok;
    cancel.Add(*buttons);
    list.Add(*buttons);

    /*
    **	Add in all the expansion scenarios.
    */
    int index;
    INIClass ini;

    char *gdi[] = {
	"scg01ea",
	"scg02ea",
	"scg03ea",
	"scg04ea",
	"scg04wa",
	"scg04wb",
	"scg05ea",
	"scg05eb",
	"scg05wa",
	"scg05wb",
	"scg06ea",
	"scg07ea",
	"scg08ea",
	"scg08eb",
	"scg09ea",
	"scg10ea",
	"scg10eb",
	"scg11ea",
	"scg12ea",
	"scg12eb",
	"scg13ea",
	"scg13eb",
	"scg14ea",
	"scg15ea",
	"scg15eb",
	"scg15ec",
    };

    // For some unknown reason, westood didn't fill the Name field in the ini of the campaign missions...
    // So I got it mainly from https://cnc.fandom.com/wiki/Category:Missions and partly from https://www.jeuxvideo.com/wikis-soluce-astuces/1235933/solution-complete-par-mission.htm
    char *gdi_name[] = {
	"1 X16-Y42",
	"2 Knock Out that Refinery",
	"3 Air Supremacy",

	"4a Stolen Property (Belarus)",
	"4aw Stolen Property (Poland)",
	"4bw Reinforce Bialystok",

	"5a Restoring Power (West Ukraine)",
	"5aw Restoring Power (East Ukraine)",
	"5b Restoring Power (West Germany)",
	"5bw Restoring Power (East Germany)",

	"6 Havoc",
	"7 Destroy the Airstrip",

	"8a U.N. Sanctions",
	"8b Doctor Mobius",

	"9 Clearing a Path",

	"10a Orcastration (Slovenia)",
	"10b Orcastration (Romania)",

	"11 Code Name Delphi",

	"12a Saving Doctor Mobius (Albania)",
	"12b Saving Doctor Mobius (Romania)",

	"13a Ion Cannon Strike (West Yugoslavia)",
	"13b Ion Cannon Strike (East Yugoslavia)",
	"14 Fish in a Barrel",
	"15a Temple Strike (West Sarajevo)",
	"15b Temple Strike (East Sarajevo)",
	"15c Temple Strike (Center Sarajevo)",
    };

    char *nod[] = {
	"scb01ea",
	"scb02ea",
	"scb02eb",
	"scb03ea",
	"scb03eb",
	"scb04ea",
	"scb04eb",
	"scb05ea",
	"scb06ea",
	"scb06eb",
	"scb06ec",
	"scb07ea",
	"scb07eb",
	"scb07ec",
	"scb08ea",
	"scb08eb",
	"scb09ea",
	"scb10ea",
	"scb10eb",
	"scb11ea",
	"scb11eb",
	"scb12ea",
	"scb13ea",
	"scb13eb",
	"scb13ec",
    };

    char *nod_name[] = {
	"1 Silencing Nikoomba",
	"2a Liberation of Egypt (North)",
	"2b Liberation of Egypt (South)",
	"3a Friends of the Brotherhood (East Sudan)",
	"3b Friends of the Brotherhood (West Sudan)",
	"4a Convoy Interception",
	"4b False Flag Operation",
	"5 Grounded",
	"6a Extract the Detonator (Ivory Coast)",
	"6b Extract the Detonator (Benign)",
	"6c Extract the Detonator (Nigeria)",
	"7a Sick and Dying (Gabon)",
	"7b Sick and Dying (Cameroon)",
	"7c Orca Heist",
	"8a New Construction Options (West Zaire)",
	"8b New Construction Options (East Zaire)",
	"9 No Mercy",
	"10a Doctor Wong",
	"10b Belly of the Beast",
	"11a Ezekiel's Wheel (Nambia)",
	"11b Ezekiel's Wheel (Mozanbique)",
	"12 Steal the Codes",
	"13a Cradle of My Temple (Northern South Africa)",
	"13b Cradle of My Temple (Center South Africa)",
	"13c Cradle of My Temple (Southern South Africa)"
    };

    Force_CD_Available(0);
    for (index = 0; index < 26; index++) {
        char buffer[128];
        CCFileClass file;

	strcpy(buffer,gdi[index]);
	strupr(buffer);
	strcat(buffer,".INI");
	file.Set_Name(buffer);
        if (file.Is_Available()) {
	    char *buffer = gdi_name[index];
            char* data = new char[strlen(buffer) + 1 + sizeof(int) + 25];
            *((int*)&data[0]) = index;
            strcpy(&data[sizeof(int)], "GDI: ");
            strcat(&data[sizeof(int)], buffer);
            list.Add_Item(data);
        }
    }

    Force_CD_Available(1);
    for (index = 0; index < 25; index++) {
        char buffer[128];
        CCFileClass file;

	strcpy(buffer,nod[index]);
	strupr(buffer);
	strcat(buffer,".INI");
	file.Set_Name(buffer);
        if (file.Is_Available()) {
	    char *buffer = nod_name[index];
            char* data = new char[strlen(buffer) + 1 + sizeof(int) + 25];
            *((int*)&data[0]) = index;
            strcpy(&data[sizeof(int)], "NOD: ");
            strcat(&data[sizeof(int)], buffer);
            list.Add_Item(data);
        }
    }

    Set_Logic_Page(SeenBuff);
    bool recalc = true;
    bool display = true;
    bool process = true;
    bool okval = true;
    while (process) {

        Call_Back();

        /*
        ** If we have just received input focus again after running in the background then
        ** we need to redraw.
        */
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = true;
        }

        if (display) {
            display = false;

            Hide_Mouse();

            /*
            **	Load the background picture.
            */
            Load_Title_Screen(TitlePicture, &HidPage, Palette);
            Blit_Hid_Page_To_Seen_Buff();

            Dialog_Box(option_x, option_y, option_width, option_height);
            Draw_Caption(TXT_MISSION_DESCRIPTION, option_x, option_y, option_width);
            buttons->Draw_All();
            Show_Mouse();
        }

        KeyNumType input = buttons->Input();
        switch (input) {
        case KN_RETURN:
        case 200 | KN_BUTTON:
            if (list.Current_Item()) {
                if (list.Current_Item()[sizeof(int)] == 'G') {
                    ScenPlayer = SCEN_PLAYER_GDI;
		    Force_CD_Available(0);
                } else {
                    ScenPlayer = SCEN_PLAYER_NOD;
		    Force_CD_Available(1);
                }
		char mis[5];
		strncpy(mis,&list.Current_Item()[9],4);
		mis[4] = 0;
		char *b = strchr(mis,' ');
		if (b) *b = 0;
		Scen.Scenario = atoi(mis);
		char *dir = &mis[0];
		while (*dir >= '0' && *dir <= '9')
		    dir++;
		if (!strcmp(dir,"a")) {
		    ScenDir = SCEN_DIR_EAST;
		    ScenVar = SCEN_VAR_A;
		} else if (!strcmp(dir,"b")) {
		    ScenDir = SCEN_DIR_EAST;
		    ScenVar = SCEN_VAR_B;
		} else if (!strcmp(dir,"aw")) {
		    ScenDir = SCEN_DIR_WEST;
		    ScenVar = SCEN_VAR_A;
		} else if (!strcmp(dir,"bw")) {
		    ScenDir = SCEN_DIR_WEST;
		    ScenVar = SCEN_VAR_B;
		} else if (!strcmp(dir,"c")) {
		    ScenDir = SCEN_DIR_EAST;
		    ScenVar = SCEN_VAR_C;
		} else {
		    printf("unknown direction for %s\n",dir);
		    exit(1);
		}

                Whom = HOUSE_GOOD;
                okval = true;
                process = false;
                break;
            }

        case KN_ESC:
        case 201 | KN_BUTTON:
            ScenPlayer = SCEN_PLAYER_GDI;
            ScenDir = SCEN_DIR_EAST;
            Whom = HOUSE_GOOD;
            if (list.Current_Item()) {
                Scen.Scenario = *(int*)list.Current_Item();
            }
            process = false;
            okval = false;
            break;

        default:
            break;
        }

        Frame_Limiter();
    }

    /*
    **	Free up the allocations for the text lines in the list box.
    */
    for (index = 0; index < list.Count(); index++) {
        delete[](char*) list.Get_Item(index);
    }

    return (okval);
}
#endif

/***********************************************************************************************
 * Bonus_Dialog -- Asks the user which bonus mission he wants to play                          *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * INPUT:    Nothing                                                                           *
 *                                                                                             *
 * OUTPUT:   Nothing                                                                           *
 *                                                                                             *
 * WARNINGS: None                                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *    3/26/97 11:07AM ST : Created                                                             *
 *=============================================================================================*/
bool Bonus_Dialog(void)
{
    int factor = (SeenBuff.Get_Width() == 320) ? 1 : 2;

    int option_width = 236 * factor;
    int option_height = 162 * factor;
    int option_x = (320 * factor - option_width) / 2;
    int option_y = (200 * factor - option_height) / 2;

    GadgetClass* buttons = NULL;

    void const* up_button;
    void const* down_button;

    if (InMainLoop || factor == 1) {
        up_button = Hires_Retrieve("BTN-UP.SHP");
        down_button = Hires_Retrieve("BTN-DN.SHP");
    } else {
        up_button = Hires_Retrieve("BTN-UP2.SHP");
        down_button = Hires_Retrieve("BTN-DN2.SHP");
    }

    TextButtonClass ok(
        200, TXT_OK, TPF_6PT_GRAD | TPF_NOSHADOW, option_x + 25 * factor, option_y + option_height - 15 * factor);
    TextButtonClass cancel(201,
                           TXT_CANCEL,
                           TPF_6PT_GRAD | TPF_NOSHADOW,
                           option_x + option_width - 50 * factor,
                           option_y + option_height - 15 * factor);
    EListClass list(202,
                    option_x + 10 * factor,
                    option_y + 20 * factor,
                    option_width - 20 * factor,
                    option_height - 40 * factor,
                    TPF_6PT_GRAD | TPF_NOSHADOW,
                    up_button,
                    down_button);

    buttons = &ok;
    cancel.Add(*buttons);
    list.Add(*buttons);

    /*
    **	Add in all the expansion scenarios.
    */
    char* sbuffer = (char*)_ShapeBuffer;
    int gdi_scen_names[3] = {TXT_BONUS_MISSION_1, TXT_BONUS_MISSION_2, TXT_BONUS_MISSION_3};

    int nod_scen_names[2] = {TXT_BONUS_MISSION_4, TXT_BONUS_MISSION_5};

    int index;
    for (index = 60; index < 63; index++) {
        char buffer[128];
        CCFileClass file;

        Set_Scenario_Name(buffer, index, SCEN_PLAYER_GDI, SCEN_DIR_EAST, SCEN_VAR_A);
        strcat(buffer, ".INI");
        file.Set_Name(buffer);
        if (file.Is_Available()) {
            memcpy(
                buffer, Text_String(gdi_scen_names[index - 60]), 1 + strlen(Text_String(gdi_scen_names[index - 60])));
            char* data = new char[strlen(buffer) + 1 + sizeof(int) + 25];
            *((int*)&data[0]) = index;
            strcpy(&data[sizeof(int)], "GDI: ");
            strcat(&data[sizeof(int)], buffer);
            list.Add_Item(data);
        }
    }

    for (index = 60; index < 62; index++) {
        char buffer[128];
        CCFileClass file;

        Set_Scenario_Name(buffer, index, SCEN_PLAYER_NOD, SCEN_DIR_EAST, SCEN_VAR_A);
        strcat(buffer, ".INI");
        file.Set_Name(buffer);
        if (file.Is_Available()) {
            memcpy(
                buffer, Text_String(nod_scen_names[index - 60]), 1 + strlen(Text_String(nod_scen_names[index - 60])));
            char* data = new char[strlen(buffer) + 1 + sizeof(int) + 25];
            *((int*)&data[0]) = index;
            strcpy(&data[sizeof(int)], "NOD: ");
            strcat(&data[sizeof(int)], buffer);
            list.Add_Item(data);
        }
    }

    Set_Logic_Page(SeenBuff);
    bool recalc = true;
    bool display = true;
    bool process = true;
    bool okval = true;
    while (process) {

        Call_Back();

        /*
        ** If we have just received input focus again after running in the background then
        ** we need to redraw.
        */
        if (AllSurfaces.SurfacesRestored) {
            AllSurfaces.SurfacesRestored = false;
            display = true;
        }

        if (display) {
            display = false;

            Hide_Mouse();

            /*
            **	Load the background picture.
            */
            Load_Title_Screen(TitlePicture, &HidPage, Palette);
            Blit_Hid_Page_To_Seen_Buff();

            Dialog_Box(option_x, option_y, option_width, option_height);
            Draw_Caption(TXT_BONUS_MISSIONS, option_x, option_y, option_width);
            buttons->Draw_All();
            Show_Mouse();
        }

        KeyNumType input = buttons->Input();
        switch (input) {
        case KN_RETURN:
        case 200 | KN_BUTTON:
            if (list.Current_Item()) {
                if (list.Current_Item()[sizeof(int)] == 'G') {
                    ScenPlayer = SCEN_PLAYER_GDI;
                } else {
                    ScenPlayer = SCEN_PLAYER_NOD;
                }
                ScenDir = SCEN_DIR_EAST;
                Whom = HOUSE_GOOD;
                Scen.Scenario = *(int*)list.Current_Item();
                process = false;
                okval = true;
                break;
            }

        case KN_ESC:
        case 201 | KN_BUTTON:
            ScenPlayer = SCEN_PLAYER_GDI;
            ScenDir = SCEN_DIR_EAST;
            Whom = HOUSE_GOOD;
            if (list.Current_Item()) {
                Scen.Scenario = *(int*)list.Current_Item();
            }
            process = false;
            okval = false;
            break;

        default:
            break;
        }
    }

    /*
    **	Free up the allocations for the text lines in the list box.
    */
    for (index = 0; index < list.Count(); index++) {
        delete[](char*) list.Get_Item(index);
    }

    return (okval);
}

#endif
