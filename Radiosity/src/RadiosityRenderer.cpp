/*
 * Copyright (C) 2007 TODO
 *
 * This file is part of rrv (Radiosity Renderer and Visualizer).
 *
 * rrv is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * rrv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with rrv.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RadiosityRenderer.h"
#include "FormFactorEngine.h"
#include "PatchRandomAccessEnumerator.h"
#include "PatchCache.h"
#include "TriangleSet.h"
#include <iostream>

RadiosityRenderer::RadiosityRenderer(
	TriangleSet &tSet,
    float formFactorTreshold,
    long maxCacheSize
):
    currentStep_(0),
    currentPatch_(0),
    colorPeak_(0.0)
{
    patchEnumerator_ = new PatchRandomAccessEnumerator(tSet);
    patchCount_ = patchEnumerator_->count();

    // Create patch cache
    patchCache_ = new PatchCache(patchEnumerator_, formFactorTreshold, maxCacheSize);

// #ifdef _DEBUG
    std::cout << "Count of patch:" << patchCount_ << std::endl;
 //   std::cout << "--- FormFactor treshold: " << formFactorTreshold << std::endl;
	//std::cout << "--- Max cache raw size: " << maxCacheSize/(1024*1024) << " MB" << std::endl;
// #endif
}

RadiosityRenderer::~RadiosityRenderer()
{
    delete patchCache_;
    delete patchEnumerator_;
}

/**
	*/
int RadiosityRenderer::stepCount() const
{
    return stepCount_;
}


/**
	* @return int
	*/
int RadiosityRenderer::currentStep() const
{
    return currentStep_;
}

/**
	*/
int RadiosityRenderer::patchCount() const
{
    return patchCount_;
}


/**
	*/
int RadiosityRenderer::currentPatch() const
{
    return currentPatch_;
}

long int RadiosityRenderer::cacheRawSize() const
{
    return patchCache_->cacheRawSize();
}

void RadiosityRenderer::computeOneStep()
{
	//notifyStep();
	computeStep();
}

void RadiosityRenderer::computeNStep(int stepCount)
{
    for(currentStep_=0; currentStep_<stepCount; currentStep_++) {
		computeOneStep();
		std::cout << "Step " << currentStep_ << " is Done" << std::endl;
		std::cout.flush();
    }
}

/**
 */
void RadiosityRenderer::computeStep()
{
    PatchRandomAccessEnumerator &patch = *patchEnumerator_;

    // Initialize radiosityLast tag of Triangle objects
    for(int i=0; i<patchCount_; i++) {
        Triangle &dest = patch[i];
        dest.radiosityLast = dest.radiosity;
    }

    // Compute new step
    for(currentPatch_=0; currentPatch_<patchCount_; currentPatch_++) {
        //notifyPerStepProgress();

        // Compute radiosity
        Triangle &dest = patch[currentPatch_];
        Color &rad = dest.radiosity;
		Color sumRadio = patchCache_->totalRadiosity(currentPatch_);
		rad.r = sumRadio.r * dest.reflectivity.r + dest.emission.r;	
		rad.g = sumRadio.g * dest.reflectivity.g + dest.emission.g;	
		rad.b = sumRadio.b * dest.reflectivity.b + dest.emission.b;	

        //updateColorPeak(rad);
    }
}

void RadiosityRenderer::normalize()
{
// #ifdef _DEBUG
    std::cout << ">>> Normalizing ... " << std::flush;
// #endif
    PatchRandomAccessEnumerator &patch = *patchEnumerator_;

    for(int i=0; i<patchCount_; i++)
        normalize(patch[i].radiosity);
// #ifdef _DEBUG
    std::cout << "ok" << std::endl;
// #endif
}
