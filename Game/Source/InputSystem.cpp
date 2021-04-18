#include "InputSystem.h"
#include "ECS.h"
#include "DialogSytem.h"

#include "App.h"
#include "Debug.h"
#include "Audio.h"
#include "Input.h"
#include "SceneManager.h"
#include "SDL/include/SDL_scancode.h"

void InputSystem::tick(ECS::World* world, float dt)
{
	world->each<KeyboardMovement>([&](ECS::Entity* entity, ECS::ComponentHandle<KeyboardMovement> keyboardMovement) {
		ECS::ComponentHandle<Position> p = entity->get<Position>();

		fPoint total = fPoint(0.f, 0.f);

		if (app->scene->menu == 0)
		{
			if (app->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT || pad.l_x > 0.0f || pad.right == true)
			{
				total += fPoint(1.f, 0.f);
			}

			if (app->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT || pad.l_y < 0.0f || pad.up == true)
			{
				total += fPoint(0.f, -1.f);
			}

			if (app->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT || pad.l_x < 0.0f || pad.left == true)
			{
				total += fPoint(-1.f, 0.f);
			}

			if (app->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT || pad.l_y > 0.0f || pad.down == true)
			{
				total += fPoint(0.f, 1.f);
			}

		}
		
		if (app->debug->godMode)
			keyboardMovement->speed = 200.0f;
		else
			keyboardMovement->speed = 70.0f;

		if (app->dialog->currentDialog == nullptr)
			p->Translate(total.Normalize() * keyboardMovement->speed * dt);
		else
			p->Translate(fPoint(0, 0));
	});
}