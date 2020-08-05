#pragma once
#include <Utils.hpp>

#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <DungeonMap.hpp>

typedef std::vector<Position> Path;

struct Node
{
    Position position;
    float costs;

    int32_t prevFieldIndex;
    int pathLength;

    Node(Position position, float costs, int32_t prevFieldIndex, int pathLength)
        : position(position), costs(costs), prevFieldIndex(prevFieldIndex), pathLength(pathLength) {}

    bool operator>(const Node &o) const
    {
        return costs > o.costs;
    }
};
typedef std::greater<Node> NodeComp;

class Pathfinder
{

public:
    Pathfinder(Position limits, DungeonMap &dungeonMap, DungeonConfiguration &dungeonConfig);
    ~Pathfinder();

    Path searchPath(Position start, Position target);

private:
    Path buildPath(Node &targetField);

    void processSuccessor(int prevIndex, const Node &prevNode, Position posOffset, Position target);
    float costCalculator(Position prevPrev, Position prev, Position node, Position goal);

    std::vector<Node> _openList;
    std::map<Position, std::size_t> _closedSet;
    std::vector<Node> _closedList;

    const Position _limits;

    DungeonMap &_dungeonMap;
    DungeonConfiguration &_dungeonConfig;

    size_t countNeighbores(Position pos, int range, TileType t);

    bool getTileType(Tile t, TileType type);
};