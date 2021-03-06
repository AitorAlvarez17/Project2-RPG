#pragma once

#include "Module.h"
#include "QuestManager.h"
#include <string>
#include <vector>
#include "QuestManager.h"

#include "SDL/include/SDL.h"

struct Member;

class BattleManager : public Module
{

public:

	enum class Selecting {
		MEMBER,
		ACTION,
		TARGET
	};

	BattleManager();
	~BattleManager();

	bool Awake();
	bool Start();
	bool PreUpdate();
	bool Update(float dt);
	bool PostUpdate(float dt);
	bool CleanUp();

	void StartBattle();
	void EndBattle();

	void DoAction();
	void CheckBattleEnd();
	void CheckAndAdvanceQuests();

	void WaitTime(float time);

	void PlayAITurn();

	bool isBattling = false;

private:
	void Draw();
	SDL_Texture** characterBar = nullptr;
	SDL_Texture** actionBox = nullptr;
	SDL_Texture** healthBars = nullptr;
	SDL_Texture** selectionArrowHorizontal = nullptr;
	SDL_Texture** selectionArrows = nullptr;

	char currentParty = 0;
	char currentMember = 0;
	char currentAction = 0;
	char currentTarget = 0;

	std::vector<Member*> targets;

	Selecting selecting = Selecting::MEMBER;

	ListItem<Quest*>* activeQuestsList;
	//ListItem<Quest*>* C;

	float pendingWaitTime = 0;

	bool aPressed = true;
	bool downPressed = true;
	bool upPressed = true;

	bool end = false;
	float endWait = 0;
};