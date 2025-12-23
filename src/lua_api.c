#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "drawlist.h"

//----------------------------------------------------------------------------------
// ui.draw_text(texto:string, x:int, y:int)
//----------------------------------------------------------------------------------
int lua_draw_text(lua_State *L) {
    char *text = luaL_checkstring(L, 1);
    int x = luaL_optinteger(L, 2, 10);
    int y = luaL_optinteger(L, 3, 10);

    add_text(text, x, y);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.draw_line(x1:int, y1:int, x2:int, y2:int, cor:int)
//----------------------------------------------------------------------------------
int lua_draw_line(lua_State *L) {
    int x1 = luaL_checkinteger(L, 1);
    int y1 = luaL_checkinteger(L, 2);
    int x2 = luaL_checkinteger(L, 3);
    int y2 = luaL_checkinteger(L, 4);
    int cor = luaL_checkinteger(L, 5);

    add_line(x1, y1, x2, y2, DARKGRAY);

    return 0;
}

//----------------------------------------------------------------------------------
// ui.draw_rect(x:int, y:int, largura:int, altura:int, prenchido:bool, cor:int)
//----------------------------------------------------------------------------------
int lua_draw_rect(lua_State *L) {
    int x = luaL_checkinteger(L, 1);
    int y = luaL_checkinteger(L, 2);
    int largura = luaL_checkinteger(L, 3);
    int altura = luaL_checkinteger(L, 4);
    bool prenchido = lua_toboolean(L, 5);
    int cor = luaL_checkinteger(L, 6);

    add_rect(x, y, largura, altura, prenchido, DARKGRAY);

    return 0;
}
