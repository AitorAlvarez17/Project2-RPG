#include "App.h"
#include "Window.h"
#include "Input.h"
#include "Render.h"
#include "Textures.h"
#include "Audio.h"
#include "Easing.h"
#include "SceneManager.h"
#include "QuestManager.h"
#include "DialogSytem.h"
#include "Map.h"
#include "Fonts.h"
#include "GuiManager.h"
#include "Debug.h"
#include "PartyManager.h"
#include "BattleManager.h"
#include "ItemManager.h"
#include "AssetsManager.h"
#include "PuzzleManager.h"
#include "ParticleSystem.h"

#include "Defs.h"
#include "Log.h"

#include <iostream>
#include <math.h>
#include <sstream>
#include <stdlib.h>

#include "SDL_mixer/include/SDL_mixer.h"

// Constructor
App::App(int argc, char* args[]) : argc(argc), args(args)
{
	win = new Window();
	input = new Input();
	render = new Render();
	assetsManager = new AssetsManager();
	tex = new Textures();
	audio = new Audio();
	easing = new Easing();
	quests = new QuestManager();
	dialog = new DialogSystem();
	map = new Map();
	scene = new SceneManager();
	fonts = new Fonts();
	ui = new GuiManager();
	debug = new Debug();
	party = new PartyManager();
	battle = new BattleManager();
	items = new ItemManager();
	puzzleManager = new PuzzleManager();
	particleSystem = new ParticleSystem();

	// Ordered for awake / Start / Update
	// Reverse order of CleanUp
	AddModule(win);
	AddModule(input);
	AddModule(tex);
	AddModule(assetsManager);
	AddModule(audio);
	AddModule(easing);
	AddModule(map);
	AddModule(scene);
	AddModule(dialog);
	AddModule(fonts);
	AddModule(particleSystem);
	AddModule(ui);
	AddModule(party);
	AddModule(quests);
	AddModule(items);
	AddModule(debug);
	AddModule(battle);
	AddModule(puzzleManager);

	// Render last to swap buffer
	AddModule(render);

	srand(time(NULL));
}

// Destructor
App::~App()
{
	// Release modules
	ListItem<Module*>* item = modules.end;

	while(item != NULL)
	{
		RELEASE(item->data);
		item = item->prev;
	}

	modules.Clear();
}

void App::AddModule(Module* module)
{
	module->Init();
	modules.Add(module);
}

// Called before render is available
bool App::Awake()
{
	// TODO 3: Load config from XML
	bool ret = LoadConfig();

	if(ret == true)
	{
		// TODO 4: Read the title from the config file
		title.Create(configApp.child("title").child_value());
		win->SetTitle(title.GetString());

		ListItem<Module*>* item;
		item = modules.start;

		while(item != NULL && ret == true)
		{
			// TODO 5: Add a new argument to the Awake method to receive a pointer to an xml node.
			// If the section with the module name exists in config.xml, fill the pointer with the valid xml_node
			// that can be used to read all variables for that module.
			// Send nullptr if the node does not exist in config.xml
			ret = item->data->Awake(config.child(item->data->name.GetString()));
			item = item->next;
		}
	}

	saveFileName = configApp.child("savefile").attribute("path").as_string();

	return ret;
}

// Called before the first frame
bool App::Start()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.start;

	while(item != NULL && ret == true)
	{
		ret = item->data->Start();
		item = item->next;
	}

	minTime = 1.0f / frameCap;

	return ret;
}

// Called each loop iteration
bool App::Update()
{
	bool ret = true;
	PrepareUpdate();

	if(input->GetWindowEvent(WE_QUIT) == true)
		ret = false;

	if(ret == true)
		ret = PreUpdate();

	if(ret == true)
		ret = DoUpdate();

	if(ret == true)
		ret = PostUpdate();

	FinishUpdate();
	return ret;
}

// Load config from XML file
bool App::LoadConfig()
{
	bool ret = true;

	// TODO 3: Load config.xml file using load_file() method from the xml_document class
	pugi::xml_parse_result result = configFile.load_file("config.xml");

	// TODO 3: Check result for loading errors
	if(result == NULL)
	{
		LOG("Could not load map xml file config.xml. pugi error: %s", result.description());
		ret = false;
	}
	else
	{
		config = configFile.child("config");
		configApp = config.child("app");
	}

	return ret;
}

// ---------------------------------------------
void App::PrepareUpdate()
{
	dt = timer.ReadSec();
	timer.Start();
}

// ---------------------------------------------
void App::FinishUpdate()
{
	// This is a good place to call Load / Save functions
	if (requestLoad == true)
	{
		Load();
	}

	if (requestSave == true)
	{
		Save();
	}

	//LOG("%f\n", 1.0f / std::max(dt, minTime));

	if (dt < minTime)
	{
		SDL_Delay(minTime - dt);
	}
}

// Call modules before each loop iteration
bool App::PreUpdate()
{
	bool ret = true;

	ListItem<Module*>* item;
	Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->PreUpdate();
	}

	return ret;
}

// Call modules on each loop iteration
bool App::DoUpdate()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.start;
	Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->Update(dt);
	}

	return ret;
}

// Call modules after each loop iteration
bool App::PostUpdate()
{
	bool ret = true;
	ListItem<Module*>* item;
	Module* pModule = NULL;

	for(item = modules.start; item != NULL && ret == true; item = item->next)
	{
		pModule = item->data;

		if(pModule->active == false) {
			continue;
		}

		ret = item->data->PostUpdate(dt);
	}

	return ret;
}

void App::UpdateVolume()
{
	float general = app->volume / 100.0f;
	float music = app->volumeMusic / 100.0f * general;
	float fx = app->volumeFx / 100.0f * general;
	Mix_VolumeMusic(music * MIX_MAX_VOLUME);
	Mix_Volume(-1, fx * MIX_MAX_VOLUME);
}

// Called before quitting
bool App::CleanUp()
{
	bool ret = true;
	ListItem<Module*>* item;
	item = modules.end;

	while(item != NULL && ret == true)
	{
		ret = item->data->CleanUp();
		item = item->prev;
	}

	return ret;
}

// ---------------------------------------
int App::GetArgc() const
{
	return argc;
}

// ---------------------------------------
const char* App::GetArgv(int index) const
{
	if(index < argc)
		return args[index];
	else
		return NULL;
}

// ---------------------------------------
const char* App::GetTitle() const
{
	return title.GetString();
}

// ---------------------------------------
const char* App::GetOrganization() const
{
	return organization.GetString();
}

bool App::Load()
{

	bool ret = true;

	pugi::xml_document saveGame;

	pugi::xml_parse_result result = saveGame.load_file(saveFileName);

	if (result == NULL)
	{
		LOG("Could not load map xml file config.xml. pugi error: %s", result.description());
		ret = false;
	}
	else
	{
		//renderer
		pugi::xml_node rend = saveGame.child("renderer");
		if (rend == NULL)
		{
			LOG("Renderer not loading");
		}

		//scene
		pugi::xml_node sce = saveGame.child("scene");
		if (sce == NULL)
		{
			LOG("Scene not loading");
		}


		app->render->Load(rend);
		app->scene->Load(sce);

	}

	requestLoad = false;

	return ret;
}

bool App::Save()
{
	bool ret = true;
	requestSave = false;
	saving = true;

	LOG("saved");
	//app->scene->continueButtonDisabled = false;

	pugi::xml_document newSave;

	pugi::xml_node rend = newSave.append_child("renderer");
	app->render->Save(rend);

	pugi::xml_node sce = newSave.append_child("scene");
	app->scene->Save(sce);




	newSave.save_file(saveFileName);

	return ret;
}


