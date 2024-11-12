#include "ofApp.h"

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

//--------------------------------------------------------------
void ofApp::setup()
{
    if (this->connection.open())
    {
        ofLogNotice(__FUNCTION__) << "Connection opened!";
    }
    else
    {
        ofLogError(__FUNCTION__) << "Failed to open connection";
    }
    
    // Set up the OSC sender.
    sendAddr = "localhost";
    sendPort = 8080;
    sender.setup(sendAddr, sendPort);
    
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
}


//--------------------------------------------------------------
void ofApp::draw() {
    ofFloatColor colors[] = {
        ofFloatColor::red,
        ofFloatColor::green,
        ofFloatColor::blue,
        ofFloatColor::yellow
    };
    
    int currY = 20;
    int handCount = 0;
    glm::vec3 leftPalmPosition, rightPalmPosition;
    bool leftHandDetected = false, rightHandDetected = false;
    
    this->cam.begin();
    {
        const auto &devices = this->connection.getDevices();
        for (auto dp : devices) {
            auto device = dp.second;
            for (auto hp : device->getHands()) {
                if (handCount < 2) {
                    auto hand = hp.second;
                    auto handId = hand->getId();
                    auto handType = hand->getType();
                    
                    if (handType == eLeapHandType_Left) {
                        leftPalmPosition = hand->getPalm().position;
                        leftHandDetected = true;
                    } else if (handType == eLeapHandType_Right) {
                        rightPalmPosition = hand->getPalm().position;
                        rightHandDetected = true;
                    }
                    
                    for (int d = 0; d < 5; ++d) {
                        auto &digit = hand->getDigits()[d];
                        this->renderBone(digit.metacarpal, colors[0]);
                        this->renderBone(digit.proximal, colors[1]);
                        this->renderBone(digit.intermediate, colors[2]);
                        this->renderBone(digit.distal, colors[3]);
                    }
                    
                    // getPalm()-Info
                    auto palm = hand->getPalm();
                    glm::vec3 currentPalmPosition = palm.position;
                    glm::vec3 normal = palm.normal;
                    glm::vec3 direction = palm.direction;
                    
                    //clap
                    if (leftHandDetected && rightHandDetected) {
                        float distance = glm::distance(leftPalmPosition, rightPalmPosition);
                        
                        std::ostringstream ossDistance;
                        ossDistance << "Distance between left and right palm: " << distance;
                        ofDrawBitmapString(ossDistance.str(), 10, currY);
                        currY += 20;
                        
                        std::vector<float> clapDistance = {distance};
                        sendOscMessage("/clap", clapDistance);
                        std::ostringstream ossC;
                        ossC << std::endl
                        << "Clap!" << std::endl;
                    }
                    
                    //map the palm value
                    float mappedPalmX = ofMap(currentPalmPosition.x, -200, 200, -10, 10);
                    float mappedPalmY = ofMap(currentPalmPosition.y, 100, 500, -10, 10);
                    
                    float currentPalmTime = ofGetElapsedTimef();
                    float deltaTime = currentPalmTime - lastUpdatePalmTime;
                    
                    // only 2D
                    glm::vec2 currentPalmPos2D(currentPalmPosition.x, currentPalmPosition.y);
                    glm::vec2 currentVelocity = (currentPalmPos2D - lastPalmPosition) / deltaTime;
                    
                    
                    if (deltaTime > 0) {
                        // calculate acceleration from second time
                        if (lastUpdatePalmTime > 0) {
                            // Acceleration:
                            currentAcceleration = (currentVelocity - lastVelocity) / (deltaTime*1000);
                        }
                        
                        lastPalmPosition = currentPalmPos2D;
                        lastVelocity = currentVelocity;
                        lastUpdatePalmTime = currentPalmTime;
                    }
                    
                    
                    // distal distance of index
                    auto index = hand->getIndex();
                    auto &distalIndexPhalange = index.distal;
                    glm::vec3 distalIndexPos = distalIndexPhalange.jointNext;
                    
                    // distal distance of pinky
                    auto pinky = hand->getPinky();
                    auto &distalPinkyPhalange = pinky.distal;
                    glm::vec3 distalPinkyPos = distalPinkyPhalange.jointNext;
                    
                    // distal distance of thumb
                    auto thumb = hand->getThumb();
                    auto &distalThumbPhalange = thumb.distal;
                    glm::vec3 distalThumbPos = distalThumbPhalange.jointNext;
                    
                    // grab&pinch Info
                    float grabDistance = hand->getGrabAngle();
                    float pinchDistance = hand->getPinchDistance();
                    float pinchStrength = hand->getPinchStrength();
                    
                    // check & record for CLICK
                    float currentTime = ofGetElapsedTimef();
                    if (grabDistance > 2.0 && grabDistance < 3.0 && pinchDistance>30 && pinchStrength > 0.6) {
                        if (grabStartTimes.find(handId) == grabStartTimes.end()) {
                            grabStartTimes[handId] = currentTime;
                        }
                    } else {
                        grabStartTimes.erase(handId);
                    }
                    
                    // check & record for SHOOT
                    float currentGunTime = ofGetElapsedTimef();
                    if (grabDistance > 2.0 && grabDistance < 3.0 && pinchStrength <= 0.6) {
                        if (gunStartTimes.find(handId) == gunStartTimes.end()) {
                            gunStartTimes[handId] = currentGunTime;
                        }
                    } else {
                        gunStartTimes.erase(handId);
                    }
                    
                    
                    // Info
                    std::ostringstream oss;
                    oss << "Hand " << (hand->getType() == eLeapHandType_Left ? "LEFT" : "RIGHT") << std::endl;
                    
                    std::vector<float> palmPositionsCurrentVelocity = {mappedPalmX, mappedPalmY,currentVelocity.x, currentVelocity.y};
                    sendOscMessage("/position", palmPositionsCurrentVelocity);
                    
                    
                    const int click=1.00;
                    const int shoot=2.00;
                    
                    if (grabStartTimes.find(handId) != grabStartTimes.end() && currentTime - grabStartTimes[handId] >= 0.3) {
                        oss << std::endl
                        << "Click!" << std::endl;
                        
                        std::vector<float> distalIndexPosInfo = {distalIndexPos.x, distalIndexPos.y};
                        sendOscMessage("/click", distalIndexPosInfo);
                        std::vector<float> clickInfo = {click, 0};
                        
                        sendOscMessage("/click", clickInfo);
                    }
                    
                    if (gunStartTimes.find(handId) != gunStartTimes.end() && currentGunTime - gunStartTimes[handId] >= 0.3) {
                        oss << std::endl
                        << "Gun shot!" << std::endl;
                        std::vector<float> shootInfo = {shoot, 0};
                        sendOscMessage("/shoot", shootInfo);
                        
                    }
                    
                    ofDrawBitmapString(oss.str(), 10, currY);
                    currY += 100; //
                } else {
                    break; //ignore other hands
                }
            }
            
            if (handCount >= 2) {
                break; // exit
            }
        }
    }
    this->cam.end();
}


void ofApp::renderBone(const ofxUltraleapGemini::Bone& bone, const ofFloatColor& color)
{
    glm::mat4 xform;
    xform *= glm::translate(bone.jointPrev);
    xform *= glm::toMat4(bone.rotation);
    xform *= glm::eulerAngleX(ofDegToRad(90));
    
    ofPushMatrix();
    ofMultMatrix(xform);
    {
        ofSetColor(color);
        ofDrawCylinder(bone.width * 0.5f, glm::distance(bone.jointNext, bone.jointPrev));
    }
    ofPopMatrix();
    
    ofSetColor(color);
    ofDrawLine(bone.jointPrev, bone.jointNext);
}


void ofApp::sendOscMessage(const std::string& address, const std::vector<float>& values) {
    ofxOscMessage oscMsg;
    oscMsg.setAddress(address);
    for (auto value : values) {
        oscMsg.addFloatArg(value);
    }
    sender.sendMessage(oscMsg, false);
}

