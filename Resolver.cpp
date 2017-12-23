#include "Resolver.h"
#include "Ragebot.h"
#include "Hooks.h"

int i = 0;

void LowerBodyYawFix(IClientEntity* pEntity)
{
	if (Menu::Window.RageBotTab.LowerbodyFix.GetState())
	{
		if (!pEntity) return;
		if (pEntity->GetClientClass()->m_ClassID != (int)CSGOClassID::CCSPlayer) return;
		if (!pEntity->IsAlive() || !pEntity->GetActiveWeaponHandle()) return;
		if (Interfaces::Engine->GetLocalPlayer()) return;

		auto EyeAngles = pEntity->GetEyeAnglesXY();
		if (pEntity->GetVelocity().Length() > 36 && (pEntity->GetFlags() & (int)pEntity->GetFlags() & FL_ONGROUND))
			EyeAngles->y = pEntity->GetLowerBodyYaw();
	}
}

void PitchCorrection()
{
	CUserCmd* pCmd;
	for (int i = 0; i < Interfaces::Engine->GetMaxClients(); ++i)
	{
		IClientEntity* pLocal = hackManager.pLocal();
		IClientEntity *player = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

		if (!player || player->IsDormant() || player->GetHealth() < 1 || (DWORD)player == (DWORD)pLocal)
			continue;

		if (!player)
			continue;

		if (pLocal)
			continue;

		if (pLocal && player && pLocal->IsAlive())
		{
			if (Menu::Window.RageBotTab.AdvancedResolver.GetState())
			{
				Vector* eyeAngles = player->GetEyeAnglesXY();
				if (eyeAngles->x < -179.f) eyeAngles->x += 360.f;
				else if (eyeAngles->x > 90.0 || eyeAngles->x < -90.0) eyeAngles->x = 89.f;
				else if (eyeAngles->x > 89.0 && eyeAngles->x < 91.0) eyeAngles->x -= 90.f;
				else if (eyeAngles->x > 179.0 && eyeAngles->x < 181.0) eyeAngles->x -= 180;
				else if (eyeAngles->x > -179.0 && eyeAngles->x < -181.0) eyeAngles->x += 180;
				else if (fabs(eyeAngles->x) == 0) eyeAngles->x = std::copysign(89.0f, eyeAngles->x);
			}
		}
	}
}

// Resolver addon -------------------------------
float Differences[65];

//Used to tell if a player is fake heading, but not using an "lby breaker"
bool HasFakeHead(IClientEntity* pEntity) {
	//lby should update if distance from lby to eye angles exceeds 35 degrees
	return abs(pEntity->GetEyeAnglesXY()->y - pEntity->GetLowerBodyYaw()) > 35;
}

//Used to tell if a player is moving
bool IsMovingOnGround(IClientEntity* pEntity) {
	//Check if player has a velocity greater than 0 (moving) and if they are onground.
	return pEntity->GetVelocity().Length2D() > 0.1f && pEntity->GetFlags() & (int)FL_ONGROUND;
}

//Used to tell if a player is fake walking
bool IsFakeWalking(IClientEntity* pEntity) {
	//Check if a player is moving, but at below a velocity of 36
	return IsMovingOnGround(pEntity) && pEntity->GetVelocity().Length2D() < 36.0f;
}

// Resolver ---------------------------------
static float lby1 = 0.0f;
static float lby2 = 0.0f;
static float lby3 = 0.0f;
static float lby4 = 0.0f;
static float lby5 = 0.0f;
static float lby6 = 0.0f;
static float lby7 = 0.0f;
static float lby8 = 0.0f;


void ResolverSetup::Resolve(IClientEntity* pEntity)
{
	int brutetick = 0;
	bool MeetsLBYReq;
	if (pEntity->GetFlags() & FL_ONGROUND)
		MeetsLBYReq = true;
	else
		MeetsLBYReq = false;

	bool IsMoving;
	if (pEntity->GetVelocity().Length2D() >= 0.5)
		IsMoving = true;
	else
		IsMoving = false;

	
	ResolverSetup::resolvedangle[pEntity->GetIndex()];
	ResolverSetup::resolvedangle2[pEntity->GetIndex()];
	ResolverSetup::resolvedangle3[pEntity->GetIndex()];
	ResolverSetup::resolvedangle4[pEntity->GetIndex()];
	ResolverSetup::resolvedangle5[pEntity->GetIndex()];
	ResolverSetup::resolvedangle6[pEntity->GetIndex()];
	ResolverSetup::resolvedangle7[pEntity->GetIndex()];
	ResolverSetup::resolvedangle8[pEntity->GetIndex()];
	ResolverSetup::NewANgles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResolverSetup::newlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::newsimtime = pEntity->GetSimulationTime();
	ResolverSetup::newdelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::newlbydelta[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::finaldelta[pEntity->GetIndex()] = ResolverSetup::newdelta[pEntity->GetIndex()] - ResolverSetup::storeddelta[pEntity->GetIndex()];
	ResolverSetup::finallbydelta[pEntity->GetIndex()] = ResolverSetup::newlbydelta[pEntity->GetIndex()] - ResolverSetup::storedlbydelta[pEntity->GetIndex()];
	ResolverSetup::lbydif[pEntity->GetIndex()] = ResolverSetup::newlby[pEntity->GetIndex()] - ResolverSetup::storedlby[pEntity->GetIndex()];
	
	if (newlby == storedlby)
		ResolverSetup::lbyupdated = false;
	else
		ResolverSetup::lbyupdated = true;
		 

	if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 0)
	{
		pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
	}
	else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 1)//level 1
	{
		if (lbyupdated)
		{
			if (lby1 == 0.0f)
				lby1 = newlby[pEntity->GetIndex()];
			if (lby1 != 0.0f && lby2 == 0.0f)
				lby2 = newlby[pEntity->GetIndex()] + 90;
			if (lby2 != 0.0f && lby3 == 0.0f)
				lby3 = newlby[pEntity->GetIndex()] - 180;
			if (lby3 != 0.0f && lby4 == 0.0f)
				lby4 = newlby[pEntity->GetIndex()] - 90;
			if (lby4 != 0.0f && lby5 == 0.0f)
				lby5 = newlby[pEntity->GetIndex()] + 45;
			if (lby5 != 0.0f && lby6 == 0.0f)
				lby6 = newlby[pEntity->GetIndex()] - 90;
			if (lby6 != 0.0f && lby7 == 0.0f)
				lby7 = newlby[pEntity->GetIndex()] + 180;
			if (lby7 != 0.0f && lby8 == 0.0f)
				lby8 = newlby[pEntity->GetIndex()] - 90;

			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
		}
			switch ((Globals::Shots) % 8)
			{
			case 0:	
				if (resolvedangle[pEntity->GetIndex()] = 0.0f)
					pEntity->GetEyeAnglesXY()->y = lby1;
				else
					pEntity->GetEyeAnglesXY()->y = resolvedangle[pEntity->GetIndex()];
				if (didhitHS)
					resolvedangle[pEntity->GetIndex()] = lby1;
				break;
			case 1:
				if (resolvedangle2[pEntity->GetIndex()] = 0.0f)
					pEntity->GetEyeAnglesXY()->y = lby2;
				else
					pEntity->GetEyeAnglesXY()->y = resolvedangle2[pEntity->GetIndex()];
				if (didhitHS)
					resolvedangle2[pEntity->GetIndex()] = lby2;
				break;
			case 2:
				if (resolvedangle3[pEntity->GetIndex()] = 0.0f)
					pEntity->GetEyeAnglesXY()->y = lby3;
				else
					pEntity->GetEyeAnglesXY()->y = resolvedangle3[pEntity->GetIndex()];
				if (didhitHS)
					resolvedangle3[pEntity->GetIndex()] = lby3;
				break;
			case 3:
				if (resolvedangle4[pEntity->GetIndex()] = 0.0f)
					pEntity->GetEyeAnglesXY()->y = lby4;
				else
					pEntity->GetEyeAnglesXY()->y = resolvedangle4[pEntity->GetIndex()];
				if (didhitHS)
					resolvedangle4[pEntity->GetIndex()] = lby4;
				break;
			case 4:
				if (resolvedangle5[pEntity->GetIndex()] = 0.0f)
					pEntity->GetEyeAnglesXY()->y = lby5;
				else
					pEntity->GetEyeAnglesXY()->y = resolvedangle5[pEntity->GetIndex()];
				if (didhitHS)
					resolvedangle5[pEntity->GetIndex()] = lby5;
				break;
			case 5:
				if (resolvedangle6[pEntity->GetIndex()] = 0.0f)
					pEntity->GetEyeAnglesXY()->y = lby6;
				else
					pEntity->GetEyeAnglesXY()->y = resolvedangle6[pEntity->GetIndex()];
				if (didhitHS)
					resolvedangle6[pEntity->GetIndex()] = lby6;
				break;
			case 6:
				if (resolvedangle7[pEntity->GetIndex()] = 0.0f)
					pEntity->GetEyeAnglesXY()->y = lby7;
				else
					pEntity->GetEyeAnglesXY()->y = resolvedangle7[pEntity->GetIndex()];
				if (didhitHS)
					resolvedangle7[pEntity->GetIndex()] = lby7;
				break;
			case 7:
				if (resolvedangle8[pEntity->GetIndex()] = 0.0f)
					pEntity->GetEyeAnglesXY()->y = lby8;
				else
					pEntity->GetEyeAnglesXY()->y = resolvedangle8[pEntity->GetIndex()];
				if (didhitHS)
					resolvedangle8[pEntity->GetIndex()] = lby8;
				break;
			}
		if (lby8 != 0.0f)
		{
			lby1 = 0.0f;
				lby2 = 0.0f;
				lby3 = 0.0f;
				lby4 = 0.0f;
				lby5 = 0.0f;
				lby6 = 0.0f;
				lby7 = 0.0f;
				lby8 = 0.0f;
		}
	}
	else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 2) //level 2 
	{
		for (int i = 1; i < 20; ++i) {
			static float OldLowerBodyYaws[64];
			static float OldYawDeltas[64];
			float CurYaw = pEntity->GetLowerBodyYaw();
			if (OldLowerBodyYaws[i] != CurYaw) {
				OldYawDeltas[i] = pEntity->GetEyeAnglesXY()->y - CurYaw;
				OldLowerBodyYaws[i] = CurYaw;
				pEntity->GetEyeAnglesXY()->y = CurYaw;
				continue;
			}
			else {
				pEntity->GetEyeAnglesXY()->y = pEntity->GetEyeAnglesXY()->y - OldYawDeltas[i];
			}
		}
	}
	else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 3)//level 3
	{
		if (IsMovingOnGround(pEntity) && !IsFakeWalking(pEntity)) {
			if (HasFakeHead(pEntity))
				Differences[pEntity->GetIndex()] = (pEntity->GetLowerBodyYaw() - pEntity->GetEyeAnglesXY()->y);
			else Differences[pEntity->GetIndex()] = -1337;
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
		}
		else {
			if (IsFakeWalking(pEntity) || Differences[pEntity->GetIndex()] == -1337)
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 180;
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - Differences[pEntity->GetIndex()];
		}
	}
	else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 4)//level 4
	{

		switch ((Globals::Shots) % 9)
		{
		case 0:
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 180;
			break;
		case 1: 
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
			break;
		case 2:
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 45;
			break;
		case 3:
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 90;
			break;
		case 4:
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 135;
			break;
		case 5:
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 180;
			break;
		case 6:
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 135;
			break;
		case 7:
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 90;
			break;
		case 8:
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 45;
			break;
		}
	}
	//LowerBodyYawFix(pEntity);
	PitchCorrection();
}

void ResolverSetup::StoreFGE(IClientEntity* pEntity)
{
	ResolverSetup::storedanglesFGE = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::storedlbyFGE = pEntity->GetLowerBodyYaw();
	ResolverSetup::storedsimtimeFGE = pEntity->GetSimulationTime();
}

void ResolverSetup::StoreThings(IClientEntity* pEntity)
{
	ResolverSetup::StoredAngles[pEntity->GetIndex()] = *pEntity->GetEyeAnglesXY();
	ResolverSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::storedsimtime = pEntity->GetSimulationTime();
	ResolverSetup::storeddelta[pEntity->GetIndex()] = pEntity->GetEyeAnglesXY()->y;
	ResolverSetup::storedlby[pEntity->GetIndex()] = pEntity->GetLowerBodyYaw();
	ResolverSetup::resolvedangle[pEntity->GetIndex()];
	ResolverSetup::resolvedangle2[pEntity->GetIndex()];
	ResolverSetup::resolvedangle3[pEntity->GetIndex()];
	ResolverSetup::resolvedangle4[pEntity->GetIndex()];
	ResolverSetup::resolvedangle5[pEntity->GetIndex()];
	ResolverSetup::resolvedangle6[pEntity->GetIndex()];
	ResolverSetup::resolvedangle7[pEntity->GetIndex()];
	ResolverSetup::resolvedangle8[pEntity->GetIndex()];
}

void ResolverSetup::CM(IClientEntity* pEntity)
{
	for (int x = 1; x < Interfaces::Engine->GetMaxClients(); x++)
	{

		pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(x);

		if (!pEntity
			|| pEntity == hackManager.pLocal()
			|| pEntity->IsDormant()
			|| !pEntity->IsAlive())
			continue;

		ResolverSetup::StoreThings(pEntity);
	}
}

void ResolverSetup::FSN(IClientEntity* pEntity, ClientFrameStage_t stage)
{
	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < Interfaces::Engine->GetMaxClients(); i++)
		{

			pEntity = (IClientEntity*)Interfaces::EntList->GetClientEntity(i);

			if (!pEntity
				|| pEntity == hackManager.pLocal()
				|| pEntity->IsDormant()
				|| !pEntity->IsAlive())
				continue;

			ResolverSetup::Resolve(pEntity);
		}
	}
}