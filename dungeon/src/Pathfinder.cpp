#include "Pathfinder.hpp"

Pathfinder::Pathfinder(Position limits, DungeonMap &dungeonMap, DungeonConfiguration &dungeonConfig) : _limits(limits), _dungeonMap(dungeonMap), _dungeonConfig(dungeonConfig)
{
}

Pathfinder::~Pathfinder() {}

Path Pathfinder::searchPath(Position start, Position target)
{
    _openList.clear();
    _closedList.clear();
    _closedSet.clear();

    Node node = {start, 0, -1, 1};
    /* node->position = start;
    node->costs = 0;
    node->prevFieldIndex = -1;
    node->pathLength = 1;*/
    _openList.push_back(node);

    // _openList.emplace_back(start, 0, -1, 1);

    std::push_heap(_openList.begin(), _openList.end(), NodeComp());

    while (!_openList.empty())
    {
        // add to closed list
        int fieldIndex = _closedList.size();
        _closedList.push_back(_openList.front());

        Node &field = _closedList.back();
        _closedSet[field.position] = fieldIndex;

        // remove from open list
        std::pop_heap(_openList.begin(), _openList.end(), NodeComp());
        _openList.pop_back();

        // target reached
        if (field.position == target)
            return buildPath(field);

        processSuccessor(fieldIndex, field, Position{-1, 0}, target);
        processSuccessor(fieldIndex, field, Position{0, 1}, target);

        processSuccessor(fieldIndex, field, Position{1, 0}, target);
        processSuccessor(fieldIndex, field, Position{0, -1}, target);
    }

    return Path();
}

Path Pathfinder::buildPath(Node &targetField)
{
    Path path(targetField.pathLength - 1);

    Node *f = &targetField;
    for (int i = path.size() - 1; f->prevFieldIndex >= 0; i--, f = &_closedList[f->prevFieldIndex])
    {
        assert(i >= 0);

        path[i] = f->position;
    }

    return path;
}

void Pathfinder::processSuccessor(int prevIndex, const Node &prevNode, Position posOffset, Position target)
{

    const Position pos = posOffset + prevNode.position;
    const float costs = prevNode.costs + costCalculator(
                                             prevNode.prevFieldIndex >= 0 ? _closedList[prevNode.prevFieldIndex].position : prevNode.position,
                                             prevNode.position,
                                             pos,
                                             target);

    if (pos.x <= 0 || pos.x >= _limits.x - 1 || pos.y <= 0 || pos.y >= _limits.y - 1 || costs >= 10000)
        return;

    if (_closedSet.find(pos) == _closedSet.end())
    {
        for (std::size_t i = 0; i < _openList.size(); ++i)
        {
            Node &n = _openList[i];

            if (n.position == pos)
            {
                // update score and path
                if (costs < n.costs)
                {
                    n.costs = costs;
                    n.prevFieldIndex = prevIndex;
                    n.pathLength = prevNode.pathLength + 1;

                    // reorder heap
                    std::make_heap(_openList.begin(), _openList.end(), NodeComp());
                }

                return;
            }
        }

        // not in open or closed list

        Node node = {pos, costs, (int)prevIndex, prevNode.pathLength + 1};
        /* node->position = pos;
        node->costs = costs;
        node->prevFieldIndex = prevIndex;
        node->pathLength = prevNode.pathLength + 1;*/
        _openList.push_back(node);

        //_openList.emplace_back(pos, costs, prevIndex, prevNode.pathLength + 1);
        std::push_heap(_openList.begin(), _openList.end(), NodeComp());
    }
}

float Pathfinder::costCalculator(Position prevPrev, Position prev, Position node, Position goal)
{

    float result = (abs(node.x - goal.x) + abs(node.y - goal.y));

    const Tile &nodeTile = _dungeonMap.get(node);

    switch (nodeTile.type)
    {
    case TileType::WALL:
        result += 10;

        if (_dungeonMap.get(prev).type == TileType::WALL)
        {
            result += 400;
        }

        result += 10 * countNeighbores(node + (node - prev), 1, TileType::WALL) +
                  10 * countNeighbores(node - (node - prev), 1, TileType::WALL) +
                  2 * countNeighbores(node + ((node - prev) * 2), 1, TileType::WALL) +
                  1 * countNeighbores(node, 1, TileType::WALL);

        {
            Position ln{node.x - 1, node.y}, rn{node.x + 1, node.y}, tn{node.x, node.y - 1}, bn{node.x, node.y + 1};

            result += (ln == prev || rn == prev || _dungeonMap.get(ln).type != _dungeonMap.get(rn).type) &&
                              (tn == prev || bn == prev || _dungeonMap.get(tn).type != _dungeonMap.get(bn).type)
                          ? 10000
                          : 0;
        }

        break;

    case TileType::NOTHING:
        if (countNeighbores(node, 2, TileType::WALL) > 3)
            result += (countNeighbores(node, 2, TileType::WALL) - 3) * 5;

        if (prevPrev.x != node.x && prevPrev.y != node.y)
            result += 2;

        result += 25;
        break;

    default:
        break;
    }

    return result;
}

bool Pathfinder::getTileType(Tile t, TileType type)
{
    return t.type == type;
}

// Count Neighbores
size_t Pathfinder::countNeighbores(Position pos, int range, TileType t)
{
    const int32_t x = pos.x;
    const int32_t y = pos.y;

    size_t count = 0;
    for (int32_t yo = -range; yo <= range; ++yo)
    {
        for (int32_t xo = -range; xo <= range; ++xo)
        {
            if (x + xo >= 0 && x + xo < _dungeonConfig.mapSize.x && y + yo >= 0 && y + yo < _dungeonConfig.mapSize.y && (xo != 0 || yo != 0))
            {

                if (getTileType(_dungeonMap.get(x + xo, y + yo), t))
                {
                    count++;
                }
            }
        }
    }
    return count;
}
