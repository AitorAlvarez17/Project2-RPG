#ifndef __GUIMANAGER_H__
#define __GUIMANAGER_H__

#include "Module.h"
#include "GuiControl.h"

#include "List.h"

struct SDL_Texture;

class GuiManager : public Module
{
public:

	GuiManager();

	// Destructor
	virtual ~GuiManager();

	// Called before render is available
	bool Awake(pugi::xml_node&);

	// Called before the first frame
	bool Start();

	bool Update(float dt);

	bool PostUpdate(float dt);

	// Called before quitting
	bool CleanUp();

	// Additional methods
	GuiControl* CreateGuiControl(GuiControlType type, SDL_Rect bounds, int id, bool anim = false, int fPos = 0, int time = 0, SplineType esType = SplineType::NONE, bool horizontal = true, Uint8 r = 255, Uint8 g = 255, Uint8 b = 255, Uint8 a = 255, int sliderValue = 0);
	void DestroyGuiControl(GuiControl* entity);

	void DestroyAllGuiControls();

	void AddGuiControl(GuiControl* entity);

	bool UpdateAll(float dt);

	void DrawAll();

public:

	List<GuiControl*> controls;

	SDL_Texture** menuButtonTex;
	SDL_Texture** optionsButtonsTex;
	SDL_Texture** questButtonTex;
	SDL_Texture** invButtonTex;

	SDL_Texture** buttonTex;
	SDL_Texture** sliderTex;
	SDL_Texture** checkBoxTex;
	SDL_Texture** backButtonTex;
	SDL_Texture** miniatureTex;

	SDL_Texture** startTex;
	SDL_Texture** continueTex;
	SDL_Texture** optionsTex;
	SDL_Texture** exitTex;
	SDL_Texture** saveTex;
	SDL_Texture** loadTex;
	SDL_Texture** mainMenuTex;
	SDL_Texture** graphicsTex;
	SDL_Texture** audioTex;
	SDL_Texture** controlsTex;
	SDL_Texture** onOffTex;
	SDL_Texture** backArrowTex;
	SDL_Texture** pixelTex;
	SDL_Texture** itemTex;
	SDL_Texture** useItemText;
	SDL_Texture** discardItemText;
	SDL_Texture** cancelItemText;
	SDL_Texture** puzzlePassTex;
	

	int lastId = 0;
};

#endif // __GUIMANAGER_H__
