#ifndef MUSIC_H
#define MUSIC_H

#include <lua.h>

void music_bind(lua_State *L);
void music_set_root(const char *dir);
void music_update(float dt);
void music_shutdown(void);

#endif
