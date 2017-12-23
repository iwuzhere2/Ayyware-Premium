#include "ESP.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "GlowManager.h"
#include "Chams.h"
#include "Hooks.h"
#include "lagcomp2.h"

extern float rffmove;
extern float rfsmove;
extern float new_sidemove;
extern float new_forwardmove;
extern int GetItDoubled;
extern char lby;
extern char fake;
extern char real;
extern float ffake;
extern float freal;
extern char fakex;
extern char realx;
extern float ffakex;
extern float frealx;

DWORD GlowManager = *(DWORD*)(Utilities::Memory::FindPatternV2("client.dll", "0F 11 05 ?? ?? ?? ?? 83 C8 01 C7 05 ?? ?? ?? ?? 00 00 00 00") + 3);

#ifdef NDEBUG
#define strenc( s ) std::string( cx_make_encrypted_string( s ) )
#define charenc( s ) strenc( s ).c_str()
#define wstrenc( s ) std::wstring( strenc( s ).begin(), strenc( s ).end() )
#define wcharenc( s ) wstrenc( s ).c_str()
#else
#define strenc( s ) ( s )
#define charenc( s ) ( s )
#define wstrenc( s ) ( s )
#define wcharenc( s ) ( s )
#define	MASK_SOLID						(CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)

#endif

#ifdef NDEBUG
#define XorStr( s ) ( XorCompileTime::XorString< sizeof( s ) - 1, __COUNTER__ >( s, std::make_index_sequence< sizeof( s ) - 1>() ).decrypt() )
#else
#define XorStr( s ) ( s )
#endif


void CEsp::Init()
{
	BombCarrier = nullptr;
}

void CEsp::Move(CUserCmd *pCmd,bool &bSendPacket) 
{

}

void CEsp::Draw()
{
	Color Color;
	IClientEntity *pLocal = hackManager.pLocal();
	CUserCmd *pCmd;

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		IClientEntity *pEntity = Interfaces::EntList->GetClientEntity(i);
		player_info_t pinfo;
		PVOID pebp;
		__asm mov pebp, ebp;
		bool* pbSendPacket = (bool*)(*(DWORD*)pebp - 0x1C);
		bool& bSendPacket = *pbSendPacket;

		if (pLocal && pLocal->IsAlive())
		{
			if (Menu::Window.VisualsTab.Filtersown.GetState() && Interfaces::Engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
			{
				DrawPlayer(pEntity, pinfo, &bSendPacket, pCmd);
			}
		}

		if (pEntity &&  pEntity != pLocal && !pEntity->IsDormant())
		{
			if (Menu::Window.VisualsTab.OtherRadar.GetState())
			{
				DWORD m_bSpotted = NetVar.GetNetVar(0x839EB159);
				*(char*)((DWORD)(pEntity)+m_bSpotted) = 1;
			}
			
			if (Menu::Window.VisualsTab.FiltersPlayers.GetState() && Interfaces::Engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
			{
				DrawPlayer(pEntity, pinfo, &bSendPacket, pCmd);
			}
			
			ClientClass* cClass = (ClientClass*)pEntity->GetClientClass();

			if (Menu::Window.VisualsTab.FiltersNades.GetState())
			{
				if (cClass->m_ClassID == (int)CSGOClassID::CBaseCSGrenadeProjectile)
					DrawHE(pEntity, cClass);

				if (cClass->m_ClassID == (int)CSGOClassID::CMolotovProjectile)
					DrawMolotov(pEntity, cClass);

				if (cClass->m_ClassID == (int)CSGOClassID::CDecoyProjectile)
					DrawDecoy(pEntity, cClass);

				if (cClass->m_ClassID == (int)CSGOClassID::CSensorGrenadeProjectile)
					DrawMolotov(pEntity, cClass);

				if (cClass->m_ClassID == (int)CSGOClassID::CSmokeGrenadeProjectile)
					DrawSmoke(pEntity, cClass);
			}

			if (Menu::Window.VisualsTab.FiltersWeapons.GetState() && cClass->m_ClassID != (int)CSGOClassID::CBaseWeaponWorldModel && ((strstr(cClass->m_pNetworkName, "Weapon") || cClass->m_ClassID == (int)CSGOClassID::CDEagle || cClass->m_ClassID == (int)CSGOClassID::CAK47)))
			{
				DrawDrop(pEntity, cClass);
			}

			if (Menu::Window.VisualsTab.FiltersC4.GetState())
			{
				if (cClass->m_ClassID == (int)CSGOClassID::CPlantedC4)
					DrawBombPlanted(pEntity, cClass);

				if (cClass->m_ClassID == (int)CSGOClassID::CC4)
					DrawBomb(pEntity, cClass);
			}

			if (Menu::Window.VisualsTab.FiltersChickens.GetState())
			{
				if (cClass->m_ClassID == (int)CSGOClassID::CChicken)
					DrawChicken(pEntity, cClass);
			}
		}
	}

	if (Menu::Window.VisualsTab.OtherNoFlash.GetState())
	{
		DWORD m_flFlashMaxAlpha = NetVar.GetNetVar(0xFE79FB98);
		*(float*)((DWORD)pLocal + m_flFlashMaxAlpha) = 0;
	}

	if (Menu::Window.VisualsTab.OptionsGlow.GetState())
	{
		DrawGlow();
	}
	if (Menu::Window.VisualsTab.EntityGlow.GetState())
	{
		EntityGlow();
	}

	if (Menu::Window.VisualsTab.GrenadeTrace.GetState())
	{
		GrenadeTrace();	
	}
	if (Menu::Window.VisualsTab.BulletTrace.GetState())
	{
		DrawTrace();
	}
}

void CEsp::DrawPlayer(IClientEntity* pEntity, player_info_t pinfo , bool bSendPacket, CUserCmd *pCmd)
{
	ESPBox Box;
	Color Color;
	int real = -1;

	if (Menu::Window.VisualsTab.FiltersEnemiesOnly.GetState() && (pEntity->GetTeamNum() == hackManager.pLocal()->GetTeamNum()))
		return;



	if (GetBox(pEntity, Box))
	{
		Color = GetPlayerColor(pEntity);

		switch (Menu::Window.VisualsTab.OptionsBox.GetIndex())
		{
		case 0:
			break;
		case 1:
			DrawBox(Box, Color);
			break;
		case 2:
			FilledBox(Box, Color);
			break;
		case 3:
			Corners(Box, Color, pEntity);
			break;
		}

		switch (Menu::Window.VisualsTab.OptionsWeapon.GetIndex())
		{
		case 0:
			break;
		case 1:

			break;
		case 2:
			DrawIcon(pEntity, Box);
			break;
		}

		if (Menu::Window.VisualsTab.OptionsName.GetState())
			DrawName(pinfo, Box);

		if (Menu::Window.VisualsTab.OptionsHealth.GetState())
			DrawHealth(pEntity, Box);

		if (Menu::Window.VisualsTab.OptionsInfo.GetState())
			DrawInfo(pEntity, Box);

		if (Menu::Window.VisualsTab.OptionsArmor.GetState())
			Armor(pEntity, Box);

		if (Menu::Window.VisualsTab.Barrels.GetState())
			Barrel(Box, Color, pEntity);

		if (Menu::Window.VisualsTab.OptionsDefusing.GetState())
			IsPlayerDefusing(pinfo, Box, pEntity);

		if (Menu::Window.VisualsTab.OptionsAimSpot.GetState())
			DrawCross(pEntity);

		if (Menu::Window.VisualsTab.OptionsSkeleton.GetState())
		{
			DrawSkeleton(pEntity);
			BacktrackingCross(pEntity);
		}
			

		if (Menu::Window.VisualsTab.Money.GetState())
			DrawMoney(pEntity, Box);

		if (Menu::Window.VisualsTab.Distance.GetState())
			DrawDistance(Box, pEntity);

			Info(pEntity, Box);

	}
}

bool CEsp::GetBox(IClientEntity* pEntity, CEsp::ESPBox &result)
{
	Vector  vOrigin, min, max, sMin, sMax, sOrigin,
		flb, brt, blb, frt, frb, brb, blt, flt;
	float left, top, right, bottom;

	vOrigin = pEntity->GetOrigin();
	min = pEntity->collisionProperty()->GetMins() + vOrigin;
	max = pEntity->collisionProperty()->GetMaxs() + vOrigin;

	Vector points[] = { Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z) };

	if (!Render::WorldToScreen(points[3], flb) || !Render::WorldToScreen(points[5], brt)
		|| !Render::WorldToScreen(points[0], blb) || !Render::WorldToScreen(points[4], frt)
		|| !Render::WorldToScreen(points[2], frb) || !Render::WorldToScreen(points[1], brb)
		|| !Render::WorldToScreen(points[6], blt) || !Render::WorldToScreen(points[7], flt))
		return false;

	Vector arr[] = { flb, brt, blb, frt, frb, brb, blt, flt };

	left = flb.x;
	top = flb.y;
	right = flb.x;
	bottom = flb.y;

	for (int i = 1; i < 8; i++)
	{
		if (left > arr[i].x)
			left = arr[i].x;
		if (bottom < arr[i].y)
			bottom = arr[i].y;
		if (right < arr[i].x)
			right = arr[i].x;
		if (top > arr[i].y)
			top = arr[i].y;
	}

	result.x = left;
	result.y = top;
	result.w = right - left;
	result.h = bottom - top;

	return true;
}

Color CEsp::GetPlayerColor(IClientEntity* pEntity)
{
	int TeamNum = pEntity->GetTeamNum();
	bool IsVis = GameUtils::IsVisible(hackManager.pLocal(), pEntity, (int)CSGOHitboxID::Head);

	Color color;

	if (TeamNum == TEAM_CS_T)
	{
		if (IsVis)
			color = Color(Menu::Window.ColorsTab.TColorVisR.GetValue(), Menu::Window.ColorsTab.TColorVisG.GetValue(), Menu::Window.ColorsTab.TColorVisB.GetValue(), 255);
		else
			color = Color(Menu::Window.ColorsTab.TColorNoVisR.GetValue(), Menu::Window.ColorsTab.TColorNoVisG.GetValue(), Menu::Window.ColorsTab.TColorNoVisB.GetValue(), 255);
	}
	else
	{
		if (IsVis)
			color = Color(Menu::Window.ColorsTab.CTColorVisR.GetValue(), Menu::Window.ColorsTab.CTColorVisG.GetValue(), Menu::Window.ColorsTab.CTColorVisB.GetValue(), 255);
		else
			color = Color(Menu::Window.ColorsTab.CTColorNoVisR.GetValue(), Menu::Window.ColorsTab.CTColorNoVisG.GetValue(), Menu::Window.ColorsTab.CTColorNoVisB.GetValue(), 255);
	}

	return color;
}

void CEsp::Corners(CEsp::ESPBox size, Color color, IClientEntity* pEntity)
{
	int VertLine = (((float)size.w) * (0.20f));
	int HorzLine = (((float)size.h) * (0.30f));

	Render::Clear(size.x, size.y - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - VertLine, size.y - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x, size.y + size.h - 1, VertLine, 1, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - VertLine, size.y + size.h - 1, VertLine, 1, Color(0, 0, 0, 255));

	Render::Clear(size.x - 1, size.y, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - 1, size.y, 1, HorzLine, Color(0, 0, 0, 255));
	Render::Clear(size.x + size.w - 1, size.y + size.h - HorzLine, 1, HorzLine, Color(0, 0, 0, 255));

	Render::Clear(size.x, size.y, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
	Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);

	Render::Clear(size.x, size.y, 1, HorzLine, color);
	Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
}

void CEsp::FilledBox(CEsp::ESPBox size, Color color)
{
	int VertLine = (((float)size.w) * (0.20f));
	int HorzLine = (((float)size.h) * (0.20f));

	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	Render::Clear(size.x, size.y, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y, VertLine, 1, color);
	Render::Clear(size.x, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + size.w - VertLine, size.y + size.h, VertLine, 1, color);
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
	Render::Clear(size.x, size.y, 1, HorzLine, color);
	Render::Clear(size.x, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y, 1, HorzLine, color);
	Render::Clear(size.x + size.w, size.y + size.h - HorzLine, 1, HorzLine, color);
	Render::Clear(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(0, 0, 0, 40));
}

void CEsp::DrawBox(CEsp::ESPBox size, Color color)
{
	Render::Outline(size.x, size.y, size.w, size.h, color);
	Render::Outline(size.x - 1, size.y - 1, size.w + 2, size.h + 2, Color(10, 10, 10, 150));
	Render::Outline(size.x + 1, size.y + 1, size.w - 2, size.h - 2, Color(10, 10, 10, 150));
}

void CEsp::Barrel(CEsp::ESPBox size, Color color, IClientEntity* pEntity)
{
	Vector src3D, src;
	src3D = pEntity->GetOrigin() - Vector(0, 0, 0);

	if (!Render::WorldToScreen(src3D, src))
		return;

	int ScreenWidth, ScreenHeight;
	Interfaces::Engine->GetScreenSize(ScreenWidth, ScreenHeight);

	int x = (int)(ScreenWidth * 0.5f);
	int y = 0;


	y = ScreenHeight;

	Render::Line((int)(src.x), (int)(src.y), x, y, Color(0, 255, 0, 255));
}

void CEsp::DrawWeapon(IClientEntity* pEntity, CEsp::ESPBox size)
{
	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	if (Menu::Window.VisualsTab.OptionsWeapon.GetIndex() == 1 && pWeapon)
	{
		RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, pWeapon->GetpWeaponName());
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 8,
			Color(255, 255, 255, 255), Render::Fonts::ESP, pWeapon->GetpWeaponName());
	}
}

void CEsp::DrawIcon(IClientEntity* pEntity, CEsp::ESPBox size)
{
	IClientEntity* pWeapon = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)pEntity->GetActiveWeaponHandle());
	if (Menu::Window.VisualsTab.OptionsWeapon.GetIndex() == 2 && pWeapon)
	{
		RECT nameSize = Render::GetTextSize(Render::Fonts::Icon, pWeapon->GetGunIcon());
		Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y + size.h + 8,
			Color(255, 255, 255, 255), Render::Fonts::Icon, pWeapon->GetGunIcon());
	}
}

void CEsp::DrawGlow()
{
	int GlowR = Menu::Window.ColorsTab.GlowR.GetValue();
	int GlowG = Menu::Window.ColorsTab.GlowG.GetValue();
	int GlowB = Menu::Window.ColorsTab.GlowB.GetValue();
	int GlowZ = Menu::Window.VisualsTab.GlowZ.GetValue();

	CGlowObjectManager* GlowObjectManager = (CGlowObjectManager*)GlowManager;

	for (int i = 0; i < GlowObjectManager->size; ++i)
	{
		CGlowObjectManager::GlowObjectDefinition_t* glowEntity = &GlowObjectManager->m_GlowObjectDefinitions[i];
		IClientEntity* Entity = glowEntity->getEntity();

		if (glowEntity->IsEmpty() || !Entity)
			continue;

		switch (Entity->GetClientClass()->m_ClassID)
		{
		case 35:
			if (Menu::Window.VisualsTab.OptionsGlow.GetState())
			{
				if (!Menu::Window.VisualsTab.FiltersPlayers.GetState() && !(Entity->GetTeamNum() == hackManager.pLocal()->GetTeamNum()))
					break;
				if (Menu::Window.VisualsTab.FiltersEnemiesOnly.GetState() && (Entity->GetTeamNum() == hackManager.pLocal()->GetTeamNum()))
					break;

				if (GameUtils::IsVisible(hackManager.pLocal(), Entity, 0))
				{
					glowEntity->set((Entity->GetTeamNum() == hackManager.pLocal()->GetTeamNum()) ? Color(GlowR, GlowG, GlowB, GlowZ) : Color(GlowR, GlowG, GlowB, GlowZ));
				}

				else
				{
					glowEntity->set((Entity->GetTeamNum() == hackManager.pLocal()->GetTeamNum()) ? Color(GlowR, GlowG, GlowB, GlowZ) : Color(GlowR, GlowG, GlowB, GlowZ));
				}
			}
		}
	}
}

void CEsp::EntityGlow()
{
	int GlowR = Menu::Window.ColorsTab.GlowR.GetValue();
	int GlowG = Menu::Window.ColorsTab.GlowG.GetValue();
	int GlowB = Menu::Window.ColorsTab.GlowB.GetValue();
	int GlowZ = Menu::Window.VisualsTab.GlowZ.GetValue();

	CGlowObjectManager* GlowObjectManager = (CGlowObjectManager*)GlowManager;

	for (int i = 0; i < GlowObjectManager->size; ++i)
	{
		CGlowObjectManager::GlowObjectDefinition_t* glowEntity = &GlowObjectManager->m_GlowObjectDefinitions[i];
		IClientEntity* Entity = glowEntity->getEntity();

		if (glowEntity->IsEmpty() || !Entity)
			continue;

		switch (Entity->GetClientClass()->m_ClassID)
		{
		case 1:
			if (Menu::Window.VisualsTab.EntityGlow.GetState())
			{
				if (Menu::Window.VisualsTab.EntityGlow.GetState())
					glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 9:
			if (Menu::Window.VisualsTab.FiltersNades.GetState())
			{
				if (Menu::Window.VisualsTab.EntityGlow.GetState())
					glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 29:
			if (Menu::Window.VisualsTab.EntityGlow.GetState())
			{
				glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 39:
			if (Menu::Window.VisualsTab.EntityGlow.GetState())
			{
				if (Menu::Window.VisualsTab.FiltersC4.GetState())
					glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 41:
			if (Menu::Window.VisualsTab.EntityGlow.GetState())
			{
				glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 66:
			if (Menu::Window.VisualsTab.EntityGlow.GetState())
			{
				glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 87:
			if (Menu::Window.VisualsTab.FiltersNades.GetState())
			{
				glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 98:
			if (Menu::Window.VisualsTab.FiltersNades.GetState())
			{
				glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 108:
			if (Menu::Window.VisualsTab.FiltersC4.GetState())
			{
				glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 130:
			if (Menu::Window.VisualsTab.FiltersNades.GetState())
			{
				glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		case 134:
			if (Menu::Window.VisualsTab.FiltersNades.GetState())
			{
				glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		default:
			if (Menu::Window.VisualsTab.EntityGlow.GetState())
			{
				if (strstr(Entity->GetClientClass()->m_pNetworkName, "Weapon"))
					glowEntity->set(Color(GlowR, GlowG, GlowB, GlowZ));
			}
		}
	}
}

/*
void CEsp::DrawLinesAA(Color color) {
	Vector src3D, dst3D, forward, src, dst;
	trace_t tr;
	Ray_t ray;
	CTraceFilter filter;

	filter.pSkip = hackManager.pLocal();

	// LBY
	AngleVectors(QAngle(0, lineLBY, 0), &forward);
	src3D = hackManager.pLocal()->GetOrigin();
	dst3D = src3D + (forward * 55.f); //replace 50 with the length you want the line to have

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, 0, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::Line(src.x, src.y, dst.x, dst.y, Color(210, 105, 30, 255));
	// REAL AGNEL
	AngleVectors(QAngle(0, lineRealAngle, 0), &forward);
	dst3D = src3D + (forward * 50.f); //replace 50 with the length you want the line to have

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, 0, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 255, 0, 255));

	// Fake AGNEL
	AngleVectors(QAngle(0, lineFakeAngle, 0), &forward);
	dst3D = src3D + (forward * 50.f); //replace 50 with the length you want the line to have

	ray.Init(src3D, dst3D);

	Interfaces::Trace->TraceRay(ray, 0, &filter, &tr);

	if (!Render::WorldToScreen(src3D, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;

	Render::Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 0, 255));
}
*/


static wchar_t* CharToWideChar(const char* text)
{
	size_t size = strlen(text) + 1;
	wchar_t* wa = new wchar_t[size];
	mbstowcs_s(NULL, wa, size/4, text, size);
	return wa;
}

void CEsp::DrawName(player_info_t pinfo, CEsp::ESPBox size)
{
	RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, pinfo.name);
	Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y - 16,
		Color(255, 255, 255, 255), Render::Fonts::ESP, pinfo.name);
}

void CEsp::DrawHealth(IClientEntity* pEntity, CEsp::ESPBox size)
{
	int HPEnemy = 100;
	HPEnemy = pEntity->GetHealth();
	char nameBuffer[512];
	sprintf_s(nameBuffer, "%d", HPEnemy);


	float h = (size.h);
	float offset = (h / 4.f) + 5;
	float w = h / 64.f;
	float health = pEntity->GetHealth();
	UINT hp = h - (UINT)((h * health) / 100);

	int Red = 255 - (health*2.55);
	int Green = health*2.55;

	Render::DrawOutlinedRect((size.x - 6) - 1, size.y - 1, 3, h + 2, Color(0, 0, 0, 180));

	Render::DrawLine((size.x - 6), size.y + hp, (size.x - 6), size.y + h, Color(Red, Green, 0, 180));

	if (health < 100) {

		Render::Text(size.x - 9, size.y + hp, Color(255, 255, 255, 255), Render::Fonts::ESP, nameBuffer);
	}
}

std::string CleanItemName(std::string name)
{
	std::string Name = name;
	if (Name[0] == 'C')
		Name.erase(Name.begin());

	auto startOfWeap = Name.find("Weapon");
	if (startOfWeap != std::string::npos)
		Name.erase(Name.begin() + startOfWeap, Name.begin() + startOfWeap + 6);

	return Name;
}

void CEsp::DrawInfo(IClientEntity* pEntity, CEsp::ESPBox size)
{
	std::vector<std::string> Info;

	if (Menu::Window.VisualsTab.OptionsInfo.GetState() && pEntity == BombCarrier)
	{
		Info.push_back("Bomb Carrier");
	}

	static RECT Size = Render::GetTextSize(Render::Fonts::Default, "Hi");
	int i = 0;
	for (auto Text : Info)
	{
		Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::ESP, Text.c_str());
		i++;
	}
}

void CEsp::DrawCross(IClientEntity* pEntity)
{
	Vector cross = pEntity->GetHeadPos(), screen;
	static int Scale = 2;
	if (Render::WorldToScreen(cross, screen))
	{
		Render::Clear(screen.x - Scale, screen.y - (Scale * 2), (Scale * 2), (Scale * 4), Color(20, 20, 20, 160));
		Render::Clear(screen.x - (Scale * 2), screen.y - Scale, (Scale * 4), (Scale * 2), Color(20, 20, 20, 160));
		Render::Clear(screen.x - Scale - 1, screen.y - (Scale * 2) - 1, (Scale * 2) - 2, (Scale * 4) - 2, Color(250, 250, 250, 160));
		Render::Clear(screen.x - (Scale * 2) - 1, screen.y - Scale - 1, (Scale * 4) - 2, (Scale * 2) - 2, Color(250, 250, 250, 160));
	}
}

void CEsp::DrawDrop(IClientEntity* pEntity, ClientClass* cClass)
{
	Vector Box;
	IClientEntity* Weapon = (IClientEntity*)pEntity;
	IClientEntity* plr = Interfaces::EntList->GetClientEntityFromHandle((HANDLE)Weapon->GetOwnerHandle());
	if (!plr && Render::WorldToScreen(Weapon->GetOrigin(), Box))
	{
		if (Menu::Window.VisualsTab.FiltersWeapons.GetState())
		{
			Render::Outline(Box.x - 6, Box.y - 6, 12, 12, Color(255, 255, 255, 255));
		}
		if (Menu::Window.VisualsTab.FiltersWeapons.GetState())
		{
			RECT TextSize = Render::GetTextSize(Render::Fonts::Icon, Weapon->GetGunIcon());
			Render::Text(Box.x - (TextSize.right / 1), Box.y - 16, Color(255, 255, 255, 255), Render::Fonts::Icon, Weapon->GetGunIcon());
		}
	}
}

void CEsp::DrawChicken(IClientEntity* pEntity, ClientClass* cClass)
{
	ESPBox Box;

	if (GetBox(pEntity, Box))
	{
		player_info_t pinfo; strcpy_s(pinfo.name, "Chicken");
		if (Menu::Window.VisualsTab.FiltersChickens.GetState())
			DrawBox(Box, Color(255,255,255,255));

		if (Menu::Window.VisualsTab.FiltersChickens.GetState())
			DrawName(pinfo, Box);
	}
}

void CEsp::DrawBombPlanted(IClientEntity* pEntity, ClientClass* cClass) 
{
	BombCarrier = nullptr;

	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	CCSBomb* Bomb = (CCSBomb*)pEntity;

	if (Render::WorldToScreen(vOrig, vScreen))
	{
		float flBlow = Bomb->GetC4BlowTime();
		float TimeRemaining = flBlow - (Interfaces::Globals->interval_per_tick * hackManager.pLocal()->GetTickBase());
		char buffer[64];
		sprintf_s(buffer, "explodes in %.1f", TimeRemaining);
		Render::Text(vScreen.x, vScreen.y, Color(255, 255, 255, 255), Render::Fonts::ESP, buffer);
	}
}

void CEsp::DrawBomb(IClientEntity* pEntity, ClientClass* cClass)
{
	BombCarrier = nullptr;
	CBaseCombatWeapon *BombWeapon = (CBaseCombatWeapon *)pEntity;
	Vector vOrig; Vector vScreen;
	vOrig = pEntity->GetOrigin();
	bool adopted = true;
	HANDLE parent = BombWeapon->GetOwnerHandle();
	if (parent || (vOrig.x == 0 && vOrig.y == 0 && vOrig.z == 0))
	{
		IClientEntity* pParentEnt = (Interfaces::EntList->GetClientEntityFromHandle(parent));
		if (pParentEnt && pParentEnt->IsAlive())
		{
			BombCarrier = pParentEnt;
			adopted = false;
		}
	}

	if (adopted)
	{
		if (Render::WorldToScreen(vOrig, vScreen))
		{
			Render::Text(vScreen.x, vScreen.y, Color(112, 230, 20, 255), Render::Fonts::ESP, "Bomb");
		}
	}
}

void DrawBoneArray(int* boneNumbers, int amount, IClientEntity* pEntity, Color color)
{
	Vector LastBoneScreen;
	for (int i = 0; i < amount; i++)
	{
		Vector Bone = pEntity->GetBonePos(boneNumbers[i]);
		Vector BoneScreen;

		if (Render::WorldToScreen(Bone, BoneScreen))
		{
			if (i>0)
			{
				Render::Line(LastBoneScreen.x, LastBoneScreen.y, BoneScreen.x, BoneScreen.y, color);
			}
		}
		LastBoneScreen = BoneScreen;
	}
}

void DrawBoneTest(IClientEntity *pEntity)
{
	for (int i = 0; i < 127; i++)
	{
		Vector BoneLoc = pEntity->GetBonePos(i);
		Vector BoneScreen;
		if (Render::WorldToScreen(BoneLoc, BoneScreen))
		{
			char buf[10];
			_itoa_s(i, buf, 10);
			Render::Text(BoneScreen.x, BoneScreen.y, Color(255, 255, 255, 180), Render::Fonts::ESP, buf);
		}
	}
}

void CEsp::DrawSkeleton(IClientEntity* pEntity)
{
	studiohdr_t* pStudioHdr = Interfaces::ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int j = 0; j < pStudioHdr->numbones; j++)
	{
		mstudiobone_t* pBone = pStudioHdr->GetBone(j);

		if (pBone && (pBone->flags & BONE_USED_BY_HITBOX) && (pBone->parent != -1))
		{
			vChild = pEntity->GetBonePos(j);
			vParent = pEntity->GetBonePos(pBone->parent);

			if (Render::WorldToScreen(vParent, sParent) && Render::WorldToScreen(vChild, sChild))
			{
				Render::Line(sParent[0], sParent[1], sChild[0], sChild[1], Color(255,255,255,255));
			}
		}
	}
}

void CEsp::IsPlayerDefusing(player_info_t pinfo, CEsp::ESPBox size, IClientEntity* pEntity)
{
	RECT defSize = Render::GetTextSize(Render::Fonts::ESP, "");
	if (pEntity->IsDefusing())
	{
		Render::Text(size.x + size.w + 3, size.y + (0.3*(defSize.bottom + 15)),
			Color(255, 0, 0, 255), Render::Fonts::ESP, charenc("Defusing"));
	}
}

void CEsp::DrawMoney(IClientEntity* pEntity, CEsp::ESPBox size)
{
	ESPBox ArmorBar = size;

	int MoneyEnemy = 100;
	MoneyEnemy = pEntity->GetMoney();
	char nameBuffer[512];
	sprintf_s(nameBuffer, "%d $", MoneyEnemy);

	RECT nameSize = Render::GetTextSize(Render::Fonts::ESP, nameBuffer);
	Render::Text(size.x + (size.w / 2) - (nameSize.right / 2), size.y - 27, Color(255, 255, 0, 255), Render::Fonts::ESP, nameBuffer);
}

void CEsp::Armor(IClientEntity* pEntity, CEsp::ESPBox size)
{
	ESPBox ArBar = size;
	ArBar.y += (ArBar.h + 3);
	ArBar.h = 6;

	float ArValue = pEntity->ArmorValue();
	float ArPerc = ArValue / 100.f;
	float Width = (size.w * ArPerc);
	ArBar.w = Width;

	Vertex_t Verts[4];
	Verts[0].Init(Vector2D(ArBar.x, ArBar.y));
	Verts[1].Init(Vector2D(ArBar.x + size.w + 0, ArBar.y));
	Verts[2].Init(Vector2D(ArBar.x + size.w, ArBar.y + 2));
	Verts[3].Init(Vector2D(ArBar.x - 0, ArBar.y + 2));

	Render::PolygonOutline(4, Verts, Color(50, 50, 50, 255), Color(50, 50, 50, 255));

	Vertex_t Verts2[4];
	Verts2[0].Init(Vector2D(ArBar.x, ArBar.y + 1));
	Verts2[1].Init(Vector2D(ArBar.x + ArBar.w + 0, ArBar.y + 1));
	Verts2[2].Init(Vector2D(ArBar.x + ArBar.w, ArBar.y + 2));
	Verts2[3].Init(Vector2D(ArBar.x, ArBar.y + 2));

	Color c = GetPlayerColor(pEntity);
	Render::Polygon(4, Verts2, Color(0, 120, 255, 200));
}


void CEsp::DrawMolotov(IClientEntity* pEntity, ClientClass* cClass)
{
	ESPBox Box;

	if (GetBox(pEntity, Box))
	{
		player_info_t pinfo; strcpy_s(pinfo.name, "Fire");

		if (Menu::Window.VisualsTab.FiltersNades.GetState())
			DrawName(pinfo, Box);
	}
}

void CEsp::DrawSmoke(IClientEntity* pEntity, ClientClass* cClass)
{
	ESPBox Box;

	if (GetBox(pEntity, Box))
	{
		player_info_t pinfo; strcpy_s(pinfo.name, "Smoke");

		if (Menu::Window.VisualsTab.FiltersNades.GetState() == 1)
			DrawName(pinfo, Box);
	}
}

void CEsp::DrawDecoy(IClientEntity* pEntity, ClientClass* cClass)
{
	ESPBox Box;

	if (GetBox(pEntity, Box))
	{
		player_info_t pinfo; strcpy_s(pinfo.name, "Decoy");

		if (Menu::Window.VisualsTab.FiltersNades.GetState())
			DrawName(pinfo, Box);
	}
}

void CEsp::DrawHE(IClientEntity* pEntity, ClientClass* cClass)
{
	ESPBox Box;

	if (GetBox(pEntity, Box))
	{
		player_info_t pinfo; strcpy_s(pinfo.name, "HE or Flash");

		if (Menu::Window.VisualsTab.FiltersNades.GetState())
			DrawName(pinfo, Box);
	}
}

void CEsp::Info(IClientEntity* pEntity, CEsp::ESPBox size)
{
	std::vector<std::string> Info;

	if (Menu::Window.VisualsTab.HasDefuser.GetState() && pEntity->HasDefuser())
	{
		Info.push_back("Has Defuser");
	}

	if (Menu::Window.VisualsTab.IsScoped.GetState() && pEntity->IsScoped())
	{
		Info.push_back("Scoped");
	}

	static RECT Size = Render::GetTextSize(Render::Fonts::ESP, "Hi");
	int i = 0;
	for (auto Text : Info)
	{
		Render::Text(size.x + size.w + 3, size.y + (i*(Size.bottom + 2)), Color(255, 255, 255, 255), Render::Fonts::ESP, Text.c_str());
		i++;
	}
}

void CEsp::GrenadeTrace()
{
	auto granade = Interfaces::CVar->FindVar("sv_grenade_trajectory");
	auto granadespoof = new SpoofedConvar(granade);
	granadespoof->SetInt(1);
}

void CEsp::DrawDistance(CEsp::ESPBox size, IClientEntity* pEntity)
{
	IClientEntity *pLocal = hackManager.pLocal();

	Vector vecOrigin = pEntity->GetOrigin();
	Vector vecOriginLocal = pLocal->GetOrigin();
	static RECT defSize = Render::GetTextSize(Render::Fonts::Default, "");

	char dist_to[32];
	sprintf_s(dist_to, "%.0f ft", DistanceTo(vecOriginLocal, vecOrigin));

	Render::Text(size.x + size.w + 3, size.y + (0.6*(defSize.bottom + 28)), Color(255, 255, 255, 255), Render::Fonts::ESP, dist_to);
}

float CEsp::DistanceTo(Vector vecSrc, Vector vecDst)
{
	Vector vDelta = vecDst - vecSrc;

	float fDistance = ::sqrtf((vDelta.Length()));

	if (fDistance < 1.0f)
		return 1.0f;

	return fDistance;
}

void CEsp::BacktrackingCross(IClientEntity* pEntity)
{


	studiohdr_t* pStudioHdr = Interfaces::ModelInfo->GetStudiomodel(pEntity->GetModel());

	if (!pStudioHdr)
		return;

	Vector vParent, vChild, sParent, sChild;

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{
		player_info_t pinfo;
		if (pEntity && pEntity != hackManager.pLocal() && !pEntity->IsDormant())
		{
			if (Interfaces::Engine->GetPlayerInfo(i, &pinfo) && pEntity->IsAlive())
			{
				if (Menu::Window.LegitBotTab.AimbotBacktrack.GetState())
				{
					if (hackManager.pLocal()->IsAlive())
					{
						for (int t = 0; t < 12; ++t)
						{
							Vector screenbacktrack[64][12];

							if (headPositions[i][t].simtime && headPositions[i][t].simtime + 1 > hackManager.pLocal()->GetSimulationTime())
							{
								if (Render::WorldToScreen(headPositions[i][t].hitboxPos, screenbacktrack[i][t]))
								{
									Interfaces::Surface->DrawSetColor(Color(255, 0, 0, 255));
									Interfaces::Surface->DrawOutlinedRect(screenbacktrack[i][t].x, screenbacktrack[i][t].y, screenbacktrack[i][t].x + 2, screenbacktrack[i][t].y + 2);

								}
							}
						}
					}
					else
					{
						memset(&headPositions[0][0], 0, sizeof(headPositions));
					}
				}
				if (Menu::Window.RageBotTab.AccuracySpread.GetState())
				{
					/*if (hackManager.pLocal()->IsAlive())
					{
					Vector screenbacktrack[64];

					if (backtracking->records[i].tick_count + 12 > Interfaces::Globals->tickcount)
					{
					if (Render::WorldToScreen(backtracking->records[i].headPosition, screenbacktrack[i]))
					{

					Interfaces::Surface->DrawSetColor(Color(255, 0, 0, 255));
					Interfaces::Surface->DrawOutlinedRect(screenbacktrack[i].x, screenbacktrack[i].y, screenbacktrack[i].x + 2, screenbacktrack[i].y + 2);

					}
					}
					}*/
					if (hackManager.pLocal()->IsAlive())
					{
						for (int t = 0; t < 12; ++t)
						{
							Vector screenbacktrack[64];

							if (backtracking->records[i].tick_count + 12 > Interfaces::Globals->tickcount)
							{
								if (Render::WorldToScreen(backtracking->records[i].headPosition, screenbacktrack[i]))
								{

									Interfaces::Surface->DrawSetColor(Color(255, 0, 0, 255));
									Interfaces::Surface->DrawOutlinedRect(screenbacktrack[i].x, screenbacktrack[i].y, screenbacktrack[i].x + 2, screenbacktrack[i].y + 2);

								}
							}
						}
					}
					else
					{
						memset(&backtracking->records[0], 0, sizeof(backtracking->records));
					}
				}
			}
		}
	}
}


void DrawAA(Vector view ,bool &bSendPacket)
{
	Vector src3, dst3, f, src, dst;
	trace_t tf;
	Ray_t ray;
	CTraceFilter filter;
	IClientEntity* plocal = hackManager.pLocal();

	filter.pSkip = hackManager.pLocal();
	src3 = QAngle(hackManager.pLocal()->GetEyePosition().x, hackManager.pLocal()->GetEyePosition().y - 63, hackManager.pLocal()->GetEyePosition().z);
	AngleVectors(view, &f);
	dst3 = src3 + (f * 150.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tf);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tf.endpos, dst))
		return;
	if (bSendPacket)
	Render::Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 0, 255));

	char dcdstf[50];
	sprintf(dcdstf, "%f", view.y);
	Render::Text(dst.x, dst.y, Color(255, 0, 0, 255), Render::Fonts::Slider, (dcdstf));
}

void DrawTrace()
{
	
	Vector src3, dst3, r, f, lby, src, dst, rx, fx;
	trace_t treal;
	trace_t tfake;
	trace_t trealx;
	trace_t tfakex;
	trace_t tlby;
	Ray_t ray;
	CTraceFilter filter;
	IClientEntity* plocal = hackManager.pLocal();


	//-------------------------------------------------------------------

	filter.pSkip = hackManager.pLocal();
	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(0, freal, 0), &r);
	dst3 = src3 + (r * 60.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, 0xffffffff, &filter, &treal);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(treal.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 255, 0, 255));
	char charreal[3];
	sprintf(charreal, "%f", freal);
	Render::Text(dst.x, dst.y, Color(0, 255, 0, 255), Render::Fonts::Slider, (charreal));
	
	//-------------------------------------------------------------------

	filter.pSkip = hackManager.pLocal();
	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(0, ffake, 0), &f);
	dst3 = src3 + (f * 60.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, 0xffffffff, &filter, &tfake);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tfake.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 0, 255));
	char charfake[3];
	sprintf(charfake, "%f", ffake);
	Render::Text(dst.x, dst.y, Color(255, 0, 0, 255), Render::Fonts::Slider, (charfake));

	//-------------------------------------------------------------------

	filter.pSkip = hackManager.pLocal();
	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(0, plocal->GetLowerBodyYaw(), 0), &lby);
	dst3 = src3 + (lby * 100.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, 0xffffffff, &filter, &tlby);
	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tlby.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 0, 200, 255));
	char charlby[3];
	sprintf(charlby, "%f", lby);
	Render::Text(dst.x, dst.y, Color(0, 0, 200, 255), Render::Fonts::Slider, (charlby));

	//-------------------------------------------------------------------
	filter.pSkip = hackManager.pLocal();
	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(frealx, 0, 0), &rx);
	dst3 = src3 + (rx * 60.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, 0xffffffff, &filter, &trealx);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(trealx.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 255, 0, 255));
	char charrealx[3];
	sprintf(charrealx, "%f", frealx);
	Render::Text(dst.x, dst.y, Color(0, 255, 0, 255), Render::Fonts::Slider, (charrealx));

	//-------------------------------------------------------------------

	filter.pSkip = hackManager.pLocal();
	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(ffakex, 0, 0), &fx);
	dst3 = src3 + (fx * 60.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, 0xffffffff, &filter, &tfakex);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tfakex.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 0, 255));
	char charfakex[3];
	sprintf(charfakex, "%f", ffakex);
	Render::Text(dst.x, dst.y, Color(255, 0, 0, 255), Render::Fonts::Slider, (charfakex));
	
	
	
	
	/*

	Vector src3, dst3, f, r, l, b, src, dst, df, dr, dl, db;
	trace_t tf;
	trace_t tr;
	trace_t tl;
	trace_t tb;
	trace_t tdf;
	trace_t tdr;
	trace_t tdl;
	trace_t tdb;
	Ray_t ray;
	CTraceFilter filter;
	IClientEntity* plocal = hackManager.pLocal();

	filter.pSkip = hackManager.pLocal();
	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(20, 0, 0), &f);
	dst3 = src3 + (f * 150.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tf);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tf.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 0, 255));

	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(20, -90, 0), &r);
	dst3 = src3 + (r * 150.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tr);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tr.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 0, 255, 255));

	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(20, 90, 0), &l);
	dst3 = src3 + (l * 150.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tl);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tl.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 255, 0, 255));

	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(20, -180, 0), &b);
	dst3 = src3 + (b * 150.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tb);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tb.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 0, 0, 255));

	filter.pSkip = hackManager.pLocal();
	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(20, 45, 0), &df);
	dst3 = src3 + (df * 150.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tdf);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tdf.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(255, 0, 0, 255));

	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(20, -45, 0), &dr);
	dst3 = src3 + (dr * 150.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tdr);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tdr.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 0, 255, 255));

	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(20, 135, 0), &dl);
	dst3 = src3 + (dl * 150.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tdl);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tdl.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 255, 0, 255));

	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(20, -135, 0), &db);
	dst3 = src3 + (db * 150.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tdb);

	if (!Render::WorldToScreen(src3, src) || !Render::WorldToScreen(tdb.endpos, dst))
		return;
	Render::Line(src.x, src.y, dst.x, dst.y, Color(0, 0, 0, 255));

	float dstf = ((tf.endpos - plocal->GetEyePosition()).Length() * 3);
	float dstr = ((tr.endpos - plocal->GetEyePosition()).Length() * 3);
	float dstb = ((tb.endpos - plocal->GetEyePosition()).Length() * 3);
	float dstl = ((tl.endpos - plocal->GetEyePosition()).Length() * 3);

	float dstdf = ((tdf.endpos - plocal->GetEyePosition()).Length() * 3);
	float dstdr = ((tdr.endpos - plocal->GetEyePosition()).Length() * 3);
	float dstdb = ((tdb.endpos - plocal->GetEyePosition()).Length() * 3);
	float dstdl = ((tdl.endpos - plocal->GetEyePosition()).Length() * 3);


	//d

	char dcdstf[50];
	char dcdstr[50];
	char dcdstb[50];
	char dcdstl[50];

	sprintf(dcdstf, "%f", dstdf);
	sprintf(dcdstr, "%f", dstdr);
	sprintf(dcdstb, "%f", dstdb);
	sprintf(dcdstl, "%f", dstdl);


	char cdstf[50];
	char cdstr[50];
	char cdstb[50];
	char cdstl[50];

	sprintf(cdstf, "%f", dstf);
	sprintf(cdstr, "%f", dstr);
	sprintf(cdstb, "%f", dstb);
	sprintf(cdstl, "%f", dstl);




	Render::Text(7, 75, Color(255, 0, 0, 255), Render::Fonts::Slider, (cdstf));
	Render::Text(7, 92, Color(0, 0, 255, 255), Render::Fonts::Slider, (cdstr));
	Render::Text(7, 109, Color(255, 255, 255, 255), Render::Fonts::Slider, (cdstb));
	Render::Text(7, 126, Color(0, 255, 0, 255), Render::Fonts::Slider, (cdstl));
	Render::Text(80, 75, Color(255, 0, 0, 255), Render::Fonts::Slider, (dcdstf));
	Render::Text(80, 92, Color(0, 0, 255, 255), Render::Fonts::Slider, (dcdstr));
	Render::Text(80, 109, Color(255, 255, 255, 255), Render::Fonts::Slider, (dcdstb));
	Render::Text(80, 126, Color(0, 255, 0, 255), Render::Fonts::Slider, (dcdstl));

	char fmove[50];
	char smove[50];

	char realfmove[50];
	char realsmove[50];

	sprintf(fmove, "%f", rffmove);
	sprintf(smove, "%f", rfsmove);
	sprintf(realfmove, "%f", new_forwardmove);
	sprintf(realsmove, "%f", new_sidemove);
	Render::Text(7, 143, Color(0, 0, 0, 255), Render::Fonts::Slider, "pCmd->move");
	Render::Text(7, 160, Color(150, 0, 0, 255), Render::Fonts::Slider, (realfmove));
	Render::Text(7, 177, Color(0, 150, 0, 255), Render::Fonts::Slider, (realsmove));

	*/
}