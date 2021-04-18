/*

  Dune II - The Maker

  Author : Stefan Hendriks
  Contact: stefanhen83@gmail.com
  Website: http://dune2themaker.fundynamic.com

  2001 - 2020 (c) code by Stefan Hendriks

  */

#define TILESIZE_WIDTH_PIXELS 32
#define TILESIZE_HEIGHT_PIXELS 32

#ifndef CMAP_H
#define CMAP_H

class cMap {

public:

	cMap();
	~cMap();

    void init(int width, int height);

	bool canDeployUnitAtCell(int iCell, int iUnitId);
	bool occupied(int iCell);
	bool occupied(int iCll, int iUnitID);
	bool occupiedInDimension(int iCell, int dimension);
	bool occupiedByType(int iCell);

	/**
	 * Returns cell , taking given map width/height into account. This includes the invisible border around the map.
	 * If you want to take the invisible border into account use getCellWithMapBorders instead.
	 * @param x
	 * @param y
	 * @return
	 */
    int getCellWithMapDimensions(int x, int y);

    /**
    * Return map cell; taking the map borders into account. If x or y falls out of bounds, this function will return
    * -1. If you want to include the invisible map borders, use getCellWithMapDimensions instead.
    *
    * @param x
    * @param y
    * @return
    */
    int getCellWithMapBorders(int x, int y);


    /**
	Shortcut method, which takes cells as arguments, creates x1, y1 and x2, y2 out of them
	and runs the normal distance method to get the distance.
    **/
    double distance(int cell1, int cell2);

    /**
        Return a cell from an X,Y coordinate.

        Remember that coordinates are 1-64 based. While the array in Map (tCell) is 0-till 4096.

        This means that the coordinate 1,1 is NOT the first row, but it is : 0,0. This also means the
        MAX at the right is *not* MAP_W_MAX, but it is MAP_W_MAX - 1.
        (it is 0-63 instead of 1-64).

        This method will not do any fancy tricks to fix the boundaries, instead it will assert its input and output.

        Use <b>getCellWithMapBorders</b> if you want a safe way to get a cell within the <i>playable</i> map boundaries.
        Use <b>getCellWithMapDimensions</b> if you want a safe way to get a cell within the <i>maximum</i> map boundaries.
    **/
    int makeCell(int x, int y);

    int getAbsoluteXPositionFromCell(int cell);

    int getAbsoluteYPositionFromCell(int cell);

    /**
     * Like absoluteX position, but then centers within cell (ie adds half tile)
     * @param cell
     * @return
     */
    int getAbsoluteXPositionFromCellCentered(int cell);

    /**
     * Like absoluteX position, but then centers within cell (ie adds half tile)
     * @param cell
     * @return
     */
    int getAbsoluteYPositionFromCellCentered(int cell);

    int getCellAbove(int c);

	int getCellBelow(int c);

    int getCellLeft(int c);

	int getCellRight(int c);

	int getCellUpperLeft(int c);

	int getCellUpperRight(int c);

	int getCellLowerLeft(int c);

    int getCellLowerRight(int c);

    /**
     * returns true if one cell is adjacent to another cell
     */
	bool isCellAdjacentToOtherCell(int thisCell, int otherCell);

    /**
        The X coordinate is found by finding out how many 'rows' (the Y) are there, then
        the remaining of that value is the X.
    **/
    int getCellX(int c);

    /**
	    The Y coordinate is found by finding as many MAP_W_MAX can fit in the given cell
    **/
    int getCellY(int c);

	/**
	 * Returns true/false when x,y coordinate is within bounds of the map. Taking invisible boundary into account.
	 * @param x
	 * @param y
	 * @return
	 */
	bool isWithinBoundaries(int x, int y) {
	    // in a map of 64x64, the outer boundaries are impassable
	    // so; the actual valid values are 1...62 (because the coordinates
	    // are 0 based. Ie, coordinate 63 means it is at the 64th tile.
	    // hence, if coordinate 63 is passed in, it means it is out of bounds.
	    // but coordinate 62 is within bounds.

	    // since the width/height passed in the constructor is not 0 based (but 1 based)
	    // we need to take that into account when checking width/height (hence the -2)

	    // Example:
	    // width=64
	    // height=64
	    // ie: x = 63 ... 63 =< (64-2=62) -> out of bounds
	    //     x = 62 ... 62 =< (64-2-62) -> at the edge
	    //     x = 0  ...  0 > 0 ? no
	    //     x = 1  ...  1 > 0 ? yes, in bounds

	    return  x > 0 && x <= (width-2) &&
	            y > 0 && y <= (height -2);
	}

    double distance(int x1, int y1, int x2, int y2);
    int findCloseMapBorderCellRelativelyToDestinationCel(int destinationCell);

    // Drawing
    int  mouse_draw_x();
    int  mouse_draw_y();
	void draw_units_2nd();
	void draw_units();
    void draw_bullets();

    void smudge_increase(int iType, int iCell);


    void thinkInteraction();
    void think_minimap();
    void draw_think();

    void clear_all(int playerId);
    void clear_spot(int cell, int size, int playerId);

    /**
     * Clears shroud for all players at cell
     * @param c
     * @param size
     */
    void clear_spot(int cell, int size);

    /**
     * returns pointer to cell or nullptr when not valid
     * @param cellNr
     * @return
     */
    tCell * getCell(int cellNr) {
        if (cellNr < 0) return nullptr;
        if (cellNr >= MAX_CELLS) return nullptr;
        return &cell[cellNr];
    }
    
    int getCellType(int cellNr) {
        tCell *pCell = getCell(cellNr);
        if (pCell) return pCell->type;
        return -1;
    }

    /**
     * Returns ID at layer on cell. Returns -1 when layer or cell is not valid.
     * @param cellNr
     * @param idLayer
     * @return
     */
    int cellGetIdFromLayer(int cellNr, int idLayer) {
        if (idLayer < 0 || idLayer >= 4) return -1; // safeguard layers
        tCell *pCell = getCell(cellNr);
        if (pCell) return pCell->id[idLayer];
        return -1;
    }

    void cellSetIdForLayer(int cellNr, int idLayer, int id) {
        if (idLayer < 0 || idLayer >= 4) return; // safeguard layers
        tCell *pCell = getCell(cellNr);
        if (!pCell) return;
        pCell->id[idLayer] = id;
    }

    void cellResetIdFromLayer(int cellNr, int idLayer) {
        if (idLayer < 0 || idLayer >= 4) return; // safeguard layers
        tCell *pCell = getCell(cellNr);
        if (!pCell) return;
        pCell->id[idLayer] = -1;
    }

    int getCellTile(int cellNr) {
        tCell *pCell = getCell(cellNr);
        if (pCell) return pCell->tile;
        return -1;
    }

    int getCellHealth(int cellNr) {
        tCell *pCell = getCell(cellNr);
        if (pCell) return pCell->health;
        return -1;
    }

    int getCellCredits(int cellNr) {
        tCell *pCell = getCell(cellNr);
        if (pCell) return pCell->credits;
        return -1;
    }

    bool isCellPassable(int cellNr) {
        tCell *pCell = getCell(cellNr);
        if (pCell) return pCell->passable;
        return false;
    }

    bool isCellPassableFoot(int cellNr) {
        tCell *pCell = getCell(cellNr);
        if (pCell) return pCell->passableFoot;
        return false;
    }

    /**
     * returns if id of layer units is filled (if so, a unit is recorded to be on this cell)
     * @param cellNr
     * @return
     */
    int getCellIdUnitLayer(int cellNr) {
        return cellGetIdFromLayer(cellNr, MAPID_UNITS);
    }

    /**
     * returns if id of layer airborn units is filled (if so, an airborn unit is recorded to be on this cell)
     * @param cellNr
     * @return
     */
    int getCellIdAirUnitLayer(int cellNr) {
        return cellGetIdFromLayer(cellNr, MAPID_AIR);
    }

    /**
     * returns if id of layer worms is filled (if so, a worm is recorded to be on this cell)
     * @param cellNr
     * @return
     */
    int getCellIdWormsLayer(int cellNr) {
        return cellGetIdFromLayer(cellNr, MAPID_AIR);
    }

    /**
     * returns if id of layer structures is filled (if so, a structure is recorded to be on this cell)
     * @param cellNr
     * @return
     */
    int getCellIdStructuresLayer(int cellNr) {
        return cellGetIdFromLayer(cellNr, MAPID_STRUCTURES);
    }
    
    bool isCellPassableForFootUnits(int cellNr) {
        bool isPassable = isCellPassableFoot(cellNr); // -> this will block for ALL units, so don't do this
        int unitId = getCellIdUnitLayer(cellNr);
        int strucId = getCellIdStructuresLayer(cellNr);
        return isPassable && unitId < 0 && strucId < 0;
    }

    void cellTakeDamage(int cellNr, int damage) {
        tCell *pCell = getCell(cellNr);
        if (pCell) {
            pCell->health -= damage;

            if (pCell->health < -25) {
                if (pCell->type == TERRAIN_ROCK) {
                    smudge_increase(SMUDGE_ROCK, cellNr);
                }

                if (pCell->type == TERRAIN_SAND ||
                    pCell->type == TERRAIN_HILL ||
                    pCell->type == TERRAIN_SPICE ||
                    pCell->type == TERRAIN_SPICEHILL) {
                    smudge_increase(SMUDGE_SAND, cellNr);
                }

                pCell->health += rnd(35);
            }
        }
    }

    void cellTakeCredits(int cellNr, int amount) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->credits -= amount;
    }

    void cellGiveHealth(int cellNr, int hp) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->health += hp;
    }

    void cellGiveCredits(int cellNr, int amount) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->credits += amount;
    }

    void cellChangeType(int cellNr, int type) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->type = type;
    }

    void cellChangeHealth(int cellNr, int value) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->health = value;
    }

    void cellChangeTile(int cellNr, int tile) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->tile = tile;
    }

    void cellChangeCredits(int cellNr, int value) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->credits = value;
    }

    void cellChangeSmudgeTile(int cellNr, int value) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->smudgetile = value;
    }

    void cellChangeSmudgeType(int cellNr, int value) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->smudgetype = value;
    }

    void cellChangePassable(int cellNr, bool value) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->passable = value;
    }

    void cellChangePassableFoot(int cellNr, bool value) {
        tCell *pCell = getCell(cellNr);
        if (pCell) pCell->passableFoot = value;
    }

    void cellInit(int cellNr) {
        tCell *pCell = getCell(cellNr);
        if (!pCell) return; // bail

        pCell->credits = 0;
        pCell->health = 0;
        pCell->passable = true;
        pCell->passableFoot = true;
        pCell->tile = 0;
        pCell->type = TERRAIN_SAND;    // refers to gfxdata!

        pCell->smudgetile = -1;
        pCell->smudgetype = -1;

        // clear out the ID stuff
        memset(pCell->id, -1, sizeof(pCell->id));

        for (int i = 0; i < MAX_PLAYERS; i++) {
            setVisible(cellNr, i, false);
        }
    }

    void remove_id(int iIndex, int iIDType);    // removes ID of IDtype (unit/structure), etc

    void increaseScrollTimer() { TIMER_scroll++; }
    void resetScrollTimer() { TIMER_scroll=0; }
    bool isTimeToScroll() { return (TIMER_scroll > iScrollSpeed); }

    bool isVisible(int iCell, int iPlayer) {
        if (iCell < 0 || iCell >= MAX_CELLS) return false;
        if (iPlayer < 0 || iPlayer >= MAX_PLAYERS) return false;
        return iVisible[iCell][iPlayer];
    }

    void setVisible(int iCell, int iPlayer, bool flag) { iVisible[iCell][iPlayer] = flag; }

    /**
     * Get height of map in cells
     * @return
     */
    int getHeight() { return height; }

    /**
     * Get width of map in cells
     * @return
     */
    int getWidth() { return width; }

    int getMaxDistanceInPixels() {
        int tileWidth = 32;
        int tileHeight = 32;
        int maxWidthDistance = width * tileWidth;
        int maxHeightDistance = height * tileHeight;
        return ABS_length(0, 0, maxWidthDistance, maxHeightDistance);
    }

    /**
     * Returns the amount of cells of a specific type
     * @param cellType
     * @return
     */
    int getTotalCountCellType(int cellType);

    /**
     * Depending on terrain type, return amount of 'slow down' is experienced by ground unit (this assumes it is about
     * ground units). 0 means no slow down (SLAB), the higher the more slow down. Default is 1 (slight slow down).
     * @param i
     * @return
     */
    int getCellSlowDown(int i);

    /**
     * Checks if param c (cell index) is within the boundaries of the Array (thus, this also accepts the outer
     * map boundaries)
     * @param c
     * @return
     */
    bool isValidCell(int c);

private:
        tCell cell[MAX_CELLS];

    	bool iVisible[MAX_CELLS][MAX_PLAYERS];      // visibility for <player>

    	// Scrolling around map, timer based
    	int TIMER_scroll;
    	int iScrollSpeed;

    	// sizes of the map (outer limits, including the invisible map boundaries)
    	int height, width;
};


#endif