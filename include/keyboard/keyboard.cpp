Keyboard::Keyboard() {
    for (int i = 0; i < n; i++)
        arr[i] = 0b00000000;
}

void Keyboard::down(SDL_Scancode s) {
    arr[s / 8] |= (1 << (s % 8));
}
void Keyboard::up(SDL_Scancode s) {
    arr[s / 8] &= ~(1 << (s % 8));
}
void Keyboard::update(SDL_Event e) {
    if (e.type == SDL_KEYDOWN)
        down(e.key.keysym.scancode);
    else if (e.type == SDL_KEYUP)
        up(e.key.keysym.scancode);
}
bool Keyboard::get(SDL_Scancode s) {
    return (arr[s / 8] & (1 << (s % 8)));
}
