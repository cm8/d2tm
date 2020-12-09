/*
 * cBuildingListFactory.cpp
 *
 *  Created on: Aug 2, 2009
 *      Author: Stefan
 */

#include "../include/d2tmh.h"

cBuildingListFactory *cBuildingListFactory::instance = NULL;

cBuildingListFactory::cBuildingListFactory() {
}

cBuildingListFactory::~cBuildingListFactory() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}


cBuildingListFactory *cBuildingListFactory::getInstance() {
	if (instance == nullptr) {
		instance = new cBuildingListFactory();
	}

	return instance;
}

int cBuildingListFactory::getButtonDrawY() {
    // 6 pixels margin below horizontal candybar, so it lines up nice horizontally with the sphere
    // of the vertical candybar at the left of the building list icons
	return cSideBar::TopBarHeight + cSideBar::HeightOfMinimap + cSideBar::HorizontalCandyBarHeight + 6;
}

int cBuildingListFactory::getButtonDrawXStart() {
	return (game.screen_x - 200) + 2;
}


/**
 * Initialize list according to techlevel. This will also remove all previously set icons.
 *
 * @param list
 * @param listId
 * @param techlevel
 * @param house
 */
void cBuildingListFactory::initializeList(cBuildingList *list, int listId, int house) {
	assert(list != NULL);

	// first clear the list
	list->removeAllItems();

	// Y is the same for all list buttons
	list->setButtonDrawY(getButtonDrawY());

    int widthOfButtonIncludingMargin = 33;
    list->setButtonWidth(widthOfButtonIncludingMargin);
//    list->setButtonHeight()

	int startX = getButtonDrawXStart();

	list->setAvailable(false);

	// now set it up
	if (listId == LIST_CONSTYARD) {
		list->setButtonDrawX(startX);
		list->setButtonIconIdPressed(LIST_BTN_CONSTYARD);
		list->setButtonIconIdUnpressed(LIST_BTN_CONSTYARD);
	}


    startX += widthOfButtonIncludingMargin;

	// other lists, have 40 pixels more Y , but the X remains the same
	// now set it up
	if (listId == LIST_FOOT_UNITS) {
        list->setButtonDrawX(startX);
        list->setButtonIconIdPressed(LIST_BTN_INFANTRY);
        list->setButtonIconIdUnpressed(LIST_BTN_INFANTRY);
	}

    startX += widthOfButtonIncludingMargin;

	if (listId == LIST_UNITS) {
		list->setButtonDrawX(startX);
		list->setButtonIconIdPressed(LIST_BTN_FACTORY);
		list->setButtonIconIdUnpressed(LIST_BTN_FACTORY);
	}

	startX += widthOfButtonIncludingMargin;

	if (listId == LIST_STARPORT) {
        list->setButtonDrawX(startX);
		list->setButtonIconIdPressed(LIST_BTN_STARPORT);
		list->setButtonIconIdUnpressed(LIST_BTN_STARPORT);

		list->addUnitToList(INFANTRY, 0);
		list->addUnitToList(TROOPERS, 0);
		list->addUnitToList(TRIKE, 0);
		list->addUnitToList(QUAD, 0);
		list->addUnitToList(TANK, 0);
		list->addUnitToList(MCV, 0);
		list->addUnitToList(HARVESTER, 0);
		list->addUnitToList(LAUNCHER, 0);
		list->addUnitToList(SIEGETANK, 0);
		list->addUnitToList(CARRYALL, 0);
	}

	startX += widthOfButtonIncludingMargin;

	if (listId == LIST_PALACE) {
        list->setButtonDrawX(startX);
		list->setButtonIconIdPressed(LIST_BTN_PALACE);
		list->setButtonIconIdUnpressed(LIST_BTN_PALACE);

		// special weapons
		switch (house) {
			case ATREIDES:
				list->addUnitToList(UNIT_FREMEN_THREE, 0);
				break;
			case HARKONNEN:
				list->addUnitToList(MISSILE,0);
				break;
			case ORDOS:
				list->addUnitToList(SABOTEUR, 0);
				break;
		}
	}

	startX += widthOfButtonIncludingMargin;

	if (listId == LIST_UPGRADES) {
        list->setButtonDrawX(startX);
        // temp, use CONST YARD
		list->setButtonIconIdPressed(LIST_BTN_UPGRADE);
		list->setButtonIconIdUnpressed(LIST_BTN_UPGRADE);

		// the contents of the list is determined elsewhere
	}

}

/**
 * Create new instance of list, initialize and return it.
 *
 * @param listId
 * @param techlevel
 * @return
 */
cBuildingList * cBuildingListFactory::createList(int listId, int house) {
	cBuildingList * list = new cBuildingList(listId);
    initializeList(list, listId, house);
	list->setTypeOfList(listId); // list id == type (see cSideBarFactory)
	return list;
}
