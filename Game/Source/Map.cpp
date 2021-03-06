#include "App.h"
#include "Render.h"
#include "Textures.h"
#include "Map.h"
#include "SceneManager.h"
#include "Scene.h"
#include "Debug.h"
#include "AssetsManager.h"

#include "Defs.h"
#include "Log.h"

#include "NPCFactory.h"

#include "simplify.h"

#include <math.h>
#include <algorithm>

Map::Map() : Module(), mapLoaded(false)
{
	name.Create("map");
	events = new std::vector<MapEvent*>();
}

// Destructor
Map::~Map()
{
	delete events;
}

int Properties::GetProperty(const char* name, int defaultValue) const
{
	for (int i = 0; i < propertyList.Count(); i++)
	{
		if (propertyList[i]->name == name)
		{
			return propertyList[i]->value;
		}
	}

	return defaultValue;
}

void Properties::SetProperty(const char* name, int value)
{
	for (int i = 0; i < propertyList.Count(); i++)
	{
		if (propertyList[i]->name == name)
		{
			propertyList[i]->value = value;
		}
	}
}

// Called before render is available
bool Map::Awake(pugi::xml_node& config)
{
	LOG("Loading Map Parser");
	bool ret = true;

	folder.Create(config.child("folder").child_value());

	return ret;
}

bool Map::Start()
{
	LoadEvents();
	return true;
}

bool Map::Update(float dt)
{
	return true;
}

bool Map::PostUpdate(float dt)
{
	Draw();

	return true;
}

// Draw the map (all requried layers)
void Map::Draw()
{

	if (mapLoaded == false) return;

	for (int i = 0; i < data.maplayers.Count(); i++)
	{
		if (!data.maplayers[i]->draw && !app->debug->bounds)
			continue;
		int layerSize = data.maplayers[i]->Size();
		for (int j = 0; j < layerSize; j++)
		{
			uint tileGid = data.maplayers[i]->data[j];
			int layerWidth = data.maplayers[i]->width;

			if (tileGid == 0)
				continue;

			for (int k = 0; k < data.tilesets.Count(); k++)
			{
				TileSet* tileset = data.tilesets[k];

				if (!tileset->loaded)
					break;

				if (data.tilesets.Count() > k + 1 && data.tilesets[k + 1]->firstgid <= tileGid)
				{
					continue;
				}

				SDL_Rect section = tileset->GetTileRect(tileGid);
				app->render->DrawTexture(tileset->texture, j % layerWidth * data.tileWidth, j / layerWidth * data.tileHeight, &section);
				break;
			}
		}
	}
}

// L04: DONE 8: Create a method that translates x,y coordinates from map positions to world positions
iPoint Map::MapToWorld(int x, int y) const
{
	iPoint ret;

	ret.x = x * data.tileWidth;
	ret.y = y * data.tileHeight;

	return ret;
}

// This function computes the rectangle that encapsulates the map
SDL_Rect Map::OuterRectangle()
{
	if (outerRectangle.w == 0)
	{
		outerRectangle = SDL_Rect({ 0, 0, data.width * data.tileWidth, data.height * data.tileHeight });
	}
	return outerRectangle;
}

std::vector<SDL_Rect>* Map::NavigationIntersection(SDL_Rect other)
{
	std::vector<SDL_Rect>* result = new std::vector<SDL_Rect>();

	for (int i = 0; i < data.maplayers.Count(); i++)
	{
		if (!data.maplayers[i]->navigation)
			continue;
		int layerSize = data.maplayers[i]->Size();
		for (int j = 0; j < layerSize; j++)
		{
			uint tileGid = data.maplayers[i]->data[j];
			int layerWidth = data.maplayers[i]->width;

			if (tileGid != 0)
			{
				SDL_Rect tile = SDL_Rect({ j % layerWidth * data.tileWidth, j / layerWidth * data.tileHeight, data.tileWidth, data.tileHeight });

				if (Intersects(tile, other))
				{
					result->push_back(tile);
				}
			}
		}
	}

	return result;
}

bool Map::EventIntersection(SDL_Rect other, std::pair<int, int>& result)
{
	for (int i = 0; i < data.maplayers.Count(); i++)
	{
		if (!data.maplayers[i]->isEvent)
			continue;
		int layerSize = data.maplayers[i]->Size();
		for (int j = 0; j < layerSize; j++)
		{
			uint tileGid = data.maplayers[i]->data[j];
			int layerWidth = data.maplayers[i]->width;

			if (tileGid != 0)
			{
				SDL_Rect tile = SDL_Rect({ j % layerWidth * data.tileWidth, j / layerWidth * data.tileHeight, data.tileWidth, data.tileHeight });

				if (Intersects(tile, other))
				{
					result.first = data.maplayers[i]->properties.GetProperty("eventLayerId", -1);
					result.second = data.GetTileSetGid(tileGid);
					return true;
				}
			}
		}
	}

	return false;
}

bool Map::BattleIntersection(SDL_Rect other)
{
	for (int i = 0; i < data.maplayers.Count(); i++)
	{
		if (!data.maplayers[i]->isBattle)
			continue;
		int layerSize = data.maplayers[i]->Size();
		for (int j = 0; j < layerSize; j++)
		{
			uint tileGid = data.maplayers[i]->data[j];
			int layerWidth = data.maplayers[i]->width;

			if (tileGid != 0)
			{
				SDL_Rect tile = SDL_Rect({ j % layerWidth * data.tileWidth, j / layerWidth * data.tileHeight, data.tileWidth, data.tileHeight });

				if (Intersects(tile, other))
				{
					return true;
				}
			}
		}
	}

	return false;
}

// Called before quitting
bool Map::CleanUp()
{

	LOG("Unloading map");
	// L03: DONE 2: Make sure you clean up any memory allocated from tilesets/map
	// Remove all tilesets
	for (int i = 0; i < data.tilesets.Count(); i++)
	{
		TileSet* t = data.tilesets[i];
		delete t;
	}
	data.tilesets.Clear();

	// L04: TODO 2: clean up all layer data
	// Remove all layers
	for (int i = 0; i < data.maplayers.Count(); i++)
	{
		delete[] data.maplayers[i]->data;
		data.maplayers[i]->properties.propertyList.Clear();
	}
	data.maplayers.Clear();
	data.properties.propertyList.Clear();
	
	// Clean up the pugui tree
	mapFile.reset();

	mapLoaded = false;

	return true;
}

// Load new map
bool Map::Load(const char* filename)
{

	bool ret = true;
	SString tmp("%s%s", folder.GetString(), filename);

	char* buffer = nullptr;
	size_t size = app->assetsManager->LoadXML(tmp.GetString(), &buffer);

	pugi::xml_parse_result result = mapFile.load_buffer(buffer, size);

	if (result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", filename, result.description());
		ret = false;
	}

	if (ret == true)
	{
		// L03: DONE 3: Create and call a private function to load and fill all your map data
		ret = LoadMap();
	}

	// L03: DONE 4: Create and call a private function to load a tileset
	// remember to support more any number of tilesets!
	pugi::xml_node tileset;
	for (tileset = mapFile.child("map").child("tileset"); tileset && ret; tileset = tileset.next_sibling("tileset"))
	{
		TileSet* set = new TileSet();

		if (ret == true) ret = LoadTilesetDetails(tileset, set);

		if (ret == true) ret = LoadTilesetImage(tileset, set);

		data.tilesets.Add(set);
	}
	// L04: TODO 4: Iterate all layers and load each of them
	pugi::xml_node layerNode;

	for (layerNode = mapFile.child("map").child("layer"); layerNode && ret; layerNode = layerNode.next_sibling("layer"))
	{
		MapLayer* layerSet = new MapLayer();

		if (ret == true) ret = LoadLayer(layerNode, layerSet);

		data.maplayers.Add(layerSet);
	}

	if (ret == true)
	{
		// L03: DONE 5: LOG all the data loaded iterate all tilesets and LOG everything
		LOG("Successfully parsed map XML file: %s", filename);
		LOG("width: %d height: %d", data.width, data.height);
		LOG("tile_width: %d tile_height: %d", data.tileWidth, data.tileHeight);

		for (int i = 0; i < data.tilesets.Count(); i++)
		{
			TileSet* t = data.tilesets[i];
			LOG("Tileset ----");
			LOG("name: %s firstgid: %d", t->name.GetString(), t->firstgid);
			LOG("tile width: %d tile height: %d", t->tileWidth, t->tileHeight);
			LOG("spacing: %d margin: %d", t->spacing, t->margin);
		}

		// L04: TODO 4: LOG the info for each loaded layer
	}

	outerRectangle = SDL_Rect({ 0, 0, 0, 0 });

	LoadNPCs();

	mapLoaded = ret;

	currentMapName = filename;

	RELEASE_ARRAY(buffer);

	return ret;
}

void Map::LoadNPCs()
{
	for (int k = 0; k < data.maplayers.Count(); k++)
	{
		MapLayer* layer = data.maplayers[k];

		if (layer->properties.GetProperty("npc", 0))
		{
			int layerSize = data.maplayers[k]->Size();
			for (int j = 0; j < layerSize; j++)
			{
				uint tileGid = data.maplayers[k]->data[j];
				int layerWidth = data.maplayers[k]->width;

				if (tileGid != 0)
				{
					int id = data.GetTileSetGid(tileGid);

					switch (id)
					{
					case 0:
						NPCFactory::Create(app->scene->currentScene->world, fPoint(j % layerWidth * data.tileWidth, j / layerWidth * data.tileHeight), NPCFactory::Type::TAVERN, 20);
						break;
					case 1:
						NPCFactory::Create(app->scene->currentScene->world, fPoint(j % layerWidth * data.tileWidth, j / layerWidth * data.tileHeight), NPCFactory::Type::REAPER, 20);
						break;
					case 3:
						NPCFactory::Create(app->scene->currentScene->world, fPoint(j % layerWidth * data.tileWidth, j / layerWidth * data.tileHeight), NPCFactory::Type::CUSTOMER, 20);
						break;
					case 4:
						NPCFactory::Create(app->scene->currentScene->world, fPoint(j % layerWidth * data.tileWidth, j / layerWidth * data.tileHeight), NPCFactory::Type::THYMA, 20);
						break;
					default:
						break;
					}
				}
			}
		}
		
	}
}

void Map::LoadEvents()
{
	SString tmp("%s", "events.xml");

	pugi::xml_parse_result result = mapFile.load_file(tmp.GetString());

	if (result == NULL)
	{
		LOG("Could not load map xml file %s. pugi error: %s", "events.xml", result.description());
	}

	pugi::xml_node map;
	for (map = mapFile.child("map"); map; map = map.next_sibling("map"))
	{
		int mapId = map.attribute("id").as_int(-1);

		pugi::xml_node layer;
		for (layer = map.child("layer"); layer; layer = layer.next_sibling("layer"))
		{
			int layerId = layer.attribute("id").as_int(-1);

			pugi::xml_node eventN;
			for (eventN = layer.child("event"); eventN; eventN = eventN.next_sibling("event"))
			{
				int eventId = eventN.attribute("id").as_int(-1);

				MapEvent* e = new MapEvent();
				e->mapId = mapId;
				e->layerId = layerId;
				e->eventId = eventId;

				e->attributes = new std::map<std::string, std::string>();

				pugi::xml_node::attribute_iterator i = eventN.attributes_begin();
				for (i; i != eventN.attributes_end(); i++)
				{
					e->attributes->insert(std::make_pair(i->name(), i->as_string()));
				}

				events->push_back(e);
			}
		}
	}
}

// L03: DONE: Load map general properties
bool Map::LoadMap()
{
	bool ret = true;
	pugi::xml_node map = mapFile.child("map");

	if (map == NULL)
	{
		LOG("Error parsing map xml file: Cannot find 'map' tag.");
		ret = false;
	}
	else
	{
		// L03: DONE: Load map general properties
		data.width = map.attribute("width").as_int(0);
		data.height = map.attribute("height").as_int(0);
		data.tileWidth = map.attribute("tilewidth").as_int(0);
		data.tileHeight = map.attribute("tileheight").as_int(0);
		data.nextObjectId = map.attribute("nextobjectid").as_int(0);
		data.nextLayerId = map.attribute("nextlayerid").as_int(0);
		SString hexColor = map.attribute("backgroundcolor").as_string("#000000");
		// Cut the string into the respective hex values for each color
		SString red = SString(hexColor).Cut(1, 2);
		SString green = SString(hexColor).Cut(3, 4);
		SString blue = SString(hexColor).Cut(5, 6);
		// Convert base-16 values to base-10 to get the final color
		data.backgroundColor.r = strtol(red.GetString(), nullptr, 16);
		data.backgroundColor.g = strtol(green.GetString(), nullptr, 16);
		data.backgroundColor.b = strtol(blue.GetString(), nullptr, 16);
		SString orientation = map.attribute("orientation").as_string();
		if (orientation == "orthogonal")
		{
			data.type = MAPTYPE_ORTHOGONAL;
		}
		else if (orientation == "isometric")
		{
			data.type = MAPTYPE_ISOMETRIC;
		}
		else if (orientation == "staggered")
		{
			data.type = MAPTYPE_STAGGERED;
		}
		else
		{
			data.type = MAPTYPE_UNKNOWN;
		}

		LoadProperties(map.child("properties"), &data.properties);
	}

	return ret;
}

// L03: TODO: Load Tileset attributes
bool Map::LoadTilesetDetails(pugi::xml_node& tilesetNode, TileSet* set)
{
	bool ret = true;
	// L03: DONE: Load Tileset attributes
	set->name.Create(tilesetNode.attribute("name").as_string("tileset"));
	set->firstgid = tilesetNode.attribute("firstgid").as_int(0);
	set->margin = tilesetNode.attribute("margin").as_int(0);
	set->spacing = tilesetNode.attribute("spacing").as_int(0);
	set->tileWidth = tilesetNode.attribute("tilewidth").as_int(0);
	set->tileHeight = tilesetNode.attribute("tileheight").as_int(0);

	return ret;
}

// L03: TODO: Load Tileset image
bool Map::LoadTilesetImage(pugi::xml_node& tileset_node, TileSet* set)
{
	bool ret = true;
	pugi::xml_node image = tileset_node.child("image");

	if (image == NULL)
	{
		LOG("Error parsing tileset xml file: Cannot find 'image' tag.");
		ret = false;
	}
	else
	{
		// L03: DONE: Load Tileset image
		SString tmp("%s%s", folder.GetString(), image.attribute("source").as_string(""));
		std::string path = tmp.GetString();
		std::string simplifiedPath = simplify(path);
		set->texture = app->tex->Load(simplifiedPath.c_str());
		set->texWidth = image.attribute("width").as_int(0);
		set->texHeight = image.attribute("height").as_int(0);
		set->numTilesWidth = set->texWidth / set->tileWidth;
		set->numTilesHeight = set->texHeight / set->tileHeight;
		set->offsetX = image.attribute("offsetx").as_int(0);
		set->offsetY = image.attribute("offsety").as_int(0);
	}

	return ret;
}

// L04: TODO 3: Create the definition for a function that loads a single layer
bool Map::LoadLayer(pugi::xml_node& node, MapLayer* layer)
{
	bool ret = true;

	layer->name.Create(node.attribute("name").as_string("Not Found"));
	layer->width = node.attribute("width").as_int(0);
	layer->height = node.attribute("height").as_int(0);
	layer->data = new uint[(data.width * data.height * sizeof(uint))];
	memset(layer->data, 0, size_t(data.width * data.height * sizeof(uint)));

	pugi::xml_node nodeID;

	int counter = 0;
	for (nodeID = node.child("data").child("tile"); nodeID && ret; nodeID = nodeID.next_sibling("tile"))
	{
		if (ret == true) ret = StoreID(nodeID, layer, counter);
		counter++;
	}

	LoadProperties(node.child("properties"), &layer->properties);

	if (!layer->properties.GetProperty("draw", 1))
		layer->draw = false;

	if (layer->properties.GetProperty("navigation", 0))
		layer->navigation = true;

	if (layer->properties.GetProperty("event", 0))
		layer->isEvent = true;

	if (layer->properties.GetProperty("battle", 0))
		layer->isBattle = true;

	return ret;
}

bool Map::LoadProperties(pugi::xml_node& node, Properties* properties)
{
	bool ret = true;

	pugi::xml_node propertyNode;

	for (propertyNode = node.child("property"); propertyNode && ret; propertyNode = propertyNode.next_sibling("property"))
	{
		Properties::Property* prop = new Properties::Property();
		prop->name.Create(propertyNode.attribute("name").as_string("Not Found"));
		prop->type.Create(propertyNode.attribute("type").as_string("Not Found"));
		prop->value = propertyNode.attribute("value").as_int(0);
		properties->propertyList.Add(prop);
	}

	return ret;
}

bool Map::StoreID(pugi::xml_node& node, MapLayer* layer, int ID)
{
	bool ret = true;

	layer->data[ID] = node.attribute("gid").as_uint(0);

	return ret;
}