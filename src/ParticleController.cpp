//
//  ParticleController.cpp
//  FlockingLines2
//
//  Created by HUGE | Ian Bartholomew on 9/30/13.
//
//

#include "ParticleController.h"

#include "cinder/Rand.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Thread.h"

ParticleController::ParticleController()
{
	mPerlin = Perlin( 4 );
    mLineColor = ColorAf( 0.0f, 0.0f, 0.0f, mLineAlpha );
        
}

void ParticleController::applyForceToParticles( float zoneRadius, float lowerThresh, float higherThresh, float attractStrength, float repelStrength, float alignStrength  )
{
	mParticleCentroid = Vec3f::zero();
	mNumParticles = mParticles.size();    
	
	for( list<Particle>::iterator p1 = mParticles.begin(); p1 != mParticles.end(); ++p1 ){
		
		list<Particle>::iterator p2 = p1;
		for( ++p2; p2 != mParticles.end(); ++p2 ) {
			Vec3f dir = p1->mPos - p2->mPos;
			float distSqrd = dir.lengthSquared();
			float zoneRadiusSqrd = zoneRadius * p1->mCrowdFactor * zoneRadius * p2->mCrowdFactor;
			
			if( distSqrd < zoneRadiusSqrd ){		// Neighbor is in the zone
				float per = distSqrd/zoneRadiusSqrd;
				p1->addNeighborPos( p2->mPos );
				p2->addNeighborPos( p1->mPos );
                
				if( per < lowerThresh ){			// Separation
					float F = ( lowerThresh/per - 1.0f ) * repelStrength;
					dir.normalize();
					dir *= F;
                    
					p1->mAcc += dir;
					p2->mAcc -= dir;
				} else if( per < higherThresh ){	// Alignment
					float threshDelta	= higherThresh - lowerThresh;
					float adjPer		= ( per - lowerThresh )/threshDelta;
					float F				= ( 1.0f - ( cos( adjPer * TWO_PI ) * -0.5f + 0.5f ) ) * alignStrength;
                    
					p1->mAcc += p2->mVelNormal * F;
					p2->mAcc += p1->mVelNormal * F;
					
				} else {							// Cohesion (prep)
					float threshDelta	= 1.0f - higherThresh;
					float adjPer		= ( per - higherThresh )/threshDelta;
					float F				= ( 1.0f - ( cos( adjPer * TWO_PI ) * -0.5f + 0.5f ) ) * attractStrength;
                    
					dir.normalize();
					dir *= F;
                    
					p1->mAcc -= dir;
					p2->mAcc += dir;
				}
			}
		}
		
		mParticleCentroid += p1->mPos;
		/*
         if( p1->mNumNeighbors > 0 ){ // Cohesion
         Vec3f neighborAveragePos = ( p1->mNeighborPos/(float)p1->mNumNeighbors );
         p1->mAcc += ( neighborAveragePos - p1->mPos ) * attractStrength;
         }
         */
		
		// ADD PERLIN NOISE INFLUENCE
		Vec3f perlin = mPerlin.dfBm( p1->mPos * PERLIN_SCALE ) * PERLIN_MULTI;
		p1->mAcc += perlin;
		
		
		// CHECK WHETHER THERE IS ANY PARTICLE/PREDATOR INTERACTION
		float predatorZoneRadiusSqrd = zoneRadius * zoneRadius * 5.0f;
		for( list<Predator>::iterator predator = mPredators.begin(); predator != mPredators.end(); ++predator ) {
            
			Vec3f dir = p1->mPos - predator->mPos;
			float distSqrd = dir.lengthSquared();
			
			if( distSqrd < predatorZoneRadiusSqrd ){
				if( distSqrd > EAT_DIST_SQRD ){
					float F = ( predatorZoneRadiusSqrd/distSqrd - 1.0f ) * 0.1f;
					p1->mFear += F * 0.1f;
					dir = dir.normalized() * F;
					p1->mAcc += dir;
					if( predator->mIsHungry )
						predator->mAcc += dir * 0.04f * predator->mHunger;
				} else {
					p1->mIsDead = true;
					predator->mHunger = 0.0f;
					predator->mIsHungry = false;
				}
			}
		}
		
	}
	mParticleCentroid /= (float)mNumParticles;
}

void ParticleController::applyForceToPredators( float zoneRadius, float lowerThresh, float higherThresh )
{
	for( list<Predator>::iterator P1 = mPredators.begin(); P1 != mPredators.end(); ++P1 ){
        
		list<Predator>::iterator P2 = P1;
		for( ++P2; P2 != mPredators.end(); ++P2 ) {
			Vec3f dir = P1->mPos - P2->mPos;
			float distSqrd = dir.lengthSquared();
			float zoneRadiusSqrd = zoneRadius * zoneRadius * 4.0f;
			
			if( distSqrd < zoneRadiusSqrd ){		// Neighbor is in the zone
				float per = distSqrd/zoneRadiusSqrd;
				if( per < lowerThresh ){			// Separation
					float F = ( lowerThresh/per - 1.0f ) * 0.01f;
					dir.normalize();
					dir *= F;
                    
					P1->mAcc += dir;
					P2->mAcc -= dir;
				} else if( per < higherThresh ){	// Alignment
					float threshDelta	= higherThresh - lowerThresh;
					float adjPer		= ( per - lowerThresh )/threshDelta;
					float F				= ( 1.0f - cos( adjPer * TWO_PI ) * -0.5f + 0.5f ) * 0.3f;
                    
					P1->mAcc += P2->mVelNormal * F;
					P2->mAcc += P1->mVelNormal * F;
					
				} else {							// Cohesion
					float threshDelta	= 1.0f - higherThresh;
					float adjPer		= ( per - higherThresh )/threshDelta;
					float F				= ( 1.0f - ( cos( adjPer * TWO_PI ) * -0.5f + 0.5f ) ) * 0.1f;
                    
					dir.normalize();
					dir *= F;
                    
					P1->mAcc -= dir;
					P2->mAcc += dir;
				}
			}
		}
	}
}

void ParticleController::pullToCenter( const ci::Vec3f &center )
{
	for( list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p ){
		p->pullToCenter( center );
	}
	
	for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
		p->pullToCenter( center );
	}
}

void ParticleController::update( bool flatten )
{
    vector<Vec3f> points;
    
//    mThreads.push_back(shared_ptr<thread>( new thread( bind( &ParticleController::updateParticlesThreadFn, this, flatten ) ) ));
//    mThreads.push_back(shared_ptr<thread>( new thread( bind( &ParticleController::updatePredatorsThreadFn, this, flatten ) ) ));
	for( list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ){
		if( p->mIsDead ){
			p = mParticles.erase( p );
		} else {
			p->update( flatten );
            points.push_back(p->mPos);
			++p;
		}
	}
    
    mSpline = BSpline3f( points, 3, true, true );
    
    for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
		p->update( flatten );
	}
}

void ParticleController::updateParticlesThreadFn(bool flatten)
{
    ci::ThreadSetup threadSetup; // instantiate this if you're talking to Cinder from a secondary thread
    
	for( list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ){
		if( p->mIsDead ){
			p = mParticles.erase( p );
		} else {
			p->update( flatten );
			++p;
		}
	}
}

void ParticleController::updatePredatorsThreadFn(bool flatten)
{
    ci::ThreadSetup threadSetup; // instantiate this if you're talking to Cinder from a secondary thread
    
	for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
		p->update( flatten );
	}
}

void ParticleController::draw()
{
    // DRAW PREDATOR ARROWS
//    for( list<Predator>::iterator p = mPredators.begin(); p != mPredators.end(); ++p ){
//        p->draw();
//    }
	
	// DRAW PARTICLE
//	for( list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p ){
//		p->draw();
//	}    
    
//    glEnable (GL_LINE_SMOOTH);
////    glEnable(GL_BLEND);
////    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glHint (GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
//    glLineWidth (2.0f);
//    
//    glBegin( GL_LINE_LOOP );
//    
//    glColor4f(mLineColor.r, mLineColor.g, mLineColor.b, mLineAlpha);
//    for( list<Particle>::iterator p = mParticles.begin(); p != mParticles.end(); ++p ){
//
//		gl::vertex( p->mPos );
//	}
//	glEnd();
    
    const int numSegments = mParticles.size();
//	gl::color( ColorA( 0.8f, 0.2f, 0.8f, 0.5f ) );
    glColor4f(mLineColor.r, mLineColor.g, mLineColor.b, mLineAlpha);
	glLineWidth( 2.0f );
	gl::begin( GL_LINE_STRIP );
	for( int s = 0; s <= numSegments * 25; ++s ) {
		float t = s / (float)numSegments;
		gl::vertex( mSpline.getPosition( t ) );
	}
	gl::end();
    
}

void ParticleController::shutdown()
{
    for (auto& t : mThreads) t->join();
    for (auto& p : mParticles) p.shutdown();
}

void ParticleController::addPredators( int amt )
{
	for( int i=0; i<amt; i++ )
	{
		Vec3f pos = Rand::randVec3f() * Rand::randFloat( 500.0f, 750.0f );
		Vec3f vel = Rand::randVec3f();
		mPredators.push_back( Predator( pos, vel ) );
	}
}

void ParticleController::addParticles( int amt )
{
	for( int i=0; i<amt; i++ )
	{
		Vec3f pos = Rand::randVec3f() * Rand::randFloat( 100.0f, 200.0f );
		Vec3f vel = Rand::randVec3f();
		
		bool followed = false;
		if( mParticles.size() == 0 ) followed = true;
		
		mParticles.push_back( Particle( pos, vel, followed ) );        
	}        
}

void ParticleController::removeParticles( int amt )
{
	for( int i=0; i<amt; i++ )
	{
		mParticles.pop_back();
	}
}

Vec3f ParticleController::getPos()
{
	return mParticles.begin()->mPos;
}