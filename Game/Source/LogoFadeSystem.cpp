#include "LogoFadeSystem.h"
#include "Render.h"
#include "Window.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Input.h"
#include "Audio.h"

#include <algorithm>

void LogoFadeSystem::tick(ECS::World* world, float dt)
{
	world->each<LogoFade>([&](ECS::Entity* entity, ECS::ComponentHandle<LogoFade> logoFade) {
		SDL_Rect fullscreen;
		fullscreen.x = 0;
		fullscreen.y = 0;
		uint w, h;
		app->win->GetWindowSize(w, h);
		fullscreen.w = w;
		fullscreen.h = h;

		if (app->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {
			direction = 1;
			alpha = 255;
		}

		alpha += logoFade->speed * dt * direction;
		if (alpha > 255 && direction == 1)
		{
			direction *= -1;
			alpha = 255;

			//Load Map
			MapScene* s = new MapScene("Town.tmx");
			app->scene->mapSceneToBeLoaded = s;

		}

		if (alpha < 0 && direction == -1)
		{
			direction *= -1;
			alpha = 0;
		}

		if (alpha > 200 && !hasSounded)
		{
			app->audio->PlayFx(1, 0);
			hasSounded = true;
		}

		app->render->DrawRectangle(fullscreen, 0, 0, 0, std::min(int(alpha), 255), true, false);
	});
}