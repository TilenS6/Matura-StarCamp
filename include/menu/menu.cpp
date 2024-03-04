#include "menu/menu.h"

template <typename... Args>
int Menu::chose(SDL_Renderer *r, string s[], uint8_t n, string title, string qeMenu[], uint8_t qeMenuN, uint8_t highlighted, Args... args) {
    int W, H;
    SDL_GetRendererOutputSize(r, &W, &H);
    int btnW = 200, btnH = 50, qeBtnW = 100, qeBtnH = 50;

    btns.clear();
    rects.clear();
    texts.clear();
    inputs.clear();
    btns.reset();

    // TITLE
    TTF_Font *font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 75);
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, title.c_str(), SDL_Color({255, 255, 255})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    SDL_Texture *titleTexture = SDL_CreateTextureFromSurface(r, textSurface);
    SDL_FreeSurface(textSurface);

    SDL_Rect titleTextureRect;
    SDL_QueryTexture(titleTexture, NULL, NULL, &titleTextureRect.w, &titleTextureRect.h);
    titleTextureRect.x = (W - titleTextureRect.w) / 2;
    titleTextureRect.y = 0;

    TTF_CloseFont(font);

    // QE
    font = TTF_OpenFont("fonts/nasalization-free/nasalization-rg.ttf", 30);
    textSurface = TTF_RenderText_Blended(font, "Q", SDL_Color({MENU_COLOUR1})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    SDL_Texture *qTx = SDL_CreateTextureFromSurface(r, textSurface);
    SDL_FreeSurface(textSurface);
    textSurface = TTF_RenderText_Blended(font, "E", SDL_Color({MENU_COLOUR1})); // use TTF_RenderText_Solid != TTF_RenderText_Blended for aliesed (stairs) edges
    SDL_Texture *eTx = SDL_CreateTextureFromSurface(r, textSurface);
    SDL_FreeSurface(textSurface);

    SDL_Rect qTxRect;
    SDL_QueryTexture(qTx, NULL, NULL, &qTxRect.w, &qTxRect.h);
    qTxRect.x = 50;
    qTxRect.y = titleTextureRect.h + 10;
    SDL_Rect eTxRect;
    SDL_QueryTexture(eTx, NULL, NULL, &eTxRect.w, &eTxRect.h);
    eTxRect.x = W - eTxRect.w - 50;
    eTxRect.y = titleTextureRect.h + 10;

    TTF_CloseFont(font);

    // BUTTONS
    for (uint8_t i = 0; i < n; ++i) {
        Button tmp;
        int id = btns.push_back(tmp);
        Button *p = btns.at_id(id);
        p->move(70, (btnH * (i + 3)) * 1.3, btnW, btnH); // na sredino ekrana (W - btnW) / 2
        p->changeStyle(0);                               // classic
        p->changeText(r, s[i]);
    }
    for (uint8_t i = 0; i < qeMenuN; ++i) {
        Button tmp;
        int id = btns.push_back(tmp);
        Button *p = btns.at_id(id);
        p->move(qTxRect.x + qTxRect.w + 40 + qeBtnW * (i * 1.5), qTxRect.y + ((qTxRect.h - qeBtnH) / 2), qeBtnW, qeBtnH); // na sredino ekrana (W - btnW) / 2
        p->changeStyle(1);                                                                                                // underlined
        p->changeText(r, qeMenu[i]);
        p->animated = false;
        if (i == highlighted)
            p->highlighted = true;
    }

    handleAditionalArgs(args...);

    // LOOP
    Mouse m;
    m.update();

    SDL_ShowCursor(SDL_DISABLE);
    Cursor c;
    c.init(&m, r, 20);

    bool running = true;
    SDL_Event event;
    Timer t;
    t.interval();
    while (running) {
        double dt = t.interval();
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_ESCAPE:
                    running = false;
                    break;
                case SDL_SCANCODE_Q:
                    if (highlighted > 0) return n + highlighted - 1;
                    break;
                case SDL_SCANCODE_E:
                    if (highlighted < qeMenuN) return n + highlighted + 1;
                    break;
                default:
                    break;
                }

                break;
            }
        }
        m.update();
        c.update(dt);

        uint8_t i = 0;
        for (int i = 0; i < btns.size; ++i) {
            if (btns.at_index(i)->clicked(&m)) {
                return i;
            }
        }
        for (int i = 0; i < inputs.size; ++i) {
            inputs.at_index(i)->update(&m, r);
        }

        SDL_SetRenderDrawColor(r, BACKGROUND_COLOUR, 255); // r b g a
        SDL_RenderClear(r);

        SDL_RenderCopy(r, titleTexture, NULL, &titleTextureRect);
        if (qeMenuN > 0) {
            SDL_RenderCopy(r, qTx, NULL, &qTxRect);
            SDL_Rect tmp = qTxRect;
            tmp.h += 4;
            tmp.w = tmp.h;
            tmp.x -= (tmp.w - qTxRect.w) / 2;
            tmp.y -= 2;
            SDL_SetRenderDrawColor(r, MENU_COLOUR2, 255); // r b g a
            SDL_RenderDrawRect(r, &tmp);
            tmp.x -= 3;
            tmp.y -= 3;
            tmp.w += 6;
            tmp.h += 6;
            SDL_SetRenderDrawColor(r, MENU_COLOUR1, 255); // r b g a
            SDL_RenderDrawRect(r, &tmp);

            SDL_RenderCopy(r, eTx, NULL, &eTxRect);
            tmp = eTxRect;
            tmp.h += 4;
            tmp.w = tmp.h;
            tmp.x -= (tmp.w - qTxRect.w) / 2;
            tmp.y -= 2;
            SDL_SetRenderDrawColor(r, MENU_COLOUR2, 255); // r b g a
            SDL_RenderDrawRect(r, &tmp);
            tmp.x -= 3;
            tmp.y -= 3;
            tmp.w += 6;
            tmp.h += 6;
            SDL_SetRenderDrawColor(r, MENU_COLOUR1, 255); // r b g a
            SDL_RenderDrawRect(r, &tmp);
        }

        for (int i = 0; i < btns.size; ++i) {
            btns.at_index(i)->render(r);
        }

        for (int i = 0; i < rects.size; ++i) {
            rects.at_index(i)->render(r);
        }
        for (int i = 0; i < texts.size; ++i) {
            texts.at_index(i)->render(r);
        }
        for (int i = 0; i < inputs.size; ++i) {
            inputs.at_index(i)->render(r);
        }
        
        c.render(r);

        SDL_RenderPresent(r);
    }
    SDL_DestroyTexture(titleTexture);
    return -1;
}

void Menu::handleAditionalArgs(){};

template <typename... Args>
void Menu::handleAditionalArgs(Button *btn, Args... args) {
    btns.push_back(*btn);
    handleAditionalArgs(args...);
}

template <typename... Args>
void Menu::handleAditionalArgs(MenuRect *rect, Args... args) {
    rects.push_back(*rect);
    handleAditionalArgs(args...);
}

template <typename... Args>
void Menu::handleAditionalArgs(Text *text, Args... args) {
    texts.push_back(*text);
    handleAditionalArgs(args...);
}
template <typename... Args>
void Menu::handleAditionalArgs(Input *input, Args... args) {
    inputs.push_back(*input);
    handleAditionalArgs(args...);
}