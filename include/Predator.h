//
//  Predator.h
//  FlockingLines2
//
//  Created by HUGE | Ian Bartholomew on 9/30/13.
//
//

#pragma once
#include "Particle.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class Predator : public Particle {
  public:
	Predator();
    Predator( Vec3f pos, Vec3f vel);    
    void update( bool flatten );
	void draw();
    void limitSpeed();

    float		mHunger;
    bool		mIsHungry;

    const float DIST_THRESH     = 600.0f;
    const float PULL_STRENGTH   = 0.0001f;
};