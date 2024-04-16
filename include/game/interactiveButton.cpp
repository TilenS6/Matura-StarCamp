#include "game.h"
#ifdef GAME_EXISTS
InteractiveButton::InteractiveButton() {
    animationK = 0.;
    pos = { 0., 0. };
    textT = nullptr;
    tw = 0;
    th = 0;
}

/// @pa-ram funct please use following function to generate proper class: std::bind(Class::method_name, &object) (ex. ShipBuilder::build, &shipbuilder)
void InteractiveButton::init(Point where, string displayText, Camera* cam, int onPressReturn) {
    pos = where;
    txt = displayText;
    onPress = onPressReturn;

    if (textT != nullptr) SDL_DestroyTexture(textT);

    TTF_Font* font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, txt.c_str(), SDL_Color({ 255, 255, 255, 255 })); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    textT = SDL_CreateTextureFromSurface(cam->r, textSurface);
    SDL_FreeSurface(textSurface);

    SDL_QueryTexture(textT, NULL, NULL, &tw, &th);
    TTF_CloseFont(font);
}

int InteractiveButton::update(Point playerPos, double dt, Keyboard* kb) {
    if (textT == nullptr) {
        cout << "InteractiveButton@update not inited!\n";
        return false;
    }
    double k = pow(ANIMATION_SPEED, dt);
    double target_animationK = 0;
    int ret = onpress_notpressed;

    Circle c;
    c.a = pos;
    c.setRadius(BUTTON_DISTANCE);
    if (collisionPointCircle(playerPos, c)) {
        target_animationK = 1;
        if (kb->pressedNow(SDL_SCANCODE_F)) {
            // fp();
            // sb->build();
            ret = onPress;
            kb->newFrame();
        }

    }
    animationK = animationK * k + target_animationK * (1 - k);

    return ret;
}

void InteractiveButton::render(Camera* cam) {
    if (textT == nullptr) {
        if (txt != "") {
            cout << "warn: InteractiveButton::render generating new text texture...\n";
            TTF_Font* font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 24);
            SDL_Surface* textSurface = TTF_RenderText_Blended(font, txt.c_str(), SDL_Color({ 255, 255, 255, 255 })); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
            textT = SDL_CreateTextureFromSurface(cam->r, textSurface);
            SDL_FreeSurface(textSurface);

            SDL_QueryTexture(textT, NULL, NULL, &tw, &th);
            TTF_CloseFont(font);
        } else {
            cout << "InteractiveButton@render not inited!\n";
            return;
        }
    }

    double centerRectWH = 5 + 5 * animationK;

    Point tmp = pos.getRenderPos(cam);
    SDL_FRect rect;
    rect.w = tw * (animationK * .5 + .5);
    rect.h = th * (animationK * .5 + .5);
    rect.x = tmp.x - rect.w / 2.;
    rect.y = tmp.y - rect.h - 10; // 10px padding

    SDL_FRect centerRect;
    centerRect.x = tmp.x - centerRectWH / 2.;
    centerRect.y = tmp.y - centerRectWH / 2.;
    centerRect.w = centerRectWH;
    centerRect.h = centerRectWH;

    // center
    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255);
    SDL_RenderDrawRectF(cam->r, &centerRect);

    // text
    SDL_SetRenderDrawColor(cam->r, 255, 255, 255, 255 * animationK);
    SDL_SetTextureAlphaMod(textT, 255 * animationK);
    SDL_RenderDrawRectF(cam->r, &rect);
    SDL_RenderCopyF(cam->r, textT, NULL, &rect);
}
#endif