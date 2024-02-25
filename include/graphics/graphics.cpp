#include "graphics/graphics.h"

#define PI 3.14159265358979323
#define PIh PI / 2
#define PI2 PI * 2

void hsv2rgb(int H, int S,int V, int &_r, int &_g, int &_b){
    if(H>360 || H<0 || S>255 || S<0 || V>255 || V<0){
        cout<<"The givem HSV values are not in valid range"<<endl;
        return;
    }
    double s = S/255.;
    double v = V/255.;
    double C = s*v;
    double X = C*(1-abs(fmod(H/60.0, 2)-1));
    double m = v-C;
    double r,g,b;
    if(H >= 0 && H < 60){
        r = C,g = X,b = 0;
    }
    else if(H >= 60 && H < 120){
        r = X,g = C,b = 0;
    }
    else if(H >= 120 && H < 180){
        r = 0,g = C,b = X;
    }
    else if(H >= 180 && H < 240){
        r = 0,g = X,b = C;
    }
    else if(H >= 240 && H < 300){
        r = X,g = 0,b = C;
    }
    else{
        r = C,g = 0,b = X;
    }
    _r = (r + m) * 255;
    _g = (g + m) * 255;
    _b = (b + m) * 255;
}

void SDL_Ellipse(SDL_Renderer *r, int x0, int y0, int radiusX, int radiusY, bool tl = true, bool tr = true, bool bl = true, bool br = true) {
    // drew  28 lines with   4x4  circle with precision of 150 0ms
    // drew 132 lines with  25x14 circle with precision of 150 0ms
    // drew 152 lines with 100x50 circle with precision of 150 3ms
    const int prec = 150; // precision value; value of 1 will draw a diamond, 27 makes pretty smooth circles.
    double theta = 0;    // angle that will be increased each loop

    // starting point
    int x = (double)radiusX * cos(theta); // start point
    int y = (double)radiusY * sin(theta); // start point
    int x1 = x;
    int y1 = y;

    // repeat until theta >= 90;
    double step = PIh / (double)prec;               // amount to add to theta each time (degrees)
    for (theta = step; theta <= PIh; theta += step) // step through only a 90 arc (1 quadrant)
    {
        // get new point location
        x1 = (double)radiusX * cos(theta) + 0.5; // new point (+.5 is a quick rounding method)
        y1 = (double)radiusY * sin(theta) + 0.5; // new point (+.5 is a quick rounding method)

        // draw line from previous point to new point, ONLY if point incremented
        if ((x != x1) || (y != y1)) // only draw if coordinate changed
        {
            if (tr) SDL_RenderDrawLine(r, x0 + x, y0 - y, x0 + x1, y0 - y1); // quadrant TR
            if (tl) SDL_RenderDrawLine(r, x0 - x, y0 - y, x0 - x1, y0 - y1); // quadrant TL
            if (bl) SDL_RenderDrawLine(r, x0 - x, y0 + y, x0 - x1, y0 + y1); // quadrant BL
            if (br) SDL_RenderDrawLine(r, x0 + x, y0 + y, x0 + x1, y0 + y1); // quadrant BR
        }
        // save previous points
        x = x1; // save new previous point
        y = y1; // save new previous point
    }
    // arc did not finish because of rounding, so finish the arc
    if (x != 0) {
        x = 0;
        if (tr) SDL_RenderDrawLine(r, x0 + x, y0 - y, x0 + x1, y0 - y1); // quadrant TR
        if (tl) SDL_RenderDrawLine(r, x0 - x, y0 - y, x0 - x1, y0 - y1); // quadrant TL
        if (bl) SDL_RenderDrawLine(r, x0 - x, y0 + y, x0 - x1, y0 + y1); // quadrant BL
        if (br) SDL_RenderDrawLine(r, x0 + x, y0 + y, x0 + x1, y0 + y1); // quadrant BR
    }
}

void SDL_Circle(SDL_Renderer *renderer, int x, int y, int radius) {
    int offsetx = 0, offsety = radius, d = radius - 1;

    while (offsety >= offsetx) {
        SDL_RenderDrawPoint(renderer, x + offsetx, y + offsety);
        SDL_RenderDrawPoint(renderer, x + offsety, y + offsetx);
        SDL_RenderDrawPoint(renderer, x - offsetx, y + offsety);
        SDL_RenderDrawPoint(renderer, x - offsety, y + offsetx);
        SDL_RenderDrawPoint(renderer, x + offsetx, y - offsety);
        SDL_RenderDrawPoint(renderer, x + offsety, y - offsetx);
        SDL_RenderDrawPoint(renderer, x - offsetx, y - offsety);
        SDL_RenderDrawPoint(renderer, x - offsety, y - offsetx);

        if (d >= 2 * offsetx) {
            d -= 2 * offsetx + 1;
            offsetx += 1;
        } else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        } else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }
}

void SDL_FilledCircle(SDL_Renderer *renderer, int x, int y, int radius) {
    int offsetx = 0, offsety = radius, d = radius - 1;

    while (offsety >= offsetx) {
        SDL_RenderDrawLine(renderer, x - offsety, y + offsetx, x + offsety, y + offsetx);
        SDL_RenderDrawLine(renderer, x - offsetx, y + offsety, x + offsetx, y + offsety);
        SDL_RenderDrawLine(renderer, x - offsetx, y - offsety, x + offsetx, y - offsety);
        SDL_RenderDrawLine(renderer, x - offsety, y - offsetx, x + offsety, y - offsetx);

        if (d >= 2 * offsetx) {
            d -= 2 * offsetx + 1;
            offsetx += 1;
        } else if (d < 2 * (radius - offsety)) {
            d += 2 * offsety - 1;
            offsety -= 1;
        } else {
            d += 2 * (offsety - offsetx - 1);
            offsety -= 1;
            offsetx += 1;
        }
    }
}