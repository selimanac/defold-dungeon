#pragma once
#include <pcg/pcg_basic.h>
#include <math.h>
#include <pcg/entropy.h>
#include <Utils.hpp>
#include <DungeonMap.hpp>
#include <Pathfinder.hpp>
#include <stdlib.h>

#include <stdio.h> // DEbug- Remove

class Generator
{

public:
    static Generator &getInstance();
    Generator(Generator const &) = delete;
    void operator=(Generator const &) = delete;

    void generate(uint32_t seed, DungeonConfiguration &config);

    void reset();

    uint32_t seed();
    DungeonMap *dungeonMap;
    
private:
    pcg32_random_t rng;
    DungeonConfiguration dungeonConfig;

    Generator();

    void generateRooms(Range<Position> positionRange);
    void generateCorridors();
    void addCorridor(Path path);
    uint32_t boundRand(uint32_t min, uint32_t max);
    
    bool getTileType(Tile t, TileType type);

    int weightFunct(DungeonRoom *r);
    int closestRoomFunct(DungeonRoom *r, DungeonRoom *currentRoom);
    DungeonRoom *findRoom(jc::Array<DungeonRoom *> &rooms, DungeonRoom *currentRoom, bool isOtherRoom);
   // DungeonRoom *findAndRemove_minsec(jc::Array<DungeonRoom *> &rooms, DungeonRoom *currentRoom, bool isOtherRoom);
     
   // void visitNeighbores(Position pos, int range, void(*visitor)(Position tp, Tile& t));
    size_t countNeighbores(Position pos, int range, TileType t);
    void generateObjects();
    void smoothRooms();
};