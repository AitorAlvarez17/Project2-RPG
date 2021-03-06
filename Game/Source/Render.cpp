#include "App.h"
#include "Window.h"
#include "Render.h"
#include "Input.h"
#include "Textures.h"

#include "Defs.h"
#include "Log.h"

#include <stdexcept>

#define VSYNC true

Render::Render() : Module()
{
	name.Create("renderer");
	background.r = 0;
	background.g = 0;
	background.b = 0;
	background.a = 0;
}

// Destructor
Render::~Render()
{}

// Called before render is available
bool Render::Awake(pugi::xml_node& config)
{
	LOG("Create SDL rendering context");
	bool ret = true;

	Uint32 flags = SDL_RENDERER_ACCELERATED;

	if(config.child("vsync").attribute("value").as_bool(true) == true)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
		vsync = true;
		LOG("Using vsync");
	}

	renderer = SDL_CreateRenderer(app->win->window, -1, flags);

	SDL_RenderSetLogicalSize(renderer, app->win->screenSurface->w, app->win->screenSurface->h);

	if(renderer == NULL)
	{
		LOG("Could not create the renderer! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		camera.w = app->win->screenSurface->w;
		camera.h = app->win->screenSurface->h;
		camera.x = 0;
		camera.y = 0;
	}

	return ret;
}

// Called before the first frame
bool Render::Start()
{
	LOG("render start");
	// back background
	SDL_RenderGetViewport(renderer, &viewport);
	return true;
}

// Called each loop iteration
bool Render::PreUpdate()
{
	SDL_RenderClear(renderer);
	return true;
}

bool Render::Update(float dt)
{
	return true;
}

bool Render::PostUpdate(float dt)
{
	SDL_SetRenderDrawColor(renderer, background.r, background.g, background.g, background.a);
	SDL_RenderPresent(renderer);
	return true;
}

// Called before quitting
bool Render::CleanUp()
{
	LOG("Destroying SDL render");
	SDL_DestroyRenderer(renderer);
	return true;
}

void Render::SetBackgroundColor(SDL_Color color)
{
	background = color;
}

void Render::SwitchVsync()
{
	vsync = !vsync;

	Uint32 flags = SDL_RENDERER_ACCELERATED;

	if (vsync)
	{
		flags |= SDL_RENDERER_PRESENTVSYNC;
	}

	SDL_DestroyRenderer(renderer);
	renderer = SDL_CreateRenderer(app->win->window, -1, flags);

	SDL_RenderSetLogicalSize(renderer, app->win->screenSurface->w, app->win->screenSurface->h);
	SDL_RenderGetViewport(renderer, &viewport);
	app->tex->ReloadAllTextures();
}

void Render::SetViewPort(const SDL_Rect& rect)
{
	SDL_RenderSetViewport(renderer, &rect);
}

void Render::ResetViewPort()
{
	SDL_RenderSetViewport(renderer, &viewport);
}

// Blit to screen
bool Render::DrawTexture(SDL_Texture** texture, int x, int y, const SDL_Rect* section, float scale, float speed, double angle, int pivotX, int pivotY, bool useCamera) const
{
	bool ret = true;
	uint winScale = app->win->GetScale();

	SDL_Rect rect;

	if (useCamera)
	{
		rect.x = (int)(camera.x * speed) + x * winScale;
		rect.y = (int)(camera.y * speed) + y * winScale;
	}
	else {
		rect.x = x * winScale;
		rect.y = y * winScale;
	}

	if(section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(*texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w *= winScale * scale;
	rect.h *= winScale * scale;

	SDL_Point* p = NULL;
	SDL_Point pivot;

	if(pivotX != INT_MAX && pivotY != INT_MAX)
	{
		pivot.x = pivotX;
		pivot.y = pivotY;
		p = &pivot;
	}

	if(SDL_RenderCopyEx(renderer, *texture, section, &rect, angle, p, SDL_FLIP_NONE) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawRectangle(const SDL_Rect& rect, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool filled, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	SDL_Rect rec(rect);
	if(use_camera)
	{
		rec.x = (int)(camera.x + rect.x * scale);
		rec.y = (int)(camera.y + rect.y * scale);
		rec.w *= scale;
		rec.h *= scale;
	}

	int result = (filled) ? SDL_RenderFillRect(renderer, &rec) : SDL_RenderDrawRect(renderer, &rec);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawLine(int x1, int y1, int x2, int y2, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;

	if(use_camera)
		result = SDL_RenderDrawLine(renderer, camera.x + x1 * scale, camera.y + y1 * scale, camera.x + x2 * scale, camera.y + y2 * scale);
	else
		result = SDL_RenderDrawLine(renderer, x1 * scale, y1 * scale, x2 * scale, y2 * scale);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawCircle(int x, int y, int radius, Uint8 r, Uint8 g, Uint8 b, Uint8 a, bool use_camera) const
{
	bool ret = true;
	uint scale = app->win->GetScale();

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);

	int result = -1;
	SDL_Point points[360];

	float factor = (float)M_PI / 180.0f;

	for(uint i = 0; i < 360; ++i)
	{
		points[i].x = (int)(x + radius * cos(i * factor));
		points[i].y = (int)(y + radius * sin(i * factor));
	}

	result = SDL_RenderDrawPoints(renderer, points, 360);

	if(result != 0)
	{
		LOG("Cannot draw quad to screen. SDL_RenderFillRect error: %s", SDL_GetError());
		ret = false;
	}

	return ret;
}

bool Render::DrawParticle(SDL_Texture** texture, int x, int y, const SDL_Rect* section, const SDL_Rect* rectSize, SDL_Color color, float size, SDL_BlendMode blendMode, float speed, double angle, int pivotX, int pivotY) const
{
	uint scale = app->win->GetScale();

	SDL_Rect rect;
	rect.x = (int)(camera.x * speed) + x * scale * size;
	rect.y = (int)(camera.y * speed) + y * scale * size;

	if (rectSize != NULL)
	{
		rect.w = rectSize->w;
		rect.h = rectSize->h;
	}
	else if (section != NULL)
	{
		rect.w = section->w;
		rect.h = section->h;
	}
	else
	{
		SDL_QueryTexture(*texture, NULL, NULL, &rect.w, &rect.h);
	}

	rect.w *= scale * size;
	rect.h *= scale * size;

	if (SDL_SetTextureColorMod(*texture, color.r, color.g, color.b) != 0)
	{
		LOG("Cannot set texture color mode. SDL_SetTextureColorMod error: %s", SDL_GetError());
	}
	if (SDL_SetTextureAlphaMod(*texture, color.a) != 0)
	{
		LOG("Cannot set texture alpha mode. SDL_SetTextureAlphaMod error: %s", SDL_GetError());
	}
	if (SDL_SetTextureBlendMode(*texture, blendMode) != 0)
	{
		LOG("Cannot set texture blend mode. SDL_SetTextureBlendMode error: %s", SDL_GetError());
	}


	if (SDL_RenderCopyEx(renderer, *texture, section, &rect, angle, NULL, SDL_FLIP_NONE) != 0)
	{
		LOG("Cannot blit to screen. SDL_RenderCopy error: %s", SDL_GetError());
		return false;
	}

	return true;
}

bool Render::Load(pugi::xml_node& savedGame)
{
	//pugi::xml_node cameraNode = savedGame.child("camera");
	//camera.x = cameraNode.attribute("x").as_int();
	//camera.y = cameraNode.attribute("y").as_int();
	return true;
}

bool Render::Save(pugi::xml_node& savedGame)
{
	pugi::xml_node cameraNode = savedGame.append_child("camera");
	pugi::xml_attribute cameraX = cameraNode.append_attribute("x");
	cameraX.set_value(120);
	pugi::xml_attribute cameraY = cameraNode.append_attribute("y");
	cameraY.set_value(camera.y);
	return true;
}
