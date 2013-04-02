#include "COrbatrix.h"
#include <engine/galaxy/common/ai/CBullet.h>
#include <engine/galaxy/common/ai/CPlayerBase.h>
#include <engine/galaxy/common/ai/CPlayerLevel.h>
#include <misc.h>

namespace galaxy
{

/*
$27E6W  #Orbatrix float 0
$2804W  #Orbatrix moving 1
$2822W  #Orbatrix moving
$2840W  #Orbatrix curls 3
$285EW  #Orbatrix curls
$287CW  #Orbatrix uncurls 5
$289AW  #Orbatrix uncurls
$28B8W  #Orbatrix moving 7
$28D6W  #Orbatrix moving
$28F4W  #Orbatrix moving
$2912W  #Orbatrix moving
$2930W  #Orbatrix bouncing 11
$294EW  #Orbatrix bouncing
$296CW  #Orbatrix bouncing
$298AW  #Orbatrix bouncing 
*/


enum ORBATRIX_ACTION
{
A_ORBATRIX_FLOAT = 0,
A_ORBATRIX_MOVE = 1,
A_ORBATRIX_CURL = 3,
A_ORBATRIX_UNCURL = 5,
A_ORBATRIX_MOVE_ALT = 7,
A_ORBATRIX_BOUNCE = 11,
A_ORBATRIX_LAND = 4,
A_ORBATRIX_SLIDE = 5
};

const int MAX_BOUNCE_BOOST = -150;
const int FLY_SPEED = 10;
const int HOR_SPEED = 80;
//const int VER_SPEED = 40;

const int TIME_UNTIL_MOVE = 20;
const int TIME_UNTIL_CURL = 20;
const int TIME_UNTIL_SLIDE = 20;
const int TIME_UNTIL_UNCURL = 200;
const int TIME_UNTIL_FLOAT = 20;

COrbatrix::COrbatrix(CMap* pmap, const Uint16 foeID, Uint32 x, Uint32 y) :
CGalaxyActionSpriteObject(pmap, foeID, x, y),
mTimer(0)
{
    mActionMap[A_ORBATRIX_FLOAT] = (void (CGalaxyActionSpriteObject::*)()) &COrbatrix::processFloat;
    mActionMap[A_ORBATRIX_MOVE] = (void (CGalaxyActionSpriteObject::*)()) &COrbatrix::processMove;
    mActionMap[A_ORBATRIX_CURL] = (void (CGalaxyActionSpriteObject::*)()) &COrbatrix::processCurl;
    mActionMap[A_ORBATRIX_UNCURL] = (void (CGalaxyActionSpriteObject::*)()) &COrbatrix::processUncurl;
    mActionMap[A_ORBATRIX_MOVE_ALT] = (void (CGalaxyActionSpriteObject::*)()) &COrbatrix::processMove;
    mActionMap[A_ORBATRIX_BOUNCE] = (void (CGalaxyActionSpriteObject::*)()) &COrbatrix::processBounce;
    mActionMap[A_ORBATRIX_LAND] = (void (CGalaxyActionSpriteObject::*)()) &COrbatrix::processLand;
    mActionMap[A_ORBATRIX_SLIDE] = (void (CGalaxyActionSpriteObject::*)()) &COrbatrix::processSlide;

    setupGalaxyObjectOnMap(0x27E6, A_ORBATRIX_FLOAT);

    xDirection = LEFT;
}

void COrbatrix::processFloat()
{
    mTimer++;
    if( mTimer < TIME_UNTIL_MOVE )  
	return;
    
    mTimer = 0;
    
    setAction(A_ORBATRIX_MOVE);
}
   
void COrbatrix::processMove()
{
    mTimer++;
    if( mTimer < TIME_UNTIL_CURL )  
	return;
    
    mTimer = 0;
    
    setAction(A_ORBATRIX_CURL);    
}

void COrbatrix::processCurl()
{
    if(getActionStatus(A_ORBATRIX_BOUNCE))
    {
	yinertia = MAX_BOUNCE_BOOST;
	setAction(A_ORBATRIX_BOUNCE);
    }
}

void COrbatrix::processUncurl()
{
    if(getActionStatus(A_ORBATRIX_SLIDE))
    {
	setAction(A_ORBATRIX_SLIDE);
    }    
}

void COrbatrix::processBounce()
{
    performGravityHigh();
    
    // Move normally in the direction
    if( xDirection == RIGHT )
    {
	moveRight( HOR_SPEED );
    }
    else
    {
	moveLeft( HOR_SPEED );
    }    
    
    if(blockedd && yinertia >= 0)
    {
	yinertia = MAX_BOUNCE_BOOST;
	xDirection = -xDirection;
	playSound(SOUND_ORBATRIX_BUMP);
	// TODO: BOUNCE Sound here!
    }
    
    mTimer++;
    if( mTimer < TIME_UNTIL_UNCURL )  
	return;
    
    mTimer = 0;
    
    setAction(A_ORBATRIX_SLIDE);       
}

void COrbatrix::processLand()
{
    if(getActionStatus(A_ORBATRIX_SLIDE))
    {
	setAction(A_ORBATRIX_SLIDE);
    }    
}

void COrbatrix::processSlide()
{
    mTimer++;
    if( mTimer < TIME_UNTIL_FLOAT )  
	return;
    
    mTimer = 0;
    
    setAction(A_ORBATRIX_FLOAT);        
}



bool COrbatrix::isNearby(CSpriteObject &theObject)
{
    if(!getActionStatus(A_ORBATRIX_FLOAT))
	return true;
    
    if( CPlayerLevel *player = dynamic_cast<CPlayerLevel*>(&theObject) )
    {	
	if( getProbability(600) )
	{	    
	    if( player->getXMidPos() < getXMidPos() )
		xDirection = LEFT;
	    else
		xDirection = RIGHT;
	}
    }	
	
    return true;
}



void COrbatrix::getTouchedBy(CSpriteObject& theObject)
{
    if(dead || theObject.dead)
	return;
        
    // Was it a bullet? Then loose health.
    if( dynamic_cast<CBullet*>(&theObject) )
    {
	theObject.dead = true;
	return;
    }
    
    if( CPlayerBase *player = dynamic_cast<CPlayerBase*>(&theObject) )
    {
	player->kill();
    } 
}

void COrbatrix::process()
{
    performCollisions();
        
    if( blockedl )
	xDirection = RIGHT;
    else if(blockedr)
	xDirection = LEFT;
    
    if(!processActionRoutine())
	exists = false;
    
    (this->*mp_processState)();        
}


};