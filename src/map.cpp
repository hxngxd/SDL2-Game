#include "datalib.h"
#include "gameobject.h"
#include "game.h"

std::vector<std::vector<int>> TileMap;
std::vector<MapTile> Tiles;
std::vector<Background> Backgrounds;

MapTile::MapTile(
    Vector2 position,
    Vector2 size,
    Vector2 index,
    int type,
    float
    wait
){
    this->position = position;
    this->size = size;
    this->type = type;
    this->currentFrame = 0;
    this->animation_delay = 0;
    this->animation_speed = 10;
    this->index = index;
    this->scale = 0;
    this->wait_for_animation = wait;
}

std::pair<float, Vector2> MapTile::CreateTiles(int map){
    int mp_size = Screen::map_size;

    TileMap.resize(mp_size, std::vector<int>(mp_size, 0));
    std::ifstream in;
    in.open("map/" + std::to_string(map) + ".map");
    for (int i=0;i<mp_size;i++){
        for (int j=0;j<mp_size;j++){
            in >> TileMap[i][j];
        }
    }
    in.close();

    float wait = SDL_GetTicks() + 500;
    for (int i=0;i<mp_size;i++){
        CreateATile(0, i, wait);
    }
    for (int i=1;i<mp_size;i++){
        CreateATile(i, mp_size-1, wait);
    }
    for (int i=mp_size-2;i>=0;i--){
        CreateATile(mp_size-1, i, wait);
    }
    for (int i=mp_size-2;i>0;i--){
        CreateATile(i, 0, wait);
    }

    int spawn_i, spawn_j;
    for (int i=1;i<mp_size-1;i++){
        for (int j=1;j<mp_size-1;j++){
            if (TileMap[i][j]==SPAWN){
                spawn_i = i;
                spawn_j = j;
                continue;
            }
            CreateATile(i, j, wait);
        }
    }
    
    CreateATile(spawn_i, spawn_j, wait);

    return std::make_pair(wait + 50, Vector2(spawn_i, spawn_j));
}

void MapTile::CreateATile(
    int i,
    int j,
    float & wait
){
    if (!TileMap[i][j]) return;
    Tiles.push_back(
        MapTile(
            Vector2(j*Screen::tile_size, i*Screen::tile_size),
            Screen::tile_size,
            Vector2(i,j),
            TileMap[i][j],
            wait
        )
    );
    wait += 25;
}

void MapTile::Draw(){
    for (auto & tile : Tiles){
        if (tile.type != TileMap[tile.index.x][tile.index.y]) continue;

        float currentTicks = SDL_GetTicks();
        if (currentTicks <= tile.wait_for_animation) continue;
        
        SDL_Rect rect = Rect::reScale(tile.position, tile.size, tile.scale * 0.9);
        incScale(tile.scale);
        switch (tile.type){
            case WIN:
                Screen::SetDrawColor(Color::green(255));
                break;
            case SPAWN:
                Screen::SetDrawColor(Color::cyan(255));
                break;
            case WALL:
                Screen::SetDrawColor(Color::white(255));
                break;
            case COIN:
                if (currentTicks > tile.animation_delay + 1000/tile.animation_speed){
                    tile.currentFrame += 1;
                    if (tile.currentFrame >= Sprites["coin"]->maxFrames) tile.currentFrame = 0;
                    tile.animation_delay = currentTicks;
                }
                Screen::SetDrawColor(Color::yellow(255));
                Screen::DrawSprite(*Sprites["coin"], tile.position, tile.size, tile.scale * 0.5, tile.currentFrame, 0);
                break;
            case DAMAGE:
                Screen::SetDrawColor(Color::red(255));
                break;
            default:
                break;
        }
        SDL_RenderDrawRect(Game::renderer, &rect);
    }
}

void MapTile::Update(){
    Draw();
}

Background::Background(
    std::string name,
    float scale
){
    this->opacity = 64;
    this->toggle = true;
    this->name = name;
    this->position = Vector2(0);
    this->size = Screen::resolution;
    this->currentFrame = 0;
    this->maxFrames = Sprites[name]->maxFrames;
    this->scale = scale;
}

void Background::setOpacity(){
    SDL_SetTextureAlphaMod(Sprites[name]->texture, opacity);
}

bool Background::loadBackground(
    std::string name,
    std::string path,
    int maxFrames,
    Vector2 realSize,
    float scale
){
    if (!loadSprite(name, path, maxFrames, realSize)) return 0;
    ShowMsg(2, normal, "creating background " + name);
    Backgrounds.push_back(Background(name, scale));
    ShowMsg(3, success, "done.");
    SDL_SetTextureBlendMode(Sprites[name]->texture, SDL_BLENDMODE_BLEND);
    return 1;
}

void Background::Draw(){
    Background & bg0 = Backgrounds[0];
    Background & bg1 = Backgrounds[1];
    Background & bg2 = Backgrounds[2];

    bg0.opacity = 150;
    bg0.setOpacity();
    Screen::DrawSprite(*Sprites[bg0.name], bg0.position, bg0.size, bg0.scale, bg0.maxFrames, 0);

    if (bg2.toggle){
        bg2.opacity+=2;
        if (bg2.opacity >= 248) bg2.toggle = false;
    }
    else{
        bg2.opacity-=2;
        if (bg2.opacity <= 8) bg2.toggle = true;
    }
    bg2.setOpacity();
    Screen::DrawSprite(*Sprites[bg2.name], bg2.position, bg2.size, bg2.scale, bg2.maxFrames, 0);

    bg1.opacity = 256-bg2.opacity;
    bg1.setOpacity();
    Screen::DrawSprite(*Sprites[bg1.name], bg1.position, bg1.size, bg1.scale, bg1.maxFrames, 1);
}

void Background::Move(
    Vector2 velocity,
    int index,
    float ratio
){    
    Background & bg = Backgrounds[index];

    if (velocity.x){
        float bound = (1 - bg.scale) * bg.size.x * 0.5;
        bg.position.x += velocity.x * ratio;
        bg.position.x = clamp(bg.position.x, bound, -bound);
    }
    if (velocity.y){
        float bound = (1 - bg.scale) * bg.size.y * 0.5;
        bg.position.y += velocity.y * ratio;
        bg.position.y = clamp(bg.position.y, bound, -bound);
    }
}