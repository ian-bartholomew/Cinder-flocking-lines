#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"
#include "cinder/Rand.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinder/ip/Fill.h"

#include "Resources.h"

#include <list>
#include "ParticleController.h"

#define NUM_INITIAL_PARTICLES 500
#define NUM_INITIAL_PREDATORS 9
#define NUM_PARTICLES_TO_SPAWN 15

using namespace ci;
using namespace ci::app;
using namespace std;

class FlockingLinesApp : public AppNative {
  public:
	void prepareSettings( Settings *settings );
    void setup();
	void keyDown( KeyEvent event );
	void update();
	void draw();
    void shutdown();
    
    // PARAMS
	params::InterfaceGlRef	mParams;
	
	// CAMERA
	CameraPersp			mCam;
	Quatf				mSceneRotation;
	Vec3f				mEye, mCenter, mUp;
	float				mCameraDistance;
	
	ParticleController	mParticleController;
	float				mZoneRadius;
	float				mLowerThresh, mHigherThresh;
	float				mAttractStrength, mRepelStrength, mOrientStrength;
	float               mFPS = 30.0f;
    float               mCounter;

	bool				mCentralGravity;
	bool				mFlatten;
    
    bool                bDrawParams;
    bool                bIsFullScreen = false;
    
    bool                bSaveMovie = false;
    
    ci::Color           mBackgroundColor;
    qtime::MovieWriterRef	mMovieWriter;
};

void FlockingLinesApp::prepareSettings(Settings *settings)
{
	settings->setWindowSize( 1280, 720 );
//    settings->setFullScreen(bIsFullScreen);
//	settings->setFrameRate( mFPS );
}

void FlockingLinesApp::setup()
{
    if (bSaveMovie){
        fs::path path = getSaveFilePath();
        if( path.empty() )
            return; // user cancelled save
        
        qtime::MovieWriter::Format format;
        format.setCodec(qtime::MovieWriter::CODEC_H264);

        mMovieWriter = qtime::MovieWriter::create( path, getWindowWidth(), getWindowHeight(), format );
        
//        if( qtime::MovieWriter::getUserCompressionSettings( &format, loadImage( loadResource( RES_PREVIEW_IMAGE ) ) ) ) {
//            mMovieWriter = qtime::MovieWriter::create( path, getWindowWidth(), getWindowHeight(), format );
//        }
    }
    
    Rand::randomize();
    
    mBackgroundColor    = Color(255.0f/255.0f, 255.0f/255.0f, 240.0f/255.0f);
	
	mCenter             = Vec3f( getWindowWidth() * 0.5f, getWindowHeight() * 0.5f, 0.0f );
	mCentralGravity     = true;
	mFlatten            = false;
	mZoneRadius         = 80.0f;
	mLowerThresh        = 0.5f;
	mHigherThresh       = 0.8f;
	mAttractStrength	= 0.004f;
	mRepelStrength		= 0.01f;
	mOrientStrength		= 0.01f;
    mCounter            = 0.0f;
    bDrawParams         = true;
    
    // SETUP CAMERA
	mCameraDistance     = 650.0f;
	mEye                = Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCenter             = Vec3f::zero();
	mUp                 = Vec3f::yAxis();
	mCam.setPerspective( 75.0f, getWindowAspectRatio(), 5.0f, 5000.0f );
    
	// SETUP PARAMS
	mParams = params::InterfaceGl::create( "Flocking", Vec2i( 200, 310 ) );
	mParams->addParam( "Scene Rotation", &mSceneRotation, "opened=1" );
	mParams->addSeparator();
	mParams->addParam( "Eye Distance", &mCameraDistance, "min=100.0 max=2000.0 step=50.0 keyIncr=s keyDecr=w" );
	mParams->addParam( "Center Gravity", &mCentralGravity, "keyIncr=g" );
	mParams->addParam( "Flatten", &mFlatten );
	mParams->addSeparator();
	mParams->addParam( "Zone Radius", &mZoneRadius, "min=10.0 max=100.0 step=1.0 keyIncr=z keyDecr=Z" );
	mParams->addParam( "Lower Thresh", &mLowerThresh, "min=0.025 max=1.0 step=0.025 keyIncr=l keyDecr=L" );
	mParams->addParam( "Higher Thresh", &mHigherThresh, "min=0.025 max=1.0 step=0.025 keyIncr=h keyDecr=H" );
	mParams->addSeparator();
	mParams->addParam( "Attract Strength", &mAttractStrength, "min=0.001 max=0.1 step=0.001 keyIncr=a keyDecr=A" );
	mParams->addParam( "Repel Strength", &mRepelStrength, "min=0.001 max=0.1 step=0.001 keyIncr=r keyDecr=R" );
	mParams->addParam( "Orient Strength", &mOrientStrength, "min=0.001 max=0.1 step=0.001 keyIncr=o keyDecr=O" );
    
	// CREATE PARTICLE CONTROLLER
	mParticleController.addParticles( NUM_INITIAL_PARTICLES );
	mParticleController.addPredators( NUM_INITIAL_PREDATORS );
    
    mParams->addParam("Line Alpha", &mParticleController.mLineAlpha, "min=0.001 max=1.0 step=0.001 keyIncr=o keyDecr=O");
    
    gl::enableAlphaBlending();
//    gl::enableAdditiveBlending();
//    gl::enableDepthRead();
//    gl::enableDepthWrite();


}

void FlockingLinesApp::keyDown(KeyEvent event)
{
    if( event.getChar() == 'c' ){
		bDrawParams = !bDrawParams;
	} else if( event.getChar() == 'f' ){
        bIsFullScreen = !bIsFullScreen;
        app::setFullScreen(bIsFullScreen);
    }
}

void FlockingLinesApp::update()
{
    if( mLowerThresh > mHigherThresh ) mHigherThresh = mLowerThresh;
    
	mParticleController.applyForceToPredators( mZoneRadius, 0.4f, 0.7f );
	mParticleController.applyForceToParticles( mZoneRadius, mLowerThresh, mHigherThresh, mAttractStrength, mRepelStrength, mOrientStrength );
	if( mCentralGravity ) mParticleController.pullToCenter( mCenter );
	mParticleController.update( mFlatten );
	
    mCounter += 0.1f;
    
	mEye	= Vec3f( 0.0f, 0.0f, mCameraDistance );
	mCam.lookAt( mEye, mCenter, mUp );
	gl::setMatrices( mCam );
	gl::rotate( mSceneRotation );
    
    gl::rotate( Quatf( Vec3f::yAxis(), mCounter*0.05f ) );
    
//    gl::enableDepthWrite( true );
//	gl::enableDepthRead( true );
//	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//	gl::enableAlphaBlending();
    
}

void FlockingLinesApp::draw()
{
	gl::clear( mBackgroundColor, true );
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glBlendFunc(GL_ZERO, GL_ZERO);
    
	mParticleController.draw();	
	
    // add this frame to our movie
	if( bSaveMovie && mMovieWriter )
		mMovieWriter->addFrame( copyWindowSurface() );
    
	// DRAW PARAMS WINDOW
	if (bDrawParams) mParams->draw();
    
}

void FlockingLinesApp::shutdown()
{
    mParticleController.shutdown();
}

CINDER_APP_NATIVE( FlockingLinesApp, RendererGl )
