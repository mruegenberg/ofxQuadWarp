//
//  ofxQuadWarp.cpp
//  Created by lukasz karluk on 19/06/11.
//

#include "ofxQuadWarp.h"
#include "matrix_funcs.h"

ofxQuadWarp::ofxQuadWarp() {
    anchorSize = 10;
    anchorSizeHalf = anchorSize * 0.5;
    selectedCornerIndex = -1;
    highlightCornerIndex = -1;
    
    bMouseEnabled = false;
    bKeyboardShortcuts = false;
    bShow = false;
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
void ofxQuadWarp::setPosition(float x, float y) {
    position.x = x;
    position.y = y;
}

void ofxQuadWarp::setAnchorSize(float value) {
    anchorSize = value;
    anchorSizeHalf = anchorSize * 0.5;
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
}

void ofxQuadWarp::disableKeyboardShortcuts() {
    if(bKeyboardShortcuts == false) {
        return;
    }
    bKeyboardShortcuts = false;
    try {
        ofRemoveListener(ofEvents().keyPressed, this, &ofxQuadWarp::keyPressed);
    }
    catch(Poco::SystemException) {
        return;
    }
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
    ofMatrix4x4 matrixTemp;
    
    // we set it to the default - 0 translation
    // and 1.0 scale for x y z and w
    for(int i = 0; i < 16; i++) {
        if(i % 5 != 0) {
            matrixTemp.getPtr()[i] = 0.0;
        }
        else {
            matrixTemp.getPtr()[i] = 1.0;
        }
    }
    
    // source and destination points
    double src[4][2];
    double dest[4][2];

    for(int i=0; i<4; ++i) {
        src[i][0] = srcPoints[i].x;
        src[i][1] = srcPoints[i].y;
        
        dest[i][0] = dstPoints[i].x;
        dest[i][1] = dstPoints[i].y;
    }

    double warpMatrix[3][3];  //< interim projection warping matrix
    
    // perform the warp calculation
    mapQuadToQuad(src, dest, warpMatrix);
    
    // copy the values
    matrixTemp.getPtr()[0] = warpMatrix[0][0];
    matrixTemp.getPtr()[1] = warpMatrix[0][1];
    matrixTemp.getPtr()[3] = warpMatrix[0][2];
    
    matrixTemp.getPtr()[4] = warpMatrix[1][0];
    matrixTemp.getPtr()[5] = warpMatrix[1][1];
    matrixTemp.getPtr()[7] = warpMatrix[1][2];
    
    matrixTemp.getPtr()[12] = warpMatrix[2][0];
    matrixTemp.getPtr()[13] = warpMatrix[2][1];
    matrixTemp.getPtr()[15] = warpMatrix[2][2];

    return matrixTemp;
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
    mousePoint -= position;
    for(int i=0; i<4; i++) {
        ofPoint & dstPoint = dstPoints[i];
        if(mousePoint.distance(dstPoint) <= anchorSizeHalf) {
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
    mousePoint -= position;
    for(int i=0; i<4; i++) {
        ofPoint & dstPoint = dstPoints[i];
        if(mousePoint.distance(dstPoint) <= anchorSizeHalf) {
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
    if(selectedCornerIndex < 0 || selectedCornerIndex > 3) {
        return;
    }
    
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    mousePoint -= position;
    dstPoints[selectedCornerIndex].set(mousePoint);
}

void ofxQuadWarp::onMouseReleased(ofMouseEventArgs& mouseArgs) {
    if(bShow == false) {
        return;
    }
    if(selectedCornerIndex < 0 || selectedCornerIndex > 3) {
        return;
    }
    
    ofPoint mousePoint(mouseArgs.x, mouseArgs.y);
    mousePoint -= position;
    dstPoints[selectedCornerIndex].set(mousePoint);
}

void ofxQuadWarp::keyPressed(ofKeyEventArgs& keyArgs) {
    if(bShow == false) {
        return;
    }

    if(keyArgs.key == 'p') {
        selectedCornerIndex = (selectedCornerIndex + 1) % 4;
    }
    
    if(selectedCornerIndex < 0 || selectedCornerIndex > 3) {
        return;
    }
    
    float nudgeAmount = 0.3;
    if(bFast) nudgeAmount = 10;
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

//----------------------------------------------------- corners.
void ofxQuadWarp::setCorners(const vector<ofPoint>& corners) {
    vector<ofPoint> _corners = corners;
    _corners.resize(4);
    setTopLeftCornerPosition(_corners[0]);
    setTopRightCornerPosition(_corners[1]);
    setBottomRightCornerPosition(_corners[2]);
    setBottomLeftCornerPosition(_corners[3]);
}

void ofxQuadWarp::setCorner(const ofPoint& p, int cornerIndex) {
    cornerIndex = ofClamp(cornerIndex, 0, 3);
    dstPoints[cornerIndex].set(p);
}

void ofxQuadWarp::setTopLeftCornerPosition(const ofPoint& p) {
    setCorner(p, 0);
}

void ofxQuadWarp::setTopRightCornerPosition(const ofPoint& p) {
    setCorner(p, 1);
}

void ofxQuadWarp::setBottomRightCornerPosition(const ofPoint& p) {
    setCorner(p, 2);
}

void ofxQuadWarp::setBottomLeftCornerPosition(const ofPoint& p) {
    setCorner(p, 3);
}

//----------------------------------------------------- show / hide.
void ofxQuadWarp::show() {
    if(bShow) {
        return;
    }
    toggleShow();
}

void ofxQuadWarp::hide() {
    if(!bShow) {
        return;
    }
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
    if(bShow == false) {
        return;
    }
    
    drawQuadOutline();
    drawCorners();
    drawHighlightedCorner();
    drawSelectedCorner();
}

void ofxQuadWarp::drawQuadOutline() {
    if(bShow == false) {
        return;
    }
    
    for(int i=0; i<4; i++) {
        int j = (i+1) % 4;
        ofLine(dstPoints[i].x + position.x,
               dstPoints[i].y + position.y,
               dstPoints[j].x + position.x,
               dstPoints[j].y + position.y);
    }
}

void ofxQuadWarp::drawCorners() {
    if(bShow == false) {
        return;
    }

    for(int i=0; i<4; i++) {
        ofPoint & point = dstPoints[i];
        drawCornerAt(point);
    }
}

void ofxQuadWarp::drawHighlightedCorner() {
    if(bShow == false) {
        return;
    }
    if(highlightCornerIndex < 0 || highlightCornerIndex > 3) {
        return;
    }

    ofPoint & point = dstPoints[highlightCornerIndex];
    drawCornerAt(point);
}

void ofxQuadWarp::drawSelectedCorner() {
    if(bShow == false) {
        return;
    }
    if(selectedCornerIndex < 0 || selectedCornerIndex > 3) {
        return;
    }
    
    ofPoint & point = dstPoints[selectedCornerIndex];
    drawCornerAt(point);
}

void ofxQuadWarp::drawCornerAt(const ofPoint & point) {
    ofRect(point.x + position.x - anchorSizeHalf,
           point.y + position.y - anchorSizeHalf,
           anchorSize, anchorSize);
}
