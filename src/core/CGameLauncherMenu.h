/*
 * CGameLauncherMenu.h
 *
 *  Created on: 26.03.2011
 *      Author: gerstrong
 */

#ifndef CGAMELAUNCHERMENU_H_
#define CGAMELAUNCHERMENU_H_

#include "mode/CGameMode.h"
#include "common/CGameLauncher.h"
#include "common/Menu/CProfilesMenu.h"
#include <base/GsEngine.h>
#include <memory>


class CGameLauncherMenu : public GsEngine
{
public:
    CGameLauncherMenu(const bool first_time,
			  const int start_game_no = -1, 
			  const int start_level = -1 );

    void start();
    void pumpEvent(const CEvent *evPtr);
    void ponder(const float deltaT);
    void render();

private:
    CGameLauncher mGameLauncher;
    bool m_firsttime;
	std::unique_ptr<CProfilesMenu> mp_FirstTimeMenu;
	int m_start_game_no;
	int m_start_level;
};

// Events
// This event switches to the GameLauncher
struct GMSwitchToGameLauncher : SwitchEngineEvent
{
	GMSwitchToGameLauncher(	const int ChosenGame=-1, const int StartLevel=-1 ) :
        SwitchEngineEvent( new CGameLauncherMenu(false, ChosenGame, ChosenGame) )
		{ }
};


#endif /* CGAMELAUNCHERMENU_H_ */
