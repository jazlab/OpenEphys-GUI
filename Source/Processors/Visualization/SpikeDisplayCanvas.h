/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef SPIKEDISPLAYCANVAS_H_
#define SPIKEDISPLAYCANVAS_H_

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../SpikeDisplayNode.h"
#include "SpikeObject.h"

#include "Visualizer.h"
#include <set>
#include <vector>

#define WAVE1 0
#define WAVE2 1
#define WAVE3 2
#define WAVE4 3
#define PROJ1x2 4
#define PROJ1x3 5
#define PROJ1x4 6
#define PROJ2x3 7
#define PROJ2x4 8
#define PROJ3x4 9

#define TETRODE_PLOT 1004
#define STEREO_PLOT  1002
#define SINGLE_PLOT  1001

#define MAX_NUMBER_OF_SPIKE_SOURCES 128
#define MAX_N_CHAN 4

class SpikeDisplayNode;

class SpikeDisplay;
class GenericAxes;
class ProjectionAxes;
struct Threshold;
class WaveAxes;
class WaveAxesRef;
class SpikePlot;
class RecordNode;

/**

  Displays spike waveforms and projections.

  @see SpikeDisplayNode, SpikeDisplayEditor, Visualizer

*/

class SpikeDisplayCanvas : public Visualizer, public Button::Listener

{
public:
    SpikeDisplayCanvas(SpikeDisplayNode* n);
    ~SpikeDisplayCanvas();

    void paint(Graphics& g);

    void refresh();

    void processSpikeEvents();

    void beginAnimation();
    void endAnimation();

    void refreshState();

    void setParameter(int, float) {}
    void setParameter(int, int, int, float) {}

	void setMirror(WaveAxes *wAx);

    void update();

    void resized();

    bool keyPressed(const KeyPress& key);

    void buttonClicked(Button* button);

    void startRecording() { } // unused
    void stopRecording() { } // unused
    
    SpikeDisplayNode* processor;

private:

    ScopedPointer<SpikeDisplay> spikeDisplay;
    ScopedPointer<Viewport> viewport;

    ScopedPointer<UtilityButton> clearButton;
	ScopedPointer<WaveAxesRef> wAxMirror;

    bool newSpike;
    SpikeObject spike;

    int scrollBarThickness;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpikeDisplayCanvas);

};

class SpikeDisplay : public Component
{
public:
    SpikeDisplay(SpikeDisplayCanvas*, Viewport*);
    ~SpikeDisplay();

    void removePlots();
    void clear();
    SpikePlot* addSpikePlot(int numChannels, int electrodeNum, String name);

    void paint(Graphics& g);

    void resized();

    void mouseDown(const MouseEvent& event);

    void plotSpike(const SpikeObject& spike, int electrodeNum);

    int getTotalHeight()
    {
        return totalHeight;
    }

private:

    //void computeColumnLayout();
    //void initializeSpikePlots();
    //void repositionSpikePlots();

    int numColumns;

    int totalHeight;

    SpikeDisplayCanvas* canvas;
    Viewport* viewport;

    OwnedArray<SpikePlot> spikePlots;

    // float tetrodePlotMinWidth, stereotrodePlotMinWidth, singleElectrodePlotMinWidth;
    // float tetrodePlotRatio, stereotrodePlotRatio, singleElectrodePlotRatio;

};

/**

  Class for drawing the waveforms and projections of incoming spikes.

  Also responsible for saving spikes.

*/

class SpikePlot : public Component, Button::Listener
{
public:
    SpikePlot(SpikeDisplayCanvas*, int elecNum, int plotType, String name_);
    virtual ~SpikePlot();

    void paint(Graphics& g);
    void resized();

    void select();
    void deselect();

    void processSpikeObject(const SpikeObject& s);

    SpikeDisplayCanvas* canvas;

    bool isSelected;

    int electrodeNumber;

    int nChannels;

    void initAxes();
    void getBestDimensions(int*, int*);

    void clear();

    float minWidth;
    float aspectRatio;

    void buttonClicked(Button* button);

    Array<Threshold> getDisplayThresholdsForChannel(int);
	void setDisplayThresholdsForChannel(Array<Threshold>, int);
	float getDetectorThresholdForChannel(int);
    void setDetectorThresholdForChannel(int, float);

    void mouseDown(const MouseEvent& event); // to allow component dragging
    void mouseDrag(const MouseEvent& event);
private:

    int plotType;
    int nWaveAx;
    int nProjAx;

    bool limitsChanged;

    double limits[MAX_N_CHAN][2];

    OwnedArray<ProjectionAxes> pAxes;
    OwnedArray<WaveAxes> wAxes;
    OwnedArray<UtilityButton> rangeButtons;
	OwnedArray<WindowButton> windowThresholdButtons;
    Array<float> ranges;

    void initLimits();
    void setLimitsOnAxes();
    void updateAxesPositions();

    String name;

    Font font;

    ResizableBorderComponent* resizeBorder; // to allow resizing of this component
    ComponentDragger* dragger; // to allow dragging of this component

};

/**

  Base class for drawing axes for spike visualization.

  @see SpikeDisplayCanvas

*/

class GenericAxes : public Component
{
public:

    GenericAxes(int t);

    virtual ~GenericAxes();

    virtual bool updateSpikeData(const SpikeObject& s);

    void setXLims(double xmin, double xmax);
    void getXLims(double* xmin, double* xmax);
    void setYLims(double ymin, double ymax);
    void getYLims(double* ymin, double* ymax);

    void setType(int type);
    int getType();

    virtual void paint(Graphics& g) = 0;

    int roundUp(int, int);
    void makeLabel(int val, int gain, bool convert, char* s);

protected:
    double xlims[2];
    double ylims[2];

    SpikeObject s;

    bool gotFirstSpike;

    int type;

    Font font;

    double ad16ToUv(int x, int gain);

};


/**
 * Struct which contains rectangular threshold parameters
 */
struct Threshold
{
    float topLeftXLevel, topLeftYLevel;
    float bottomRightXLevel, bottomRightYLevel;
    Threshold() :
		topLeftXLevel(0.5f),
		topLeftYLevel(50.0f),
		bottomRightXLevel(0.9f),
		bottomRightYLevel(10.0f)
	{}
    Threshold(float tlX, float tlY, float brX, float brY) :
		topLeftXLevel(tlX),
		topLeftYLevel(tlY),
		bottomRightXLevel(brX),
		bottomRightYLevel(brY)
    {}
};

/**

  Class for drawing spike waveforms.

*/

class WaveAxes : public GenericAxes, KeyListener
{
public:
    WaveAxes(SpikeDisplayCanvas* sdc, int channel);
    ~WaveAxes() {}

    bool updateSpikeData(const SpikeObject& s);
    bool checkThreshold(const SpikeObject& spike);

	void paintImpl(Graphics& g, int width, int height);
    void paint(Graphics& g);

    void plotSpike(const SpikeObject& s, Graphics& g, int width, int height);

    void clear();

	void mouseMoveImpl(const MouseEvent& event, int width, int height);
    void mouseMove(const MouseEvent& event);
    void mouseExit(const MouseEvent& event);
    void mouseDown(const MouseEvent& event);
    void mouseDragImpl(const MouseEvent& event, int width, int height);
    void mouseDrag(const MouseEvent& event);
	void mouseUp(const MouseEvent& event);
	void mouseDoubleClick(const MouseEvent& event);

	bool keyPressed(const KeyPress& key, Component* originatingComponent);

    void setRange(float);
    float getRange()
    {
        return range;
    }

    Array<Threshold> getDisplayThresholds();
	void setDisplayThresholds(Array<Threshold>);
	float getDetectorThreshold();
    void setDetectorThreshold(float);

	void addWindowThreshold();

    //MouseCursor getMouseCursor();

	void addObserver(WaveAxesRef* mirror);
	void removeObserver(WaveAxesRef* mirror);

private:
    Colour waveColour;
    Array<Colour> thresholdColours;
    Colour gridColour;

	SpikeDisplayCanvas* canvas;

    bool drawGrid;

    Array<Threshold> displayThresholdLevels;
	Threshold displayThresholdLevelAtStartOfDrag;
	int draggedIndex;
    float detectorThresholdLevel;

    void drawWaveformGrid(Graphics& g, int width, int height);

    void drawThresholdSlider(Graphics& g, int width, int height);

    int spikesReceivedSinceLastRedraw;

    Font font;

    Array<SpikeObject> spikeBuffer;

    int spikeIndex;
    int bufferSize;

    float range;

    bool isOverThresholdSliderTopLeft;
    bool isOverThresholdSliderBottomRight;
	bool isOverThresholdSliderMid;
	int overIndex;
	bool startDrag;

    MouseCursor::StandardCursorType cursorType;

	std::set<WaveAxesRef*> observers;
};

class WaveAxesRef : public Component, KeyListener
{
public:
    WaveAxesRef(WaveAxes *wAx);
    ~WaveAxesRef();

    void paint(Graphics& g);

    void mouseMove(const MouseEvent& event);
    void mouseExit(const MouseEvent& event);
    void mouseDown(const MouseEvent& event);
    void mouseDrag(const MouseEvent& event);
	void mouseUp(const MouseEvent& event);

	bool keyPressed(const KeyPress& key, Component* originatingComponent);

	void setMirrored(WaveAxes *wAx);

private:
	WaveAxes *mirroredWaveAxes;
};





/**

  Class for drawing the peak projections of spike waveforms.

*/

class ProjectionAxes : public GenericAxes
{
public:
    ProjectionAxes(int projectionNum);
    ~ProjectionAxes() {}

    bool updateSpikeData(const SpikeObject& s);

    void paint(Graphics& g);

    void clear();

    void setRange(float, float);

    static void n2ProjIdx(int i, int* p1, int* p2);

private:

    void updateProjectionImage(uint16_t, uint16_t, uint16_t);

    void calcWaveformPeakIdx(const SpikeObject&, int, int, int*, int*);

    int ampDim1, ampDim2;

    Image projectionImage;

    Colour pointColour;
    Colour gridColour;

    int imageDim;

    int rangeX;
    int rangeY;

    int spikesReceivedSinceLastRedraw;

};



#endif  // SPIKEDISPLAYCANVAS_H_
