#!/bin/bash

echo Prepare binary package "(./release)" of D2TM.
echo Depends on a compiled "d2tm" in the build/ dir.

echo 1. Check for old version
if [[ -d "./release" || -d "./bin" ]]
then echo -n "Ok to remove old bin / release directories [y/n]? "
     read ans
     [[ "$ans" == "y" ]] && rm -rvf bin release || { echo Ok, quitting. ; exit 0 ; }
else echo "No old version found, continuing.."
fi

echo 2. Creating new bin directory
mkdir bin

echo 3. Creating campaign directory
mkdir bin/campaign
mkdir bin/campaign/atreides
mkdir bin/campaign/ordos
mkdir bin/campaign/harkonnen
mkdir bin/campaign/maps
mkdir bin/campaign/maps/seed
mkdir bin/campaign/briefings

echo 4. Copying campaign files
cp -a campaign/atreides/* bin/campaign/atreides
cp -a campaign/ordos/* bin/campaign/ordos
cp -a campaign/harkonnen/* bin/campaign/harkonnen
cp -a campaign/maps/* bin/campaign/maps
cp -a campaign/maps/seed/* bin/campaign/maps/seed
cp -a campaign/briefings/* bin/campaign/briefings
cp -a campaign/info.txt bin/campaign/info.txt

echo 5. Creating new data directory
mkdir bin/data
mkdir bin/data/scenes
mkdir bin/data/bmp

echo 6. Copying data files
cp -a data/*.dat bin/data
cp -a data/*.fon bin/data
cp -a data/*.ttf bin/data
cp -a data/bmp/* bin/data/bmp

echo 7. Copying scenes
cp -a data/scenes/* bin/data/scenes

echo 8. Create save game directories
mkdir bin/save
mkdir bin/save/atreides
mkdir bin/save/harkonnen
mkdir bin/save/ordos
mkdir bin/save/skirmish

echo 9. Create skirmish directory
mkdir bin/skirmish
cp -a skirmish/*.ini bin/skirmish

echo 10. Copy executable from %1
cp -a build/d2tm bin

echo 11. Copy game rules file "(game.ini)"
cp -a game.ini.org bin/game.ini
cp -a settings.ini bin/settings.ini

echo 12. Copy txt files
cp -a *.txt bin
rm -f bin/CMakeLists.txt
cp -a doc/versionhistory.txt bin
cp -a doc/controls.txt bin
#cp -a windowed.bat bin
cp -a d2tm.cfg bin

#echo 13. Copying dll files
#cp -a dll/mingw32/*.* bin

mv -iv ./bin ./release
echo Done.
