#define _CRT_SECURE_NO_WARNINGS

#include "MiscHacks.h"
#include "Interfaces.h"
#include "RenderManager.h"
#include "ESP.h"

#include <time.h>

float rffmove = 0;
float rfsmove = 0;
float new_sidemove;
float new_forwardmove;
int GetItDoubled;

template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}

inline float bitsToFloat(unsigned long i)
{
	return *reinterpret_cast<float*>(&i);
}

inline float FloatNegate(float f)
{
	return bitsToFloat(FloatBits(f) ^ 0x80000000);
}

Vector AutoStrafeView;

void CMiscHacks::Init()
{
}

void CMiscHacks::Draw()
{
	switch (Menu::Window.MiscTab.NameChanger.GetIndex())
	{
	case 0:
		break;
	case 1:
		Namespam();
		break;
	case 2:
		NoName();
		break;
	case 3:
		NameSteal();
		break;
	}

	if (Menu::Window.MiscTab.ChatSpam.GetState())
		ChatSpam();
}

void CMiscHacks::Move(CUserCmd *pCmd, bool &bSendPacket)
{
	IClientEntity *pLocal = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	if (Menu::Window.MiscTab.OtherAutoJump.GetState())
		AutoJump(pCmd);

	Interfaces::Engine->GetViewAngles(AutoStrafeView);
	switch (Menu::Window.MiscTab.OtherAutoStrafe.GetIndex())
	{
	case 0:
		break;
	case 1:
		LegitStrafe(pCmd);
		break;
	case 2:
		RageStrafe(pCmd);
		break;
	}

	if (Menu::Window.MiscTab.OtherCircleStrafe.GetState())
		CircularStrafe(pCmd, pCmd->viewangles);

	if (Menu::Window.MiscTab.FakeWalk.GetKey())
		FakeWalk(pCmd, bSendPacket);

	if (Menu::Window.MiscTab.OtherSlowMotion.GetKey())
		SlowMo(pCmd, bSendPacket);

	if (Menu::Window.MiscTab.AutoPistol.GetState())
		AutoPistol(pCmd);

	if (Menu::Window.VisualsTab.DisablePostProcess.GetState())
		PostProcces();

}

static __declspec(naked) void __cdecl Invoke_NET_SetConVar(void* pfn, const char* cvar, const char* value)
{
	__asm 
	{
		push    ebp
			mov     ebp, esp
			and     esp, 0FFFFFFF8h
			sub     esp, 44h
			push    ebx
			push    esi
			push    edi
			mov     edi, cvar
			mov     esi, value
			jmp     pfn
	}
}

void DECLSPEC_NOINLINE NET_SetConVar(const char* value, const char* cvar)
{
	static DWORD setaddr = Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x8D\x4C\x24\x1C\xE8\x00\x00\x00\x00\x56", "xxxxx????x");
	if (setaddr != 0) 
	{
		void* pvSetConVar = (char*)setaddr;
		Invoke_NET_SetConVar(pvSetConVar, cvar, value);
	}
}

void change_name(const char* name)
{
	if (Interfaces::Engine->IsInGame() && Interfaces::Engine->IsConnected())
		NET_SetConVar(name, "name");
}

void CMiscHacks::AutoPistol(CUserCmd* pCmd)
{
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)Interfaces::EntList->GetClientEntityFromHandle(hackManager.pLocal()->GetActiveWeaponHandle());

	if (pWeapon)
	{
		if (GameUtils::IsBomb(pWeapon))
		{
			return;
		}

		if (!GameUtils::IsNotPistol(pWeapon))
		{
			return;
		}
	}

	static bool WasFiring = false;

	if (GameUtils::IsPistol)
	{
		if (pCmd->buttons & IN_ATTACK)
		{
			if (WasFiring)
			{
				pCmd->buttons &= ~IN_ATTACK;
			}
		}
		WasFiring = pCmd->buttons & IN_ATTACK ? true : false;
	}
}

void CMiscHacks::PostProcces()
{
	ConVar* Meme = Interfaces::CVar->FindVar("mat_postprocess_enable");
	SpoofedConvar* meme_spoofed = new SpoofedConvar(Meme);
	meme_spoofed->SetString("mat_postprocess_enable 0");
}

void CMiscHacks::SvCheats()
{
}

void CMiscHacks::FakeWalk(CUserCmd* pCmd, bool &bSendPacket)
{
	IClientEntity* pLocal = hackManager.pLocal();

	int FakeWalkKey = Menu::Window.MiscTab.FakeWalk.GetKey();
	if (FakeWalkKey > 0 && GUI.GetKeyState(FakeWalkKey))
	{
		static int iChoked = -1;
		iChoked++;

		if (iChoked < 1)
		{
			bSendPacket = false;

			pCmd->tick_count += 10;
			pCmd->command_number += 7 + pCmd->tick_count % 2 ? 0 : 1;

			pCmd->buttons |= pLocal->GetMoveType() == IN_BACK;
			pCmd->forwardmove = pCmd->sidemove = 0.f;
		}
		else
		{
			bSendPacket = true;
			iChoked = -1;

			Interfaces::Globals->frametime *= (pLocal->GetVelocity().Length2D()) / 1.f;
			pCmd->buttons |= pLocal->GetMoveType() == IN_FORWARD;
		}
	}
}

void CMiscHacks::SlowMo(CUserCmd *pCmd, bool &bSendPacket)
{
	int SlowMotionKey = Menu::Window.MiscTab.OtherSlowMotion.GetKey();

	IClientEntity* plocal = hackManager.pLocal();

	if (SlowMotionKey > 0 && GUI.GetKeyState(SlowMotionKey))
	{
		if (plocal->GetFlags() & FL_ONGROUND)
			pCmd->buttons |= IN_DUCK;
	}
}

void CMiscHacks::Namespam()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	static bool wasSpamming = true;

	if (wasSpamming)
	{
		static bool useSpace = true;
		if (useSpace)
		{
			change_name("™AVOZ");
			useSpace = !useSpace;
		}
		else
		{
			change_name("™ AVOZ");
			useSpace = !useSpace;
		}
	}

	start_t = clock();
}

void CMiscHacks::NoName()
{
	change_name("\n­­­");
}

void CMiscHacks::NameSteal()
{
	static clock_t start_t = clock();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < 0.001)
		return;

	std::vector < std::string > Names;

	for (int i = 0; i < Interfaces::EntList->GetHighestEntityIndex(); i++)
	{

		IClientEntity *entity = Interfaces::EntList->GetClientEntity(i);

		player_info_t pInfo;

		if (entity && hackManager.pLocal()->GetTeamNum() == entity->GetTeamNum() && entity != hackManager.pLocal())
		{
			ClientClass* cClass = (ClientClass*)entity->GetClientClass();

			if (cClass->m_ClassID == (int)CSGOClassID::CCSPlayer)
			{
				if (Interfaces::Engine->GetPlayerInfo(i, &pInfo))
				{
					if (!strstr(pInfo.name, "GOTV"))
						Names.push_back(pInfo.name);
				}
			}
		}
	}

	static bool wasSpamming = true;

	int randomIndex = rand() % Names.size();
	char buffer[128];
	sprintf_s(buffer, "%s ", Names[randomIndex].c_str());

	if (wasSpamming)
	{
		change_name(buffer);
	}
	else
	{
		change_name("p$i 1337");
	}

	start_t = clock();
}
	/*Vector src3, dst3, f, r, l, b, src, dst;
	trace_t tf;
	trace_t tr;
	trace_t tl;
	trace_t tb;
	Ray_t ray;
	CTraceFilter filter;
	IClientEntity* plocal = hackManager.pLocal();

	filter.pSkip = hackManager.pLocal();
	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(0, 0, 0), &f);
	dst3 = src3 + (f * 200.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tf);

	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(0, -90, 0), &r);
	dst3 = src3 + (r * 200.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tr);
	
	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(0, 90, 0), &l);
	dst3 = src3 + (l * 200.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tl);

	src3 = hackManager.pLocal()->GetEyePosition();
	AngleVectors(QAngle(0, 180, 0), &b);
	dst3 = src3 + (b * 200.f);
	ray.Init(src3, dst3);
	Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tb);

	float dstf = ((tf.endpos - plocal->GetEyePosition()).Length() / 200.f);
	float dstr = ((tr.endpos - plocal->GetEyePosition()).Length() / 200.f);
	float dstb = ((tb.endpos - plocal->GetEyePosition()).Length() / 200.f);
	float dstl = ((tl.endpos - plocal->GetEyePosition()).Length() / 200.f);*/

	void CMiscHacks::RotateMovement(CUserCmd* pCmd, float rotation)
	{

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

		src3 = hackManager.pLocal()->GetEyePosition();
		AngleVectors(QAngle(20, -90, 0), &r);
		dst3 = src3 + (r * 150.f);
		ray.Init(src3, dst3);
		Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tr);

		src3 = hackManager.pLocal()->GetEyePosition();
		AngleVectors(QAngle(20, 90, 0), &l);
		dst3 = src3 + (l * 150.f);
		ray.Init(src3, dst3);
		Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tl);

		src3 = hackManager.pLocal()->GetEyePosition();
		AngleVectors(QAngle(20, -180, 0), &b);
		dst3 = src3 + (b * 150.f);
		ray.Init(src3, dst3);
		Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tb);

		float dstf = ((tf.endpos - plocal->GetEyePosition()).Length() * 3);
		float dstr = ((tr.endpos - plocal->GetEyePosition()).Length() * 3);
		float dstb = ((tb.endpos - plocal->GetEyePosition()).Length() * 3);
		float dstl = ((tl.endpos - plocal->GetEyePosition()).Length() * 3);
		//diagunal


		filter.pSkip = hackManager.pLocal();
		src3 = hackManager.pLocal()->GetEyePosition();
		AngleVectors(QAngle(20, 45, 0), &df);
		dst3 = src3 + (df * 150.f);
		ray.Init(src3, dst3);
		Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tdf);

		src3 = hackManager.pLocal()->GetEyePosition();
		AngleVectors(QAngle(20, -45, 0), &dr);
		dst3 = src3 + (dr * 150.f);
		ray.Init(src3, dst3);
		Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tdr);

		src3 = hackManager.pLocal()->GetEyePosition();
		AngleVectors(QAngle(20, 135, 0), &dl);
		dst3 = src3 + (dl * 150.f);
		ray.Init(src3, dst3);
		Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tdl);

		src3 = hackManager.pLocal()->GetEyePosition();
		AngleVectors(QAngle(20, -135, 0), &db);
		dst3 = src3 + (db * 150.f);
		ray.Init(src3, dst3);
		Interfaces::Trace->TraceRay(ray, MASK_SOLID, &filter, &tdb);


		float dstdf = ((tdf.endpos - plocal->GetEyePosition()).Length() * 3);
		float dstdr = ((tdr.endpos - plocal->GetEyePosition()).Length() * 3);
		float dstdb = ((tdb.endpos - plocal->GetEyePosition()).Length() * 3);
		float dstdl = ((tdl.endpos - plocal->GetEyePosition()).Length() * 3);

		rotation = DEG2RAD(rotation);

		float cos_rot = cos(rotation);
		float sin_rot = sin(rotation);

		new_forwardmove = (cos_rot * pCmd->forwardmove) - (sin_rot * pCmd->sidemove);
		new_sidemove = (sin_rot * pCmd->forwardmove) + (cos_rot * pCmd->sidemove);

		if (dstf < 440)
		{
			new_forwardmove -= (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
		}
		if (dstr < 440)
		{
			new_sidemove -= (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
		}
		if (dstl < 440)
		{
			new_sidemove += (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
		}
		if (dstb < 440)
		{
			new_forwardmove += (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
		}
		if (dstdf < 440) 
		{
			new_forwardmove -= (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
			new_sidemove += (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
		}
		if (dstdr < 440)
		{
			new_forwardmove -= (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
			new_sidemove += (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
		}
		if (dstdl < 440)
		{
			new_forwardmove += (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
			new_sidemove -= (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
		}
		if (dstdb < 440)
		{
			new_forwardmove += (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
			new_sidemove -= (20 * Menu::Window.MiscTab.OtherCircleSlider.GetValue());
		}
		
		pCmd->forwardmove = new_forwardmove;
		pCmd->sidemove = new_sidemove;


	}

	bool CMiscHacks::CircularStrafe(CUserCmd* pCmd, Vector& OriginalView)
	{
		if (!(Menu::Window.MiscTab.OtherCircleStrafe.GetState()))
			return false;

		IClientEntity* pLocalEntity = Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

		if (!pLocalEntity)
			return false;

		if (!pLocalEntity->IsAlive())
			return false;

		CircleFactor++;
		if (CircleFactor > 360)
			CircleFactor = 0;
		GetItDoubled = Menu::Window.MiscTab.OtherCircleSlider.GetValue() * CircleFactor - Interfaces::Globals->interval_per_tick;

		Vector StoredViewAngles = pCmd->viewangles;

		int CIRCLEKey = Menu::Window.MiscTab.OtherCircleButton.GetKey();

		if (CIRCLEKey > 0 && GUI.GetKeyState(CIRCLEKey))
		{
			pCmd->viewangles = OriginalView;
			RotateMovement(pCmd, GetItDoubled);
		}
		return true;
	}

void CMiscHacks::AutoJump(CUserCmd *pCmd)
{
	if (pCmd->buttons & IN_JUMP && GUI.GetKeyState(VK_SPACE))
	{
		int iFlags = hackManager.pLocal()->GetFlags();
		if (!(iFlags & FL_ONGROUND))
			pCmd->buttons &= ~IN_JUMP;

		if (hackManager.pLocal()->GetVelocity().Length() <= 50)
		{
			pCmd->forwardmove = 450.f;
		}
	}
}

void CMiscHacks::LegitStrafe(CUserCmd *pCmd)
{
	IClientEntity* pLocal = hackManager.pLocal();
	if (!(pLocal->GetFlags() & FL_ONGROUND))
	{
		pCmd->forwardmove = 0.0f;

		if (pCmd->mousedx < 0)
		{
			pCmd->sidemove = -450.0f;
		}
		else if (pCmd->mousedx > 0)
		{
			pCmd->sidemove = 450.0f;
		}
	}
}

void CMiscHacks::RageStrafe(CUserCmd *pCmd)
{

	IClientEntity* pLocal = (IClientEntity*)Interfaces::EntList->GetClientEntity(Interfaces::Engine->GetLocalPlayer());

	static bool bDirection = true;

	static float move = 450.f;
	float s_move = move * 0.5065f;
	static float strafe = pCmd->viewangles.y;
	float rt = pCmd->viewangles.y, rotation;

	if ((pCmd->buttons & IN_JUMP) || !(pLocal->GetFlags() & FL_ONGROUND))
	{

		pCmd->forwardmove = move * 0.015f;
		pCmd->sidemove += (float)(((pCmd->tick_count % 2) * 2) - 1) * s_move;

		if (pCmd->mousedx)
			pCmd->sidemove = (float)clamp(pCmd->mousedx, -1, 1) * s_move;

		rotation = strafe - rt;

		strafe = rt;

		IClientEntity* pLocal = hackManager.pLocal();
		static bool bDirection = true;

		bool bKeysPressed = true;

		if (GUI.GetKeyState(0x41) || GUI.GetKeyState(0x57) || GUI.GetKeyState(0x53) || GUI.GetKeyState(0x44))
			bKeysPressed = false;
		if (pCmd->buttons & IN_ATTACK)
			bKeysPressed = false;

		float flYawBhop = 0.f;

		float sdmw = pCmd->sidemove;
		float fdmw = pCmd->forwardmove;

		static float move = 450.f;
		float s_move = move * 0.5276f;
		static float strafe = pCmd->viewangles.y;

		if (Menu::Window.MiscTab.OtherAutoStrafe.GetIndex() == 2 && !GetAsyncKeyState(VK_RBUTTON))
		{
			if (pLocal->GetVelocity().Length() > 45.f)
			{
				float x = 30.f, y = pLocal->GetVelocity().Length(), z = 0.f, a = 0.f;

				z = x / y;
				z = fabsf(z);

				a = x * z;

				flYawBhop = a;
			}

			if ((GetAsyncKeyState(VK_SPACE) && !(pLocal->GetFlags() & FL_ONGROUND)) && bKeysPressed)
			{

				if (bDirection)
				{
					AutoStrafeView -= flYawBhop;
					GameUtils::NormaliseViewAngle(AutoStrafeView);
					pCmd->sidemove = -450;
					bDirection = false;
				}
				else
				{
					AutoStrafeView += flYawBhop;
					GameUtils::NormaliseViewAngle(AutoStrafeView);
					pCmd->sidemove = 430;
					bDirection = true;
				}

				if (pCmd->mousedx < 0)
				{
					pCmd->forwardmove = 22;
					pCmd->sidemove = -450;
				}

				if (pCmd->mousedx > 0)
				{
					pCmd->forwardmove = +22;
					pCmd->sidemove = 450;
				}
			}
		}
	}
}

Vector GetAutostrafeView()
{
	return AutoStrafeView;
}

void CMiscHacks::ChatSpam()
{
	static clock_t start_t = clock();
	int spamtime = Menu::Window.MiscTab.OtherChatDelay.GetValue();
	double timeSoFar = (double)(clock() - start_t) / CLOCKS_PER_SEC;
	if (timeSoFar < spamtime)
		return;

	static bool holzed = true;

	if (Menu::Window.MiscTab.OtherTeamChat.GetState())
	{
		SayInTeamChat("avoz - Private CSGO Cheat");
	}
	else
	{
		SayInChat("avoz - Private CSGO Cheat");
	}

	start_t = clock();
}
