//
//  Particle.cpp
//  FlockingLines2
//
//  Created by HUGE | Ian Bartholomew on 9/30/13.
//
//

#include "Particle.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Rand.h"
#include "cinder/gl/gl.h"

Particle::Particle(){}

Particle::Particle( Vec3f pos, Vec3f vel, bool followed )
{
	mPos			= pos;
	mTailPos		= pos;
	mVel			= vel;
	mVelNormal		= Vec3f::yAxis();
	mAcc			= Vec3f::zero();
	
	mNeighborPos	= Vec3f::zero();
	mNumNeighbors	= 0;
	mMaxSpeed		= Rand::randFloat( 2.5f, 4.0f );
	mMaxSpeedSqrd	= mMaxSpeed * mMaxSpeed;
	mMinSpeed		= Rand::randFloat( 1.0f, 1.5f );
	mMinSpeedSqrd	= mMinSpeed * mMinSpeed;	
    
	mDecay			= 0.99f;
	mRadius			= 1.0f;
	mLength			= 5.0f;
	mFear			= 1.0f;
	mCrowdFactor	= 1.0f;
	
	mIsDead			= false;
	mFollowed		= followed;
    
    mColor			= ColorA( 1.0f, 1.0f, 1.0f, 0.01f );
}

void Particle::pullToCenter( const Vec3f &center )
{
	Vec3f dirToCenter = mPos - center;
	float distToCenter = dirToCenter.length();
	
	if( distToCenter > DIST_THRESH ){
		dirToCenter.normalize();
		mVel -= dirToCenter * ( ( distToCenter - DIST_THRESH ) * PULL_STRENGTH );
	}
}

void Particle::update( bool flatten )
{
    mCrowdFactor -= ( mCrowdFactor - ( 1.0f - mNumNeighbors * 0.02f ) ) * 0.1f;
	mCrowdFactor = constrain( mCrowdFactor, 0.5f, 1.0f );
	
	mFear -= ( mFear - 0.0f ) * 0.2f;
	
	if( flatten )
		mAcc.z = 0.0f;
	
	mVel += mAcc;
	mVelNormal = mVel.normalized();
	
	limitSpeed();
    
	mPos += mVel;
	if( flatten )
		mPos.z = 0.0f;
	
	mVel *= mDecay;
	
	mAcc = Vec3f::zero();
	mNeighborPos = Vec3f::zero();
	mNumNeighbors = 0;
    
}

void Particle::limitSpeed()
{
	float maxSpeed = mMaxSpeed + mCrowdFactor;
	float maxSpeedSqrd = maxSpeed * maxSpeed;
	
	float vLengthSqrd = mVel.lengthSquared();
	if( vLengthSqrd > maxSpeedSqrd ){
		mVel = mVelNormal * maxSpeed;
		
	} else if( vLengthSqrd < mMinSpeedSqrd ){
		mVel = mVelNormal * mMinSpeed;
	}
	mVel *= (1.0 + mFear );
}

void Particle::draw()
{
    
	glColor4f( mColor );
    glBegin( GL_POINTS );
    glVertex3f(mPos);
    glEnd();
    
    //    glBegin( GL_LINE_STRIP );
    //    float a = 1.0f;
    //    for (auto &pt : boost::adaptors::reverse(mPrevPos)) {
    //        glColor4f(1.0f, 1.0f, 1.0f, a);
    //		gl::vertex( pt );
    //        a *= 0.95f;
    //	}
    //	glEnd();
	
}

void Particle::shutdown()
{
    for (auto& t : mThreads) t->join();
}

void Particle::addNeighborPos( Vec3f pos )
{
	mNeighborPos += pos;
	mNumNeighbors ++;
}