#include "Generator.hpp"

Generator::Generator()
{
    uint64_t seeds[2];
    entropy_getbytes((void *)seeds, sizeof(seeds));
    pcg32_srandom_r(&rng, seeds[0], seeds[1]);
}
Generator &Generator::getInstance()
{
    static Generator instance;
    return instance;
}

// Dungeon seed
uint32_t Generator::seed()
{
    return pcg32_random_r(&rng);
}

// Random Range
uint32_t Generator::boundRand(uint32_t min, uint32_t max)
{
    return pcg32_boundedrand_r(&rng, (max - min)) + min;
}

bool Generator::getTileType(Tile t, TileType type)
{
    return t.type == type;
}

// Count Neighbores
size_t Generator::countNeighbores(Position pos, int range, TileType t)
{
    const int32_t x = pos.x;
    const int32_t y = pos.y;

    size_t count = 0;
    for (int32_t yo = -range; yo <= range; ++yo)
    {
        for (int32_t xo = -range; xo <= range; ++xo)
        {
            if (x + xo >= 0 && x + xo < dungeonConfig.mapSize.x && y + yo >= 0 && y + yo < dungeonConfig.mapSize.y && (xo != 0 || yo != 0))
            {

                if (getTileType(dungeonMap->get(x + xo, y + yo), t))
                {
                    count++;
                }
            }
        }
    }
    return count;
}

int Generator::weightFunct(DungeonRoom *r)
{
    return r->getCenter().x + r->getCenter().y;
}

int Generator::closestRoomFunct(DungeonRoom *r, DungeonRoom *currentRoom)
{
    int distX = abs(r->getCenter().x - currentRoom->getCenter().x);
    int distY = abs(r->getCenter().y - currentRoom->getCenter().y);
    if (abs(distX - distY) < 10)
    {
        return fmin(distX, distY);
    }
    else
    {
        return distX + distY;
    }
}

DungeonRoom *Generator::findRoom(jc::Array<DungeonRoom *> &rooms, DungeonRoom *currentRoom = nullptr, bool isOtherRoom = false)
{
    //Find min
    DungeonRoom **cMin;
    int a = 0;

    // If only one room
    if (rooms.Size() == 1)
    {
        a = 0;
        cMin = rooms.Begin();
    }

    cMin = rooms.Begin();
    int cWeight;

    if (isOtherRoom)
    {
        //Which room is closest to this room
        cWeight = closestRoomFunct(rooms[0], currentRoom);
    }
    else
    {
        //Which room is closest to top-left
        cWeight = weightFunct(rooms[0]);
    }

    int nw;
    for (size_t i = 0; i < rooms.Size(); i++)
    {
        nw = weightFunct(rooms[i]);
        if (nw < cWeight)
        {
            a = i;
            cMin = rooms.Begin() + i;
            cWeight = nw;
        }
    }

    //return value
    DungeonRoom *returnValue = *cMin;

    //Remove it from openRooms array
    rooms.EraseSwap(a);

    return returnValue;
}

void Generator::reset()
{
    dungeonMap->reset();
}

void Generator::generate(uint32_t seed, DungeonConfiguration &config)
{
    dungeonMap = &DungeonMap::getInstance(config.mapSize.x, config.mapSize.y);
    dungeonConfig = config;

    // Seed with integer

    pcg32_srandom_r(&rng, seed, sizeof(seed));

    //Lazy solution for room position limits
    Range<Position> positionRange;
    positionRange.min = {1, 1};
    positionRange.max.x = config.mapSize.x - config.roomSize.max.x-1;
    positionRange.max.y = config.mapSize.y - config.roomSize.max.y-1;

    generateRooms(positionRange);

    smoothRooms();

    generateCorridors();

    generateObjects();

    // Duvar türleri En sona eklenecel
    // Top
    /*
                if (y == roomPos.y)
                {
                    tt = TileType::WALL_TOP;
                }
                //Bottom
                else if (y == (roomSize.y + roomPos.y - 1))
                {
                    tt = TileType::WALL_BOTTOM;
                }
                //LEFT
                else if (x == roomPos.x)
                {
                     tt = TileType::WALL_LEFT;
                }
                //RIGHT
                else if (x == (roomSize.x + roomPos.x) - 1){
                    tt = TileType::WALL_RIGHT;
                }
                    */
}

void Generator::generateRooms(Range<Position> positionRange)
{
    int triesLeft = 200;
    Position roomPos;
    Position roomSize;
    Position p;

    // Set complex rooms
    jc::Array<bool> roomIntersectionAllowed;
    roomIntersectionAllowed.SetCapacity(dungeonConfig.rooms);

    // Intersection probability
    for (size_t i = 0; i < roomIntersectionAllowed.Capacity(); ++i)
    {
        roomIntersectionAllowed.Push(pcg32_boundedrand_r(&rng, 2) ? true : false);
    }

    //Room loop
    bool joinThisRoomWithOthers = false;
    uint32_t existingRoomId = 0;

    for (int i = 0; i < dungeonConfig.rooms; i++)
    {
        existingRoomId = 0;
        joinThisRoomWithOthers = false;

        // Random room position and size
        roomPos.x = boundRand(positionRange.min.x, positionRange.max.x + 1);
        roomPos.y = boundRand(positionRange.min.y, positionRange.max.y + 1);

        roomSize.x = boundRand(dungeonConfig.roomSize.min.x, dungeonConfig.roomSize.max.x + 1);
        roomSize.y = boundRand(dungeonConfig.roomSize.min.y, dungeonConfig.roomSize.max.y + 1);

        if (abs(roomSize.x - roomSize.y) >= 5)
        {
            if (roomSize.x < roomSize.y)
            {
                roomSize.x += (roomSize.y - roomSize.x) / 2;
            }
            else
            {
                roomSize.y += (roomSize.x - roomSize.y) / 2;
            }
        }

        //Is complex/merge
        if (dungeonConfig.complexRooms)
        {
            joinThisRoomWithOthers = roomIntersectionAllowed[i];
        }

        // Room Intersection
        const int dmz = 0;
        for (int y = roomPos.y - dmz; y < roomSize.y + roomPos.y + dmz && (joinThisRoomWithOthers || existingRoomId == 0); ++y)
        {
            for (int x = roomPos.x - dmz; x < roomSize.x + roomPos.x + dmz && (joinThisRoomWithOthers || existingRoomId == 0); ++x)
            {
                if (dungeonMap->get(x, y).roomId != 0)
                {
                    if (existingRoomId == 0)
                    {
                        existingRoomId = dungeonMap->get(x, y).roomId;
                    }
                    else if (dungeonMap->get(x, y).roomId != existingRoomId)
                    {
                        DungeonRoom *roomA = dungeonMap->findRoom(dungeonMap->get(x, y).roomId);
                        DungeonRoom *roomB = dungeonMap->findRoom(existingRoomId);
                        p.x = x;
                        p.y = y;
                        if (roomA->fields.size() > roomB->fields.size())
                        {
                            existingRoomId = dungeonMap->get(x, y).roomId;
                            dungeonMap->setTile(roomA->id, p);
                        }
                        else
                        {
                            dungeonMap->setTile(roomB->id, p);
                        }
                    }
                }
            }
        }

        //Insert new room
        if (existingRoomId == 0)
        {
            existingRoomId = dungeonMap->insertRoom(roomPos, roomSize);
        }
        else if (!joinThisRoomWithOthers && triesLeft > 0)
        {
            //try new one
            i--;
            triesLeft--;
            continue;
        }

        // Fill room
        for (int y = roomPos.y; y < roomSize.y + roomPos.y; ++y)
        {
            for (int x = roomPos.x; x < roomSize.x + roomPos.x; ++x)
            {
                Tile &tile = dungeonMap->get(x, y);
                TileType tt = TileType::FLOOR;

                // Add Walls
                if (y == roomPos.y || y == (roomSize.y + roomPos.y - 1) || x == roomPos.x || x == (roomSize.x + roomPos.x) - 1)
                {
                    tt = TileType::WALL;
                }

                // Fill inside
                if (tile.type != TileType::NOTHING && tile.type != TileType::WALL)
                {
                    tt = TileType::FLOOR;
                }

                p.x = x;
                p.y = y;

                if (countNeighbores(p, 1, TileType::FLOOR) > 4)
                {
                    tt = TileType::FLOOR;
                }

                tile.type = tt;

                dungeonMap->setTile(existingRoomId, p);
            }
        }
    } // Rooms loop
}
/*
void Generator::visitNeighbores(Position pos, int range, void (*visitor)(Position tp, Tile &t))
{
    int32_t x = pos.x;
    int32_t y = pos.y;

    for (int32_t yo = -range; yo <= range; ++yo)
    {
        for (int32_t xo = -range; xo <= range; ++xo)
        {
            if (x + xo >= 0 && x + xo < dungeonMap->mapWidth && y + yo >= 0 && y + yo < dungeonMap->mapHeight && (xo != 0 || yo != 0))
                visitor(Position{x + xo, y + yo}, dungeonMap->get(x + xo, y + yo));
        }
    }
}
*/
void Generator::addCorridor(Path path)
{

    int32_t lastDoorDist = 9999;
    int32_t lastRoomId = 0;
    for (Position &node : path)
    {

        Tile &tile = dungeonMap->get(node);
        lastDoorDist++;
        switch (tile.type)
        {
        case TileType::WALL:

            if (tile.roomId != 0 && tile.roomId != lastRoomId && lastDoorDist >= 5)
            {
                lastDoorDist = 0;
                tile.type = TileType::DOOR;
                lastRoomId = tile.roomId;
            }
            else
                tile.type = TileType::CORRIDOR_FLOOR;

            if (tile.roomId != 0)
            {

                DungeonRoom *room = dungeonMap->findRoom(tile.roomId);
                room->doorCount++;
            }
            break;

        case TileType::NOTHING:
            tile.type = TileType::FLOOR;
            break;
        } // Switch

        // VISITNEIGHBORES
        int32_t x = node.x;
        int32_t y = node.y;
        int range = 1;

        for (int32_t yo = -range; yo <= range; ++yo)
        {
            for (int32_t xo = -range; xo <= range; ++xo)
            {
                if (x + xo >= 0 && x + xo < dungeonMap->mapWidth && y + yo >= 0 && y + yo < dungeonMap->mapHeight && (xo != 0 || yo != 0))
                {
                    //visitor(Position{x + xo, y + yo}, dungeonMap->get(x + xo, y + yo));
                    Tile &t = dungeonMap->get(x + xo, y + yo);
                    if (t.type == TileType::NOTHING)
                    {
                        t.type = TileType::WALL;
                    }
                }
            }
        }
        // END
        /*
        visitNeighbores(Position{node.x, node.y}, 1, [&](Position tp, Tile &t) {
            if (t.type == TileType::NOTHING)
                t.type = TileType::WALL;
        });
*/
    } // Loop
}

void Generator::generateCorridors()
{
    Position pp;
    pp.x = dungeonMap->mapWidth;
    pp.y = dungeonMap->mapHeight;

    // Init Pathfinder
    Pathfinder pathFinder = Pathfinder(pp, *dungeonMap, dungeonConfig);

    //Copy all rooms
    jc::Array<DungeonRoom *> openRooms;
    openRooms.SetCapacity(dungeonMap->rooms.Size());
    dungeonMap->copyRooms(openRooms);

    // Find the first room -> Nearest to top-left
    DungeonRoom *currentRoom = findRoom(openRooms);
    // Set it as start
    currentRoom->type = RoomType::START;

    //findRoom  -> removes selected room from array
    while (!openRooms.Empty())
    {
        // Find the nearest next room a
        DungeonRoom *nextRoom = findRoom(openRooms, currentRoom, true);

        Path pat = pathFinder.searchPath(currentRoom->getCenter(), nextRoom->getCenter());
        addCorridor(pathFinder.searchPath(currentRoom->getCenter(), nextRoom->getCenter()));

        currentRoom = nextRoom;
    }

    if (currentRoom->type != RoomType::START)
    {
        currentRoom->type = RoomType::END;
    }

    openRooms.SetCapacity(0);
    openRooms.SetSize(0);
    openRooms.Empty();
}

void Generator::generateObjects()
{
    Position roomPos;
    Range<Position> positionRange;
    dungeonMap->update();
    for (; dungeonMap->it != dungeonMap->itend; ++dungeonMap->it)
    {
        switch (dungeonMap->it.GetValue()->room->type)
        {
        case RoomType::START:

            positionRange.min = dungeonMap->it.GetValue()->room->position + Position{1, 1};
            positionRange.max = dungeonMap->it.GetValue()->room->position + dungeonMap->it.GetValue()->room->size - Position{2, 2};

            roomPos.x = boundRand(positionRange.min.x, positionRange.max.x);
            roomPos.y = boundRand(positionRange.min.y, positionRange.max.y);
            dungeonMap->get(roomPos).type = TileType::STAIRS_UP;

            break;
        case RoomType::END:

            positionRange.min = dungeonMap->it.GetValue()->room->position + Position{1, 1};
            positionRange.max = dungeonMap->it.GetValue()->room->position + dungeonMap->it.GetValue()->room->size - Position{2, 2};

            roomPos.x = boundRand(positionRange.min.x, positionRange.max.x);
            roomPos.y = boundRand(positionRange.min.y, positionRange.max.y);
            dungeonMap->get(roomPos).type = TileType::STAIRS_DOWN;

            break;

        default:
            dungeonMap->get(dungeonMap->it.GetValue()->room->getCenter()).type = TileType::BARRIER;
            break;
        }
    }
}

void Generator::smoothRooms()
{
    jc::Array<TileType> nextTT;
    nextTT.SetCapacity(dungeonMap->tiles.Size());

    for (size_t i = 0; i < nextTT.Capacity(); ++i)
    {
        nextTT.Push(TileType::NOTHING);
    }
    Position pos;

    for (std::size_t i = 0; i < dungeonConfig.smoothRuns; ++i)
    {

        for (int y = 0; y < dungeonMap->mapHeight; ++y)
        {

            for (int x = 0; x < dungeonMap->mapWidth; ++x)
            {
                pos.x = x;
                pos.y = y;

                size_t nothingCount = countNeighbores(pos, 2, TileType::NOTHING);
                size_t wallCount = countNeighbores(pos, 1, TileType::WALL);
                size_t floorCount = countNeighbores(pos, 1, TileType::FLOOR);

                switch (dungeonMap->get(x, y).type)
                {
                case TileType::NOTHING:
                    if (dungeonConfig.growRooms && wallCount > 5)
                        nextTT[y * dungeonMap->mapWidth + x] = TileType::WALL;
                    break;

                case TileType::WALL:
                    if (wallCount + floorCount > 7 && nothingCount < 5)
                        nextTT[y * dungeonMap->mapWidth + x] = TileType::FLOOR;
                    break;
                }

            } // X
        }     // Y
    }         // Turn

    for (int y = 0; y < dungeonMap->mapHeight; ++y)
    {
        for (int x = 0; x < dungeonMap->mapWidth; ++x)
        {
            if (nextTT[y * dungeonMap->mapWidth + x] != TileType::NOTHING)
            {

                Tile &tile = dungeonMap->get(x, y);
                uint32_t borderRoom = tile.roomId;

                pos.x = x;
                pos.y = y;

                // VISITNEIGHBORES
                int32_t x = pos.x;
                int32_t y = pos.y;

                int range = 1;

                for (int32_t yo = -range; yo <= range; ++yo)
                {
                    for (int32_t xo = -range; xo <= range; ++xo)
                    {
                        if (x + xo >= 0 && x + xo < dungeonMap->mapWidth && y + yo >= 0 && y + yo < dungeonMap->mapHeight && (xo != 0 || yo != 0))
                        {
                            //visitor(Position{x + xo, y + yo}, dungeonMap->get(x + xo, y + yo));
                            Tile &t = dungeonMap->get(x + xo, y + yo);
                            if (t.type == TileType::NOTHING)
                            {
                                t.type = TileType::WALL;
                            }

                            if (t.roomId != 0 && borderRoom == 0)
                                borderRoom = t.roomId;
                        }
                    }
                }
                // END

                /*
                visitNeighbores(pos, 1, [&](Position tp, Tile &t) {
                    if (t.type == TileType::NOTHING)
                        t.type = TileType::WALL;

                    if (t.roomId != 0 && borderRoom == 0)
                        borderRoom = t.roomId;
                });
                */
                if (borderRoom != 0)
                {
                    tile.type = TileType::FLOOR;
                    nextTT[y * dungeonMap->mapWidth + x] = TileType::NOTHING;

                    DungeonRoom *room = dungeonMap->findRoom(borderRoom);

                    pos.x = x;
                    pos.y = y;

                    room->fields.insert(pos);

                    // VISITNEIGHBORES
                    int32_t x = pos.x;
                    int32_t y = pos.y;

                    int range = 1;

                    for (int32_t yo = -range; yo <= range; ++yo)
                    {
                        for (int32_t xo = -range; xo <= range; ++xo)
                        {
                            if (x + xo >= 0 && x + xo < dungeonMap->mapWidth && y + yo >= 0 && y + yo < dungeonMap->mapHeight && (xo != 0 || yo != 0))
                            {
                                //visitor(Position{x + xo, y + yo}, dungeonMap->get(x + xo, y + yo));
                                Tile &t = dungeonMap->get(x + xo, y + yo);
                                Position tp;
                                tp.x = x + xo;
                                tp.y = y + yo;
                                // Position{x + xo, y + yo}
                                if (t.type == TileType::WALL && countNeighbores(tp, 1, TileType::NOTHING) == 0)
                                {
                                    t.type = TileType::FLOOR;
                                }

                                room->fields.insert(tp);
                            }
                        }
                    }
                    // END

                    /* visitNeighbores(pos, 1, [&](Position tp, Tile &t) {
                        if (t.type == TileType::WALL && countNeighbores(tp, 1, TileType::NOTHING) == 0)
                        {
                            t.type = TileType::FLOOR;
                        }
                        room->fields.insert(tp);
                    }); */
                }
            }

        } // X
    }     // Y

} //smoothRooms