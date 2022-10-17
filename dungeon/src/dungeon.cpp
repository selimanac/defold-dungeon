#define LIB_NAME "Dungeon"
#define MODULE_NAME "dungeon"

#include <dmsdk/sdk.h>
#include <Generator.hpp>

DungeonConfiguration rDungeon;
Generator *generator = &Generator::getInstance();


static int seedgen(lua_State *L)
{
    uint32_t seed = generator->seed();
    lua_pushinteger(L, seed);
    return 1;
}
static int Generate(lua_State *L)
{
    int top = lua_gettop(L);

    int _seed = luaL_checkint(L, 1);
    int map_width = luaL_checkint(L, 2);
    int map_height = luaL_checkint(L, 3);
    int room_min_width = luaL_checkint(L, 4);
    int room_min_height = luaL_checkint(L, 5);
    int room_max_width = luaL_checkint(L, 6);
    int room_max_height = luaL_checkint(L, 7);
    int room_count = luaL_checkint(L, 8);
    bool complex_rooms = lua_toboolean(L, 9);
    int smooth_rooms = luaL_checkint(L, 10);
    bool grow_rooms = lua_toboolean(L, 11);

    rDungeon.mapSize.x = map_width;  //width
    rDungeon.mapSize.y = map_height; //height
    rDungeon.roomSize.min = {room_min_width, room_min_height};
    rDungeon.roomSize.max = {room_max_width, room_max_height};
    rDungeon.rooms = room_count;
    rDungeon.complexRooms = complex_rooms;
    rDungeon.smoothRuns = smooth_rooms;
    rDungeon.growRooms = grow_rooms;

    //printf("seed: %u\n", _seed);
    generator->generate(_seed, rDungeon);

    lua_createtable(L, map_height, 0);
    for (auto y = 0; y < generator->dungeonMap->mapHeight; ++y)
    {
        lua_createtable(L, map_width, 0);
        for (auto x = 0; x < generator->dungeonMap->mapWidth; ++x)
        {
            const Tile &tile = generator->dungeonMap->get(x, y);
            lua_pushnumber(L, (int)tile.type); // push some value, e.g. "pos[i][j]"
            lua_rawseti(L, -2, x + 1);
        }
        lua_rawseti(L, -2, y + 1);
    }

    // Rooms
    generator->dungeonMap->update();

    int i = 1;
    lua_createtable(L, generator->dungeonMap->rooms.Size(), 0);
    int newTable = lua_gettop(L);
    for (; generator->dungeonMap->it != generator->dungeonMap->itend; ++generator->dungeonMap->it)
    {
        lua_createtable(L, 4, 0);

        lua_pushstring(L, "size");
        lua_createtable(L, 2, 0);
        lua_pushstring(L, "width");
        lua_pushinteger(L, generator->dungeonMap->it.GetValue()->room->size.x);
        lua_settable(L, -3);
        lua_pushstring(L, "height");
        lua_pushinteger(L, generator->dungeonMap->it.GetValue()->room->size.y);
        lua_settable(L, -3);
        lua_settable(L, -3);

        lua_pushstring(L, "position");
        lua_createtable(L, 2, 0);
        lua_pushstring(L, "x");
        lua_pushinteger(L, generator->dungeonMap->it.GetValue()->room->position.x);
        lua_settable(L, -3);
        lua_pushstring(L, "y");
        lua_pushinteger(L, generator->dungeonMap->it.GetValue()->room->position.y);
        lua_settable(L, -3);
        lua_settable(L, -3);

        lua_pushstring(L, "type");
        lua_pushinteger(L, (int)generator->dungeonMap->it.GetValue()->room->type);
        lua_settable(L, -3);

        lua_pushstring(L, "doors");
        lua_pushinteger(L, generator->dungeonMap->it.GetValue()->room->doorCount);
        lua_settable(L, -3);

        lua_rawseti(L, newTable, i);
        i++;
    }
    generator->reset();
    return 2;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
    {
        {"seed", seedgen},
        {"generate", Generate},
        {0, 0}};

static void LuaInit(lua_State *L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeDungeon(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeDungeon(dmExtension::Params *params)
{
    // Init Lua
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeDungeon(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeDungeon(dmExtension::Params *params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(Dungeon, LIB_NAME, AppInitializeDungeon, AppFinalizeDungeon, InitializeDungeon, 0, 0, FinalizeDungeon)
