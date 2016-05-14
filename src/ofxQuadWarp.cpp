//
//  ofxQuadWarp.cpp
//  Created by lukasz karluk on 19/06/11.
//

#include "ofxQuadWarp.h"
#include "ofxHomography.h"

ofxQuadWarp::ofxQuadWarp() {
    anchorSize = 10;
    selectedCornerIndex = -1;
    highlightCornerIndex = -1;
    
    bMouseEnabled = false;
    bKeyboardShortcuts = false;
    bShow = false;
    bShiftPressed = false;
}

ofxQuadWarp::~ofxQuadWarp() {
    disableMouseControls();
    disableKeyboardShortcuts();
}

//----------------------------------------------------- setup.
void ofxQuadWarp::setup() {
    enableMouseControls();
    enableKeyboardShortcuts();
    show();
}

//----------------------------------------------------- setters.

void ofxQuadWarp::setAnchorSize(float value) {
    anchorSize = value;
}

//----------------------------------------------------- enable / disable.
void ofxQuadWarp::enableMouseControls() {
    if(bMouseEnabled == true) {
        return;
    }
    bMouseEnabled = true;
    ofAddListener(ofEvents().mouseMoved, this, &ofxQuadWarp::onMouseMoved);
    ofAddListener(ofEvents().mousePressed, this, &ofxQuadWarp::onMousePressed);
    ofAddListener(ofEvents().mouseDragged, this, &ofxQuadWarp::onMouseDragged);
    ofAddListener(ofEvents().mouseReleased, this, &ofxQuadWarp::onMouseReleased);
}

void ofxQuadWarp::disableMouseControls() {
    if(bMouseEnabled == false) {
        return;
    }
    bMouseEnabled = false;
    try {
        ofRemoveListener(ofEvents().mouseMoved, this, &ofxQuadWarp::onMouseMoved);
        ofRemoveListener(ofEvents().mousePressed, this, &ofxQuadWarp::onMousePressed);
        ofRemoveListener(ofEvents().mouseDragged, this, &ofxQuadWarp::onMouseDragged);
        ofRemoveListener(ofEvents().mouseReleased, this, &ofxQuadWarp::onMouseReleased);
    }
    catch(Poco::SystemException) {
        return;
    }
}

void ofxQuadWarp::enableKeyboardShortcuts() {
    if(bKeyboardShortcuts == true) {
        return;
    }
    bKeyboardShortcuts = true;
    ofAddListener(ofEvents().keyPressed, this, &ofxQuadWarp::keyPressed);
    ofAddListener(ofEvents().keyReleased, this, &ofxQuadWarp::keyReleased);
}

void ofxQuadWarp::disableKeyboardShortcuts() {
    if(bKeyboardShortcuts == false) {
        return;
    }
    bKeyboardShortcuts = false;
    try {
        ofRemoveListener(ofEvents().keyPressed, this, &ofxQuadWarp::keyPressed);
        ofRemoveListener(ofEvents().keyReleased, this, &ofxQuadWarp::keyReleased);
    }
    catch(Poco::SystemException) {
        return;
    }
}


bool ofxQuadWarp::hitTest(ofVec2f pos) {
    // apply the inverse transformation to our hit point and then
    // compare to the input rectangle
    
    ofMatrix4x4 mat = getMatrix().getInverse();
    ofVec4f posTransformed = mat.preMult(ofVec3f(pos.x, pos.y, 0.0));
    float a2 = anchorSize * 0.5;
    if(posTransformed.x > srcPoints[0].x - a2 && posTransformed.y > srcPoints[0].y - a2 &&
       posTransformed.x < srcPoints[2].x + a2 && posTransformed.y < srcPoints[2].y + a2) {
        return true;
    }
    return false;
}       

//----------------------------------------------------- source / target points.
void ofxQuadWarp::setSourceRect(const ofRectangle& r) {
    srcPoints[0].set(r.x, r.y);
    srcPoints[1].set(r.x + r.width, r.y);
    srcPoints[2].set(r.x + r.width, r.y + r.height);
    srcPoints[3].set(r.x, r.y + r.height);
}

void ofxQuadWarp::setTargetRect(const ofRectangle& r) {
    dstPoints[0].set(r.x, r.y);
    dstPoints[1].set(r.x + r.width, r.y);
    dstPoints[2].set(r.x + r.width, r.y + r.height);
    dstPoints[3].set(r.x, r.y + r.height);
}

void ofxQuadWarp::setTargetPoints(const vector<ofPoint>& points) {
    int t = MIN(4, points.size());
    for(int i=0; i<t; i++) {
        dstPoints[i].set(points[i]);
    }
}

//----------------------------------------------------- matrix.
ofMatrix4x4 ofxQuadWarp::getMatrix() const {
    return getMatrix(&srcPoints[0], &dstPoints[0]);
}

ofMatrix4x4 ofxQuadWarp::getMatrixInverse() const {
    return getMatrix(&dstPoints[0], &srcPoints[0]);
}

ofMatrix4x4 ofxQuadWarp::getMatrix(const ofPoint* srcPoints, const ofPoint* dstPoints) const {
    return ofxHomography::findHomography(srcPoints, dstPoints);
}

void ofxQuadWarp::reset() {
    dstPoints[0].set(srcPoints[0]);
    dstPoints[1].set(srcPoints[1]);
    dstPoints[2].set(srcPoints[2]);
    dstPoints[3].set(srcPoints[3]);
}

//----------------------------------------------------- interaction.
void ofxQuadWarp::onMouseMoved(ofMouseEventArgs& mouseArgs) {
    if(bShow == false) {
        return;
    }
    
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    for(int i=0; i<4; i++) {
        ofPoint & dstPoint = dstPoints[i];
        if(mousePoint.distance(dstPoint) <= anchorSize * 0.5) {
            highlightCornerIndex = i;
            return;
        }
    }
    highlightCornerIndex = -1;
}

void ofxQuadWarp::onMousePressed(ofMouseEventArgs& mouseArgs) {
    if(bShow == false) {
        return;
    }
    
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    for(int i=0; i<4; i++) {
        ofPoint & dstPoint = dstPoints[i];
        if(mousePoint.distance(dstPoint) <= anchorSize * 0.5) {
            dstPoint.set(mousePoint);
            selectedCornerIndex = i;
            return;
        }
    }
    selectedCornerIndex = -1;
}

void ofxQuadWarp::onMouseDragged(ofMouseEventArgs& mouseArgs) {
    if(bShow == false) {
        return;
    }
    if(selectedCornerIndex < 0) return; // no corner selected
    
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    dstPoints[selectedCornerIndex].set(mousePoint);
}

void ofxQuadWarp::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    if(bShow == false) {
        return;
    }
    if(selectedCornerIndex < 0) return; // none selected
    
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    dstPoints[selectedCornerIndex].set(mousePoint);
}

void ofxQuadWarp::keyPressed(ofKeyEventArgs& keyArgs) {
    if(bShow == false) {
        return;
    }

    if(keyArgs.key == OF_KEY_SHIFT)
        bShiftPressed = true;

    if(keyArgs.key == 'p') { // select nxt pt
        selectedCornerIndex = (selectedCornerIndex + 1) % 4;
    }
    else if(keyArgs.key == 'o') { // select prev pt
        selectedCornerIndex = (selectedCornerIndex - 1);
        if(selectedCornerIndex < 0)
            selectedCornerIndex = 3;
    }
    
    if(selectedCornerIndex < 0) return; // no corner selected. only happens if we didn't select one using 'o'/'p' before
    
    float nudgeAmount = 0.25;
    if(bShiftPressed) nudgeAmount = 10;
    ofPoint & selectedPoint = dstPoints[selectedCornerIndex];
    
    switch (keyArgs.key) {
    case OF_KEY_LEFT:
        selectedPoint.x -= nudgeAmount;
        break;
    case OF_KEY_RIGHT:
        selectedPoint.x += nudgeAmount;
        break;
    case OF_KEY_UP:
        selectedPoint.y -= nudgeAmount;
        break;
    case OF_KEY_DOWN:
        selectedPoint.y += nudgeAmount;
        break;
    default:
        break;
    }
}

void ofxQuadWarp::keyReleased(ofKeyEventArgs& keyArgs) {
    if(keyArgs.key == OF_KEY_SHIFT)
        bShiftPressed = false;
}

//----------------------------------------------------- show / hide.
void ofxQuadWarp::show() {
    if(bShow) return;
    toggleShow();
}

void ofxQuadWarp::hide() {
    if(!bShow) return;
    toggleShow();
}

void ofxQuadWarp::toggleShow() {
    bShow = !bShow;
}

bool ofxQuadWarp::isShowing() {
    return bShow;
}

//----------------------------------------------------- save / load.
void ofxQuadWarp::save(const string& path) {
    ofXml xml;
    xml.addChild("quadwarp");
    xml.setTo("quadwarp");
    xml.addChild("src");
    xml.setTo("src");
    for(int i=0; i<4; i++) {
        xml.addChild("point");
        xml.setToChild(i);
        xml.setAttribute("x", ofToString(srcPoints[i].x));
        xml.setAttribute("y", ofToString(srcPoints[i].y));
        xml.setToParent();
    }
    xml.setToParent();
    xml.addChild("dst");
    xml.setTo("dst");
    for(int i=0; i<4; i++) {
        xml.addChild("point");
        xml.setToChild(i);
        xml.setAttribute("x", ofToString(dstPoints[i].x));
        xml.setAttribute("y", ofToString(dstPoints[i].y));
        xml.setToParent();
    }
    xml.setToParent();
    
    xml.setToParent();
    xml.save(path);
}


inline bool fileExists (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }   
}

void ofxQuadWarp::load(const string& path) {    
    ofXml xml;
    bool bOk = xml.load(path);
    if(bOk == false) {
        return;
    }
    
    bOk = xml.setTo("quadwarp");
    if(bOk == false) {
        return;
    }
    
    bOk = xml.setTo("src");
    if(bOk == false) {
        return;
    }
    
    for(int i=0; i<xml.getNumChildren(); i++) {
        bOk = xml.setToChild(i);
        if(bOk == false) {
            continue;
        }
        srcPoints[i].x = ofToFloat(xml.getAttribute("x"));
        srcPoints[i].y = ofToFloat(xml.getAttribute("y"));
        xml.setToParent();
    }
    xml.setToParent();
    
    bOk = xml.setTo("dst");
    if(bOk == false) {
        return;
    }
    
    for(int i=0; i<xml.getNumChildren(); i++) {
        bOk = xml.setToChild(i);
        if(bOk == false) {
            continue;
        }
        dstPoints[i].x = ofToFloat(xml.getAttribute("x"));
        dstPoints[i].y = ofToFloat(xml.getAttribute("y"));
        xml.setToParent();
    }
    xml.setToParent();
    xml.setToParent();
}

//----------------------------------------------------- show / hide.
void ofxQuadWarp::draw() {
    if(bShow == false) return;

    ofSetColor(ofColor::cyan);
    drawQuadOutline();
    
    ofSetColor(ofColor::yellow);
    drawCorners();
    
    ofSetColor(ofColor::magenta);
    drawHighlightedCorner();
    
    ofSetColor(ofColor::red);
    drawSelectedCorner();

    ofSetColor(ofColor::white);
}

void ofxQuadWarp::drawQuadOutline() {
    if(bShow == false) return;
    
    for(int i=0; i<4; i++) {
        int j = (i+1) % 4;
        ofDrawLine(dstPoints[i].x,
                   dstPoints[i].y,
                   dstPoints[j].x,
                   dstPoints[j].y);
    }
}

void ofxQuadWarp::drawCorners() {
    if(bShow == false) return;

    for(int i=0; i<4; i++) {
        ofPoint & point = dstPoints[i];
        drawCornerAt(point);
    }
}

void ofxQuadWarp::drawHighlightedCorner() {
    if(bShow == false) return;
    
    if(highlightCornerIndex < 0) return;

    ofPoint & point = dstPoints[highlightCornerIndex];
    drawCornerAt(point);
}

void ofxQuadWarp::drawSelectedCorner() {
    if(bShow == false) return;
    if(selectedCornerIndex < 0) return;
    
    ofPoint & point = dstPoints[selectedCornerIndex];
    drawCornerAt(point);
}

void ofxQuadWarp::drawCornerAt(const ofPoint & point) {
    ofDrawRectangle(point.x - anchorSize * 0.5,
                    point.y - anchorSize * 0.5,
                    anchorSize, anchorSize);
}
