/*
 *
 * Skye's Castle Adventure
 *
 * A sequel to The Abandoned Farmhouse Adventure.
 *
 * Jeff Tranter <tranter@pobox.com>
 *
 * Written in standard C but designed to run on the Apple II or other
 * platforms using the CC65 6502 assembler.
 *
 * Copyright 2012-2019 Jeff Tranter
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Revision History:
 *
 * Version  Date         Comments
 * -------  ----         --------
 * 0.0      07 Sep 2015  Started development.
 * 0.1      16 Feb 2019  Most game logic implemented.
 * 0.9      17 Feb 2019  Seems to be fully working.
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __CC65__
#include <conio.h>
#endif

/* CONSTANTS */

/* Maximum number of items user can carry */
#define MAXITEMS 5

/* Number of locations */
#define NUMLOCATIONS 60

/* TYPES */

/* To optimize for code size and speed, most numbers are 8-bit chars when compiling for the Replica 1. */
#ifdef __CC65__
typedef char number;
#else
typedef int number;
#endif

/* Directions */
typedef enum {
    North,
    South,
    East,
    West,
    Up,
    Down
} Direction_t;

/* Items */
typedef enum {
    NoItem,
    PoolCue,
    Umbrella,
    Newspaper,
    Sandwich,
    Cat,
    Wine,
    Knife,
    Candle,
    Matches,
    Auntie,
    Doll,
    Skye,
    SteelBar,
    Book,
    Hairbrush,
    Note,
    Perfume,
    BusinessCard,
    Soap,
    Menu,
    LightBulb,
    Key,
    LastItem=Key
} Item_t;

/* Locations */
typedef enum {
    NoLocation,
    FrontEntrance,
    Vestibule,
    Entry,
    PeacockAlley1,
    PeacockAlley2,
    PeacockAlley3,
    PeacockAlley4,
    PeacockAlley5,
    DiningRoom,
    Conservatory,
    BreakfastRoom,
    ServingRoom,
    Kitchen,
    Hallway1,
    Hallway2,
    Elevator1,
    Study,
    Library,
    GreatHall,
    Stairs1,
    Landing,
    OakDrawingRoom,
    SmokingRoom,
    CoveredPorch,
    BilliardsRoom,
    Hallway10,
    Hallway11,
    Hallway12,
    Hallway13,
    Hallway14,
    Corridor1,
    Corridor2,
    Narrowhallway,
    RoundRoom,
    PipeOrganLoft,
    Bedroom1,
    SittingRoom1,
    Bedroom3,
    SirHenrysBedroom,
    SittingRoom3,
    LadysBedroom,
    SittingRoom2,
    GuestBedroom,
    Elevator2,
    ChildrensBedroom,
    ServantsBedroom,
    Bedroom4,
    LinenRoom,
    Bedroom2,
    Bath,
    Stairs2,
    Stairs3,
    WineCellar,
    Tunnel1,
    Tunnel2,
    Tunnel3,
    SteamPlant,
    Tunnel4,
    Stables,
} Location_t;

/* TABLES */

/* Names of directions */
const char *DescriptionOfDirection[] = {
    "north", "south", "east", "west", "up", "down"
};

/* Names of items */
const char *DescriptionOfItem[LastItem+1] = {
    "",
    "pool cue",
    "umbrella",
    "newspaper",
    "sandwich",
    "cat",
    "wine",
    "knife",
    "candle",
    "matches",
    "Auntie",
    "doll",
    "Skye",
    "steel bar",
    "book",
    "hairbrush",
    "note",
    "perfume",
    "business card",
    "soap",
    "menu",
    "light bulb",
    "key"
};

/* Names of locations */
const char *DescriptionOfLocation[NUMLOCATIONS] = {
    "",
    "at the front door to the castle",
    "in the vestibule",
    "in the entry hall",
    "in Peacock Alley",
    "in Peacock Alley",
    "in Peacock Alley",
    "in Peacock Alley",
    "in Peacock Alley",
    "in the large formal dining room",
    "in a large conservatory with a stained glass dome sky light",
    "in the breakfast room",
    "in the serving room",
    "in the kitchen",
    "in a hallway",
    "in a hallway",
    "in the elevator on the main floor",
    "in a study, with a large desk and fireplace",
    "in the library",
    "in the Great Hall",
    "on the lower staircase",
    "on the landing",
    "in the oak drawing room",
    "in the smoking room",
    "on the covered porch",
    "in the billiards room",
    "at the west end of a hallway",
    "in a hallway",
    "in a hallway",
    "in a hallway",
    "at the east end of a hallway",
    "in a corridor",
    "in a corridor",
    "in a narrow hallway",
    "in the Round Room",
    "in the pipe organ loft",
    "in a large bedroom",
    "in the lady's sitting room",
    "in a small bedroom",
    "in Sir Henry's bedroom",
    "in a sitting room",
    "in Lady Pellatt's bedroom",
    "in Sir Henry's sitting room",
    "in the guest bedroom",
    "in the elevator on the second floor",
    "in the children's bedroom",
    "in the servant's bedroom",
    "in the corner bedroom",
    "in the linen room",
    "in a bedroom",
    "in the bath room",
    "on the upper staircase",
    "on a narrow staircase",
    "in the wine cellar",
    "in a damp underground tunnel",
    "in a damp underground tunnel",
    "in a damp underground tunnel",
    "in the steam plant",
    "in a damp underground tunnel",
    "in the stables",
};

/* DATA */

/* Inventory of what player is carrying */
Item_t Inventory[MAXITEMS];

/* Location of each item. Index is the item number, returns the location. 0 if item is gone */
Location_t locationOfItem[LastItem+1];

/* Map. Given a location and a direction to move, returns the location it connects to, or 0 if not a valid move. Map can change during game play. */
Location_t Move[NUMLOCATIONS][6] = {
    /* N  S  E  W  U  D */
    {  0, 0, 0, 0, 0, 0 },    /* 0 NoLocation */
    {  0, 0, 0, 0, 0, 0 },    /* 1 FrontEntrance */
    {  3, 1, 0, 0, 0, 0 },    /* 2 Vestibule */
    {  6, 2, 0, 0, 0, 0 },    /* 3 Entry */
    {  9,14, 5,10, 0, 0 },    /* 4 PeacockAlley1 */
    { 18,17, 6, 4, 0, 0 },    /* 5 PeacockAlley2 */
    {  0, 3, 7, 5, 0, 0 },    /* 6 PeacockAlley3 */
    { 19, 0, 8, 6,20, 0 },    /* 7 PeacockAlley4 */
    { 22,23, 0, 7, 0, 0 },    /* 8 PeacockAlley5 */
    {  0, 4, 0, 0, 0, 0 },    /* 9 DiningRoom */
    {  0, 0, 4, 0, 0, 0 },    /* 10 Conservatory */
    {  0,12, 14,0, 0, 0 },    /* 11 BreakfastRoom */
    { 11,13, 15,0, 0, 0 },    /* 12 ServingRoom */
    { 12, 0, 0, 0, 0, 0 },    /* 13 Kitchen */
    { 4, 15,16,11, 0, 0 },    /* 14 Hallway1 */
    { 14, 0, 0,12, 0, 0 },    /* 15 Hallway2 */
    {  0, 0, 0,14,44, 0 },    /* 16 Elevator1 */
    {  5, 0, 0, 0, 0, 0 },    /* 17 Study */
    { 24, 5,19, 0, 0, 0 },    /* 18 Library */
    {  0, 7,22,18, 0, 0 },    /* 19 GreatHall */
    {  7, 0, 0, 0,21, 0 },    /* 20 Stairs1 */
    {  0, 0, 0, 0,51,20 },    /* 21 Landing */
    {  0, 8, 0,19, 0, 0 },    /* 22 OakDrawingRoom */
    {  8,25, 0, 0, 0, 0 },    /* 23 SmokingRoom */
    {  0,18, 0, 0, 0, 0 },    /* 24 CoveredPorch */
    { 23, 0, 0, 0, 0, 0 },    /* 25 BilliardsRoom */
    { 41,31,27, 0, 0, 0 },    /* 26 Hallway10 */
    { 42,48,28,26, 0, 0 },    /* 27 Hallway11 */
    { 39,36,29,27, 0, 0 },    /* 28 Hallway12 */
    {  0, 0,30,28, 0,51 },    /* 29 Hallway13 */
    { 33,49, 0,29, 0, 0 },    /* 30 Hallway14 */
    { 26,32,38,43, 0, 0 },    /* 31 Corridor1 */
    { 31, 0,46,45, 0, 0 },    /* 32 Corridor2 */
    { 47,30,34,35, 0, 0 },    /* 33 Narrowhallway */
    {  0, 0, 0,33, 0, 0 },    /* 34 RoundRoom */
    {  0, 0,33, 0, 0, 0 },    /* 35 PipeOrganLoft */
    { 28,50, 0, 0, 0, 0 },    /* 36 Bedroom1 */
    {  0,41, 0, 0, 0, 0 },    /* 37 SittingRoom1 */
    {  0, 0, 0,31, 0, 0 },    /* 38 Bedroom3 */
    {  0,28, 0,42, 0, 0 },    /* 39 SirHenrysBedroom */
    {  0, 0,47, 0, 0, 0 },    /* 40 SittingRoom3 */
    { 37,26, 0, 0, 0, 0 },    /* 41 LadysBedroom */
    {  0,27,39, 0, 0, 0 },    /* 42 SittingRoom2 */
    {  0, 0,31, 0, 0, 0 },    /* 43 GuestBedroom */
    {  0, 0,48, 0, 0,16 },    /* 44 Elevator2 */
    {  0, 0,32, 0, 0, 0 },    /* 45 ChildrensBedroom */
    {  0, 0, 0,32, 0, 0 },    /* 46 ServantsBedroom */
    {  0,33, 0,40, 0, 0 },    /* 47 Bedroom4 */
    { 27, 0, 0,44, 0, 0 },    /* 48 LinenRoom */
    { 30, 0, 0, 0, 0, 0 },    /* 49 Bedroom2 */
    { 36, 0, 0, 0, 0, 0 },    /* 50 Bath */
    {  0, 0, 0, 0,29,21 },    /* 51 Stairs2 */
    {  0, 0, 0, 0,17,54 },    /* 52 Stairs3 */
    { 54, 0, 0, 0, 0, 0 },    /* 53 WineCellar */
    {  0,53,55, 0,52, 0 },    /* 54 Tunnel1 */
    {  0, 0,56,54, 0, 0 },    /* 55 Tunnel2 */
    { 57, 0,58,55, 0, 0 },    /* 56 Tunnel3 */
    {  0,56, 0, 0, 0, 0 },    /* 57 SteamPlant */
    {  0, 0, 0,56,59, 0 },    /* 58 Tunnel4 */
    {  0, 0, 0, 0, 0,58 },    /* 59 Stables */

};

/* Current location */
Location_t currentLocation;

/* Number of turns played in game */
int turnsPlayed;

/* True if player has lit the candle. */
number candleLit;

/* True if Auntie is tied up. */
number auntieTied;

/* Set when game is over */
number gameOver;

const char *introText =
"       Skye's Castle Adventure\n"
"           By Jeff Tranter\n\n"
"Your great-great-grandfather built a\n"
"castle, but the family fell on hard\n"
"times and it has been vacant for 80\n"
"years. Occasionally, family members\n"
"visit the castle, although it is old\n"
"and dusty and possibly not safe. Today\n"
"your young granddaughter went to\n"
"visit the castle with her aunt, but\n"
"they did not return in the evening.\n"
"Maybe you should have called the police,\n"
"but instead you decide to go over\n"
"there and find them on your own. It\n"
"looks like a bad storm is brewing, and\n"
"the castle has no electricity, so you\n"
"had better find them before it gets\n"
"too dark.\n";

const char *helpString = "Valid commands:\ngo east/west/north/south/up/down \nlook\nuse <object>\nexamine <object>\ntake <object>\ndrop <object>\ninventory\nhelp\nquit\nYou can abbreviate commands and\ndirections to the first letter.\nType just the first letter of\na direction to move.\n";

/* Line of user input */
char buffer[40];

/* Clear the screen */
void clearScreen()
{
#if defined(__APPLE2__)
    clrscr();
#else
    number i;
    for (i = 0; i < 24; ++i)
        printf("\n");
#endif
}

/* Return 1 if carrying an item */
number carryingItem(const char *item)
{
    number i;

    for (i = 0; i < MAXITEMS; i++) {
        if ((Inventory[i] != 0) && (!strcasecmp(DescriptionOfItem[Inventory[i]], item)))
            return 1;
    }
    return 0;
}

/* Return 1 if item it at current location (not carried) */
number itemIsHere(const char *item)
{
    number i;

    /* Find number of the item. */
    for (i = 1; i <= LastItem; i++) {
        if (!strcasecmp(item, DescriptionOfItem[i])) {
            /* Found it, but is it here? */
            if (locationOfItem[i] == currentLocation) {
                return 1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

/* Inventory command */
void doInventory()
{
    number i;
    int found = 0;

    printf("%s", "You are carrying:\n");
    for (i = 0; i < MAXITEMS; i++) {
        if (Inventory[i] != 0) {
            printf("  %s\n", DescriptionOfItem[Inventory[i]]);
            found = 1;
        }
    }
    if (!found)
        printf("  nothing\n");
}

/* Help command */
void doHelp()
{
    printf("%s", helpString);
}

/* Look command */
void doLook()
{
    number i, loc, seen;

    printf("You are %s.\n", DescriptionOfLocation[currentLocation]);

    if ((turnsPlayed > 20) && !candleLit) {
        printf("It is dark. You can't see.\n");
    } else {
        seen = 0;
        printf("You see:\n");
        for (i = 1; i <= LastItem; i++) {
            if (locationOfItem[i] == currentLocation) {
                printf("  %s\n", DescriptionOfItem[i]);
                seen = 1;
            }
        }
        if (!seen)
            printf("  nothing special\n");
    }

    printf("You can go:");

    for (i = North; i <= Down; i++) {
        loc = Move[currentLocation][i];
        if (loc != 0) {
            printf(" %s", DescriptionOfDirection[i]);
        }
    }
    printf("\n");
}

/* Quit command */
void doQuit()
{
    printf("%s", "Are you sure you want to quit (y/n)? ");
    fgets(buffer, sizeof(buffer)-1, stdin);
    if (tolower(buffer[0]) == 'y') {
        gameOver = 1;
    }
}

/* Drop command */
void doDrop()
{
    number i;
    char *sp;
    char *item;

    /* Command line should be like "D[ROP] ITEM" Item name will be after after first space. */
    sp = strchr(buffer, ' ');
    if (sp == NULL) {
        printf("Drop what?\n");
        return;
    }

    item = sp + 1;

    /* See if we have this item */
    for (i = 0; i < MAXITEMS; i++) {
        if ((Inventory[i] != 0) && (!strcasecmp(DescriptionOfItem[Inventory[i]], item))) {
            /* We have it. Add to location. */
            locationOfItem[Inventory[i]] = currentLocation;
            /* And remove from inventory */
            Inventory[i] = (Item_t)0;
            printf("Dropped %s.\n", item);
            ++turnsPlayed;
            return;
        }
    }
    /* If here, don't have it. */
    printf("Not carrying %s.\n", item);
}

/* Take command */
void doTake()
{
    number i, j;
    char *sp;
    char *item;

    /* Command line should be like "T[AKE] ITEM" Item name will be after after first space. */
    sp = strchr(buffer, ' ');
    if (sp == NULL) {
        printf("Take what?\n");
        return;
    }

    item = sp + 1;

    if (carryingItem(item)) {
        printf("Already carrying it.\n");
        return;
    }

    /* Find number of the item. */
    for (i = 1; i <= LastItem; i++) {
        if (!strcasecmp(item, DescriptionOfItem[i])) {
            /* Found it, but is it here? */
            if (locationOfItem[i] == currentLocation) {
            /* It is here. Add to inventory. */
            for (j = 0; j < MAXITEMS; j++) {
                if (Inventory[j] == 0) {
                    Inventory[j] = (Item_t)i;
                    /* And remove from location. */
                    locationOfItem[i] = NoLocation;
                    printf("Took %s.\n", item);
                    ++turnsPlayed;
                    return;
                }
            }

            /* Reached maximum number of items to carry */
            printf("You can't carry any more. Drop something.\n");
            return;
            }
        }
    }

    /* If here, don't see it. */
    printf("I see no %s here.\n", item);
}

/* Go command */
void doGo()
{
    char *sp;
    char dirChar;
    Direction_t dir;

    /* Command line should be like "G[O] N[ORTH]" Direction will be
       the first letter after a space. Or just a single letter
       direction N S E W U D or full directon NORTH etc. */

    sp = strrchr(buffer, ' ');
    if (sp != NULL) {
        dirChar = *(sp+1);
    } else {
        dirChar = buffer[0];
    }
    dirChar = tolower(dirChar);

    if (dirChar == 'n') {
        dir = North;
    } else if (dirChar == 's') {
        dir = South;
    } else if (dirChar == 'e') {
        dir = East;
    } else if (dirChar == 'w') {
        dir = West;
    } else if (dirChar == 'u') {
        dir = Up;
    } else if (dirChar == 'd') {
        dir = Down;
    } else {
        printf("Go where?\n");
        return;
    }

    if (Move[currentLocation][dir] == 0) {
        printf("You can't go %s from here.\n", DescriptionOfDirection[dir]);
        return;
    }

    /* Special case: can't leave castle until you have Auntie, Skye,
       cat, and doll. */
    if ((currentLocation == Vestibule) && (dir == South)) {
        if (!carryingItem("Skye") && !itemIsHere("Skye")) {
            printf("You can't leave without Skye!\n");
            return;
        }
        if (!carryingItem("Auntie") && !itemIsHere("Auntie")) {
            printf("You can't leave without Auntie!\n");
            return;
        }
        if (!carryingItem("cat") && !itemIsHere("cat")) {
            printf("Skye won't leave without her cat, Bailey!\n");
            return;
        }
        if (!carryingItem("doll") && !itemIsHere("doll")) {
            printf("Skye won't leave without her doll!\n");
            return;
        }
        /* You won! */
        printf("Congratulations, you won the game!\nI hope you had as much fun playing\nthe game as I did creating it.\nJeff Tranter <tranter@pobox.com>\n");
        gameOver = 1;
        return;
    }

    /* We can move */
    currentLocation = Move[currentLocation][dir];
    printf("You are %s.\n", DescriptionOfLocation[currentLocation]);
    ++turnsPlayed;
}

/* Examine command */
void doExamine()
{
    char *sp;
    char *item;

    /* Command line should be like "E[XAMINE] ITEM" Item name will be after after first space. */
    sp = strchr(buffer, ' ');
    if (sp == NULL) {
        printf("Examine what?\n");
        return;
    }

    item = sp + 1;
    ++turnsPlayed;

    /* Examine fireplace - not an object */
    if (!strcasecmp(item, "fireplace") && (currentLocation == Study)) {
        printf("On either side of the fireplace are\nsecret panels, which now open and\nreveal staircases.\n");
        Move[Study][Up] = Hallway11;
        Move[Study][Down] = Stairs3;
        return;
    }

    /* Make sure item is being carried or is in the current location */
    if (!carryingItem(item) && !itemIsHere(item)) {
        printf("I don't see it here.\n");
        return;
    }

    /* Examine Newspaper */
    if (!strcasecmp(item, "newspaper")) {
        printf("It reads:\n"
               "\"Pellatt's Folly? Five million dollar \"house on the hill\" took\n"
               "three years to build. Sole occupants of 98 room castle are Toronto\n"
               "financier and invalid wife.\"\n");
        return;
    }

    /* Examine Sandwich */
    if (!strcasecmp(item, "sandwich")) {
        printf("A peanut butter sandwich. It looks\nfresh, so someone must have been here\nrecently.\n");
        return;
    }

    /* Examine Cat */
    if (!strcasecmp(item, "cat")) {
        printf("It is Skye's cat, Bailey. How did he get here?\n");
        return;
    }

    /* Examine Wine */
    if (!strcasecmp(item, "wine")) {
        printf("An old bottle of wine. It looks very dusty and dirty.\n");
        return;
    }

    /* Examine Auntie */
    if (!strcasecmp(item, "auntie")) {
        if (auntieTied) {
            printf("She is tied up and gagged and can't speak.\nBut where is Skye?\n");
        } else {
            printf("Auntie says: Someone attacked me and\ntied me up! Skye went back into the\ntunnel to get help.\n");
        }
        return;
    }

    /* Examine doll */
    if (!strcasecmp(item, "doll")) {
        printf("It is a doll. It looks like one that belongs to Skye.\n");
        return;
    }

    /* Examine book */
    if (!strcasecmp(item, "book")) {
        printf("It is titled \"The Curse of the Pharaohs\"\nby George Crabtree.\n");
        return;
    }

    /* Examine note */
    if (!strcasecmp(item, "note")) {
        printf("It says: \"Reginald was here.\"\n");
        return;
    }

    /* Examine business card */
    if (!strcasecmp(item, "business card")) {
        printf("It reads: \"Thomas Ridgway - chauffeur\"\n");
        return;
    }

    /* Examine menu */
    if (!strcasecmp(item, "menu")) {
        printf("It reads:\n"
               "First Course\n"
               "Hors D'Oeuvres\n"
               "Oysters\n"
               "\n"
               "Second Course\n"
               "Consomme Olga\n"
               "Cream of Barley\n"
               "\n"
               "Third Course\n"
               "Poached Salmon with Mousseline Sauce, Cucumbers\n"
               "\n"
               "Fourth Course\n"
               "Filet Mignons Lili\n"
               "Saute of Chicken, Lyonnaise\n"
               "Vegetable Marrow Farci\n"
               "\n"
               "Fifth Course\n"
               "Lamb, Mint Sauce\n"
               "Roast Duckling, Apple Sauce\n"
               "Sirloin of Beef, Chateau Potatoes\n"
               "Green Pea\n"
               "Creamed Carrots\n"
               "Boiled Rice\n"
               "Parmentier & Boiled New Potatoes\n"
               "\n"
               "Sixth Course\n"
               "Punch Romaine\n"
               "\n"
               "Seventh Course\n"
               "Roast Squab & Cress\n"
               "\n"
               "Eighth Course\n"
               "Cold Asparagus Vinaigrette\n"
               "\n"
               "Ninth Course\n"
               "Pate de Foie Gras\n"
               "Celery\n"
               "\n"
               "Tenth Course\n"
               "Waldorf Pudding\n"
               "Peaches in Chartreuse Jelly\n"
               "Chocolate & Vanilla Eclairs\n"
               "French Ice Cream\n");
        return;
    }

   /* Nothing special about this item */
   printf("You see nothing special about it.\n");
}

/* Use command */
void doUse()
{
    char *sp;
    char *item;

    /* Command line should be like "U[SE] ITEM" Item name will be after after first space. */
    sp = strchr(buffer, ' ');
    if (sp == NULL) {
        printf("Use what?\n");
        return;
    }

    item = sp + 1;

    /* Make sure item is being carried or is in the current location */
    if (!carryingItem(item) && !itemIsHere(item)) {
        printf("I don't see it here.\n");
        return;
    }

    ++turnsPlayed;

    /* Use key */
    if (!strcasecmp(item, "key") && (currentLocation == FrontEntrance)) {
        printf("You insert the key and unlock the door.\n");
        Move[FrontEntrance][North] = Vestibule;
        return;
    }

    /* Use (drink) wine. */
    if (!strcasecmp(item, "wine")) {
        printf("It looks and smells bad, but you drink some anyway.\n");
        printf("You feel very sick and pass out.\n");
        gameOver = 1;
        return;
    }

    /* Use knife */
    if (!strcasecmp(item, "knife")) {
        if ((carryingItem("auntie") || itemIsHere("auntie")) && (auntieTied == 1)) {
            printf("You cut the ropes with the knife.\n");
            printf("Auntie says: Someone attacked me and tied me up!\nSkye went back into the tunnel to get help.\n");
            auntieTied = 0;
            return;
        }
    }

    /* Use matches to light candle */
    if (!strcasecmp(item, "matches")) {
        if (carryingItem("candle") || itemIsHere("candle")) {
            printf("You light the candle. You can see!\n");
            candleLit = 1;
            return;
        } else {
            printf("Nothing here to light\n");
        }
    }

    /* Use steel bar to get Skye out of steam boiler */
    if (!strcasecmp(item, "steel bar") && (currentLocation == SteamPlant)) {
        if (locationOfItem[Skye] == 0) {
            printf("You pry the boiler open with the steel bar.\nSkye was trapped inside! She is safe now.\n");
            /* Skye is now in steam plant. */
            locationOfItem[Skye] = SteamPlant;
            return;
        }
    }

    /* Default */
    printf("Nothing happens\n");
}

/* Prompt user and get a line of input */
void prompt()
{
    printf("? ");
    fgets(buffer, sizeof(buffer)-1, stdin);

    /* Remove trailing newline */
    buffer[strlen(buffer)-1] = '\0';
}

/* Do special things unrelated to command typed. */
void doActions()
{
    /* Check for getting dark. */
    if ((turnsPlayed == 10) && !candleLit) {
        printf("It will be getting dark soon. You need\nsome kind of light or soon you won't\nbe able to see.\n");
    }

    /* Check if it got dark before you lit the candle. */
    if ((turnsPlayed == 20) && !candleLit) {
        printf("It is dark out and you have no light.\nYou stumble around for a while and\nthen fall, hit your head, and pass out.\n");
        gameOver = 1;
        return;
    }

    /* Once lit, blow out candle every 15 turns. */
    if ((turnsPlayed % 15 == 0) && candleLit) {
        printf("The candle blows out!\n");
        candleLit = 0;
    }

    /* Give hint if in steam plant */
    if ((currentLocation == SteamPlant) && (auntieTied == 0) && (locationOfItem[Skye] == 0) && !carryingItem("Skye")) {
        printf("You see a steam boiler here. You can hear someone crying inside.\n");
    }
}

/* Set variables to values for start of game */
void initialize()
{
    currentLocation = FrontEntrance;
    turnsPlayed = 0;
    gameOver= 0;
    candleLit = 0;
    auntieTied = 1;

    /* These doors can get changed during game and may need to be reset */
    Move[FrontEntrance][North] = NoLocation;
    Move[Study][Up] = NoLocation;
    Move[Study][Down] = NoLocation;

    /* Set inventory to default */
    memset(Inventory, 0, sizeof(Inventory[0])*MAXITEMS);
    Inventory[0] = Key;

    /* Put items in their default locations */
    locationOfItem[0]            = NoLocation;
    locationOfItem[PoolCue]      = BilliardsRoom;
    locationOfItem[Umbrella]     = Vestibule;
    locationOfItem[Newspaper]    = Library;
    locationOfItem[Sandwich]     = Study;
    locationOfItem[Cat]          = PipeOrganLoft;
    locationOfItem[Wine]         = WineCellar;
    locationOfItem[Knife]        = Kitchen;
    locationOfItem[Candle]       = GreatHall;
    locationOfItem[Matches]      = SmokingRoom;
    locationOfItem[Auntie]       = Stables;
    locationOfItem[Doll]         = ChildrensBedroom;
    locationOfItem[SteelBar]     = Conservatory;
    locationOfItem[Skye]         = NoLocation; /* Added later */
    locationOfItem[Book]         = SirHenrysBedroom;
    locationOfItem[Hairbrush]    = SittingRoom3;
    locationOfItem[Note]         = Bedroom2;
    locationOfItem[Perfume]      = LadysBedroom;
    locationOfItem[BusinessCard] = ServantsBedroom;
    locationOfItem[Soap]         = Bath;
    locationOfItem[Menu]         = DiningRoom;
    locationOfItem[LightBulb]    = OakDrawingRoom;
}

/* Main program (obviously) */
int main(void)
{
    while (1) {
        initialize();
        clearScreen();
        printf("%s", introText);

        while (!gameOver) {
            prompt();
            if (buffer[0] == '\0') {
                /* Ignore empty line */
            } else if (tolower(buffer[0]) == 'h') {
                doHelp();
            } else if (tolower(buffer[0]) == 'i') {
                doInventory();
            } else if ((tolower(buffer[0]) == 'g')
                       || !strcasecmp(buffer, "n") || !strcasecmp(buffer, "s")
                       || !strcasecmp(buffer, "e") || !strcasecmp(buffer, "w")
                       || !strcasecmp(buffer, "u") || !strcasecmp(buffer, "d")
                       || !strcasecmp(buffer, "north") || !strcasecmp(buffer, "south")
                       || !strcasecmp(buffer, "east") || !strcasecmp(buffer, "west")
                       || !strcasecmp(buffer, "up") || !strcasecmp(buffer, "down")) {
                doGo();
            } else if (tolower(buffer[0]) == 'l') {
                doLook();
            } else if (tolower(buffer[0]) == 't') {
                doTake();
            } else if (tolower(buffer[0]) == 'e') {
                doExamine();
            } else if (tolower(buffer[0]) == 'u') {
                doUse();
            } else if (tolower(buffer[0]) == 'd') {
                doDrop();
            } else if (tolower(buffer[0]) == 'q') {
                doQuit();
            } else {
                printf("I don't understand. Try 'help'.\n");
            }

            /* Handle special actions. */
            doActions();
        }

        printf("Game over after %d turns.\n", turnsPlayed);
        printf("%s", "Do you want to play again (y/n)? ");
        fgets(buffer, sizeof(buffer)-1, stdin);
        if (tolower(buffer[0]) == 'n') {
            break;
        }
    }
    return 0;
}
