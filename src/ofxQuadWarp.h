//
//  ofxQuadWarp.h
//  Created by lukasz karluk on 19/06/11.
//
//  nay added hack to invert on 2011/06/21
//
#pragma once

#include "ofMain.h"

class ofxQuadWarp 
{
public: 
    
     ofxQuadWarp();
    ~ofxQuadWarp();
    
    void setup();
    
    void setAnchorSize(float value);
    
    void setSourceRect(const ofRectangle& rect);
    void setTargetRect(const ofRectangle& rect);
    void setTargetPoints(const vector<ofPoint>& points);
    
    void enableMouseControls();
    void disableMouseControls();
    
    void enableKeyboardShortcuts();
    void disableKeyboardShortcuts();
    
    void update();
    void reset();
    
    ofMatrix4x4 getMatrix() const;
    ofMatrix4x4 getMatrixInverse() const;
    ofMatrix4x4 getMatrix(const ofPoint * srcPoints, const ofPoint * dstPoints) const;
    
    void show();
    void hide();
    void toggleShow();
    bool isShowing();
    
    void save(const string& path="quadwarp.xml");
    void load(const string& path="quadwarp.xml");
    
    void draw();
    void drawQuadOutline();
    void drawCorners();
    void drawHighlightedCorner();
    void drawSelectedCorner();
    void drawCornerAt(const ofPoint& point);

    // FIXME: disallow explicit access to this
    ofPoint srcPoints[4];
    ofPoint dstPoints[4];

    void onMouseMoved(ofMouseEventArgs & mouseArgs);
    void onMousePressed(ofMouseEventArgs & mouseArgs);
    void onMouseDragged(ofMouseEventArgs & mouseArgs);
    void onMouseReleased(ofMouseEventArgs & mouseArgs);
    void keyPressed(ofKeyEventArgs & keyArgs);
    void keyReleased(ofKeyEventArgs & keyArgs);

protected:    
    float anchorSize;
    int selectedCornerIndex;
    int highlightCornerIndex;
    
    bool bMouseEnabled;
    bool bKeyboardShortcuts;
    bool bShow;

    bool bShiftPressed;
};
