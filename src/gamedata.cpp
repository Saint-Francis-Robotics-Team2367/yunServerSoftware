#include "gamedata.h"

GameData::GameData(JoyStickHandler *joystickRef)
{
    this->jsData = joystickRef;
    this->gameSyncs = 0;
    this->gameTime = 0;
    this->gameDataTimer.setInterval(100);
    connect(&this->gameDataTimer,  SIGNAL(timeout()), this, SLOT(updateGameData()));
}
GameData::~GameData()
{

}

void GameData::startGameUpdates() {
    gameDataTimer.start();
}
void GameData::endGameUpdates() {
    gameDataTimer.stop();
}
void GameData::updateGameData() {
    // not actually using this but I don't want to remove it yet
    this->gameTime += 0.1;

    // colon symbol after gmd, pound symbol between game time and everything else, semicolons between individual player data
    QTextStream allGameData;
    allGameData << "gmd:" << this->gameTime << "#";
    for (int i = 0; i < 6; i++) {
        // THE SECOND TO LAST COMPONENT THAT GETS STREAMED (READBUTTON) NEEDS TO BE CHANGED TO A READ ALL BUTTONS METHOD
            allGameData << "P" << i + 1 << ":" << this->robotNames[i] << ":" << this->joystickConnections[i] << ":" << this->clientConnections[i] << ":"
                                  << QString::number(this->jsData[i].readAxis(0)) << ":"
                                  << QString::number(this->jsData[i].readAxis(1)) << ":"
                                  << QString::number(this->jsData[i].readAxis(2)) << ":"
                                  << QString::number(this->jsData[i].readAxis(3)) << ":"
                                  << QString::number(this->jsData[i].readAxis(4)) << ":"
                                  << QString::number(this->jsData[i].readAxis(5)) << ":"
                                  << QString::number(this->jsData[i].readRawBttn(0))
                                  << ";";
    }
    this->gameSyncs++;

    emit signalToHost(allGameData.readAll().toUtf8());
}

float GameData::getGameTime() {
    return this->gameTime;
}
int GameData::getGameSyncs() {
    return this->gameSyncs;
}
