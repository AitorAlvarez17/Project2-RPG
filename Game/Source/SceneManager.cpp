#include "App.h"
#include "Input.h"
#include "Textures.h"
#include "Audio.h"
#include "Render.h"
#include "Window.h"
#include "Map.h"
#include "SceneManager.h"
#include "Scene.h"
#include "ECS.h"
#include "Components.h"
#include "GuiControl.h"
#include "GuiManager.h"
#include "QuestManager.h"
#include "BattleManager.h"
#include "ItemManager.h"
#include "Audio.h"
#include "SceneTransitionSystem.h"
#include "GuiCheckBox.h"
#include "GuiSlider.h"
#include "ItemManager.h"
#include "PartyManager.h"

#include <Windows.h>
#include <iostream>
#include <conio.h>
using namespace std;

#include "Defs.h"
#include "Log.h"


#include "SDL_mixer/include/SDL_mixer.h"


SceneManager::SceneManager() : Module()
{
	name.Create("scene");
}

// Destructor
SceneManager::~SceneManager()
{}

// Called before render is available
bool SceneManager::Awake()
{
	LOG("Loading Scene");
	bool ret = true;

	return ret;
}

// Called before the first frame
bool SceneManager::Start()
{
	//MapScene* s = new MapScene("Town.tmx");

	LogoScene* s = new LogoScene();
	menuTex = app->tex->Load("Textures/UI/MainPauseMenu/pause_menu.png");
	optionsTex = app->tex->Load("Textures/UI/OptionsMenu/options_menu.png");
	controlsMenuTex = app->tex->Load("Textures/UI/OptionsMenu/controls_menu.png");
	questMenuTex = app->tex->Load("Textures/UI/HUD/quest_menu.png");
	invMenuThyma = app->tex->Load("Textures/UI/HUD/charactermenu_thyma.png");
	invMenuToisto = app->tex->Load("Textures/UI/HUD/charactermenu_toisto.png");
	hudIngame = app->tex->Load("Textures/UI/HUD/ingame_hud.png");
	hudIngame1 = app->tex->Load("Textures/UI/HUD/ingame_hud1.png");
	hudIngame2 = app->tex->Load("Textures/UI/HUD/ingame_hud2.png");
	hudIngame3 = app->tex->Load("Textures/UI/HUD/ingame_hud3.png");
	hudIngame4 = app->tex->Load("Textures/UI/HUD/ingame_hud4.png");
	hudIngame5 = app->tex->Load("Textures/UI/HUD/ingame_hud5.png");
	hudIngame6 = app->tex->Load("Textures/UI/HUD/ingame_hud6.png");
	hudIngame7 = app->tex->Load("Textures/UI/HUD/ingame_hud7.png");
	hudIngame8 = app->tex->Load("Textures/UI/HUD/ingame_hud8.png");

	audioMenuTex = app->tex->Load("Textures/UI/OptionsMenu/audio_menu.png");
	graphicsMenuTex = app->tex->Load("Textures/UI/OptionsMenu/graphics_menu.png");

	app->audio->songToBeLoaded = "Audio/Music/Originals/town_main.wav";

	sceneToBeLoaded = (Scene*)s;
	playerPositionToBeLoaded = fPoint(30, 250);

	startPressed = true;

	POINT m;
	GetCursorPos(&m);

	mousePosition.x = m.x;
	mousePosition.y = m.y;

	sCreated = false;
	ResetInitialPositions();
	return true;
}

// Called each loop iteration
bool SceneManager::PreUpdate()
{
	return true;
}

// Called each loop iteration
bool SceneManager::Update(float dt)
{
	if (sceneToBeLoaded != nullptr)
	{

		if (currentScene != nullptr && currentScene->type == Scene::TYPE::MAP)
		{
			if (isFinished == false)
			{
				alpha += speed * dt;


				if (alpha >= 255)
				{
					isFinished = true;
				}


				if (app->volume > 0 && app->volumeUp == false)
				{
					app->volume -= volSpeed * dt;
					app->volumeDown = true;

				}
				//LOG("%f", app->volume);
				if (app->volume == 0.0f)
				{
					app->volume = 0.0f;
					app->volumeDown = false;
				}

				app->UpdateVolume();
			}
			else
			{
				if (sceneToBeLoaded->type == Scene::TYPE::MAP)
					LoadScene((MapScene*)sceneToBeLoaded, playerPositionToBeLoaded);
				else
					LoadScene(sceneToBeLoaded);
				sceneToBeLoaded = nullptr;
				isFinished = false;
				alpha = 0;
			}
		}
		else
		{
			if (sceneToBeLoaded->type == Scene::TYPE::MAP)
				LoadScene((MapScene*)sceneToBeLoaded, playerPositionToBeLoaded);
			else
				LoadScene(sceneToBeLoaded);
			sceneToBeLoaded = nullptr;
			isFinished = false;

		}

	}

	changingScene = false;

	return true;
}

// Called each loop iteration
bool SceneManager::PostUpdate(float dt)
{
	bool ret = true;
	if (app->scene->currentScene->type == Scene::TYPE::MAP)
	{

		int maximumHP = app->party->allyParty->FindByName("Thyma")->data.GetMaxHealth();
		int health = app->party->allyParty->FindByName("Thyma")->data.GetHealth();
		int resultHP = (100 * health) / maximumHP;

		if (resultHP > 85)
			app->render->DrawTexture(hudIngame8, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);//BLUE

		if (resultHP <= 85)
			app->render->DrawTexture(hudIngame8, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);// LIGHT BLUE

		if (resultHP <= 75)
			app->render->DrawTexture(hudIngame7, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);//DARK GREEN

		if (resultHP <= 67)
			app->render->DrawTexture(hudIngame6, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);//GREEN

		if (resultHP <= 60)
			app->render->DrawTexture(hudIngame5, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);//LIGHT GREEN

		if (resultHP <= 50)
			app->render->DrawTexture(hudIngame4, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);//YELLOW

		if (resultHP <= 40)
			app->render->DrawTexture(hudIngame3, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);//LIGHT GREEN

		if (resultHP <= 30)
			app->render->DrawTexture(hudIngame2, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);//ORANGE

		if (resultHP <= 20)
			app->render->DrawTexture(hudIngame1, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);//DARK ORANGE

		if (resultHP <= 10)
			app->render->DrawTexture(hudIngame, 0, 0, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);//DARK RED

	}


	GamePad& pad = app->input->pads[0];

	currentScene->world->tick(dt);

	SDL_Rect fullscreen;
	fullscreen.x = 0;
	fullscreen.y = 0;
	uint w, h;
	app->win->GetWindowSize(w, h);
	fullscreen.w = w;
	fullscreen.h = h;

	int squares = 16;
	int squareSize = fullscreen.w / squares;

	for (int i = 0; i < squares; i++) {
		for (int j = 0; j < squares; j++) {
			SDL_Rect rect;
			float prop = min(alpha, 255.0f) / 255.0f;
			if ((i - j) % 2 == 0)
				rect = SDL_Rect({ i * squareSize, j * squareSize, squareSize, int(std::round(squareSize * prop)) });
			else
				rect = SDL_Rect({ i * squareSize, j * squareSize + int(std::round(squareSize * (1 - prop))), squareSize, int(std::round(squareSize * prop)) });
			app->render->DrawRectangle(rect, 0, 0, 0, 255, true, false);
		}
	}

	if ((app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN || pad.start == true) && !optionsMenu && app->dialog->currentDialog == nullptr && !app->battle->isBattling && !app->quests->questInvOpened && !app->items->invOpened && startPressed && currentScene->type == Scene::TYPE::MAP)
	{
		if (menu)
		{
			ResetInitialPositions();
			sCreated = false;
		}
		app->audio->PlayFx(8, 0);
		if (startPressed)menu = !menu;

		startPressed = false;
	}

	if (pad.b == true && bPressed)
	{
		if (menu && !optionsMenu && app->dialog->currentDialog == nullptr && !app->battle->isBattling && !app->quests->questInvOpened && !app->items->invOpened)
		{
			if (menu == true)
			{
				ResetInitialPositions();
				sCreated = false;
			}
			app->audio->PlayFx(8, 0);
			menu = false;
			bPressed = false;
		}
	}

	if (pad.start == false) startPressed = true;
	if (pad.b == false) bPressed = true;

	if (app->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)
	{
		app->RequestSave();
		LOG("saving");
	}

	if (app->input->GetKey(SDL_SCANCODE_L) == KEY_DOWN)
	{
		app->RequestLoad();
		LOG("loading");
	}
	if (menu || currentScene->type == Scene::TYPE::MENU || optionsMenu || app->quests->questInvOpened || app->items->invOpened) {


		float dtSpeed = padSpeed * dt;

		//LOG("padx: %f pady: %f", pad.l_x, pad.l_y);

		if (pad.l_x > 0.6f || pad.right)
		{
			SetCursorPos(mousePosition.x, mousePosition.y);
			mousePosition.x += dtSpeed;
		}
		if (pad.l_x < -0.6f || pad.left)
		{
			SetCursorPos(mousePosition.x, mousePosition.y);
			mousePosition.x -= dtSpeed;
		}
		if (pad.l_y < -0.6f || pad.up)
		{
			SetCursorPos(mousePosition.x, mousePosition.y);
			mousePosition.y -= dtSpeed;
		}
		if (pad.l_y > 0.6f || pad.down)
		{
			SetCursorPos(mousePosition.x, mousePosition.y);
			mousePosition.y += dtSpeed;
		}
	}

	if (currentScene->type == Scene::TYPE::MENU && !optionsMenu && !app->quests->questInvOpened)
	{
		if (!buttons)
		{
			app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ -300, 148, 120, 32 }), 0, true, 247, 500, SplineType::BACK); //start
			app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ -300, 201, 120, 32 }), 1, true, 247, 600, SplineType::BACK); //continue
			app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ -300, 256, 120, 32 }), 2, true, 247, 700, SplineType::BACK); //options
			app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ -300, 311, 120, 32 }), 3, true, 247, 800, SplineType::BACK); //exit
			buttons = true;
			padSpeed = 400;
		}
	}

	if (currentScene->type != Scene::TYPE::MENU && currentScene->type != Scene::TYPE::LOGO && currentScene->type != Scene::TYPE::BATTLE) {

		padSpeed = 600;

		if (app->quests->questInvOpened)
		{
			if (sCreated == false)
			{
				pos = &questMenuInitPos;
				app->easing->CreateSpline(pos, 0, 600, SplineType::BACK);
				sCreated = true;
			}
			app->render->DrawTexture(questMenuTex, 0, -*pos, nullptr, .5f, 0.0f, 0.0f, INT_MAX, INT_MAX, false);
		}
		if (app->items->invOpened)
		{
			if (sCreated == false)
			{
				pos = &invMenuInitPos;
				app->easing->CreateSpline(pos, 0, 600, SplineType::BACK);
				sCreated = true;
			}
			if (app->items->partyMember)
				app->render->DrawTexture(invMenuToisto, 0, -*pos, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);
			if (!app->items->partyMember)
				app->render->DrawTexture(invMenuThyma, 0, -*pos, &SDL_Rect({ 0,0,1280,720 }), 0.5f, 1, 0, 0, 0, false);


		}
		if (menu)
		{

			if (!optionsMenu)
			{

				if (sCreated == false)
				{
					pos = &pauseMenuInitPos;
					app->easing->CreateSpline(pos, 0, 300, SplineType::BACK);
					sCreated = true;
				}

				app->render->DrawTexture(menuTex, 0, *pos, nullptr, .5f, 0.0f, 0.0f, INT_MAX, INT_MAX, false);

				if (buttons == false)
				{
					app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 260, 1000, 120, 32 }), 4, true, 82, 450, SplineType::BACK, false);//continue
					app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 260, 1000, 120, 32 }), 5, true, 271 / 2, 550, SplineType::BACK, false);//save
					app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 260, 1000, 120, 32 }), 6, true, 381 / 2, 650, SplineType::BACK, false);//load
					app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 260, 1000, 120, 32 }), 2, true, 491 / 2, 750, SplineType::BACK, false);//options
					app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 260, 1000, 120, 32 }), 7, true, 601 / 2, 850, SplineType::BACK, false);//back to menu
					buttons = true;
				}

				if (app->volume > 25)
				{
					app->volumeDown = true;
					app->volume -= volSpeed * 1.25f * dt;
				}

				if (app->volume < 25)
				{
					app->volumeDown = false;
					app->volume = 25;
				}

				Mix_VolumeMusic(app->volume);
			}
		}

		if (menu == false && !app->quests->questInvOpened)
		{
			if (app->volume < 100 && app->volumeDown == false)
			{
				app->volumeUp = true;
				app->volume += volSpeed * dt;
			}
			if (app->volume <= 0.0f)
			{
				app->volumeDown = false;
			}
			if (app->volume > 100)
			{
				app->volume = 100;
				app->volumeUp = false;
			}


			Mix_VolumeMusic(app->volume);
			if (buttons)app->ui->DestroyAllGuiControls();
			buttons = false;
		}
	}

	if (optionsMenu)
	{
		if (!pad.r1) r1Pressed = true;
		if (!pad.l1) l1Pressed = true;

		if (app->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN || (pad.b && bPressed))
		{
			app->audio->PlayFx(6, 0);
			optionsMenu = false;
			audioSelected = false;
			controlsSelected = false;
			graphicsSelected = false;
			app->ui->DestroyAllGuiControls();
			bPressed = false;
			buttons = false;
			ResetInitialPositions();
			sCreated = false;
		}

		if (sCreated == false)
		{
			pos = &pauseMenuInitPos;
			app->easing->CreateSpline(pos, 0, 300, SplineType::BACK);
			sCreated = true;
		}


		if (graphicsSelected)
		{
			app->render->DrawTexture(optionsTex, -*pos, 0, nullptr, .5f, 0.0f, 0.0f, INT_MAX, INT_MAX, false);
			app->render->DrawTexture(graphicsMenuTex, -*pos, 0, nullptr, .5f, 0.0f, 0.0f, INT_MAX, INT_MAX, false);

			if (buttons == false)
			{

				app->ui->CreateGuiControl(GuiControlType::CHECKBOX, SDL_Rect({ -1000, 323 / 2, 183 / 2, 50 / 2 }), 15, true, 728 / 2, 300, SplineType::BACK); //fullscreen checkbox
				GuiControl* c = app->ui->CreateGuiControl(GuiControlType::CHECKBOX, SDL_Rect({ -1000, 462 / 2, 183 / 2, 50 / 2 }), 16, true, 728 / 2, 300, SplineType::BACK); //vsync checkbox
				((GuiCheckBox*)c)->checked = true;
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ -1000, 152 / 2, 340 / 2, 65 / 2 }), 11, true, 140 / 2, 300, SplineType::BACK); //graphics button
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ -1000, 152 / 2, 340 / 2, 65 / 2 }), 12, true, 468 / 2, 300, SplineType::BACK); //audio button
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ -1000, 152 / 2, 340 / 2, 65 / 2 }), 13, true, 798 / 2, 300, SplineType::BACK); //controls button
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ -1000, 34 / 2, 60 / 2, 60 / 2 }), 14, true, 58 / 2, 300, SplineType::BACK); //back button

				buttons = true;
			}
			if (pad.r1 && r1Pressed)
			{
				app->ui->DestroyAllGuiControls();
				audioSelected = true;
				graphicsSelected = false;
				controlsSelected = false;
				buttons = false;
				r1Pressed = false;
				ResetInitialPositions();
				sCreated = false;
			}
			if (pad.l1 && l1Pressed)
			{
				app->ui->DestroyAllGuiControls();
				audioSelected = false;
				graphicsSelected = false;
				controlsSelected = true;
				buttons = false;
				l1Pressed = false;
				ResetInitialPositions();
				sCreated = false;
			}
		}

		if (audioSelected)
		{
			app->render->DrawTexture(optionsTex, 0, *pos - 1, nullptr, .5f, 0.0f, 0.0f, INT_MAX, INT_MAX, false);
			app->render->DrawTexture(audioMenuTex, 0, *pos, nullptr, .5f, 0.0f, 0.0f, INT_MAX, INT_MAX, false);

			if (buttons == false)
			{
				app->ui->CreateGuiControl(GuiControlType::SLIDER, SDL_Rect({ 715 / 2, 1000, 300 / 2, 30 / 2 }), 8, true, 330 / 2, 300, SplineType::BACK, false); //general volume slider
				app->ui->CreateGuiControl(GuiControlType::SLIDER, SDL_Rect({ 715 / 2, 1000, 300 / 2, 30 / 2 }), 9, true, 412 / 2, 300, SplineType::BACK, false); //music volume slider
				app->ui->CreateGuiControl(GuiControlType::SLIDER, SDL_Rect({ 715 / 2, 1000, 300 / 2, 30 / 2 }), 10, true, 494 / 2, 300, SplineType::BACK, false); //fx volume slider
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 140 / 2, 1000, 340 / 2, 65 / 2 }), 11, true, 152 / 2, 300, SplineType::BACK, false); //graphics button
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 468 / 2, 1000, 340 / 2, 65 / 2 }), 12, true, 152 / 2, 300, SplineType::BACK, false); //audio button
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 798 / 2, 1000, 340 / 2, 65 / 2 }), 13, true, 152 / 2, 300, SplineType::BACK, false); //controls button
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 58 / 2, 1000, 60 / 2, 60 / 2 }), 14, true, 34 / 2, 300, SplineType::BACK, false); //back button

				buttons = true;
			}
			if (pad.r1 == true && r1Pressed)
			{
				app->ui->DestroyAllGuiControls();
				audioSelected = false;
				graphicsSelected = false;
				controlsSelected = true;
				buttons = false;
				r1Pressed = false;
				ResetInitialPositions();
				sCreated = false;
			}
			if (pad.l1 && l1Pressed)
			{
				app->ui->DestroyAllGuiControls();
				audioSelected = false;
				graphicsSelected = true;
				controlsSelected = false;
				buttons = false;
				l1Pressed = false;
				ResetInitialPositions();
				sCreated = false;
			}
		}

		if (controlsSelected)
		{
			app->render->DrawTexture(optionsTex, *pos, 0, nullptr, .5f, 0.0f, 0.0f, INT_MAX, INT_MAX, false);
			app->render->DrawTexture(controlsMenuTex, *pos, 0, nullptr, .5f, 0.0f, 0.0f, INT_MAX, INT_MAX, false);

			if (buttons == false)
			{
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 1000, 152 / 2, 340 / 2, 65 / 2 }), 11, true, 140 / 2, 300, SplineType::BACK); //graphics button
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 1000, 152 / 2, 340 / 2, 65 / 2 }), 12, true, 468 / 2, 300, SplineType::BACK); //audio button
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 1000, 152 / 2, 340 / 2, 65 / 2 }), 13, true, 798 / 2, 300, SplineType::BACK); //controls button
				app->ui->CreateGuiControl(GuiControlType::BUTTON, SDL_Rect({ 1000, 34 / 2, 60 / 2, 60 / 2 }), 14, true, 58 / 2, 300, SplineType::BACK); //back button

				buttons = true;
			}
			if (pad.l1 == true && l1Pressed)
			{
				app->ui->DestroyAllGuiControls();
				audioSelected = true;
				graphicsSelected = false;
				controlsSelected = false;
				buttons = false;
				l1Pressed = false;
				ResetInitialPositions();
				sCreated = false;
			}
			if (pad.r1 && r1Pressed)
			{
				app->ui->DestroyAllGuiControls();
				audioSelected = false;
				graphicsSelected = true;
				controlsSelected = false;
				buttons = false;
				r1Pressed = false;
				ResetInitialPositions();
				sCreated = false;
			}
		}

	}
	return ret;
}

// Called before quitting
bool SceneManager::CleanUp()
{
	if (currentScene == nullptr) return true;
	currentScene->CleanUp();

	delete currentScene;

	return true;
}

void SceneManager::LoadScene(Scene* scene)
{
	CleanUp();
	currentScene = scene;
	currentScene->Load();
}

void SceneManager::LoadScene(MapScene* scene, fPoint playerPosition)
{
	CleanUp();
	currentScene = (Scene*)scene;
	scene->Load(playerPosition);
}

bool SceneManager::Load(pugi::xml_node& savedGame)
{
	//passar la escena a LoadScene

	pugi::xml_node sceneNode = savedGame.child("currentScene");
	const char* string = sceneNode.attribute("name").as_string();

	pugi::xml_node entityNode = savedGame.child("entity");
	pugi::xml_node positionNode = entityNode.child("position");

	MapScene* newS = new MapScene(string);

	sceneToBeLoaded = (Scene*)newS;
	playerPositionToBeLoaded = fPoint(positionNode.attribute("x").as_int(), positionNode.attribute("y").as_int());

	currentScene->world->all([&](ECS::Entity* ent)
		{
			ECS::ComponentHandle<Position> position = ent->get<Position>();
			if (position.isValid())
			{
				position->position.x = positionNode.attribute("x").as_int();
				position->position.y = positionNode.attribute("y").as_int();
			}
			// do something with ent
		});

	return true;
}

bool SceneManager::Save(pugi::xml_node& savedGame)
{
	LOG("scene saved");
	pugi::xml_node sceneNode = savedGame.append_child("currentScene");
	pugi::xml_attribute currentSceneAtt = sceneNode.append_attribute("name");

	if (currentScene->type == Scene::TYPE::MAP)
	{
		MapScene* mapScene = (MapScene*)currentScene;
		currentSceneAtt.set_value(mapScene->filename);
	}

	pugi::xml_node entityNode = savedGame.append_child("entity");
	pugi::xml_node positionNode = entityNode.append_child("position");

	currentScene->world->all([&](ECS::Entity* ent)
		{
			ECS::ComponentHandle<Position> position = ent->get<Position>();
			if (position.isValid())
			{
				pugi::xml_attribute positionEntityx = positionNode.append_attribute("x");
				positionEntityx.set_value(position->position.x);
				pugi::xml_attribute positionEntityy = positionNode.append_attribute("y");
				positionEntityy.set_value(position->position.y);
			}
			// do something with ent
		});
	//guardar entitats etc
	return true;
}

bool SceneManager::OnGuiMouseClickEvent(GuiControl* control)
{
	Scene* s;
	GuiSlider* slider;

	switch (control->id)
	{
	case 0: //start button
		app->ui->DestroyAllGuiControls();
		s = (Scene*)(new MapScene("Town.tmx"));
		app->scene->sceneToBeLoaded = s;
		app->scene->playerPositionToBeLoaded = fPoint(30, 250);
		break;
	case 1: //continue button
		app->ui->DestroyAllGuiControls();
		app->RequestLoad();
		app->scene->menu = 0;
		break;
	case 2: //options button
		app->ui->DestroyAllGuiControls();
		buttons = false;
		optionsMenu = true;
		audioSelected = true;
		ResetInitialPositions();
		sCreated = false; break;
	case 3: //exit
		return false;
		break;
	case 4: //continue (exit menu) button
		app->ui->DestroyAllGuiControls();
		menu = 0;
		ResetInitialPositions();
		sCreated = false;
		break;
	case 5: //save
		app->RequestSave();

		break;
	case 6: //load
		app->ui->DestroyAllGuiControls();
		app->RequestLoad();
		app->scene->menu = 0;

		break;
	case 7: //back to menu
		app->ui->DestroyAllGuiControls();
		s = (Scene*)(new MenuScene());
		sceneToBeLoaded = s;
		menu = false;
		ResetInitialPositions();
		sCreated = false;
		break;
	case 8:
		slider = (GuiSlider*)control;
		app->volume = slider->value;
		app->UpdateVolume();
		break;
	case 9:
		slider = (GuiSlider*)control;
		app->volumeMusic = slider->value;
		app->UpdateVolume();
		break;
	case 10:
		slider = (GuiSlider*)control;
		app->volumeFx = slider->value;
		app->UpdateVolume();
		break;
	case 11: //graphics window button
		app->ui->DestroyAllGuiControls();
		graphicsSelected = true;
		audioSelected = false;
		controlsSelected = false;
		ResetInitialPositions();
		buttons = false;
		sCreated = false;
		break;
	case 12: //audio window button
		app->ui->DestroyAllGuiControls();
		audioSelected = true;
		graphicsSelected = false;
		controlsSelected = false;
		ResetInitialPositions();
		buttons = false;
		sCreated = false;
		break;
	case 13: //controls window button
		app->ui->DestroyAllGuiControls();
		controlsSelected = true;
		audioSelected = false;
		graphicsSelected = false;
		ResetInitialPositions();
		buttons = false;
		sCreated = false;
		break;
	case 14: //back button
		app->ui->DestroyAllGuiControls();
		optionsMenu = false;
		audioSelected = false;
		controlsSelected = false;
		graphicsSelected = false;
		buttons = false;
		app->items->invOpened = false;
		app->quests->questInvOpened = false;
		ResetInitialPositions();
		sCreated = false;
		break;
	case 15: //fullscreen checkbox
		app->win->ToggleFullscreen();
		break;
	case 16:
		app->render->SwitchVsync();
		break;
	case 19:
		//app->items->useItem = true;
		break;
	case 20:
		break;

	default:
		break;
	}
	//8,9,10 are audio sliders

	return true;
}

void SceneManager::CheckSpline(int position, int finaPos, float time)
{
	if (sCreated == false)
	{
		pos = &position;
		app->easing->CreateSpline(pos, finaPos, time, SplineType::BACK);
		sCreated = true;
	}
}

void SceneManager::ResetInitialPositions()
{
	pauseMenuInitPos = 800;
	optionsMenuInitPos = 1000;
	questMenuInitPos = 1000;
	invMenuInitPos = 1000;
	battleMenuInitPos = 1000;
}