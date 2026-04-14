#ifndef MENU_PROFILES_H
#define MENU_PROFILES_H

#include "main.h"

/* Full profiles editor — called from settings_menu when "Profiles" group selected */
void profiles_menu(void);

/* Quick profile selector popup — called on handle change if enabled */
void profiles_popup(enum handles h);

#endif /* MENU_PROFILES_H */
