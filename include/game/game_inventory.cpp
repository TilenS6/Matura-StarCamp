#include "game.h"

void Game::updatePlayersPickupFromFloor() {
    for (int i = 0; i < phisics.points.size(); ++i) {
        PhPoint *p = phisics.points.at_index(i);
        if (!p->virt) continue;
        if (server.getLastData(p->ownership) == nullptr) continue; // server ownership

        for (int j = 0; j < droppedItems.size(); ++j) {
            DroppedItem *it = droppedItems.at_index(j);

            if (distancePow2(it->pos, p->pos) <= PICKUP_RANGE_POW2) {
                send_pickup(p->ownership, *it);
                droppedItems.remove_index(j);
                j--;
            }
        }
    }
}

int Game::dropInventoryItem(int what, int howMuch, Point where) {
    if (what < 0 || what >= INVENTORY_SIZE) return -1;
    if (client_inventory.inv[what].count < howMuch) return -2;

    DroppedItem tmp;
    tmp.entr.ID = client_inventory.inv[what].ID;
    tmp.entr.count = client_inventory.inv[what].count;
    tmp.pos = where;
    uint32_t tmpid = droppedItems.push_back(tmp);

    client_inventory.inv[what].count -= howMuch;
    if (client_inventory.inv[what].count <= 0) client_inventory.inv[what].ID = none;

    updateInteractiveItems();

    if (droppedItems.at_id(tmpid) != nullptr)
        send_drop(tmp);

    return howMuch;
}

void Game::renderDroppedItems(Camera *cam) {
    for (int i = 0; i < droppedItems.size(); ++i) {
        // cout << "render @ Game::renderDroppedItems\n";
        droppedItems.at_index(i)->render(cam);
    }
}