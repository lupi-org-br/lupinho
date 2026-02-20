#ifndef LUA_API_H
#define LUA_API_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

extern lua_State *globalLuaState;

void lua_api_init(void);
void lua_api_setup_game(const char *game_dir);
void lua_api_call_update(void);
void lua_api_close(void);

#endif // LUA_API_H
