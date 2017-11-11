#include "Resolver.h"
#include "Ragebot.h"
#include "Hooks.h"

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


void ResolverSetup::Resolve(IClientEntity* pEntity)
{
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

	int i = 0;

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

	}
	else if (Menu::Window.RageBotTab.AimbotResolver.GetIndex() == 1)//level 1
	{
		if (i == 0 || lbyupdated)
		{
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
			i++;
		}
		if (i == 1 && !lbyupdated)
		{
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 35.f;
			i++;
		}
		if (i == 2 && !lbyupdated)
		{
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 105.f;
			i++;
		}
		if (i == 3)
		{
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 140.f;
			i++;
		}
		if (i == 4)
		{
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 210.f;
			i = 0;
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
		if (IsMovingOnGround(pEntity) && !IsFakeWalking(pEntity)) {
			pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw();
		}
		else 
		{
			if (ResolverSetup::lbydif[pEntity->GetIndex()] >= 20.f)
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 30.f;
			}
			else if (ResolverSetup::lbydif[pEntity->GetIndex()] >= 70.f)
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 90.f;
			}
			else if (ResolverSetup::lbydif[pEntity->GetIndex()] >= 125.f)
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 130.f;
			}
			else if (ResolverSetup::lbydif[pEntity->GetIndex()] >= 170.f)
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() - 180.f;
			}
			else if (ResolverSetup::lbydif[pEntity->GetIndex()] <= -170.f)
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 30.f;
			}
			else if (ResolverSetup::lbydif[pEntity->GetIndex()] <= -125.f)
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 130.f;
			}
			else if (ResolverSetup::lbydif[pEntity->GetIndex()] <= -70.f)
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 90.f;
			}
			else if (ResolverSetup::lbydif[pEntity->GetIndex()] <= -20.f)
			{
				pEntity->GetEyeAnglesXY()->y = pEntity->GetLowerBodyYaw() + 30.f;
			}
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