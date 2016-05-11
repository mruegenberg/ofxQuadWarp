//
//  ofxQuadWarp.cpp
//  Created by lukasz karluk on 19/06/11.
//

#include "ofxQuadWarp.h"
#include "ofxOpenCv.h"

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
	//we need our points as opencv points
	//be nice to do this without opencv?
	CvPoint2D32f cvsrc[4];
	CvPoint2D32f cvdst[4];
	//we set the warp coordinates
	//source coordinates as the dimensions of our window
	for(int i=0; i<4; ++i) {
	cvsrc[i].x = srcPoints[i].x;
	cvsrc[i].y = srcPoints[i].y;
	cvdst[i].x = dstPoints[i].x;
	cvdst[i].y = dstPoints[i].y;
	}
	//we create a matrix that will store the results
	//from openCV - this is a 3x3 2D matrix that is
	//row ordered
	CvMat * translate = cvCreateMat(3,3,CV_32FC1);
	//this is the slightly easier - but supposidly less
	//accurate warping method
	//cvWarpPerspectiveQMatrix(cvsrc, cvdst, translate);
	//for the more accurate method we need to create
	//a couple of matrixes that just act as containers
	//to store our points - the nice thing with this
	//method is you can give it more than four points!
	CvMat* src_mat = cvCreateMat(4, 1, CV_32FC2);
	CvMat* dst_mat = cvCreateMat(4, 1, CV_32FC2);
	//copy our points into the matrixes
	cvSetData(src_mat, cvsrc, sizeof(CvPoint2D32f));
	cvSetData(dst_mat, cvdst, sizeof(CvPoint2D32f));
	//figure out the warping!
	//warning - older versions of openCV had a bug
	//in this function.
	cvFindHomography(src_mat, dst_mat, translate);
	//get the matrix as a list of floats
	float *mat = translate->data.fl;
	//we need to copy these values
	//from the 3x3 2D openCV matrix which is row ordered
	//
	// ie: [0][1][2] x
	// [3][4][5] y
	// [6][7][8] w
	//to openGL's 4x4 3D column ordered matrix
	// x y z w
	// ie: [0][3][ ][6]
	// [1][4][ ][7]
	// [ ][ ][ ][ ]
	// [2][5][ ][9]
	//
	ofMatrix4x4 matrixTemp;
	matrixTemp.getPtr()[0] = mat[0];
	matrixTemp.getPtr()[4] = mat[1];
	matrixTemp.getPtr()[12] = mat[2];
	matrixTemp.getPtr()[1] = mat[3];
	matrixTemp.getPtr()[5] = mat[4];
	matrixTemp.getPtr()[13] = mat[5];
	matrixTemp.getPtr()[3] = mat[6];
	matrixTemp.getPtr()[7] = mat[7];
	matrixTemp.getPtr()[15] = mat[8];
	cvReleaseMat(&translate);
	cvReleaseMat(&src_mat);
	cvReleaseMat(&dst_mat);
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
    
    float nudgeAmount = 0.3;
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
    
    drawQuadOutline();
    drawCorners();
    drawHighlightedCorner();
    drawSelectedCorner();
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
