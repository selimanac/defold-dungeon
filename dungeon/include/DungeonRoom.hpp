#pragma once
#include <Utils.hpp>
#include <set>

class DungeonRoom
{

public:
    DungeonRoom(Position position, Position size);
    ~DungeonRoom();

    uint32_t id;
    const Position position;
    const Position size;
    int8_t doorCount;
    RoomType type;

    std::set<Position> fields;
    
    Position getCenter()const {
        return position + size/2;
    }
    int8_t calcMaxDoors();

    //void addTile(Position pos, Tile& tile);

   //  void addField(Position pos);

private:
    static uint32_t genNextId();

    int8_t maxDoors;
};