#include "DungeonRoom.hpp"

DungeonRoom::DungeonRoom(Position position, Position size) : id(genNextId()), position(position), size(size), doorCount(0), type(RoomType::NORMAL), maxDoors(0)
{
}

DungeonRoom::~DungeonRoom()
{
    printf("deleted %i\n", id);
}

u_int32_t DungeonRoom::genNextId()
{
    static u_int32_t id = 0;
    return ++id;
}
/*
void DungeonRoom::addTile(Position pos, Tile& tile)
{
    if(tile.roomId == 0){
        fields.insert(pos);
        tile.roomId = id;
    } else if( tile.roomId!=id ) {
        
    }
  
}
*/
/*
 void DungeonRoom::addField(Position pos){
     fields.insert(pos);
 }
 */