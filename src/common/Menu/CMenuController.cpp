/*
 * CMenuController.cpp
 *
 *  Created on: 19.02.2012
 *      Author: gerstrong
 */

#include "CMenuController.h"
#include "common/CBehaviorEngine.h"
#include "common/Menu/CControlsettings.h"

void CMenuController::process()
{

	// process any triggered Game Control related event
	CEventContainer &EventContainer = g_pBehaviorEngine->EventList();

	if(!EventContainer.empty())
	{

		if( OpenMenuEvent* openMenu = EventContainer.occurredEvent<OpenMenuEvent>() )
		{
			mpMenu = openMenu->mMenuDialogPointer;
			mpMenu->init();

			if( !mMenuStack.empty() )
				mpMenu->setProperty( CBaseMenu::CANGOBACK );

			mMenuStack.push_back( mpMenu );
			EventContainer.pop_Event();
		}

		if( EventContainer.occurredEvent<CloseMenuEvent>() )
		{
			mpMenu->release();
			mMenuStack.pop_back();
			mpMenu = mMenuStack.back();
			EventContainer.pop_Event();
		}

		if( OpenControlMenuEvent* ctrlMenu = EventContainer.occurredEvent<OpenControlMenuEvent>() )
		{
			EventContainer.pop_Event();

			EventContainer.add( new OpenMenuEvent(
									new CControlsettings(ctrlMenu->mDlgTheme,
														 ctrlMenu->mNumPlayers) ) );
		}


	}

	if( !mMenuStack.empty() )
		mpMenu->process();

}
