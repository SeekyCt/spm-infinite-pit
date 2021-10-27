/*
    Infinite Pit: Badge Pouch
    Storage & saving of badge info
*/

#pragma once

#include "ip_badges.h"

#include <types.h>

namespace ip {

#define POUCH_BADGE_COUNT 256
#define BADGE_POUCH_LSW_BASE (1024 - (POUCH_BADGE_COUNT * sizeof(PouchBadgeInfo)))

struct PouchBadgeInfo
{
    BadgeId id;
    bool equipped;
};

// Returns the badge info for a badge pouch slot
PouchBadgeInfo * pouchGetBadgeInfo(s32 slot);

// Returns the badge def for a badge in a pouch slot
BadgeDef * pouchGetBadgeDef(s32 slot);

// Returns the number of badges in the pouch
s32 pouchCountBadges();

// Returns the number of equipped badges in the pouch
s32 pouchCountEquippedBadges();

// Tries to add a badge to the pouch, true if successful
bool pouchAddBadge(BadgeId badge);

// Tries to remove a badge from the pouch, true if successful
bool pouchRemoveBadge(BadgeId badge);

// Clears pouch for new save
void badgePouchInit();

// Game code patches required by the badge pouch
void badgePouchPatch();

}
