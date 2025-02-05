#include "util.h"
#include "iomanip"
#include "msg.h"

//----------------------------------------

TTF_Font *myFont = nullptr;

//----------------------------------------

std::string strRound(float value, int n)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(n) << value;
    return out.str();
}

//----------------------------------------

std::string getFileName(std::string path, int maximum_length)
{
    size_t lastSlash = path.find_last_of('\\');
    std::string name = path.substr(lastSlash + 1);

    size_t lastDot = name.find_last_of('.');
    if (lastDot != std::string::npos)
        name = name.substr(0, lastDot);

    if (name.size() > maximum_length)
        return name.substr(0, maximum_length) + "...";
    else
        return name;
}

//----------------------------------------

bool RandomChoice(int possibility)
{
    int n = IntegralRandom<int>(0, 100);
    return n <= possibility;
}

//----------------------------------------

std::string FormatMS(int ms)
{
    int hour = ms / 3600000;
    int minute = (ms % 3600000) / 60000;
    int second = (ms % 60000) / 1000;
    int millisecond = ms % 1000;

    std::ostringstream out;
    out << std::setfill('0') << std::setw(2) << hour << ":" << std::setw(2) << minute << ":" << std::setw(2) << second
        << "." << std::setw(3) << millisecond;

    return out.str();
}

//----------------------------------------

Keys left_keys(SDLK_d, SDLK_a, SDLK_s, SDLK_w, SDLK_SPACE);
Keys right_keys(SDLK_RIGHT, SDLK_LEFT, SDLK_DOWN, SDLK_UP, SDLK_KP_ENTER);

//----------------------------------------

SDL_Color Color::transparent = {0, 0, 0, 0};

//----------------------------------------

bool Rect::IsColliding(const Vector2 &first_position, const Vector2 &first_size, const Vector2 &second_position,
                       const Vector2 &second_size, float eps)
{
    float c_x = abs(first_position.x - second_position.x) - first_size.x / 2 - second_size.x / 2;
    float c_y = abs(first_position.y - second_position.y) - first_size.y / 2 - second_size.y / 2;
    return c_x <= eps && c_y <= eps;
}

Vector2 Rect::GetCenter(const Vector2 &position, const Vector2 &size)
{
    return position + size / 2;
}

SDL_Rect Rect::Rescale(const Vector2 &position, const Vector2 &size, float scale)
{
    SDL_Rect newRect;
    newRect.x = position.x + ceil((1.0f - scale) * size.x * 0.5);
    newRect.y = position.y + ceil((1.0f - scale) * size.y * 0.5);
    newRect.w = ceil(scale * size.x);
    newRect.h = ceil(scale * size.y);
    return newRect;
}

SDL_Rect Rect::Rescale(SDL_Rect rect, float scale)
{
    return Rescale(Vector2(rect.x, rect.y), Vector2(rect.w, rect.h), scale);
}

Vector2 Rect::GetPosition(SDL_Rect rect)
{
    return Vector2(rect.x, rect.y);
}

Vector2 Rect::GetSize(SDL_Rect rect)
{
    return Vector2(rect.w, rect.h);
}