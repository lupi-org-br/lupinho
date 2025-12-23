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
