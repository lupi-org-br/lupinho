#include "raylib.h"
#include <string.h>

#define MAX_TEXT_LENGTH 100
#define MAX_TEXTS 100

// List Objects
struct NodeObject {
    void *object;
    struct NodeObject *next;
};

typedef struct Drawable {
    void (*draw)(void);
};

typedef struct {
    Drawable drawable;
    char text[MAX_TEXT_LENGTH];
    int x;
    int y;
    int fontSize;
    Color color;
    bool active;
} TextItem;
