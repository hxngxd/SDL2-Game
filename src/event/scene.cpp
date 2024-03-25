#include "scene.h"
#include "../datalib/mixer.h"
#include "../datalib/msg.h"
#include "../game.h"
#include "../gameobject/gameobject.h"
#include "ui.h"

Scene::Scene()
{
    print("creating new scene...");
    print("creating border...");
    MapTile::CreateBorder();
    print("border created...");

    print("creating buttons...");
    auto start = []() {
        Game::scene->DeleteScene();
        auto new_scene = []() {
            Game::scene = new Scene(Game::Properties["map"].i);
            return 1;
        };
        LinkedFunction *lf = new LinkedFunction(new_scene, 500);
        lf->Execute();
        return 1;
    };
    Button::CreateButton("start", Screen::resolution / 2 - Vector2(0, 75), "Start", start);

    Button::CreateButton("settings", Screen::resolution / 2, "Settings");

    auto exit = []() {
        LinkedFunction *lf = new LinkedFunction([]() {
            game->Stop();
            return 1;
        });
        lf->Execute();
    };
    Button::CreateButton("exit", Screen::resolution / 2 + Vector2(0, 75), "Exit", exit);

    print("button created");

    print("scene created");
}

Scene::Scene(int map)
{
    print("creating new scene...");

    if (Texts.find("time") == Texts.end())
    {
        print("creating time text...");
        Text::CreateText("time", Screen::resolution / 2, "0", 50);
        print("time text created");
    }

    print("creating tiles...");
    MapTile::CreateTiles(map);

    LinkedFunction *lf = new LinkedFunction(std::bind([]() {
                                                print("tiles created");
                                                print("creating player...");
                                                PlaySound("spawn", CHANNEL_SPAWN_WIN, 0);

                                                Vector2 player_position(MapTile::SpawnTile.y * Screen::tile_size,
                                                                        MapTile::SpawnTile.x * Screen::tile_size);
                                                Game::player = new Player(player_position);
                                                if (!createTime)
                                                    createTime = SDL_GetTicks();
                                                startTime = SDL_GetTicks();
                                                if (stopTime)
                                                    createTime += (startTime - stopTime);
                                                return 1;
                                            }),
                                            (MapTile::nEmptyTiles + 4) * Game::Properties["map_animation_delay"].f);
    lf->NextFunction([]() { return TransformValue(&Game::player->scale, 1.0f, Game::Properties["rescale_speed"].f); },
                     0);
    lf->NextFunction(
        []() {
            return TransformValue(&TileMap[MapTile::SpawnTile.x][MapTile::SpawnTile.y].second->scale, 0.0f,
                                  Game::Properties["rescale_speed"].f);
        },
        500);
    lf->NextFunction(
        []() {
            std::pair<int, MapTile *> &spawn_tile = TileMap[MapTile::SpawnTile.x][MapTile::SpawnTile.y];
            spawn_tile.first = 0;
            delete spawn_tile.second;
            spawn_tile.second = nullptr;
            print("player created");
            return 1;
        },
        100);
    lf->NextFunction([]() {
        print("scene created");
        Game::player_won = false;
        return 1;
    });
    lf->Execute();
}

void Scene::DeleteScene()
{
    delete this;
    print("deleting current scene...");
    Button::DeleteButtons();

    if (Game::player)
    {
        print("deleting player...");
        PlaySound("win", CHANNEL_SPAWN_WIN, 0);
        LinkedFunction *lf = new LinkedFunction(
            []() { return TransformValue(&Game::player->scale, 0.0f, Game::Properties["rescale_speed"].f); });
        lf->NextFunction([]() {
            delete Game::player;
            Game::player = nullptr;
            stopTime = SDL_GetTicks();
            print("player deleted");
            print("deleting tiles (without border)...");
            MapTile::DeleteTiles();
            return 1;
        });
        lf->NextFunction(
            []() {
                if (Game::scene)
                {
                    delete Game::scene;
                    Game::scene = nullptr;
                }
                print("tiles deleted");
                print("scene deleted");
                return 1;
            },
            (MapTile::nEmptyTiles + 4) * Game::Properties["map_animation_delay"].f);
        lf->NextFunction([]() {
            Game::scene = new Scene(++Game::Properties["map"].i);
            return 1;
        });
        lf->Execute();
    }
}