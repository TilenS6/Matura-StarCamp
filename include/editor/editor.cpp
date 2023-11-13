#include "editor/editor.h"
#include "basicui/basicui.h"

void ElementOptions::inputData(string csv_prompts, Camera* cam) {
    vector<string> prompts;
    string buff = "";
    for (int i = 0; i < csv_prompts.length(); ++i) {
        if (csv_prompts[i] == ',') {
            prompts.push_back(buff);
            buff = "";
        } else {
            buff += csv_prompts[i];
        }
    }
    if (buff.size() > 0)
        prompts.push_back(buff);

    BUI ui;
    ui.assignCamera(cam);

    bool running = true;
    SDL_Event event;

    int sel = 0;
    bool editing = false;
    string editBuffer = "";

    while (running) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;

            case SDL_KEYDOWN:
            {
                char c = *SDL_GetKeyName(event.key.keysym.sym);
                if (editing) {
                    if ((c >= '0' && c <= '9') || (c == '.' && editBuffer.find('.') == -1))
                        editBuffer += c;
                }

            }
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                running = false;
                break;

            case SDL_SCANCODE_DOWN:
                if (sel < prompts.size() - 1) sel++;
                break;
            case SDL_SCANCODE_UP:
                if (sel > 0) sel--;
                break;
            case SDL_SCANCODE_RETURN:
                editing = !editing;
                if (!editing) {
                    if (editBuffer != "")
                        data[sel] = stod(editBuffer);
                    editBuffer = "";
                }
                break;


                running = false;
                break;

            default:
                break;
            }

            break;

            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    cam->w = event.window.data1;
                    cam->h = event.window.data2;
                    cout << "resizan na " << event.window.data1 << ", " << event.window.data2 << endl;
                }
                break;
                break;
            default:
                break;
            }
        } // end: while (events)

        SDL_SetRenderDrawColor(cam->r, 5, 5, 5, 255); // r b g a
        SDL_RenderClear(cam->r);
        for (int i = 0;i < prompts.size();++i) {
            SDL_Rect r = ui.drawText(prompts[i] + ":", 20, 20, 20 + 50 * i);
            int colour = 155 + (i == sel) * 100;
            if (editing && i == sel) {
                ui.drawText(editBuffer, 20, r.x + r.w + 10, 20 + 50 * i, colour, colour, colour);
            } else {
                ui.drawText(to_string(data[i]), 20, r.x + r.w + 10, 20 + 50 * i, colour, colour, colour);
            }
        }

        SDL_RenderPresent(cam->r);
    }

    ui.destroy();
}