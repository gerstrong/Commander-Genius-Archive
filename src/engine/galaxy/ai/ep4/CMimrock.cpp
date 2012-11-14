/*
 * CMimrock.cpp
 *
 *  Created on: 18.09.2011
 *      Author: gerstrong
 */

#include "CMimrock.h"
#include "engine/galaxy/ai/CPlayerBase.h"
#include "misc.h"

namespace galaxy {
    
enum MIMROCK_ACTION
{
    A_MIMROCK_SIT = 0,
    A_MIMROCK_WALK = 1,
    A_MIMROCK_JUMP = 7,
    A_MIMROCK_BOUNCE = 10,
    A_MIMROCK_SHOT = 11,
    A_MIMROCK_STUNNED = 12,
};

const int CSF_DISTANCE_TO_FOLLOW_TOLERANCE = 10<<CSF;
const int CSF_DISTANCE_TO_JUMP_TOLERANCE = 3<<CSF;
const int WALK_SPEED = 10;
const int JUMP_SPEED = 30;

const int JUMP_HEIGHT = 148;
const int BOUNCE_HEIGHT = 74;
const int JUMP_TIME = 500;
const int BOUNCE_TIME = 250;
const int TIME_UNTIL_LOOK = 100;

CMimrock::CMimrock(CMap *pmap, const Uint16 foeID, Uint32 x, Uint32 y) :
CStunnable(pmap, foeID, x, y),
mTimer(0)
{
    mActionMap[A_MIMROCK_SIT] = (void (CStunnable::*)()) &CMimrock::processSit;
    mActionMap[A_MIMROCK_WALK] = (void (CStunnable::*)()) &CMimrock::processWalk;
    mActionMap[A_MIMROCK_JUMP] = (void (CStunnable::*)()) &CMimrock::processJump;
    mActionMap[A_MIMROCK_BOUNCE] = (void (CStunnable::*)()) &CMimrock::processBounce;
    mActionMap[A_MIMROCK_STUNNED] = &CStunnable::processGettingStunned;
    
    setupGalaxyObjectOnMap(0x343A, A_MIMROCK_SIT);
    xDirection = RIGHT;
    yDirection = CENTER;
    honorPriority = false;	
}


void CMimrock::getTouchedBy(CSpriteObject &theObject)
{
    if(dead || theObject.dead)
	return;
    
    if( !getActionNumber(A_MIMROCK_SIT) )
    {    
	CStunnable::getTouchedBy(theObject);
	
	// Was it a bullet? Than make it stunned.
	if( dynamic_cast<CBullet*>(&theObject) )
	{
	    setAction( A_MIMROCK_STUNNED );
	    theObject.dead = true;
	}
    }
    
    if( getActionNumber(A_MIMROCK_WALK) || getActionNumber(A_MIMROCK_JUMP) || getActionNumber(A_MIMROCK_BOUNCE) )
    {
	if( CPlayerBase *player = dynamic_cast<CPlayerBase*>(&theObject) )
	{
	    player->kill();
	}
    }
}


bool CMimrock::isNearby(CSpriteObject &theObject)
{
    if(dead || theObject.dead || mTimer > 0 )
	return true;    

    if( !blockedd || getActionStatus(A_MIMROCK_JUMP) || getActionStatus(A_MIMROCK_BOUNCE) )
	return true;    
        
    if( CPlayerBase *player = dynamic_cast<CPlayerBase*>(&theObject) )
    {
	const int dx = player->getXMidPos() - getXMidPos();
	
	if( dx>-CSF_DISTANCE_TO_FOLLOW_TOLERANCE &&
	    dx<+CSF_DISTANCE_TO_FOLLOW_TOLERANCE )
	{
	    if( dx<0 )
		xDirection = LEFT;
	    else
		xDirection = RIGHT;
	    
	    if( dx>-CSF_DISTANCE_TO_JUMP_TOLERANCE &&
		dx<+CSF_DISTANCE_TO_JUMP_TOLERANCE )
	    {
		yinertia = -JUMP_HEIGHT;
		mTimer = JUMP_TIME;
		setAction(A_MIMROCK_JUMP);
	    }
	    else
	    {
		setAction(A_MIMROCK_WALK);
	    }
	    
	}
    }
    
    return true;
}


void CMimrock::processSit()
{
    // When sitting the rock doesn't do any thing, so this stays empty.
    if(mTimer > 0)
	mTimer--;
}

void CMimrock::processWalk()
{
    if(xDirection == LEFT)
    {
	moveLeft(WALK_SPEED);
    }
    else
    {
	moveRight(WALK_SPEED);
    }
    
    if(getActionStatus(A_MIMROCK_SIT))
    {
	setAction(A_MIMROCK_SIT);
    }
    
}

void CMimrock::processJump()
{
    if(xDirection == LEFT)
    {
	moveLeft(JUMP_SPEED);
    }
    else
    {
	moveRight(JUMP_SPEED);
    }
    
    mTimer--;
    
    if(blockedd && yinertia>=0)
    {
	mTimer = BOUNCE_TIME;
	yinertia = -BOUNCE_HEIGHT;
	setAction(A_MIMROCK_BOUNCE);
    }
    
}

void CMimrock::processBounce()
{
    if(xDirection == LEFT)
    {
	moveLeft(JUMP_SPEED);
    }
    else
    {
	moveRight(JUMP_SPEED);
    }
    
    mTimer--;
    
    if(mTimer == 0 || blockedd)
    {
	mTimer = TIME_UNTIL_LOOK;
	setAction(A_MIMROCK_SIT);
    }
}

void CMimrock::process()
{        
    performGravityMid();    
    performCollisions();
    
    if(dead)
	return;
    
    (this->*mp_processState)();
    
    
    processActionRoutine();	
    
    if(getActionStatus(A_MIMROCK_STUNNED))
	dead = true;
}
    
    
} /* namespace galaxy */
