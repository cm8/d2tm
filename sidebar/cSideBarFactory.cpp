/*
 * cSideBarFactory.cpp
 *
 *  Created on: Aug 2, 2009
 *      Author: Stefan
 */

#include "../include/d2tmh.h"

cSideBarFactory *cSideBarFactory::instance = NULL;

cSideBarFactory::cSideBarFactory() {
}

cSideBarFactory::~cSideBarFactory() {
    if (instance) {
        delete instance;
        instance = nullptr;
    }
}

cSideBarFactory *cSideBarFactory::getInstance() {
	if (instance == NULL) {
		instance = new cSideBarFactory();
	}

	return instance;
}

// construct a cSideBar
cSideBar * cSideBarFactory::createSideBar(cPlayer& thePlayer, int house) {
	assert(&thePlayer);
	cSideBar *sidebar = new cSideBar(thePlayer);

	for (int listId = LIST_NONE; listId < LIST_MAX; listId++) {
		cBuildingList *list = cBuildingListFactory::getInstance()->createList(listId, house);
		sidebar->setList(listId, list);
	}

	return sidebar;
}

