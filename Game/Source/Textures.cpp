#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "AssetsManager.h"

#include "Defs.h"
#include "Log.h"

#include "SDL_image/include/SDL_image.h"
//#pragma comment(lib, "../Game/Source/External/SDL_image/libx86/SDL2_image.lib")

Textures::Textures() : Module()
{
	name.Create("textures");
}

// Destructor
Textures::~Textures()
{}

// Called before render is available
bool Textures::Awake(pugi::xml_node& config)
{
	LOG("Init Image library");
	bool ret = true;

	// Load support for the PNG image format
	int flags = IMG_INIT_PNG;
	int init = IMG_Init(flags);

	if((init & flags) != flags)
	{
		LOG("Could not initialize Image lib. IMG_Init: %s", IMG_GetError());
		ret = false;
	}

	return ret;
}

// Called before the first frame
bool Textures::Start()
{
	LOG("start textures");
	bool ret = true;
	return ret;
}

// Called before quitting
bool Textures::CleanUp()
{
	LOG("Freeing textures and Image library");
	std::map<std::string, SDL_Texture*>::iterator item;

	for(item = textures.begin(); item != textures.end(); item++)
	{
		SDL_DestroyTexture(item->second);
	}

	textures.clear();
	IMG_Quit();
	return true;
}

// Load new texture from file path
SDL_Texture** const Textures::Load(const char* path)
{
	SDL_Texture* texture = NULL;

	// (SOLVED) TODO 7: Load the texture using the SDL_RWops structure
	SDL_RWops* rw = app->assetsManager->LoadAsset(path);
	SDL_Surface* surface = NULL;
	if (rw != nullptr)
		surface = IMG_Load_RW(rw, 0);

	if (surface == NULL)
	{
		LOG("Could not load surface with path: %s. IMG_Load: %s", path, IMG_GetError());
	}
	else
	{
		texture = LoadSurface(surface);
		SDL_FreeSurface(surface);
	}

	if (textures.find(path) == textures.end())
		textures.insert(std::make_pair(path, texture));
	else
	{
		UnLoad(&textures.at(path), false);
		textures.at(path) = texture;
	}

	// (SOLVED) TODO 7: Close the allocated SDL_RWops structure
	if (rw != nullptr)
		SDL_RWclose(rw);

	return &textures.at(path);
}

// Unload texture
bool Textures::UnLoad(SDL_Texture** texture, bool eraseItem)
{
	std::map<std::string, SDL_Texture*>::iterator item;

	for(item = textures.begin(); item != textures.end(); item++)
	{
		if(*texture == item->second)
		{
			SDL_DestroyTexture(item->second);
			if (eraseItem)
				textures.erase(item);
		}
	}

	return true;
}

// Translate a surface into a texture
SDL_Texture* const Textures::LoadSurface(SDL_Surface* surface)
{
	SDL_Texture* texture = SDL_CreateTextureFromSurface(app->render->renderer, surface);

	if(texture == NULL)
	{
		LOG("Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
	}

	return texture;
}

// Retrieve size of a texture
void Textures::GetSize(SDL_Texture** texture, uint& width, uint& height) const
{
	SDL_QueryTexture(*texture, NULL, NULL, (int*) &width, (int*) &height);
}

void Textures::ReloadAllTextures()
{
	std::map<std::string, SDL_Texture*>::iterator item;

	for (item = textures.begin(); item != textures.end(); item++)
	{
		SDL_DestroyTexture(item->second);
		SDL_Texture** tex = Load(item->first.c_str());
	}
}
