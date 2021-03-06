#include "BattleManager.h"

#include "App.h"
#include "SceneManager.h"
#include "QuestManager.h"
#include "Quest.h"
#include "Scene.h"
#include "PartyManager.h"
#include "ParticleSystem.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Input.h"
#include "Fonts.h"
#include <cmath>

#include <algorithm>
#include <string>
#include <random>

BattleManager::BattleManager()
{
}

BattleManager::~BattleManager()
{
}

bool BattleManager::Awake()
{
	return true;
}

bool BattleManager::Start()
{
	activeQuestsList = app->quests->questsActive.start;
	characterBar = app->tex->Load("Textures/UI/BattleMenu/character_bar.png");
	actionBox = app->tex->Load("Textures/UI/BattleMenu/action_box.png");
	healthBars = app->tex->Load("Textures/UI/BattleMenu/health_bars.png");
	selectionArrowHorizontal = app->tex->Load("Textures/UI/BattleMenu/selection_arrow_horizontal.png");
	selectionArrows = app->tex->Load("Textures/UI/BattleMenu/selection_arrows.png");
	
	//C = app->quests->questsActive.start;
	return true;
}

bool BattleManager::PreUpdate()
{
	activeQuestsList = app->quests->questsActive.start;
	return true;
}

bool BattleManager::Update(float dt)
{
	
	if (!isBattling) return true;
	if (app->input->pads[0].a == false) aPressed = true;
	if (app->input->pads[0].l_y == 0.0f && app->input->pads[0].down == false) downPressed = true;
	if (app->input->pads[0].l_y == 0.0f && app->input->pads[0].up == false) upPressed = true;

	if (end)
	{
		endWait -= dt;

		if (endWait <= 0)
			EndBattle();

		return true;
	}

	Party* p;
	Party* o;

	if (currentParty == 0)
	{
		p = app->party->allyParty;
		o = app->party->enemyParty;
	}
	else
	{
		p = app->party->enemyParty;
		o = app->party->allyParty;
	}

	if (currentParty != 0)
	{
		if (p->list.at(currentMember)->data.dead)
		{
			while (activeQuestsList != nullptr)
			{
				if (p->list.at(currentMember)->name == activeQuestsList->data->objective.GetString())
				{
					if (activeQuestsList->data->progress < activeQuestsList->data->quantity)
					{
						activeQuestsList->data->progress += 3;
						if (activeQuestsList->data->progress >= activeQuestsList->data->quantity)
						{
							activeQuestsList->data->progress = activeQuestsList->data->quantity;
						}
					}
				}
				activeQuestsList = activeQuestsList->next;

			}
		}
	}
	else
	{
		if (o->list.at(currentMember)->data.dead)
		{
			while (activeQuestsList != nullptr)
			{
				if (o->list.at(currentMember)->name == activeQuestsList->data->objective.GetString())
				{
					if (activeQuestsList->data->progress < activeQuestsList->data->quantity)
					{
						activeQuestsList->data->progress += 3;
						if (activeQuestsList->data->progress >= activeQuestsList->data->quantity)
						{
							activeQuestsList->data->progress = activeQuestsList->data->quantity;
						}
					}
				}
				activeQuestsList = activeQuestsList->next;

			}
		}
	}
	

	while (currentMember < p->list.size() && p->list.at(currentMember)->data.dead)
		currentMember += 1;

	if (currentMember == p->list.size())
	{
		end = true;
		return true;
	}

	if (pendingWaitTime > 0)
	{
		pendingWaitTime -= dt;
		return true;
	}

	if ((app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN || (app->input->pads[0].a == true && aPressed)) && currentParty == 0)
	{
		aPressed = false;

		if (selecting == Selecting::MEMBER)
			selecting = Selecting::ACTION;
		else if (selecting == Selecting::ACTION)
		{
			app->audio->PlayFx(6, 0);
			selecting = Selecting::TARGET;

			Member* m = p->list.at(currentMember);
			Action* a = m->data.actions.at(currentAction);

			if (a->filter == Action::Filter::ENEMY)
			{
				for (int i = 0; i < o->list.size(); i++)
				{
					if (!o->list.at(i)->data.dead)
						targets.push_back(o->list.at(i));
				}
			}
			else if (a->filter == Action::Filter::ALLY)
			{
				for (int i = 0; i < p->list.size(); i++)
				{
					if (!p->list.at(i)->data.dead)
						targets.push_back(p->list.at(i));
				}
			}
		}
		else if (selecting == Selecting::TARGET)
		{
			DoAction();
			WaitTime(2.0f);
		}
	}

	if ((app->input->GetKey(SDL_SCANCODE_DOWN) == KEY_DOWN || (app->input->pads[0].l_y > 0.0f || app->input->pads[0].down == true) && downPressed) && currentParty == 0)
	{
		downPressed = false;
		if (selecting == Selecting::ACTION)
		{
			app->audio->PlayFx(8, 0);
			currentAction -= 1;
			if (currentAction < 0) currentAction = 0;
		}
		if (selecting == Selecting::TARGET)
		{
			app->audio->PlayFx(8, 0);
			currentTarget += 1;
			if (currentTarget == targets.size()) currentTarget -= 1;
		}
	}

	if ((app->input->GetKey(SDL_SCANCODE_UP) == KEY_DOWN || (app->input->pads[0].l_y < 0.0f || app->input->pads[0].up == true) && upPressed) && currentParty == 0)
	{
		upPressed = false;
		if (selecting == Selecting::ACTION)
		{
			app->audio->PlayFx(8, 0);
			currentAction += 1;
			if (currentAction == p->list.at(currentMember)->data.actions.size()) currentAction -= 1;
		}
		if (selecting == Selecting::TARGET)
		{
			app->audio->PlayFx(8, 0);
			currentTarget -= 1;
			if (currentTarget < 0) currentTarget = 0;
		}
	}

	if (app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) selecting = Selecting::ACTION;

	if (currentParty == 1 && pendingWaitTime <= 0)
	{
		PlayAITurn();
		WaitTime(2.0f);
	}

	return true;
}

bool BattleManager::PostUpdate(float dt)
{
	Draw();
	return true;
}

bool BattleManager::CleanUp()
{
	return true;
}

void BattleManager::StartBattle()
{
	app->party->InitializeEnemyParties();

	isBattling = true;
	currentParty = 0;
	currentMember = 0;
	selecting = Selecting::ACTION;

	end = false;
	endWait = 4.0f;

	app->scene->ResetInitialPositions();
	app->scene->pos = &app->scene->battleMenuInitPos;
	app->easing->CreateSpline(app->scene->pos, 0, 300, SplineType::BACK);

}

void BattleManager::EndBattle()
{
	isBattling = false;
	MapScene* s = new MapScene("Forest.tmx");
	app->scene->sceneToBeLoaded = (Scene*)s;
	app->scene->playerPositionToBeLoaded.x = 944;
	app->scene->playerPositionToBeLoaded.y = 240;

	app->party->allyParty->FindByName("Thyma")->data.AddExp(5 * app->party->allyParty->FindByName("Thyma")->data.level);
	app->party->allyParty->FindByName("Toisto")->data.AddExp(5 * app->party->allyParty->FindByName("Toisto")->data.level);

	app->party->RemoveParties();

	end = false;
}

void BattleManager::DoAction()
{
	Party* p;
	Party* o;

	if (currentParty == 0)
	{
		p = app->party->allyParty;
		o = app->party->enemyParty;
	}
	else
	{
		p = app->party->enemyParty;
		o = app->party->allyParty;
	}

	Member* m = p->list.at(currentMember);
	Action* a = m->data.actions.at(currentAction);

	if (currentAction == 0) app->audio->PlayFx(9, 0);
	if (currentAction == 1) app->audio->PlayFx(10, 0);

	fPoint pos;
	if (currentParty == 0) {
		if (a->type == Action::Type::ATTACK)
		{
			pos = fPoint(512.0f, 52 + currentTarget * 80.5f) * 5;
		}
		else {
			pos = fPoint(72.0f, 82 + currentTarget * 80.5f) * 5;
		}
	}
	else {
		if (a->type == Action::Type::ATTACK)
		{
			pos = fPoint(72.0f, 82 + currentTarget * 80.5f) * 5;
		}
		else {
			pos = fPoint(512.0f, 52 + currentTarget * 80.5f) * 5;
		}
	}

	Member* t = nullptr;
	if (!targets.empty()) {
		t = targets.at(currentTarget);
		if (a->type == Action::Type::ATTACK) {
			app->scene->currentScene->slashes.Add(app->particleSystem->AddEmitter(pos, EmitterData::EmitterType::SLASH));
		}
		else {
			app->scene->currentScene->heals.Add(app->particleSystem->AddEmitter(pos, EmitterData::EmitterType::HEAL));
		}
	}
	else
	{
		if (a->type == Action::Type::ATTACK)
		{
			t = app->party->allyParty->list.at(currentTarget);
			app->scene->currentScene->slashes.Add(app->particleSystem->AddEmitter(pos, EmitterData::EmitterType::SLASH));
		}
		else if (a->type == Action::Type::DEFENSE)
		{
			t = app->party->enemyParty->list.at(currentTarget);
			app->scene->currentScene->heals.Add(app->particleSystem->AddEmitter(pos, EmitterData::EmitterType::HEAL));
		}
	}

	a->Apply(t);

	currentMember += 1;
	currentAction = 0;
	currentTarget = 0;

	targets.clear();

	selecting = Selecting::ACTION;

	if (currentMember == p->list.size())
	{
		if (currentParty == 0) currentParty = 1;
		else currentParty = 0;
		currentMember = 0;
	}

	CheckBattleEnd();
}

void BattleManager::CheckBattleEnd()
{
	Party* p;
	Party* o;

	if (currentParty == 0)
	{
		p = app->party->allyParty;
		o = app->party->enemyParty;
	}
	else
	{
		p = app->party->enemyParty;
		o = app->party->allyParty;
	}

	bool A = true;

	for (int i = 0; i < p->list.size(); i++)
	{
		if (!p->list.at(i)->data.dead) A = false;
	}

	bool B = true;

	for (int i = 0; i < o->list.size(); i++)
	{
		if (!o->list.at(i)->data.dead) B = false;
	}

	if (A || B) end = true;

}

void BattleManager::WaitTime(float time)
{
	pendingWaitTime = time;
}

void BattleManager::PlayAITurn()
{
	struct ProbabilityAction {
		int action;
		int target;
		float probability;
	};

	Party* party = app->party->enemyParty;
	Member* member = party->list.at(currentMember);
	std::vector<Action*> actions = member->data.actions;

	std::vector<ProbabilityAction*>* probActions = new std::vector<ProbabilityAction*>();

	float accuProb = 0.0f;

	for (int i = 0; i < actions.size(); i++)
	{
		Action* a = actions.at(i);
		if (a->type == Action::Type::ATTACK)
		{
			Party* allyParty = app->party->allyParty;
			for (int j = 0; j < allyParty->list.size(); j++)
			{
				if (allyParty->list.at(j)->data.dead)
					continue;
				float prob = allyParty->list.at(j)->data.health / allyParty->list.at(j)->data.maxHealth;
				prob = prob * prob;
				accuProb += prob;
				ProbabilityAction* p = new ProbabilityAction();
				p->action = i;
				p->probability = accuProb;
				p->target = j;
				probActions->push_back(p);
			}
		}
		else if (a->type == Action::Type::DEFENSE)
		{
			for (int j = 0; j < party->list.size(); j++)
			{
				if (party->list.at(j)->data.dead)
					continue;
				float prob = 1.0f - (party->list.at(j)->data.health / party->list.at(j)->data.maxHealth);
				prob = prob * prob;
				accuProb += prob;
				ProbabilityAction* p = new ProbabilityAction();
				p->action = i;
				p->probability = accuProb;
				p->target = j;
				probActions->push_back(p);
			}
		}
	}

	float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	r *= accuProb;

	for (int i = 0; i < probActions->size(); i++) {
		if (r < probActions->at(i)->probability) {
			currentAction = probActions->at(i)->action;
			currentTarget = probActions->at(i)->target;
			DoAction();
			break;
		}
	}

	for (int i = 0; i < probActions->size(); i++) {
		delete probActions->at(i);
	}
	delete probActions;
}

void BattleManager::Draw()
{
	if (!isBattling) return;

	Party* party = app->party->allyParty;

	for (int i = 0; i < party->list.size(); i++) {
		int x = 70;
		uint w, h;
		app->win->GetWindowSize(w, h);
		int y = h / 2 - 45 - 38 * i;
		app->render->DrawTexture(characterBar, (-*app->scene->pos) + x, y, NULL, 0.5f);
		std::string name = party->list.at(i)->name;
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);
		app->fonts->BlitText((-*app->scene->pos) + x + 30, y + 20, 1, name.c_str());
		int p = 10 - std::floor(party->list.at(i)->data.health / party->list.at(i)->data.maxHealth * 11.0f);
		if (p < 0) p = 0;
		if (p > 10) p = 10;
		SDL_Rect section = SDL_Rect({ 0, 30 * p, 300, 30 });
		app->render->DrawTexture(healthBars, (-*app->scene->pos) + x + 250, y + 15, &section, 0.5f);

		if (i == currentMember && currentParty == 0)
		{
			app->render->DrawTexture(selectionArrows, (-*app->scene->pos) + 70, 30 + i * 80.5f, &SDL_Rect({ 104, 0, 52, 57 }), .5f);

			std::vector<Action*>* actions = &party->list.at(i)->data.actions;
			for (int j = 0; j < actions->size(); j++) {
				Action* a = actions->at(j);

				x = 90;
				y = h / 2 - 45 - 38 * (party->list.size() - 1) - 32 - j * 25;
				app->render->DrawTexture(actionBox, (-*app->scene->pos) + x, y, NULL, .5f);
				std::transform(a->name.begin(), a->name.end(), a->name.begin(), ::toupper);
				app->fonts->BlitText((-*app->scene->pos) + x + 25, y + 13, 1, a->name.c_str());

				if (j == currentAction) {
					if(selecting != Selecting::TARGET) app->render->DrawTexture(selectionArrowHorizontal, (-*app->scene->pos) + x + 80, y + 12, NULL, 0.3f);
				}
			}
		}
	}

	if (selecting == Selecting::TARGET && currentParty == 0) {
		for (int i = 0; i < party->list.size(); i++) {
			if (party->list.at(i) == targets.at(currentTarget)) {
				app->render->DrawTexture(selectionArrows, 70, 30 + i * 80.5f, &SDL_Rect({ 0, 0, 52, 57 }), .5f);
			}
		}

		party = app->party->enemyParty;

		for (int i = 0; i < party->list.size(); i++) {
			if (party->list.at(i) == targets.at(currentTarget)) {
				app->render->DrawTexture(selectionArrows, 495, -5 + i * 80.5f, &SDL_Rect({ 52, 0, 52, 57 }), .5f);
			}
		}
	}

	party = app->party->enemyParty;

	for (int i = 0; i < party->list.size(); i++) {
		Member* m = party->list.at(i);

		int p = 10 - std::floor(m->data.health / m->data.maxHealth * 11.0f);
		if (p == -1) p = 0;
		if (p > 10) p = 10;
		SDL_Rect section = SDL_Rect({ 0, 30 * p, 300, 30 });

		app->render->DrawTexture(healthBars, (*app->scene->pos) + 480.0f, 20 + i * 80.5f, &section, 0.25f);
	}
}
