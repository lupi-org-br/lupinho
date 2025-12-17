#include "types.h"
#include "drawlist.h"

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

/**
Global objects
**/
extern TextItem texts[MAX_TEXTS];
extern int textCount;

//----------------------------------------------------------------------------------
// Função Lua: escreva(texto, x, y, tamanho)
// Adiciona um texto para ser desenhado na tela
//----------------------------------------------------------------------------------
int lua_escreva(lua_State *L) {
    const char *text = luaL_checkstring(L, 1);
    int x = luaL_optinteger(L, 2, 10);
    int y = luaL_optinteger(L, 3, 10);
    int fontSize = luaL_optinteger(L, 4, 20);

    if (textCount < MAX_TEXTS) {
        strncpy(texts[textCount].text, text, MAX_TEXT_LENGTH - 1);
        texts[textCount].text[MAX_TEXT_LENGTH - 1] = '\0';
        texts[textCount].x = x;
        texts[textCount].y = y;
        texts[textCount].fontSize = fontSize;
        texts[textCount].color = DARKGRAY;
        texts[textCount].active = true;
        textCount++;
    }

    return 0;
}

//----------------------------------------------------------------------------------
// Função Lua: limpar_textos()
// Remove todos os textos da tela
//----------------------------------------------------------------------------------
int lua_limpar_textos(lua_State *L) {
    textCount = 0;
    for (int i = 0; i < MAX_TEXTS; i++) {
        texts[i].active = false;
    }
    return 0;
}
