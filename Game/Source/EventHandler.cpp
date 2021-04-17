#include "EventHandler.h"
#include "App.h"
#include "Audio.h"
#include "Components.h"
#include "ECS.h"
#include "Scene.h"
#include "SceneManager.h"

#include <string>

void EventHandler::FireEvent(MapEvent* e)
{
	std::string type = e->attributes->at("type");

	if (type == "changeArea")
	{
		/*app->scene->currentScene->world->all([&](ECS::Entity* ent)
		{
			ECS::ComponentHandle<KeyboardMovement> speed = ent->get<KeyboardMovement>();
			if (speed.isValid())
			{
				speed = 0;
			}
		});*/

		MapScene* s = new MapScene(e->attributes->at("target").c_str());

		int playerX = std::stoi(e->attributes->at("playerX"));
		int playerY = std::stoi(e->attributes->at("playerY"));

		app->scene->sceneToBeLoaded = (Scene*)s;
		std::string str = s->filename;
		if (str == "Tavern.tmx" || str == "NPC House.tmx")
		{
			app->audio->PlayFx(4, 0);
		}
		app->scene->playerPositionToBeLoaded = fPoint(playerX, playerY);
	}
}
