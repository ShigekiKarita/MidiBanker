//------------------------------------------------------------------------
// Project     : VST SDK
//
// Category    : Examples
// Filename    : plugcontroller.cpp
// Created by  : Steinberg, 01/2018
// Description : MidiBanker Example for VST 3
//
//-----------------------------------------------------------------------------
// LICENSE
// (c) 2019, Steinberg Media Technologies GmbH, All Rights Reserved
//-----------------------------------------------------------------------------
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
//   * Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation 
//     and/or other materials provided with the distribution.
//   * Neither the name of the Steinberg Media Technologies nor the names of its
//     contributors may be used to endorse or promote products derived from this 
//     software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
// IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
// OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

#include "plugcontroller.h"
#include "plugids.h"

#include "base/source/fdebug.h"
#include "base/source/fstreamer.h"
#include "pluginterfaces/base/ibstream.h"
#include "public.sdk/source/vst/vstparameters.h"

#include<filesystem>
#include<iostream>


namespace Steinberg {
namespace MidiBanker {

using Vst::StringListParameter;
namespace fs = std::filesystem;

void registerDevicePatch(StringListParameter* list, const fs::path& path, int depth = 0)
{
	if (!std::filesystem::exists(path))
	{
		SMTG_WARNING(path.string() + " is not found");
		return;
	}
	
	for (auto&& i : std::filesystem::directory_iterator(path))
	{
		auto p = i.path();
		SMTG_DBPRT0(p.string().c_str());

		if (p.extension() == ".txt")
		{
			list->appendString(p.stem().c_str());
		}
		
		if (i.is_directory())
		{
			registerDevicePatch(list, p, depth + 1);
		}
	}
}

//-----------------------------------------------------------------------------
tresult PLUGIN_API PlugController::initialize (FUnknown* context)
{
	tresult result = EditController::initialize (context);
	if (result != kResultOk) return result;
	
	//---Create Parameters------------
	parameters.addParameter (STR16 ("Bypass"), 0, 1, 0,
													 Vst::ParameterInfo::kCanAutomate | Vst::ParameterInfo::kIsBypass,
													 MidiBankerParams::kBypassId);

	parameters.addParameter (STR16 ("Parameter 1"), STR16 ("dB"), 0, .5,
													 Vst::ParameterInfo::kCanAutomate, MidiBankerParams::kParamVolId, 0,
													 STR16 ("Param1"));
	parameters.addParameter (STR16 ("Parameter 2"), STR16 ("On/Off"), 1, 1.,
													 Vst::ParameterInfo::kCanAutomate, MidiBankerParams::kParamOnId, 0,
													 STR16 ("Param2"));
	
	//---PrefetchMode parameter
	auto deviceList = new StringListParameter (STR16 ("Device"), MidiBankerParams::kStrList);
	parameters.addParameter(deviceList);

	deviceList->appendString (STR16 ("None"));
	registerDevicePatch(deviceList, "/MidiBankerPatch");
	// prefetchList->setNormalized (kIsYetPrefetchable / (kNumPrefetchableSupport - 1));

	return result;
}

//------------------------------------------------------------------------
tresult PLUGIN_API PlugController::setComponentState (IBStream* state)
{
	// we receive the current state of the component (processor part)
	// we read our parameters and bypass value...
	if (!state)
		return kResultFalse;

	IBStreamer streamer (state, kLittleEndian);

	float savedParam1 = 0.f;
	if (streamer.readFloat (savedParam1) == false)
		return kResultFalse;
	setParamNormalized (MidiBankerParams::kParamVolId, savedParam1);

	int8 savedParam2 = 0;
	if (streamer.readInt8 (savedParam2) == false)
		return kResultFalse;
	setParamNormalized (MidiBankerParams::kParamOnId, savedParam2);

	// read the bypass
	int32 bypassState;
	if (streamer.readInt32 (bypassState) == false)
		return kResultFalse;
	setParamNormalized (kBypassId, bypassState ? 1 : 0);

	return kResultOk;
}

//------------------------------------------------------------------------
} // namespace
} // namespace Steinberg
