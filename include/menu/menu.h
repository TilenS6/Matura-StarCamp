#pragma once
#include "camera/camera.h"
#include "mouse/mouse.h"
#include "shapes/shapes.h"
#include "FastCont/FastCont.h"
#include "graphics/graphics.h"

#define TEXT_COLOUR 255, 66, 61      // bright red
#define MENU_COLOUR1 255, 66, 61     // bright red
#define MENU_COLOUR2 84, 36, 36      // less bright red
#define DETAILS_COLOUR 227, 227, 227 // white

#define BACKGROUND_COLOUR 26, 26, 26

#define CURSOR_STIFFNESS 1e-7 // s kksno vrednostjo ostane stara vrednost pozicije kamere po 1s
class Button {
    int _x, _y, _w, _h;
    bool lastMLeft;
    SDL_Texture *text;
    SDL_Rect textRect;
    Timer t;
    int style; // 0-classic, 1-underlined text
    void renderStyle0(SDL_Renderer *);
    void renderStyle1(SDL_Renderer *);

public:
    bool highlighted, animated;
    Button();
    void destroy();
    void move(int, int, int, int);
    void changeText(SDL_Renderer *, string);
    void changeStyle(int);
    bool clicked(Mouse *);
    void render(SDL_Renderer *);
};

class Text {
    Timer t;
    SDL_Texture *text;
    SDL_Rect textRect;

    SDL_Renderer *last_r;
    string last_a;
    int last_size;

    string *strLink;
    string current;

public:
    Text();
    void destroy();
    void changeText(SDL_Renderer *, string, int);
    void linkText(string *);
    void move(int, int);
    void render(SDL_Renderer *);
};

class MenuRect {
    Timer t;

public:
    int x, y, w, h;
    MenuRect();
    void render(SDL_Renderer *);
};

class Cursor {
    SDL_Texture *texture;
    Mouse *m;
    Point pos;
    double dir, spd;
    double rad;

public:
    Cursor();
    ~Cursor();
    void init(Mouse *, SDL_Renderer *, double);
    void update(double);
    void render(SDL_Renderer *);
};

class Input {
    Timer t;
    bool lastMLeft = false, sensitive;
    SDL_Texture *texture;
    int textW, textH;
    string *str;

    string getStringFromSDL(SDL_Renderer *);

public:
    int x, y, w, h;

    Input();
    void destroy();
    void linkText(SDL_Renderer *, string *, bool);
    void update(Mouse *, SDL_Renderer *);
    void render(SDL_Renderer *);
};

class Menu {
    FastCont<Button> btns;
    FastCont<Input> inputs;
    // visual
    FastCont<MenuRect> rects;
    FastCont<Text> texts;

public:
    template <typename... Args>
    int chose(SDL_Renderer *, string[], uint8_t, string, string[], uint8_t, uint8_t, Args...);

    void handleAditionalArgs();
    template <typename... Args>
    void handleAditionalArgs(Button *, Args...);
    template <typename... Args>
    void handleAditionalArgs(MenuRect *, Args...);
    template <typename... Args>
    void handleAditionalArgs(Text *, Args...);
    template <typename... Args>
    void handleAditionalArgs(Input *, Args...);
};

#include "menu/button.cpp"
#include "menu/cursor.cpp"
#include "menu/text.cpp"
#include "menu/menurect.cpp"
#include "menu/input.cpp"
#include "menu/menu.cpp"