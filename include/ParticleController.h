//
//  ParticleController.h
//  FlockingLines2
//
//  Created by HUGE | Ian Bartholomew on 9/30/13.
//
//

#pragma once
#include "Particle.h"
#include "Predator.h"

#include "cinder/Perlin.h"
#include "cinder/Thread.h"

#include <list>

using namespace ci;
using namespace ci::app;
using namespace std;

class ParticleController {
  public:
	ParticleController();
    void applyForceToParticles( float zoneRadius, float lowerThresh, float higherThresh, float attractStrength, float repelStrength, float orientStrength );
	void applyForceToPredators( float zoneRadius, float lowerThresh, float higherThresh );
	void pullToCenter( const ci::Vec3f &center );
	void update( bool flatten );
	void draw();
    void shutdown();
    
	void addParticles( int amt );
	void addPredators( int amt );
	void removeParticles( int amt );
	ci::Vec3f getPos();
	
	ci::Perlin mPerlin;
	
	std::list<Particle>	mParticles;
	std::list<Predator> mPredators;
	ci::Vec3f mParticleCentroid;
	int mNumParticles;
    
    ci::ColorAf       mLineColor;
    float             mLineAlpha = 0.033;
    
    const float     TWO_PI        = M_PI * 2.0f;
    const float     PERLIN_SCALE  = 0.002f;
    const float     PERLIN_MULTI  = 0.01f;
    const float     EAT_DIST_SQRD = 50.0f;
    
    BSpline3f       mSpline;
  
  private:
    void updateParticlesThreadFn(bool flatten);
    void updatePredatorsThreadFn(bool flatten);
    
    std::vector<shared_ptr<thread>>		mThreads;
    
};