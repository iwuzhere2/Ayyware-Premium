#import "backtracking.h"

struct BacktrackRecord
{
	CEntity* entity;
	Vector head;
	Vector origin;
};

struct BacktrackTick
{
	int tickcount;
	std::vector<BacktrackRecord> records;
};

class CBacktracking
{
	std::vector<BacktrackTick> ticks;
	CEntity* entity;
	Vector prevOrig;

public:
	void RegisterTick(CUserCmd* cmd)
	{
		ticks.insert(ticks.begin(), BacktrackTick{ cmd->tick_count });
		auto& cur = ticks[0];

		while (ticks.size() > 25)
			ticks.pop_back();

		for (int i = 1; i < Interfaces.Engine->GetMaxClients(); i++)
		{
			CEntity* entity = Interfaces.EntityList->GetClientEntity(i);

			if (!entity ||
				entity->IsDormant() ||
				entity->GetHealth() <= 0 ||
				entity->GetTeam() == CEntity::GetLocalPlayer()->GetTeam() ||
				entity->IsImmune())
				continue;

			cur.records.emplace_back(BacktrackRecord{ entity, entity->GetBonePos(8), entity->GetOrigin() });
		}
	}

	void Begin(CUserCmd* cmd)
	{
		entity = nullptr;
		CEntity* localPlayer = CEntity::GetLocalPlayer();

		float serverTime = localPlayer->GetTickBase() * Interfaces.GlobalVars->interval_per_tick;
		auto weapon = localPlayer->GetWeapon();

		if (cmd->buttons & IN_ATTACK && weapon->GetNextPrimaryAttack() < serverTime + 0.001)
		{
			float fov = 0.5f;
			int tickcount = 0;
			bool hasTarget = false;
			Vector orig;

			for (auto& tick : ticks)
			{
				for (auto& record : tick.records)
				{
					Vector angle = Math.CalcAngle(localPlayer->GetEyeAngles(), record.head);
					float tmpFOV = Math.GetFOV(cmd->viewangles, angle);

					if (tmpFOV < fov)
					{
						fov = tmpFOV;
						tickcount = tick.tickcount;
						hasTarget = true;
						entity = record.entity;
						orig = record.origin;
					}
				}
			}

			if (entity && hasTarget)
			{
				cmd->tick_count = tickcount;
				prevOrig = entity->GetOrigin();
				entity->GetOrigin() = orig;
			}
		}
	}

	void End()
	{
		if (entity)
			entity->GetOrigin() = prevOrig;

		entity = nullptr;
	}

	void Draw()
	{
		for (auto& tick : ticks)
		{
			for (auto& record : tick.records)
			{
				Vector screenPos;
				if (Utils.WorldScreen(record.head, screenPos))
				{
					Drawing.FilledRectangle(screenPos.x, screenPos.y, 4, 4, Color{ 255, 0, 0, 255 });
				}
			}
		}
	}
};