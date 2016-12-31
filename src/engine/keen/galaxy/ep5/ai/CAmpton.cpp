/*
 * CAmpton.cpp
 *
 *  Created on: 29 Dez 2012
 *      Author: Gerstrong
 */


#include "CAmpton.h"
#include "../../common/ai/CPlayerLevel.h"
#include "../../common/ai/CEnemyShot.h"
#include <base/utils/misc.h>
#include <base/GsPython.h>


namespace galaxy {  


enum AMPTONACTIONS
{
    A_AMPTON_WALK = 0,
    A_AMPTON_TURN = 4,
    A_AMPTON_START_POLE = 5,
    A_AMPTON_POLE_SLIDE = 7,
    A_AMPTON_STOP_POLE = 8,
    A_AMPTON_FLIP_SWITCH = 10,
    A_AMPTON_STUNNED = 15,
    A_AMPTON_SHOOT = 32
};



const int CSF_DISTANCE_TO_FOLLOW = 6<<CSF;

const int WALK_SPEED = 25;
const int SLIDE_SPEED = 25;

const int UMOUNT_TIME = 30;

const int TIME_UNTIL_SHOOT = 500;

const uint mAmptonOffset = 0x21DC;
const uint mShikadiMasterOffset = 0x2B6C; // For Yeti shoot sprite (Keen 9)



// TODO: There is a pole sound for Amptoms, find its slot and implement it!

CAmpton::CAmpton(CMap *pmap, const Uint16 foeID, const Uint32 x, const Uint32 y) :
    CStunnable(pmap, foeID, x, y)
{
    mActionMap[A_AMPTON_WALK] = (GASOFctr) &CAmpton::processWalking;
    mActionMap[A_AMPTON_TURN] = (GASOFctr) &CAmpton::processTurn;
    mActionMap[A_AMPTON_START_POLE] = (GASOFctr) &CAmpton::processStartPole;
    mActionMap[A_AMPTON_POLE_SLIDE] = (GASOFctr) &CAmpton::processPoleSlide;
    mActionMap[A_AMPTON_STOP_POLE] = (GASOFctr) &CAmpton::processStopPole;
    mActionMap[A_AMPTON_FLIP_SWITCH] = (GASOFctr) &CAmpton::processFlipSwitch;
    mActionMap[A_AMPTON_STUNNED] = (GASOFctr) &CStunnable::processGettingStunned;
    mActionMap[A_AMPTON_SHOOT] = (GASOFctr) &CAmpton::processShooting;

    // Adapt this AI
    setupGalaxyObjectOnMap(0x21DC, A_AMPTON_WALK);

    mHealthPoints = 1;

    auto diff = gpBehaviorEngine->mDifficulty;

    mWalkSound = SOUND_AMPTONWALK0;


    xDirection = LEFT;

    loadPythonScripts("ampton");

    if(diff > NINJA && foeID == 0x2C)
    {
        mSprVar = 3;
        mHealthPoints += 3;
    }
    if(diff > EXPERT && foeID == 0x2b)
    {
        mSprVar = 2;
        mHealthPoints += 2;
    }
    if(diff > HARD && foeID == 0x2A)
    {
        mSprVar = 1;
        mHealthPoints += 1;
    }
}


bool CAmpton::loadPythonScripts(const std::string &scriptBaseName)
{
    #if USE_PYTHON3
    auto pModule = gPython.loadModule( scriptBaseName, JoinPaths(gKeenFiles.gameDir ,"ai") );

    if (pModule != nullptr)
    {
        loadAiGetterBool(pModule, "screamAfterShoot", mScreamAfterShoot);

        loadAiGetterBool(pModule, "mayShoot", mMayShoot);

        loadAiGetterBool(pModule, "allowClimbing", mAllowClimbing);

        int health = mHealthPoints;
        loadAiGetterInteger(pModule, "healthPoints", health);
        mHealthPoints = health;

        int walksound = mWalkSound;
        loadAiGetterInteger(pModule, "walkSound", walksound);
        mWalkSound = GameSound(walksound);


        Py_DECREF(pModule);
    }
    else
    {
        return false;
    }

    Py_Finalize();
    #endif

    return true;
}



void CAmpton::processWalking()
{
    int walkSound = int(mWalkSound);

    // Play tic toc sound
    if(getActionStatus(A_AMPTON_WALK+1))
    {
        playSound(GameSound(walkSound));
    }
    else if(getActionStatus(A_AMPTON_WALK+1))
    {
        walkSound++;
        playSound(GameSound(walkSound));
    }

    playSound(GameSound(walkSound));
    
    int l_x_l = getXLeftPos();
    int l_x_r = getXRightPos();
    int l_w = getXRightPos() - getXLeftPos();
    int l_h = getYDownPos() - getYUpPos();
    int l_y = getYMidPos();
    int l_x_mid = getXMidPos();

    if ( (l_x_mid & 0x1FF) <= WALK_SPEED)
    {
        if(hitdetectWithTilePropertyRectRO(31, l_x_mid, l_y, l_w, l_h, 1<<CSF))
        {
            setAction(A_AMPTON_FLIP_SWITCH);
        }

        if(mAllowClimbing)
        {
            if(hitdetectWithTilePropertyRectRO(1, l_x_mid, l_y, l_w, l_h, 1<<CSF))
            {
                if( getProbability(600) )
                {
                    bool polebelow = hitdetectWithTilePropertyHor(1, l_x_l, l_x_r, getYDownPos(), 1<<CSF);
                    bool poleabove = hitdetectWithTilePropertyHor(1, l_x_l, l_x_r, getYUpPos(), 1<<CSF);

                    if( getProbability(400) )
                    {
                        poleabove = false;
                    }
                    else
                    {
                        polebelow = false;
                    }


                    // Climb up
                    if (poleabove)
                    {
                        setAction(A_AMPTON_START_POLE);
                        yDirection = UP;
                        return;
                    }
                    else if (polebelow) // Or down
                    {
                        setAction(A_AMPTON_START_POLE);
                        yDirection = DOWN;
                        return;
                    }

                }
            }
        }
    }

    // Move normally in the direction
    if( xDirection == RIGHT )
    {
        moveRight( WALK_SPEED );
    }
    else
    {
        moveLeft( WALK_SPEED );
    }


    if(mMayShoot)
    {
        mTimer++;
        if(mTimer > TIME_UNTIL_SHOOT)
        {
            // ... and spawn a shot that might hurt Keen
            const int newX = (xDirection == LEFT) ? getXLeftPos()+(4<<STC) : getXRightPos()-(4<<STC);
            spawnObj( new CEnemyShot(mp_Map, 0,
                                     newX, getYUpPos()+(8<<STC),
                                     0x2C3E, xDirection, CENTER,  150, mSprVar) );

            // Special set of base offset. Here the action of master shikadi is stolen (Keen 9)
            setupGalaxyObjectOnMap(0x27AC, A_AMPTON_SHOOT);
            mTimer = 0;
        }

    }
}


void CAmpton::processTurn()
{
    if(getActionStatus(A_AMPTON_WALK))
    {
        setAction(A_AMPTON_WALK);
    }
}


void CAmpton::processStartPole()
{
    solid = false;
    setAction(A_AMPTON_POLE_SLIDE);
}


void CAmpton::processPoleSlide()
{
    
    int l_x_l = getXLeftPos();
    int l_x = getXMidPos();
    int l_x_r = getXRightPos();
    int l_y_mid = getYMidPos();
    int l_y_down = getYDownPos();

    // Move normally in the direction
    if( yDirection == UP )
    {
        // Check for the upper side and don't let him move if the pole ends
        if( hitdetectWithTileProperty(1, l_x_l, l_y_mid) ||
                hitdetectWithTileProperty(1, l_x, l_y_mid) ||
                hitdetectWithTileProperty(1, l_x_r, l_y_mid) )
        {
            moveUp( SLIDE_SPEED );
        }
        else
        {
            yDirection = DOWN;
        }
    }
    else // Down
    {
        // Check for the upper side and don't let him move if the pole ends
        if( hitdetectWithTileProperty(1, l_x_l, l_y_down) ||
                hitdetectWithTileProperty(1, l_x, l_y_down) ||
                hitdetectWithTileProperty(1, l_x_r, l_y_down) )
        {
            moveDown( SLIDE_SPEED );
        }
        else
        {
            yDirection = UP;
        }
    }

    mTimer++;
    if(mTimer < UMOUNT_TIME)
        return;

    mTimer = 0;

    // Check for Floor here!
    const int fall1 = mp_Map->getPlaneDataAt(1, l_x, l_y_down+(1<<CSF));
    //const int fall1 = mp_Map->getPlaneDataAt(1, l_x, l_y_down);
    const CTileProperties &TileProp1 = gpBehaviorEngine->getTileProperties(1)[fall1];
    const bool leavePole = (TileProp1.bup != 0);

    if(leavePole)
    {
        setAction(A_AMPTON_STOP_POLE);
        moveXDir(2*xDirection*WALK_SPEED);
        moveUp(1<<CSF);
        solid = true;
        blockedd = true;
    }
}


void CAmpton::processStopPole()
{
    if(getActionStatus(A_AMPTON_WALK))
    {
        setAction(A_AMPTON_WALK);
    }
}


void CAmpton::processFlipSwitch()
{
    if(getActionStatus(A_AMPTON_WALK))
    {
        setAction(A_AMPTON_WALK);
    }
}


bool CAmpton::isNearby(CSpriteObject &theObject)
{
    if( !getProbability(10) )
        return false;

    return true;
}

void CAmpton::getTouchedBy(CSpriteObject &theObject)
{
    if(dead || theObject.dead)
        return;

    CStunnable::getTouchedBy(theObject);

    // Was it a bullet? Than make it stunned when health goes to zero.
    if( dynamic_cast<CBullet*>(&theObject) )
    {
        mHealthPoints--;
        theObject.dead = true;

        if(mHealthPoints == 0)
        {
            if(mScreamAfterShoot)
            {
                playSound(SOUND_ROBO_STUN);
            }

            setAction(A_AMPTON_STUNNED);
            dead = true;
            solid = true;
        }
        else
        {
            blink(10);
        }
    }



    if( CPlayerLevel *player = dynamic_cast<CPlayerLevel*>(&theObject) )
    {
        player->push(*this);
    }
}


int CAmpton::checkSolidD( int x1, int x2, int y2, const bool push_mode )
{	
    if(getActionNumber(A_AMPTON_WALK))
    {
        if(turnAroundOnCliff( x1, x2, y2 ))
            setAction(A_AMPTON_TURN);
    }

    return CGalaxySpriteObject::checkSolidD(x1, x2, y2, push_mode);
}

void CAmpton::processShooting()
{
    if(!getActionStatus(A_AMPTON_SHOOT))
    {
        setupGalaxyObjectOnMap(0x21DC, A_AMPTON_WALK);
    }
}

void CAmpton::process()
{
    performCollisions();

    if(!getActionNumber(A_AMPTON_POLE_SLIDE))
        performGravityMid();

    if(!dead) // If we is dead, there is no way to continue moving or turning
    {
        if( blockedl )
        {
            if(xDirection == LEFT)
            {
                setAction(A_AMPTON_TURN);
            }

            xDirection = RIGHT;
        }
        else if(blockedr)
        {
            if(xDirection == RIGHT)
            {
                setAction(A_AMPTON_TURN);
            }

            xDirection = LEFT;
        }
    }

    if(!processActionRoutine())
        exists = false;

    (this->*mp_processState)();
}

}
