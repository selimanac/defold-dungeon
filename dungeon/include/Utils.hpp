#pragma once

#include <stdlib.h>

struct Position
{
    int32_t x, y;

    Position operator+(const Position &o) const
    {
        return Position{x + o.x, y + o.y};
    }
    Position operator-(const Position &o) const
    {
        return Position{x - o.x, y - o.y};
    }
    Position operator/(int factor) const
    {
        return Position{x / factor, y / factor};
    }
    Position operator*(int factor) const
    {
        return Position{x * factor, y * factor};
    }
    bool operator==(const Position &o) const
    {
        return x == o.x && y == o.y;
    }
    bool operator<(const Position &o) const
    {
        if (x < o.x)
            return true;
        if (x > o.x)
            return false;
        else
            return y < o.y;
    }
};

template <typename T>
struct Range
{
    T min;
    T max;
};

struct DungeonConfiguration
{
    Position mapSize;
    Range<Position> roomSize;
    size_t rooms;
    bool complexRooms;
    size_t smoothRuns;
    bool growRooms;
};

enum class TileType
{
    NOTHING = 0,
    WALL,
    FLOOR,
    DOOR,
    STAIRS_UP,
    STAIRS_DOWN,
    BARRIER,
    CORRIDOR_FLOOR,
    WALL_TOP,
    WALL_LEFT,
    WALL_RIGHT,
    WALL_BOTTOM
};

struct Tile
{
    TileType type;
    uint32_t roomId;
};


enum class RoomType {
    NORMAL = 0,
    START,
    END,
    BOSS_ROOM,
    POSITIV_ROOM,
    NEGATIV_ROOM
};


