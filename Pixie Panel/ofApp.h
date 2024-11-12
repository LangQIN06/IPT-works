#pragma once

#include "ofMain.h"

#include "ofxUltraleapGemini.h"
#include "ofxOsc.h"

class ofApp : public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
        
    void renderBone(const ofxUltraleapGemini::Bone& bone, const ofFloatColor& color);
    
    ofxUltraleapGemini::Connection connection;
    
    ofEasyCam cam;
    
    std::string sendAddr;
    int sendPort;
    ofxOscSender sender;
    
    std::map<int, float> grabStartTimes; // start time of specific gesture
    std::map<int, float> gunStartTimes;
    
    
    glm::vec2 lastPalmPosition; // last palm (x, z)
    float lastUpdatePalmTime;
    
    glm::vec2 lastVelocity;
    glm::vec2 currentAcceleration; 
 
    void sendOscMessage(const std::string& address, const std::vector<float>& values);
};
