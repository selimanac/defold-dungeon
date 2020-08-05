#pragma once

#define JC_ARRAY_IMPLEMENTATION
#include <jc/array.h>

#include <jc/hashtable.h>
#include <Utils.hpp>
#include <DungeonRoom.hpp>
#include <stdio.h>

class DungeonMap
{

public:
    static DungeonMap &getInstance(int32_t &width, int32_t &height);

    DungeonMap(DungeonMap const &) = delete;
    void operator=(DungeonMap const &) = delete;

    uint32_t insertRoom(Position roomPos, Position roomSize);
    void setTile(int id, Position pos);
    DungeonRoom *findRoom(int id);

    Tile &get(int32_t x, int32_t y);
    Tile &get(Position pos);

    void reset();

    // Rooms
    struct Rooms
    {
        uint32_t roomID;
        DungeonRoom *room;
    };

    typedef jc::HashTable<uint32_t, Rooms> hashtable_t;
    hashtable_t rooms;

    hashtable_t::Iterator it = rooms.Begin();
    hashtable_t::Iterator itend = rooms.End();

    const int32_t &mapHeight;
    const int32_t &mapWidth;

    void copyRooms(jc::Array<DungeonRoom *> &target);
    void update();

    jc::Array<Tile> tiles;

private:
    DungeonMap(int32_t &width, int32_t &height);

    uint32_t numelements = 1000; // The maximum number of entries to store
    uint32_t load_factor = 85;   // percent
    uint32_t tablesize = uint32_t(numelements / (load_factor / 100.0f));
    uint32_t sizeneeded = hashtable_t::CalcSize(tablesize);

    void *mem = malloc(sizeneeded);
};