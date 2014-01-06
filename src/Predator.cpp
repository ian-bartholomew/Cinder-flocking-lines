//
//  Predator.cpp
//  FlockingLines2
//
//  Created by HUGE | Ian Bartholomew on 9/30/13.
//
//

#include "Predator.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"

Predator::Predator(){}

Predator::Predator( Vec3f pos, Vec3f vel ):Particle(pos, vel, false)
{
	mMaxSpeed		= Rand::randFloat( 4.0f, 4.5f );
	mMaxSpeedSqrd	= mMaxSpeed * mMaxSpeed;
	mMinSpeed		= Rand::randFloat( 1.0f, 1.5f );
	mMinSpeedSqrd	= mMinSpeed * mMinSpeed;
    
	mColor			= ColorA( 1.0f, 0.0f, 0.0f, 1.0f );
	
	mDecay			= 0.99f;
	mRadius			= 2.0f;
	mLength			= 25.0f;
    
	mHunger			= 1.0f;
    mIsHungry		= true;
}

void Predator::update( bool flatten )
{
    mVel += mAcc;
	
	if( flatten ) mAcc.z = 0.0f;
	mVel += mAcc;
	mVelNormal = mVel.safeNormalized();
	
	limitSpeed();	
	
	mPos += mVel;
	
	if( flatten )
		mPos.z = 0.0f;	
	mVel *= mDecay;
	
	mAcc = Vec3f::zero();
	mNeighborPos = Vec3f::zero();
	mNumNeighbors = 0;
	
	mHunger += 0.001f;
	mHunger = constrain( mHunger, 0.0f, 1.0f );
	
	if( mHunger > 0.5f ) mIsHungry = true;
    
}

void Predator::limitSpeed()
{
	float maxSpeed = mMaxSpeed + mHunger * 3.0f;
	float maxSpeedSqrd = maxSpeed * maxSpeed;
	float vLengthSqrd = mVel.lengthSquared();
	if( vLengthSqrd > maxSpeedSqrd ){
		mVel = mVelNormal * maxSpeed;
		
	} else if( vLengthSqrd < mMinSpeedSqrd ){
		mVel = mVelNormal * mMinSpeed;
	}
}

void Predator::draw()
{
    
	glColor4f( mColor );
//    glBegin( GL_POINTS );
//    glVertex3f(mPos);
//    glEnd();
    
    gl::drawSphere(mPos, mRadius);
    
}