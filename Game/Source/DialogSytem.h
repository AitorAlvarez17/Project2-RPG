#pragma once

#include <map>
#include <string>
#include <functional>

#include "Module.h"

#include "SDL/include/SDL.h"

class DialogNode;

class DialogSystem : public Module {
public:
	DialogSystem();

	// Destructor
	virtual ~DialogSystem();

	// Called before render is available
	bool Awake(pugi::xml_node& config);

	// Called before the first frame
	bool Start();

	// Called each loop iteration
	bool PreUpdate();

	bool Update(float dt);

	bool PostUpdate(float dt);

	void DrawDialog();

	// Called before quitting
	bool CleanUp();

	bool LoadDialog(const char* filename);

	void StartDialog(const char* id);

	void NextDialog();

	DialogNode* currentDialog = nullptr;

	bool aPressed;
	bool downPressed;
	bool upPressed;

private:
	DialogNode* ParseDialogXML(pugi::xml_node dialogRoot);


	std::map<std::string, DialogNode*> dialogues;

	std::map<std::string, DialogNode*> nodeRoutes;

	std::map<std::string, std::function<void()>> callbacks;

	int selectedOption = 0;

	std::string folder;
	pugi::xml_document dialogFile;
	pugi::xml_node optionRoot;
	std::string nameNPC;

	SDL_Texture** dialogFrame;
	SDL_Texture** tavernLady;
	SDL_Texture** reaperSr;
	SDL_Texture** customer;
	SDL_Texture** thyma;
};