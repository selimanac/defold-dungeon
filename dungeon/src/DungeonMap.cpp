#include "DungeonMap.hpp"

DungeonMap::DungeonMap(int32_t &width, int32_t &height) : mapHeight(height), mapWidth(width)
{
    rooms.Create(numelements, mem);
    tiles.SetCapacity(height * width);
    for (size_t i = 0; i < tiles.Capacity(); ++i)
        tiles.Push(Tile{TileType::NOTHING, 0});
}

DungeonMap &DungeonMap::getInstance(int32_t &width, int32_t &height)
{
    static DungeonMap instance(width, height);
    return instance;
}

void DungeonMap::reset(){
     it = rooms.Begin();
    itend = rooms.End();
    for (; it != itend; ++it)
    {
       
        it.GetValue()->room->fields.empty();
        delete it.GetValue()->room;
    }
    
    tiles.Empty();
    tiles.SetSize(0);
    rooms.Empty();
    rooms.Clear();

    tiles.SetCapacity(mapHeight * mapWidth);
    for (size_t i = 0; i < tiles.Capacity(); ++i)
        tiles.Push(Tile{TileType::NOTHING, 0});

    it = rooms.Begin();
    itend = rooms.End();
}

void DungeonMap::update(){
    it = rooms.Begin();
    itend = rooms.End();
}

Tile &DungeonMap::get(int32_t x, int32_t y)
{
    return tiles[y * mapWidth + x];
}

Tile &DungeonMap::get(Position pos)
{
    return get(pos.x, pos.y);
}

uint32_t DungeonMap::insertRoom(Position roomPos, Position roomSize)
{

    Rooms value;

    value.room = new DungeonRoom(roomPos, roomSize);
    value.roomID = value.room->id;
    rooms.Put(value.room->id, value);
    it = rooms.Begin();
    itend = rooms.End();

    return value.room->id;
}

void DungeonMap::setTile(int id, Position pos)
{
    Tile &tile = this->get(pos);

    if (tile.roomId == 0)
    {
        rooms.Get(id)->room->fields.insert(pos);
        tile.roomId = id;
    }
    else if (tile.roomId != id)
    {
        DungeonRoom *otherRoom = rooms.Get(tile.roomId)->room;

        rooms.Get(id)->room->fields.insert(otherRoom->fields.begin(), otherRoom->fields.end());
        for (Position tp : otherRoom->fields)
        {
            this->get(tp).roomId = id;
            
        }
        rooms.Erase(otherRoom->id);
    }
}

DungeonRoom *DungeonMap::findRoom(int id)
{
    return rooms.Get(id)->room;
}

void DungeonMap::copyRooms(jc::Array<DungeonRoom*> &target){
  
    hashtable_t::Iterator it = rooms.Begin();
    hashtable_t::Iterator itend = rooms.End();
    for(; it != itend; ++it)
    {
        target.Push(it.GetValue()->room);
    }

}