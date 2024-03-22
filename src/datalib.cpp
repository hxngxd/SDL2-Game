#include "datalib.h"

#include "game.h"

//----------------------------------------

Vector2 v2right = Vector2(1, 0);
Vector2 v2left = Vector2(-1, 0);
Vector2 v2down = Vector2(0, 1);
Vector2 v2up = Vector2(0, -1);
Vector2 mousePosition = Vector2();

float Vector2::distance(const Vector2 &other)
{
    float dx = other.x - x;
    float dy = other.y - y;
    return sqrt(dx * dx + dy * dy);
}

Vector2 IntV2(const Vector2 &v)
{
    return Vector2((int)v.x, (int)v.y);
}

std::ostream &operator<<(std::ostream &out, const Vector2 &v)
{
    out << "(" << v.x << ", " << v.y << ")";
}

void operator+=(Vector2 &v1, const Vector2 &v2)
{
    v1.x += v2.x;
    v1.y += v2.y;
}

void operator-=(Vector2 &v1, const Vector2 &v2)
{
    v1.x -= v2.x;
    v1.y -= v2.y;
}

void operator*=(Vector2 &v, float k)
{
    v.x *= k;
    v.y *= k;
}

void operator/=(Vector2 &v, float k)
{
    v.x /= k;
    v.y /= k;
}

Vector2 operator+(const Vector2 &v1, const Vector2 &v2)
{
    return Vector2(v1.x + v2.x, v1.y + v2.y);
}

Vector2 operator-(const Vector2 &v1, const Vector2 &v2)
{
    return Vector2(v1.x - v2.x, v1.y - v2.y);
}

Vector2 operator*(const Vector2 &v, float k)
{
    return Vector2(v.x * k, v.y * k);
}

Vector2 operator/(const Vector2 &v, float k)
{
    return Vector2(v.x / k, v.y / k);
}

Vector2 operator*(float k, const Vector2 &v)
{
    return Vector2(v.x * k, v.y * k);
}

Vector2 operator-(const Vector2 &v)
{
    return Vector2(-v.x, -v.y);
}

Vector2 max(const Vector2 &v1, const Vector2 &v2)
{
    return {std::max(v1.x, v2.x), std::max(v1.y, v2.y)};
}

Vector2 min(const Vector2 &v1, const Vector2 &v2)
{
    return {std::min(v1.x, v2.x), std::min(v1.y, v2.y)};
}

bool IsV2InRange(Vector2 &v, const Vector2 &mn, const Vector2 &mx)
{
    return mn.x <= v.x && v.x <= mx.x && mn.y <= v.y && v.y <= mx.y;
}

std::size_t Vector2Hash::operator()(const Vector2 &v) const
{
    std::hash<float> hash_fn;
    return hash_fn(v.x) ^ hash_fn(v.y);
}

bool Vector2Equal::operator()(const Vector2 &lhs, const Vector2 &rhs) const
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

//----------------------------------------

bool Rect::isCollide(const Vector2 &first_position, const Vector2 &first_size, const Vector2 &second_position,
                     const Vector2 &second_size, float eps)
{
    float c_x = abs(first_position.x - second_position.x) - first_size.x / 2 - second_size.x / 2;
    float c_y = abs(first_position.y - second_position.y) - first_size.y / 2 - second_size.y / 2;
    return c_x <= eps && c_y <= eps;
}

Vector2 Rect::getCenter(const Vector2 &position, const Vector2 &size)
{
    return position + size / 2;
}

SDL_Rect Rect::reScale(const Vector2 &position, const Vector2 &size, float scale)
{
    SDL_Rect newRect;
    newRect.x = position.x + (1 - scale) * size.x * 0.5;
    newRect.y = position.y + (1 - scale) * size.y * 0.5;
    newRect.w = scale * size.x;
    newRect.h = scale * size.y;
    return newRect;
}

SDL_Rect Rect::reScale(SDL_Rect rect, float scale)
{
    return reScale(Vector2(rect.x, rect.y), Vector2(rect.w, rect.h), scale);
}

//----------------------------------------

SDL_Color Color::transparent = {0, 0, 0, 0};

//----------------------------------------

bool loadSprite(std::string name, std::string path, int maxFrames, Vector2 realSize)
{
    print("trying to load sprite", path, "...");
    Sprites[name] = new Sprite();
    SDL_Surface *sf = IMG_Load(path.c_str());
    Sprites[name]->texture = SDL_CreateTextureFromSurface(Game::renderer, sf);
    if (!Sprites[name]->texture)
    {
        delete Sprites[name];
        Sprites[name] = nullptr;
        Sprites.erase(name);
        print("failed to load sprite", path);
        return 0;
    }
    print(path, "ok");
    Sprites[name]->path = path;
    Sprites[name]->maxFrames = maxFrames;
    Sprites[name]->realSize = realSize;
    SDL_FreeSurface(sf);
    return 1;
}

void setTextureOpacity(SDL_Texture *texture, int opacity)
{
    SDL_SetTextureAlphaMod(texture, opacity);
}

void DrawSprite(Sprite *sprite, const Vector2 &position, const Vector2 &size, float scale, int currentFrame, bool flip)
{
    SDL_Rect src = {(currentFrame % sprite->maxFrames) * sprite->realSize.x, 0, sprite->realSize.x, sprite->realSize.y};
    SDL_Rect dst = Rect::reScale(position, size, scale);
    SDL_RenderCopyEx(Game::renderer, sprite->texture, &src, &dst, 0, NULL,
                     (flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}

std::map<std::string, Sprite *> Sprites;

//----------------------------------------

bool loadSound(std::string name, std::string path)
{
    print("trying to load sound", path, "...");
    Sounds[name] = Mix_LoadWAV(path.c_str());
    if (!Sounds[name])
    {
        Mix_FreeChunk(Sounds[name]);
        Sounds.erase(name);
        print("failed to load sound", path);
        return 0;
    }
    print(path, "ok");
    return 1;
}

void playSound(std::string name, int channel, int loop)
{
    Mix_FadeInChannel(channel, Sounds[name], loop, 15);
}

void stopSound(int channel)
{
    Mix_FadeOutChannel(channel, 50);
}

void stopAllSound()
{
    stopSound(-1);
}

bool loadMusic(std::string name, std::string path)
{
    print("trying to load music", path, "...");
    Musics[name] = Mix_LoadMUS(path.c_str());
    if (!Musics[name])
    {
        Mix_FreeMusic(Musics[name]);
        Musics.erase(name);
        print("failed to load music", path);
        return 0;
    }
    print(path, "ok");
    return 1;
}

void playMusic(std::string name, int loop)
{
    Mix_PlayMusic(Musics[name], loop);
}

void stopMusic()
{
    Mix_HaltMusic();
}

std::map<std::string, Mix_Chunk *> Sounds;
std::map<std::string, Mix_Music *> Musics;

//----------------------------------------

float clamp(float value, float mn, float mx)
{
    if (value < mn)
        return mn;
    if (value > mx)
        return mx;
    return value;
}

bool IsInRange(float value, float mn, float mx)
{
    return mn <= value && value <= mx;
}

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::mt19937 gen(seed);
std::uniform_int_distribution<> distrib(std::numeric_limits<Uint32>::min(), std::numeric_limits<Uint32>::max());

Uint32 randUint32()
{
    return distrib(gen);
}

Uint32 transformFValue(float *value, float dst, float speed, float delay, std::function<void()> post_function)
{
    if (!value)
        return 0;

    if (abs(*value - dst) <= 0.005)
    {
        *value = dst;
        post_function();
        return 0;
    }

    float *tmp_speed = new float(speed);
    bool increasing = dst > *value;

    auto transform = [](float *value, float dst, float *tmp_speed, bool increasing) {
        if (!value)
            return 1;

        if (increasing && *value < dst)
        {
            *value += *tmp_speed;
            if (*value >= dst)
            {
                *value = dst;
                delete tmp_speed;
                tmp_speed = nullptr;
                return 1;
            }
            *tmp_speed /= 1.05;
            return 0;
        }

        if (!increasing && *value > dst)
        {
            *value -= *tmp_speed;
            if (*value <= dst)
            {
                *value = dst;
                delete tmp_speed;
                tmp_speed = nullptr;
                return 1;
            }
            *tmp_speed /= 1.05;
            return 0;
        }

        *value = dst;
        delete tmp_speed;
        tmp_speed = nullptr;
        return 1;
    };

    return DelayFunction::Create(delay, std::bind(transform, value, dst, tmp_speed, increasing), post_function);
}

//----------------------------------------

Keys leftKeys(SDLK_d, SDLK_a, SDLK_s, SDLK_w, SDLK_SPACE);
Keys rightKeys(SDLK_RIGHT, SDLK_LEFT, SDLK_DOWN, SDLK_UP, SDLK_KP_ENTER);

TTF_Font *myFont = nullptr;