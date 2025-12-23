#include "raylib.h"
#include <string.h>

#define MAX_TEXT_LENGTH 100

// Text
typedef struct {
    char *text;
    int x;
    int y;
    int fontSize;
    Color color;
} TextItem;

// Line
typedef struct {
    int x1;
    int y1;
    int x2;
    int y2;
    Color color;
} LineItem;

// List Objects
typedef struct NodeDrawable NodeDrawable;

struct NodeDrawable{
    char type;
    void *drawable;
    NodeDrawable *next;
};

typedef struct {
    int count;
    NodeDrawable *root;
} Drawlist;
