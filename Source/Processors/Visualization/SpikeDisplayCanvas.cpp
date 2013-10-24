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

#include "SpikeDisplayCanvas.h"


SpikeDisplayCanvas::SpikeDisplayCanvas(SpikeDisplayNode* n) :
    processor(n), newSpike(false)
{

    viewport = new Viewport();
    spikeDisplay = new SpikeDisplay(this, viewport);

    viewport->setViewedComponent(spikeDisplay, false);
    viewport->setScrollBarsShown(true, false);

    scrollBarThickness = viewport->getScrollBarThickness();

    clearButton = new UtilityButton("Clear plots", Font("Small Text", 13, Font::plain));
    clearButton->setRadius(3.0f);
    clearButton->addListener(this);
    addAndMakeVisible(clearButton);

	wAxMirror = new WaveAxes(WAVE1);
    addAndMakeVisible(wAxMirror);

    addAndMakeVisible(viewport);

    setWantsKeyboardFocus(true);

    update();

}

SpikeDisplayCanvas::~SpikeDisplayCanvas()
{
    processor->removeSpikePlots();
}

void SpikeDisplayCanvas::beginAnimation()
{
    std::cout << "SpikeDisplayCanvas beginning animation." << std::endl;

    startCallbacks();
}

void SpikeDisplayCanvas::endAnimation()
{
    std::cout << "SpikeDisplayCanvas ending animation." << std::endl;

    stopCallbacks();
}

void SpikeDisplayCanvas::update()
{

    std::cout << "Updating SpikeDisplayCanvas" << std::endl;

    int nPlots = processor->getNumElectrodes();
    spikeDisplay->removePlots();
    processor->removeSpikePlots();

    for (int i = 0; i < nPlots; i++)
    {
        SpikePlot* sp = spikeDisplay->addSpikePlot(processor->getNumberOfChannelsForElectrode(i), i,
                                   processor->getNameForElectrode(i));
        processor->addSpikePlotForElectrode(sp, i);
    }

    spikeDisplay->resized();
    spikeDisplay->repaint();
}


void SpikeDisplayCanvas::refreshState()
{
    // called when the component's tab becomes visible again
    resized();
}

void SpikeDisplayCanvas::resized()
{
    viewport->setBounds(0,0,getWidth(),getHeight()-90);

    spikeDisplay->setBounds(0,0,getWidth()-scrollBarThickness, spikeDisplay->getTotalHeight());

    clearButton->setBounds(10, getHeight()-40, 100,20);

}

void SpikeDisplayCanvas::paint(Graphics& g)
{

    g.fillAll(Colours::darkgrey);

}

void SpikeDisplayCanvas::refresh()
{
    processSpikeEvents();

    repaint();
}


void SpikeDisplayCanvas::processSpikeEvents()
{

    processor->setParameter(2, 0.0f); // request redraw

}

bool SpikeDisplayCanvas::keyPressed(const KeyPress& key)
{

    KeyPress c = KeyPress::createFromDescription("c");

    if (key.isKeyCode(c.getKeyCode())) // C
    {
        spikeDisplay->clear();

        std::cout << "Clearing display" << std::endl;
        return true;
    }

    return false;

}

void SpikeDisplayCanvas::buttonClicked(Button* button)
{

    if (button == clearButton)
    {
        spikeDisplay->clear();
    }
}



// ----------------------------------------------------------------

SpikeDisplay::SpikeDisplay(SpikeDisplayCanvas* sdc, Viewport* v) :
    canvas(sdc), viewport(v)
{

    totalHeight = 1000;
}

SpikeDisplay::~SpikeDisplay()
{
}

void SpikeDisplay::clear()
{
    if (spikePlots.size() > 0)
    {
        for (int i = 0; i < spikePlots.size(); i++)
        {
            spikePlots[i]->clear();
        }
    }

}


void SpikeDisplay::removePlots()
{
    spikePlots.clear();

}

SpikePlot* SpikeDisplay::addSpikePlot(int numChannels, int electrodeNum, String name_)
{

    std::cout << "Adding new spike plot." << std::endl;

    SpikePlot* spikePlot = new SpikePlot(canvas, electrodeNum, 1000 + numChannels, name_);
    spikePlots.add(spikePlot);
    addAndMakeVisible(spikePlot);

    return spikePlot;
}

void SpikeDisplay::paint(Graphics& g)
{

    g.fillAll(Colours::grey);

}

void SpikeDisplay::resized()
{
    // this is kind of a mess -- is there any way to optimize it?

    if (spikePlots.size() > 0)
    {

        int w = getWidth();

        int numColumns = 1;
        int column, row;

        int stereotrodeStart = 0;
        int tetrodeStart = 0;

        int singlePlotIndex = -1;
        int stereotrodePlotIndex = -1;
        int tetrodePlotIndex = -1;
        int index = -1;

        float width, height;


        float maxHeight = 0;

        for (int i = 0; i < spikePlots.size(); i++)
        {

            if (spikePlots[i]->nChannels == 1)
            {
                index = ++singlePlotIndex;
                numColumns = (int) jmax(w / spikePlots[i]->minWidth, 1.0f);
                width = jmin((float) w / (float) numColumns, (float) getWidth());
                height = width * spikePlots[i]->aspectRatio;

            }
            else if (spikePlots[i]->nChannels == 2)
            {
                index = ++stereotrodePlotIndex;
                numColumns = (int) jmax(w / spikePlots[i]->minWidth, 1.0f);
                width = jmin((float) w / (float) numColumns, (float) getWidth());
                height = width * spikePlots[i]->aspectRatio;

            }
            else if (spikePlots[i]->nChannels == 4)
            {
                index = ++tetrodePlotIndex;
                numColumns = (int) jmax(w / spikePlots[i]->minWidth, 1.0f);
                width = jmin((float) w / (float) numColumns, (float) getWidth());
                height = width * spikePlots[i]->aspectRatio;
            }

            column = index % numColumns;

            row = index / numColumns;

            spikePlots[i]->setBounds(width*column, row*height, width, height);

            maxHeight = jmax(maxHeight, row*height + height);

            if (spikePlots[i]->nChannels == 1)
            {
                stereotrodeStart = (int)(height*(float(row)+1));
            }
            else if (spikePlots[i]->nChannels == 2)
            {
                tetrodeStart = (int)(height*(float(row)+1));
            }

        }


        for (int i = 0; i < spikePlots.size(); i++)
        {

            int x = spikePlots[i]->getX();
            int y = spikePlots[i]->getY();
            int w2 = spikePlots[i]->getWidth();
            int h2 = spikePlots[i]->getHeight();

            if (spikePlots[i]->nChannels == 2)
            {
                spikePlots[i]->setBounds(x, y+stereotrodeStart, w2, h2);
                maxHeight = jmax(maxHeight, (float) y+stereotrodeStart+h2);

            }
            else if (spikePlots[i]->nChannels == 4)
            {
                spikePlots[i]->setBounds(x, y+stereotrodeStart+tetrodeStart, w2, h2);
                maxHeight = jmax(maxHeight, (float) y+stereotrodeStart+tetrodeStart+h2);
            }


        }

        totalHeight = (int) maxHeight + 50;

        // std::cout << "New height = " << totalHeight << std::endl;

        setBounds(0, 0, getWidth(), totalHeight);
    }

}

void SpikeDisplay::mouseDown(const MouseEvent& event)
{

}

void SpikeDisplay::plotSpike(const SpikeObject& spike, int electrodeNum)
{
    spikePlots[electrodeNum]->processSpikeObject(spike);
}


// ----------------------------------------------------------------

SpikePlot::SpikePlot(SpikeDisplayCanvas* sdc, int elecNum, int p, String name_) :
    canvas(sdc), isSelected(false), electrodeNumber(elecNum),  plotType(p),
    limitsChanged(true), name(name_), 
    resizeBorder(new ResizableBorderComponent(this, nullptr)), dragger(new ComponentDragger())

{
    addChildComponent(resizeBorder);
    resizeBorder->setBounds(0,0,getWidth(),getHeight());
    resizeBorder->setVisible(true);

    font = Font("Default", 15, Font::plain);

    switch (p)
    {
        case SINGLE_PLOT:
            // std::cout<<"SpikePlot as SINGLE_PLOT"<<std::endl;
            nWaveAx = 1;
            nProjAx = 0;
            nChannels = 1;
            minWidth = 200;
            aspectRatio = 1.0f;
            break;
        case STEREO_PLOT:
            //  std::cout<<"SpikePlot as STEREO_PLOT"<<std::endl;
            nWaveAx = 2;
            nProjAx = 1;
            nChannels = 2;
            minWidth = 300;
            aspectRatio = 0.5f;
            break;
        case TETRODE_PLOT:
            // std::cout<<"SpikePlot as TETRODE_PLOT"<<std::endl;
            nWaveAx = 4;
            nProjAx = 6;
            nChannels = 4;
            minWidth = 400;
            aspectRatio = 0.5f;
            break;
            //        case HIST_PLOT:
            //            nWaveAx = 1;
            //            nProjAx = 0;
            //            nHistAx = 1;
            //            break;
        default: // unsupported number of axes provided
            std::cout << "SpikePlot as UNKNOWN, defaulting to SINGLE_PLOT" << std::endl;
            nWaveAx = 1;
            nProjAx = 0;
            plotType = SINGLE_PLOT;
            nChannels = 1;
    }

    initAxes();

    for (int i = 0; i < nChannels; i++)
    {
        UtilityButton* rangeButton = new UtilityButton("250", Font("Small Text", 10, Font::plain));
        rangeButton->setRadius(3.0f);
        rangeButton->addListener(this);
        addAndMakeVisible(rangeButton);

        rangeButtons.add(rangeButton);

		WindowButton* addWindowThresholdButton = new WindowButton(); // this button allows us to add new window thresholds in each channel
		addWindowThresholdButton->addListener(this);
		addAndMakeVisible(addWindowThresholdButton);

        windowThresholdButtons.add(addWindowThresholdButton);
    }

}

SpikePlot::~SpikePlot()
{
    delete resizeBorder; // remember to clean up the resizing border
    delete dragger;
}

void SpikePlot::paint(Graphics& g)
{
    
    //const MessageManagerLock mmLock;

    g.setColour(Colours::white);
    g.drawRect(0,0,getWidth(),getHeight());

    g.setFont(font);

    g.drawText(name,10,0,200,20,Justification::left,false);
    
}

void SpikePlot::processSpikeObject(const SpikeObject& s)
{
   // std::cout << "ElectrodePlot::processSpikeObject()" << std::endl;

    // first, check if it's above threshold
    // bool aboveThreshold = false;

    // for (int i = 0; i < nWaveAx; i++)
    // {
    //     aboveThreshold = aboveThreshold | wAxes[i]->checkThreshold(s);
    // }

    // if (aboveThreshold)
    // {
        for (int i = 0; i < nWaveAx; i++)
			if (wAxes[i]->checkThreshold(s))
				wAxes[i]->updateSpikeData(s);

        for (int i = 0; i < nProjAx; i++)
            pAxes[i]->updateSpikeData(s);
    // }

    //     if (aboveThreshold && isRecording)
    //     {
    //         // write spike to disk
    //       writeSpike(s);
    //     }

}

void SpikePlot::select()
{
    isSelected = true;
}

void SpikePlot::deselect()
{
    isSelected = false;
}

void SpikePlot::initAxes()
{
    initLimits();

    for (int i = 0; i < nWaveAx; i++)
    {
        WaveAxes* wAx = new WaveAxes(WAVE1 + i);
        wAxes.add(wAx);
        addAndMakeVisible(wAx);
        ranges.add(250.0f); // default range is 250 microvolts
    }

    for (int i = 0; i < nProjAx; i++)
    {
        ProjectionAxes* pAx = new ProjectionAxes(PROJ1x2 + i);
        pAxes.add(pAx);
        addAndMakeVisible(pAx);
    }

    setLimitsOnAxes(); // initialize the ranges
}

void SpikePlot::resized()
{

    float width = getWidth()-10;
    float height = getHeight()-25;

    float axesWidth, axesHeight;

    // to compute the axes positions we need to know how many columns of proj and wave axes should exist
    // using these two values we can calculate the positions of all of the sub axes
    int nProjCols, nWaveCols;

    switch (plotType)
    {
        case SINGLE_PLOT:
            nProjCols = 0;
            nWaveCols = 1;
            axesWidth = width;
            axesHeight = height;
            break;

        case STEREO_PLOT:
            nProjCols = 1;
            nWaveCols = 2;
            axesWidth = width/2;
            axesHeight = height;
            break;
        case TETRODE_PLOT:
            nProjCols = 3;
            nWaveCols = 2;
            axesWidth = width/4;
            axesHeight = height/2;
            break;
    }

    for (int i = 0; i < nWaveAx; i++)
    {
        wAxes[i]->setBounds(5 + (i % nWaveCols) * axesWidth/nWaveCols, 20 + (i/nWaveCols) * axesHeight, axesWidth/nWaveCols, axesHeight);
        rangeButtons[i]->setBounds(8 + (i % nWaveCols) * axesWidth/nWaveCols,
                                   20 + (i/nWaveCols) * axesHeight + axesHeight - 18,
                                   25, 15);
		windowThresholdButtons[i]->setBounds(38 + (i % nWaveCols) * axesWidth/nWaveCols,
                                   20 + (i/nWaveCols) * axesHeight + axesHeight - 18,
                                   15, 15);
    }

    for (int i = 0; i < nProjAx; i++)
        pAxes[i]->setBounds(5 + (1 + i%nProjCols) * axesWidth, 20 + (i/nProjCols) * axesHeight, axesWidth, axesHeight);

    resizeBorder->setBounds(0, 0, getWidth(), getHeight()); // resize the resizing component to be the same size as this component
}

void SpikePlot::buttonClicked(Button* button)
{
    UtilityButton* buttonThatWasClicked = dynamic_cast<UtilityButton*>(button);

	if (buttonThatWasClicked == NULL)
	{
		WindowButton* buttonThatWasClicked = static_cast<WindowButton*>(button); // we know it MUST be a WindowButton
		wAxes[windowThresholdButtons.indexOf(buttonThatWasClicked)]->addWindowThreshold();
	}
	else
	{
		int index = rangeButtons.indexOf(buttonThatWasClicked);
		String label;

		if (ranges[index] == 250.0f)
		{
			ranges.set(index, 500.0f);
			label = "500";
		}
		else if (ranges[index] == 500.0f)
		{
			ranges.set(index, 100.0f);
			label = "100";
		}
		else if (ranges[index] == 100.0f)
		{
			ranges.set(index, 250.0f);
			label = "250";
		}

		buttonThatWasClicked->setLabel(label);

		setLimitsOnAxes(); 
	}

}

void SpikePlot::setLimitsOnAxes()
{
    //std::cout<<"SpikePlot::setLimitsOnAxes()"<<std::endl;

    for (int i = 0; i < nWaveAx; i++)
        wAxes[i]->setRange(ranges[i]);

    // Each projection sets its limits using the limits of the two waveform dims it represents.
    // Convert projection number to indices, and then set the limits using those indices
    int j1, j2;
    for (int i = 0; i < nProjAx; i++)
    {
        pAxes[i]->n2ProjIdx(pAxes[i]->getType(), &j1, &j2);
        pAxes[i]->setRange(ranges[j1], ranges[j2]);
    }
}

void SpikePlot::initLimits()
{
    for (int i = 0; i < nChannels; i++)
    {
        limits[i][0] = 1209;//-1*pow(2,11);
        limits[i][1] = 11059;//pow(2,14)*1.6;
    }

}

void SpikePlot::getBestDimensions(int* w, int* h)
{
    switch (plotType)
    {
        case TETRODE_PLOT:
            *w = 4;
            *h = 2;
            break;
        case STEREO_PLOT:
            *w = 2;
            *h = 1;
            break;
        case SINGLE_PLOT:
            *w = 1;
            *h = 1;
            break;
        default:
            *w = 1;
            *h = 1;
            break;
    }
}

void SpikePlot::clear()
{
    std::cout << "SpikePlot::clear()" << std::endl;

    for (int i = 0; i < nWaveAx; i++)
        wAxes[i]->clear();
    for (int i = 0; i < nProjAx; i++)
        pAxes[i]->clear();
}

Array<Threshold> SpikePlot::getDisplayThresholdsForChannel(int i)
{
    return wAxes[i]->getDisplayThresholds();
}

void SpikePlot::setDetectorThresholdForChannel(int i, float t)
{
   // std::cout << "Setting threshold to " << t << std::endl;
    wAxes[i]->setDetectorThreshold(t);
}

void SpikePlot::mouseDown(const MouseEvent& event) // to allow component dragging
{
    dragger->startDraggingComponent(this, event);
}

void SpikePlot::mouseDrag(const MouseEvent& event)
{
    dragger->dragComponent(this, event, nullptr);
}

// --------------------------------------------------


WaveAxes::WaveAxes(int channel) : GenericAxes(channel), drawGrid(true),
    bufferSize(5), spikeIndex(0),
    detectorThresholdLevel(0.0f), range(250.0f),
    isOverThresholdSliderTopLeft(false), isOverThresholdSliderBottomRight(false), isOverThresholdSliderMid(false), 
	startDrag(false),
    spikesReceivedSinceLastRedraw(0)
{
	displayThresholdLevels.add(Threshold(0.5f, 50.0f, 0.9f, 10.0f));

    addMouseListener(this, true);

	setWantsKeyboardFocus(true);
	addKeyListener(this);

    thresholdColours.add(Colours::red);

    font = Font("Small Text",10,Font::plain);

    for (int n = 0; n < bufferSize; n++)
    {
        SpikeObject so;
        generateEmptySpike(&so, 4);

        spikeBuffer.add(so);
    }
}

void WaveAxes::setRange(float r)
{

    //std::cout << "Setting range to " << r << std::endl;

    range = r;

    repaint();
}

void WaveAxes::paint(Graphics& g)
{
    g.setColour(Colours::black);
    g.fillRect(0,0,getWidth(), getHeight());

    int chan = 0;

    // draw the grid lines for the waveforms

    if (drawGrid)
        drawWaveformGrid(g);

    // draw the threshold line and labels
    drawThresholdSlider(g);
    //drawBoundingBox(g);

    // if no spikes have been received then don't plot anything
    if (!gotFirstSpike)
    {
        return;
    }


     for (int spikeNum = 0; spikeNum < bufferSize; spikeNum++)
     {

         if (spikeNum != spikeIndex)
         {
             g.setColour(Colours::grey);
             plotSpike(spikeBuffer[spikeNum], g);
         }

     }

    g.setColour(Colours::white);
    plotSpike(spikeBuffer[spikeIndex], g);


    spikesReceivedSinceLastRedraw = 0;

}

void WaveAxes::plotSpike(const SpikeObject& s, Graphics& g)
{

    float h = getHeight();

    //compute the spatial width for each waveform sample
    float dx = getWidth()/float(spikeBuffer[0].nSamples);

    // type corresponds to channel so we need to calculate the starting
    // sample based upon which channel is getting plotted
    int sampIdx = 40*type; //spikeBuffer[0].nSamples * type; //

    int dSamples = 1;

    float x = 0.0f;

    for (int i = 0; i < s.nSamples-1; i++)
    {
        //std::cout << s.data[sampIdx] << std::endl;

        if (*s.gain != 0)
        {
            float s1 = h/2 + float(s.data[sampIdx]-32768)/float(*s.gain)*1000.0f / range * h;
            float s2 =  h/2 + float(s.data[sampIdx+1]-32768)/float(*s.gain)*1000.0f / range * h;

            g.drawLine(x,
                       s1,
                       x+dx,
                       s2);
        }



        sampIdx += dSamples;
        x += dx;
    }

}

void WaveAxes::drawThresholdSlider(Graphics& g)
{

    // draw display threshold (editable)
	for (Threshold* it = displayThresholdLevels.begin(); it != displayThresholdLevels.end(); ++it)
	{
		float tlW = getWidth() * it->topLeftXLevel;
		float brW = getWidth() * it->bottomRightXLevel;
		float tlH = getHeight()*(0.5f - it->topLeftYLevel/range);
		float brH = getHeight()*(0.5f - it->bottomRightYLevel/range);

		g.setColour(thresholdColours[it - displayThresholdLevels.begin()]);
		g.drawLine(tlW, tlH, brW, tlH);
		g.drawLine(tlW, tlH, tlW, brH);
		g.drawLine(brW, tlH, brW, brH);
		g.drawLine(tlW, brH, brW, brH);
		g.drawText(String(roundFloatToInt(it->topLeftXLevel * spikeBuffer[0].nSamples)) + ", " +
				String(roundFloatToInt(it->topLeftYLevel)),2,tlH,50,10,Justification::left, false);
		g.drawText(String(roundFloatToInt(it->bottomRightXLevel * spikeBuffer[0].nSamples)) + ", " +
				String(roundFloatToInt(it->bottomRightYLevel)),getWidth() - 52,brH,getWidth() - 2,10,Justification::left, false);
	}

    // draw detector threshold (not editable)
    float h = getHeight()*(0.5f - detectorThresholdLevel/range);
    
    g.setColour(Colours::orange);
    g.drawLine(0, h, getWidth(), h);
}

void WaveAxes::drawWaveformGrid(Graphics& g)
{

    float h = getHeight();
    float w = getWidth();

    g.setColour(Colours::darkgrey);

    for (float y = -range/2; y < range/2; y += 25.0f)
    {
        if (y == 0)
            g.drawLine(0,h/2 + y/range*h, w, h/2+ y/range*h,2.0f);
        else
            g.drawLine(0,h/2 + y/range*h, w, h/2+ y/range*h);
    }

}

bool WaveAxes::updateSpikeData(const SpikeObject& s)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    if (spikesReceivedSinceLastRedraw < bufferSize)
    {

        SpikeObject newSpike = s;

        spikeIndex++;
        spikeIndex %= bufferSize;

        spikeBuffer.set(spikeIndex, newSpike);

        spikesReceivedSinceLastRedraw++;
        
    }

    return true;

}

bool WaveAxes::checkThreshold(const SpikeObject& s)
{
	// TODO: Use line-rectangle intersection test instead

	for (Threshold* it = displayThresholdLevels.begin(); it != displayThresholdLevels.end(); ++it)
	{
		int sampIdx = 40*type;
		bool intersected = false;
		// Make sure all thresholds are intersected
		for (int i = 0; i < s.nSamples-1; i++)
		{
			float x = float(i) / float(s.nSamples);
			float y = 1.0f-float(s.data[sampIdx]-32768)/float(*s.gain)*1000.0f;
			if (x >= it->topLeftXLevel && x <= it->bottomRightXLevel &&
				y >= it->bottomRightYLevel && y <= it->topLeftYLevel)
			{
				intersected = true;
			}

			sampIdx++;
		}

		if (!intersected)
		{
			return false;
		}
	}

    return true; // all thresholds were intersected

}

void WaveAxes::clear()
{

    spikeBuffer.clear();
    spikeIndex = 0;

    for (int n = 0; n < bufferSize; n++)
    {
        SpikeObject so;
        generateEmptySpike(&so, 4);

        spikeBuffer.add(so);
    }

    repaint();
}

void WaveAxes::mouseMove(const MouseEvent& event)
{
	grabKeyboardFocus();

    // Point<int> pos = event.getPosition();

    float x = event.x;
    float y = event.y;

	for (int ii = displayThresholdLevels.size() - 1; ii >= 0; ii--) // loop from back to ensure last added window gets highest Z-order
	{
		float tlW = getWidth() * displayThresholdLevels[ii].topLeftXLevel;
		float brW = getWidth() * displayThresholdLevels[ii].bottomRightXLevel;
		float tlH = getHeight()*(0.5f - displayThresholdLevels[ii].topLeftYLevel/range);
		float brH = getHeight()*(0.5f - displayThresholdLevels[ii].bottomRightYLevel/range);

		bool aboveTl = y >= (tlH - 5.0f) && y <= (tlH + 2.0f);
		bool leftOfTl = x >= (tlW - 5.0f) && x <= (tlW + 2.0f);
		bool belowBr = y >= (brH - 2.0f) && y <= (brH + 5.0f);
		bool rightOfBr = x >= (brW - 2.0f) && x <= (brW + 5.0f);
		bool insideWindow = x > (tlW + 2.0f) && x < (brW - 2.0f) && y > (tlH + 2.0f) && y < (brH - 2.0f);

		// std::cout << y << " " << h << std::endl;

		if (aboveTl && leftOfTl && !isOverThresholdSliderTopLeft)
		{
			thresholdColours.getReference(ii) = Colours::yellow;

			//  std::cout << "Yes." << std::endl;

			repaint();

			isOverThresholdSliderTopLeft = true;
			isOverThresholdSliderBottomRight = false;
			overIndex = ii;

			break;

			// cursorType = MouseCursor::DraggingHandCursor;

		}
		else if (belowBr && rightOfBr && !isOverThresholdSliderBottomRight)
		{
			thresholdColours.getReference(ii) = Colours::blue;

			//  std::cout << "Yes." << std::endl;

			repaint();

			isOverThresholdSliderTopLeft = false;
			isOverThresholdSliderBottomRight = true;
			overIndex = ii;

			break;

			// cursorType = MouseCursor::DraggingHandCursor;

		}
		else if (insideWindow && !isOverThresholdSliderMid) // switch to dragging the entire window
		{
			thresholdColours.getReference(ii) = Colours::green;

			//  std::cout << "Yes." << std::endl;

			repaint();

			isOverThresholdSliderTopLeft = false;
			isOverThresholdSliderBottomRight = false;
			isOverThresholdSliderMid = true;
			overIndex = ii;

			break;

			// cursorType = MouseCursor::DraggingHandCursor;

		}
		else if ((!aboveTl || !leftOfTl) && isOverThresholdSliderTopLeft && overIndex == ii)
		{

			thresholdColours.getReference(overIndex) = Colours::red;
			repaint();

			isOverThresholdSliderTopLeft = false;

			break;

			//   cursorType = MouseCursor::NormalCursor;

		}
		else if ((!belowBr || !rightOfBr) && isOverThresholdSliderBottomRight && overIndex == ii)
		{

			thresholdColours.getReference(overIndex) = Colours::red;
			repaint();

			isOverThresholdSliderBottomRight = false;

			break;

			//   cursorType = MouseCursor::NormalCursor;

		}
		else if (!insideWindow && isOverThresholdSliderMid && overIndex == ii)
		{

			thresholdColours.getReference(overIndex) = Colours::red;
			repaint();

			isOverThresholdSliderMid = false;

			break;

			//	cursorType = MouseCursor::NormalCursor;

		}
	}
}

void WaveAxes::mouseDown(const MouseEvent& event)
{
    // if (isOverThresholdSlider)
    // {
    //     cursorType = MouseCursor::DraggingHandCursor;
    // }
	if (isOverThresholdSliderMid && !startDrag)
	{
		startDrag = true;
		displayThresholdLevelAtStartOfDrag = displayThresholdLevels[overIndex];
	}
}

void WaveAxes::mouseDrag(const MouseEvent& event)
{
    float thresholdSliderPositionX = float(event.x) / float(getWidth());
    float thresholdSliderPositionY = float(event.y) / float(getHeight());

    if (thresholdSliderPositionX >= 1.0f)
        thresholdSliderPositionX = float(getWidth() - 1) / float(getWidth());
    else if (thresholdSliderPositionX < 0.0f)
        thresholdSliderPositionX = 0.0f;

    if (thresholdSliderPositionY > 1.0f)
        thresholdSliderPositionY = 1.0f;
    else if (thresholdSliderPositionY < 0.0f)
        thresholdSliderPositionY = 0.0f;

    if (isOverThresholdSliderTopLeft)
    {
        if ((thresholdSliderPositionX - displayThresholdLevels[overIndex].bottomRightXLevel) * float(getWidth()) <= -10.0f)
            displayThresholdLevels.getReference(overIndex).topLeftXLevel = thresholdSliderPositionX;
        if (getHeight()*(0.5f - displayThresholdLevels[overIndex].bottomRightYLevel/range) - event.y >= 10.0f)
            displayThresholdLevels.getReference(overIndex).topLeftYLevel = (0.5f - thresholdSliderPositionY) * range;
    }
    else if (isOverThresholdSliderBottomRight)
    {
        if ((displayThresholdLevels[overIndex].topLeftXLevel - thresholdSliderPositionX) * float(getWidth()) <= -10.0f)
            displayThresholdLevels.getReference(overIndex).bottomRightXLevel = thresholdSliderPositionX;
        if (event.y - getHeight()*(0.5f - displayThresholdLevels[overIndex].topLeftYLevel/range) >= 10.0f)
            displayThresholdLevels.getReference(overIndex).bottomRightYLevel = (0.5f - thresholdSliderPositionY) * range;
    }
	else if (isOverThresholdSliderMid)
	{
		float xOffset = event.getOffsetFromDragStart().getX() / float(getWidth());
		float yOffset = -(event.getOffsetFromDragStart().getY() / float(getHeight()) * range);

		float newTopLeftXLevel = displayThresholdLevelAtStartOfDrag.topLeftXLevel + xOffset;
		float newTopLeftYLevel = displayThresholdLevelAtStartOfDrag.topLeftYLevel + yOffset;
		float newBottomRightXLevel = displayThresholdLevelAtStartOfDrag.bottomRightXLevel + xOffset;
		float newBottomRightYLevel = displayThresholdLevelAtStartOfDrag.bottomRightYLevel + yOffset;

		if (newTopLeftXLevel >= 0.0f && newTopLeftXLevel <= 1.0f &&
			newTopLeftYLevel >= -range/2.0f && newTopLeftYLevel <= range/2.0f &&
			newBottomRightXLevel >= 0.0f && newBottomRightXLevel <= 1.0f &&
			newBottomRightYLevel >= -range/2.0f && newBottomRightYLevel <= range/2.0f)
		{
			displayThresholdLevels.getReference(overIndex).topLeftXLevel = newTopLeftXLevel;
			displayThresholdLevels.getReference(overIndex).topLeftYLevel = newTopLeftYLevel;
			displayThresholdLevels.getReference(overIndex).bottomRightXLevel = newBottomRightXLevel;
			displayThresholdLevels.getReference(overIndex).bottomRightYLevel = newBottomRightYLevel;
		}
	}

    repaint();
}

// MouseCursor WaveAxes::getMouseCursor()
// {
//     MouseCursor c = MouseCursor(cursorType);

//     return c;
// }

void WaveAxes::mouseExit(const MouseEvent& event)
{
    if (isOverThresholdSliderTopLeft)
    {
        isOverThresholdSliderTopLeft = false;
        thresholdColours.getReference(overIndex) = Colours::red;
        repaint();
    }
    else if (isOverThresholdSliderBottomRight)
    {
        isOverThresholdSliderBottomRight = false;
        thresholdColours.getReference(overIndex) = Colours::red;
        repaint();
    }
    else if (isOverThresholdSliderMid)
    {
        isOverThresholdSliderMid = false;
        thresholdColours.getReference(overIndex) = Colours::red;
        repaint();
    }

	if (startDrag)
		startDrag = false;
}

void WaveAxes::mouseUp(const MouseEvent& event)
{
	if (startDrag)
		startDrag = false;
}

bool WaveAxes::keyPressed(const KeyPress& key, Component* originatingComponent)
{
	if (key.getKeyCode() == KeyPress::deleteKey)
	{
		if (isOverThresholdSliderTopLeft || isOverThresholdSliderBottomRight || isOverThresholdSliderMid)
		{
			displayThresholdLevels.remove(overIndex);
			thresholdColours.remove(overIndex);
			isOverThresholdSliderTopLeft = isOverThresholdSliderBottomRight = isOverThresholdSliderMid = false;
			repaint();
		}
	}
	return true;
}

Array<Threshold> WaveAxes::getDisplayThresholds()
{
    return displayThresholdLevels;
}

void WaveAxes::setDetectorThreshold(float t)
{
    detectorThresholdLevel = t;
}

void WaveAxes::addWindowThreshold()
{
	displayThresholdLevels.add(Threshold(0.5f, 50.0f, 0.9f, 10.0f));
	thresholdColours.add(Colours::red);
	repaint();
}

// --------------------------------------------------

ProjectionAxes::ProjectionAxes(int projectionNum) : GenericAxes(projectionNum), imageDim(500),
    rangeX(250), rangeY(250), spikesReceivedSinceLastRedraw(0)
{
    projectionImage = Image(Image::RGB, imageDim, imageDim, true);

    clear();
    //Graphics g(projectionImage);
    //g.setColour(Colours::red);
    //g.fillEllipse(20, 20, 300, 200);

    n2ProjIdx(projectionNum, &ampDim1, &ampDim2);


}

void ProjectionAxes::setRange(float x, float y)
{
    rangeX = (int) x;
    rangeY = (int) y;

    //std::cout << "Setting range to " << x << " " << y << std::endl;

    repaint();
}

void ProjectionAxes::paint(Graphics& g)
{
    //g.setColour(Colours::orange);
    //g.fillRect(5,5,getWidth()-5, getHeight()-5);

    g.drawImage(projectionImage,
                0, 0, getWidth(), getHeight(),
                0, imageDim-rangeY, rangeX, rangeY);
}

bool ProjectionAxes::updateSpikeData(const SpikeObject& s)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    int idx1, idx2;
    calcWaveformPeakIdx(s, ampDim1, ampDim2, &idx1, &idx2);

    // add peaks to image

    updateProjectionImage(s.data[idx1], s.data[idx2], *s.gain);

    return true;
}

void ProjectionAxes::updateProjectionImage(uint16_t x, uint16_t y, uint16_t gain)
{
    Graphics g(projectionImage);

    // h/2 + float(s.data[sampIdx]-32768)/float(*s.gain)*1000.0f / range * h;

    if (gain != 0)
    {
        float xf = float(x-32768)/float(gain)*1000.0f; // in microvolts
        float yf = float(imageDim) - float(y-32768)/float(gain)*1000.0f; // in microvolts

        g.setColour(Colours::white);
        g.fillEllipse(xf,yf,2.0f,2.0f);
    }

}

void ProjectionAxes::calcWaveformPeakIdx(const SpikeObject& s, int d1, int d2, int* idx1, int* idx2)
{

    int max1 = -1*pow(2.0,15);
    int max2 = max1;

    for (int i = 0; i < s.nSamples; i++)
    {
        if (s.data[d1*s.nSamples + i] > max1)
        {
            *idx1 = d1*s.nSamples+i;
            max1 = s.data[*idx1];
        }
        if (s.data[d2*s.nSamples+i] > max2)
        {
            *idx2 = d2*s.nSamples+i;
            max2 = s.data[*idx2];
        }
    }
}



void ProjectionAxes::clear()
{
    projectionImage.clear(Rectangle<int>(0, 0, projectionImage.getWidth(), projectionImage.getHeight()),
                          Colours::black);

    repaint();
}

void ProjectionAxes::n2ProjIdx(int proj, int* p1, int* p2)
{
    int d1, d2;
    if (proj==PROJ1x2)
    {
        d1 = 0;
        d2 = 1;
    }
    else if (proj==PROJ1x3)
    {
        d1 = 0;
        d2 = 2;
    }
    else if (proj==PROJ1x4)
    {
        d1 = 0;
        d2 = 3;
    }
    else if (proj==PROJ2x3)
    {
        d1 = 1;
        d2 = 2;
    }
    else if (proj==PROJ2x4)
    {
        d1 = 1;
        d2 = 3;
    }
    else if (proj==PROJ3x4)
    {
        d1 = 2;
        d2 = 3;
    }
    else
    {
        std::cout<<"Invalid projection:"<<proj<<"! Cannot determine d1 and d2"<<std::endl;
        *p1 = -1;
        *p2 = -1;
        return;
    }
    *p1 = d1;
    *p2 = d2;
}

// --------------------------------------------------

GenericAxes::GenericAxes(int t)
    : gotFirstSpike(false), type(t)
{
    ylims[0] = 0;
    ylims[1] = 1;

    xlims[0] = 0;
    xlims[1] = 1;

    font = Font("Default", 12, Font::plain);

}

GenericAxes::~GenericAxes()
{

}

bool GenericAxes::updateSpikeData(const SpikeObject& newSpike)
{
    if (!gotFirstSpike)
    {
        gotFirstSpike = true;
    }

    s = newSpike;
    return true;
}

void GenericAxes::setYLims(double ymin, double ymax)
{

    //std::cout << "setting y limits to " << ymin << " " << ymax << std::endl;
    ylims[0] = ymin;
    ylims[1] = ymax;
}
void GenericAxes::getYLims(double* min, double* max)
{
    *min = ylims[0];
    *max = ylims[1];
}
void GenericAxes::setXLims(double xmin, double xmax)
{
    xlims[0] = xmin;
    xlims[1] = xmax;
}
void GenericAxes::getXLims(double* min, double* max)
{
    *min = xlims[0];
    *max = xlims[1];
}


void GenericAxes::setType(int t)
{
    if (t < WAVE1 || t > PROJ3x4)
    {
        std::cout<<"Invalid Axes type specified";
        return;
    }
    type = t;
}

int GenericAxes::getType()
{
    return type;
}

int GenericAxes::roundUp(int numToRound, int multiple)
{
    if (multiple == 0)
    {
        return numToRound;
    }

    int remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;
    return numToRound + multiple - remainder;
}


void GenericAxes::makeLabel(int val, int gain, bool convert, char* s)
{
    if (convert)
    {
        double volt = ad16ToUv(val, gain)/1000.;
        if (abs(val)>1e6)
        {
            //val = val/(1e6);
            sprintf(s, "%.2fV", volt);
        }
        else if (abs(val)>1e3)
        {
            //val = val/(1e3);
            sprintf(s, "%.2fmV", volt);
        }
        else
            sprintf(s, "%.2fuV", volt);
    }
    else
    {
        sprintf(s,"%d", (int)val);
    }
}

double GenericAxes::ad16ToUv(int x, int gain)
{
    int result = (double)(x * 20e6) / (double)(gain * pow(2.0,16));
    return result;
}
