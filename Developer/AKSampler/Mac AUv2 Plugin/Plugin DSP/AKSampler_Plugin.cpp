#include "AKSampler_Plugin.h"
#include "AKSampler_Params.h"
#include "AUMidiDefs.h"
#include <cstring>
#include <ctype.h>

// OSErr definitions taken from deprecated CarbonCore/MacErrors.h
// Somewhere there's a newer header file I should be using
enum {
    fnfErr                        = -43,  /*File not found*/
};

AUDIOCOMPONENT_ENTRY(AUMusicDeviceFactory, AKSampler_Plugin)

static const CFStringRef paramName[] =
{
    CFSTR("Master Volume"),
    CFSTR("Pitch Offset"),
    CFSTR("Vibrato Depth"),
    CFSTR("Filter Enable"),
    CFSTR("Filter Cutoff"),
    CFSTR("Filter Resonance"),
    
    CFSTR("Amp EG Attack"),
    CFSTR("Amp EG Decay"),
    CFSTR("Amp EG Sustain"),
    CFSTR("Amp EG Release"),

    CFSTR("Flt EG Attack"),
    CFSTR("Flt EG Decay"),
    CFSTR("Flt EG Sustain"),
    CFSTR("Flt EG Release"),
};


#define NOTE_HZ(midiNoteNumber) ( 440.0f * pow(2.0f, ((midiNoteNumber) - 69.0f)/12.0f) )


AKSampler_Plugin::AKSampler_Plugin(AudioUnit inComponentInstance)
	: AUInstrumentBase(inComponentInstance, 0, 1)    // 0 inputs, 1 output
    , AudioKitCore::Sampler()
{
    presetPath = nil;
	CreateElements();
	Globals()->UseIndexedParameters(kNumberOfParams);
}

AKSampler_Plugin::~AKSampler_Plugin()
{
}

OSStatus AKSampler_Plugin::Initialize()
{
	AUInstrumentBase::Initialize();
    AudioKitCore::Sampler::init(GetOutput(0)->GetStreamFormat().mSampleRate);
    printf("AudioKitCore::AKSampler_Plugin::Initialize %f samples/sec\n", GetOutput(0)->GetStreamFormat().mSampleRate);
    
    masterVolume = 1.0f;
    pitchOffset = 0.0f;
    vibratoDepth = 0.0f;
    
    cutoffMultiple = 1000.0f;
    filterEnable = false;
    
    ampEGParams.setAttackTimeSeconds(0.01f);
    ampEGParams.setDecayTimeSeconds(0.1f);
    ampEGParams.sustainFraction = 0.8f;
    ampEGParams.setReleaseTimeSeconds(0.5f);
    
    //loadDemoSamples();

    return noErr;
}

void AKSampler_Plugin::Cleanup()
{
    AudioKitCore::Sampler::deinit();
    printf("AudioKitCore::AKSampler_Plugin::Cleanup\n");
}

void AKSampler_Plugin::loadDemoSamples()
{
    // Example showing how to load a group of samples when you don't have a .sfz metadata file.
    
    // Download http://getdunne.com/download/TX_LoTine81z.zip
    // These are Wavpack-compressed versions of the similarly-named samples in ROMPlayer.
    // Put folder wherever you wish (e.g. inside a "Compressed Sounds" folder on your Mac desktop
    // and edit paths below accordingly
    
    char pathBuffer[200];
    const char* baseDir = "/Users/shane/Desktop/Compressed Sounds/";
    const char* samplePrefix = "TX LoTine81z/TX LoTine81z_ms";
    AKSampleFileDescriptor sfd;
    sfd.path = pathBuffer;
    sfd.sd.bLoop = false;    // set true to test looping with fractional endpoints
    sfd.sd.fStart = 0.0;
    sfd.sd.fLoopStart = 0.2f;
    sfd.sd.fLoopEnd = 0.3f;
    sfd.sd.fEnd = 0.0f;
    
    sfd.sd.noteNumber = 48;
    sfd.sd.noteHz = NOTE_HZ(sfd.sd.noteNumber);
    sfd.sd.min_note = 0; sfd.sd.max_note = 51;
    sfd.sd.min_vel = 0; sfd.sd.max_vel = 43;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 2, sfd.sd.noteNumber, "c2");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 44; sfd.sd.max_vel = 86;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 1, sfd.sd.noteNumber, "c2");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 87; sfd.sd.max_vel = 127;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 0, sfd.sd.noteNumber, "c2");
    loadCompressedSampleFile(sfd);
    
    sfd.sd.noteNumber = 54;
    sfd.sd.noteHz = NOTE_HZ(sfd.sd.noteNumber);
    sfd.sd.min_note = 52; sfd.sd.max_note = 57;
    sfd.sd.min_vel = 0; sfd.sd.max_vel = 43;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 2, sfd.sd.noteNumber, "f#2");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 44; sfd.sd.max_vel = 86;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 1, sfd.sd.noteNumber, "f#2");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 87; sfd.sd.max_vel = 127;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 0, sfd.sd.noteNumber, "f#2");
    loadCompressedSampleFile(sfd);
    
    sfd.sd.noteNumber = 60;
    sfd.sd.noteHz = NOTE_HZ(sfd.sd.noteNumber);
    sfd.sd.min_note = 58; sfd.sd.max_note = 63;
    sfd.sd.min_vel = 0; sfd.sd.max_vel = 43;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 2, sfd.sd.noteNumber, "c3");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 44; sfd.sd.max_vel = 86;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 1, sfd.sd.noteNumber, "c3");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 87; sfd.sd.max_vel = 127;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 0, sfd.sd.noteNumber, "c3");
    loadCompressedSampleFile(sfd);
    
    sfd.sd.noteNumber = 66;
    sfd.sd.noteHz = NOTE_HZ(sfd.sd.noteNumber);
    sfd.sd.min_note = 64; sfd.sd.max_note = 69;
    sfd.sd.min_vel = 0; sfd.sd.max_vel = 43;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 2, sfd.sd.noteNumber, "f#3");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 44; sfd.sd.max_vel = 86;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 1, sfd.sd.noteNumber, "f#3");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 87; sfd.sd.max_vel = 127;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 0, sfd.sd.noteNumber, "f#3");
    loadCompressedSampleFile(sfd);
    
    sfd.sd.noteNumber = 72;
    sfd.sd.noteHz = NOTE_HZ(sfd.sd.noteNumber);
    sfd.sd.min_note = 70; sfd.sd.max_note = 75;
    sfd.sd.min_vel = 0; sfd.sd.max_vel = 43;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 2, sfd.sd.noteNumber, "c4");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 44; sfd.sd.max_vel = 86;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 1, sfd.sd.noteNumber, "c4");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 87; sfd.sd.max_vel = 127;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 0, sfd.sd.noteNumber, "c4");
    loadCompressedSampleFile(sfd);
    
    sfd.sd.noteNumber = 78;
    sfd.sd.noteHz = NOTE_HZ(sfd.sd.noteNumber);
    sfd.sd.min_note = 76; sfd.sd.max_note = 81;
    sfd.sd.min_vel = 0; sfd.sd.max_vel = 43;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 2, sfd.sd.noteNumber, "f#4");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 44; sfd.sd.max_vel = 86;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 1, sfd.sd.noteNumber, "f#4");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 87; sfd.sd.max_vel = 127;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 0, sfd.sd.noteNumber, "f#4");
    loadCompressedSampleFile(sfd);
    
    sfd.sd.noteNumber = 84;
    sfd.sd.noteHz = NOTE_HZ(sfd.sd.noteNumber);
    sfd.sd.min_note = 82; sfd.sd.max_note = 127;
    sfd.sd.min_vel = 0; sfd.sd.max_vel = 43;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 2, sfd.sd.noteNumber, "c5");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 44; sfd.sd.max_vel = 86;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 1, sfd.sd.noteNumber, "c5");
    loadCompressedSampleFile(sfd);
    sfd.sd.min_vel = 87; sfd.sd.max_vel = 127;
    sprintf(pathBuffer, "%s%s%d_%03d_%s.wv", baseDir, samplePrefix, 0, sfd.sd.noteNumber, "c5");
    loadCompressedSampleFile(sfd);
    
    buildKeyMap();
}

static bool hasPrefix(char* string, const char* prefix)
{
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

OSStatus AKSampler_Plugin::loadPreset()
{
    // Nicer way to load presets using .sfz metadata files. See bottom of AKSampler_Params.h
    // for instructions to download and set up these presets.
    const char *presetName = CFStringGetCStringPtr(presetPath, kCFStringEncodingMacRoman);
    printf("loadPreset: %s...", presetName);
    
    this->deinit();     // unload any samples already present
    
    char buf[1000];
    sprintf(buf, "%s/%s.sfz", PRESETS_DIR_PATH, presetName);
    
    FILE* pfile = fopen(buf, "r");
    if (!pfile) return fnfErr;
    
    int lokey, hikey, pitch, lovel, hivel;
    bool bLoop;
    float fLoopStart, fLoopEnd;
    char sampleFileName[100];
    char *p, *pp;

    while (fgets(buf, sizeof(buf), pfile))
    {
        p = buf;
        while (*p != 0 && isspace(*p)) p++;
        
        pp = strrchr(p, '\n');
        if (pp) *pp = 0;
        
        if (hasPrefix(p, "<group>"))
        {
            p += 7;
            lokey = 0;
            hikey = 127;
            pitch = 60;
            
            pp = strstr(p, "lokey");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) lokey = atoi(pp);
            }
            
            pp= strstr(p, "hikey");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) hikey = atoi(pp);
            }
            
            pp= strstr(p, "pitch_keycenter");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) pitch = atoi(pp);
            }
        }
        else if (hasPrefix(p, "<region>"))
        {
            p += 8;
            lovel = 0;
            hivel = 127;
            sampleFileName[0] = 0;
            bLoop = false;
            fLoopStart = 0.0f;
            fLoopEnd = 0.0f;
            
            pp = strstr(p, "lovel");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) lovel = atoi(pp);
            }
            
            pp = strstr(p, "hivel");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) hivel = atoi(pp);
            }
            
            pp = strstr(p, "loop_mode");
            if (pp)
            {
                bLoop = true;
            }

            pp = strstr(p, "loop_start");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) fLoopStart = atof(pp);
            }
            
            pp = strstr(p, "loop_end");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                if (pp) fLoopEnd = atof(pp);
            }
            
            pp = strstr(p, "sample");
            if (pp)
            {
                pp = strchr(pp, '=');
                if (pp) pp++;
                while (*pp != 0 && isspace(*pp)) pp++;
                char* pq = sampleFileName;
                while (*pp != '.') *pq++ = *pp++;
                strcpy(pq, ".wv");
            }
            
            sprintf(buf, "%s/%s", PRESETS_DIR_PATH, sampleFileName);

            AKSampleFileDescriptor sfd;
            sfd.path = buf;
            sfd.sd.bLoop = bLoop;
            sfd.sd.fStart = 0.0;
            sfd.sd.fLoopStart = fLoopStart;
            sfd.sd.fLoopEnd = fLoopEnd;
            sfd.sd.fEnd = 0.0f;
            sfd.sd.noteNumber = pitch;
            sfd.sd.noteHz = NOTE_HZ(sfd.sd.noteNumber);
            sfd.sd.min_note = lokey;
            sfd.sd.max_note = hikey;
            sfd.sd.min_vel = lovel;
            sfd.sd.max_vel = hivel;
            loadCompressedSampleFile(sfd);
        }
    }
    fclose(pfile);
    
    buildKeyMap();
    printf("done\n");
    return noErr;
}

OSStatus AKSampler_Plugin::GetPropertyInfo( AudioUnitPropertyID         inPropertyID,
                                            AudioUnitScope              inScope,
                                            AudioUnitElement            inElement,
                                            UInt32&                     outDataSize,
                                            Boolean&                    outWritable )
{
    if (inScope == kAudioUnitScope_Global)
    {
        switch (inPropertyID)
        {
            case kAudioUnitProperty_CocoaUI:
                outWritable = false;
                outDataSize = sizeof (AudioUnitCocoaViewInfo);
                return noErr;

            case kPresetNameProperty:
                outWritable = true;
                outDataSize = sizeof(CFStringRef);
                return noErr;
        }
    }
    
    return AUInstrumentBase::GetPropertyInfo (inPropertyID, inScope, inElement, outDataSize, outWritable);
}

OSStatus AKSampler_Plugin::GetProperty( AudioUnitPropertyID         inPropertyID,
                                        AudioUnitScope              inScope,
                                        AudioUnitElement            inElement,
                                        void*                       outData)
{
    if (inScope == kAudioUnitScope_Global) {
        switch (inPropertyID) {
            case kAudioUnitProperty_CocoaUI:
            {
                // Look for a resource in the main bundle by name and type.
                CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("io.audiokit.AKSampler") );
                
                if (bundle == NULL) {
                    printf("Could not find bundle specified for GUI resources\n");
                    return fnfErr;
                }
                
                CFURLRef bundleURL = CFBundleCopyResourceURL( bundle,
                                                             CFSTR("AKSamplerUI"),
                                                             CFSTR("bundle"),
                                                             NULL);
                
                if (bundleURL == NULL) {
                    printf("Could not create resource URL for GUI\n");
                    return fnfErr;
                }
                
                CFStringRef className = CFSTR("AKSampler_ViewFactory");
                AudioUnitCocoaViewInfo cocoaInfo = { bundleURL, { className } };
                *((AudioUnitCocoaViewInfo *)outData) = cocoaInfo;
                
                return noErr;
            }

            case kPresetNameProperty:
                outData = (void*)presetPath;
                return noErr;
        }
    }

    return AUInstrumentBase::GetProperty (inPropertyID, inScope, inElement, outData);
}

OSStatus AKSampler_Plugin::SetProperty(         AudioUnitPropertyID         inPropertyID,
                                                AudioUnitScope              inScope,
                                                AudioUnitElement            inElement,
                                                const void *                inData,
                                                UInt32                      inDataSize)
{
    if (inScope == kAudioUnitScope_Global && inPropertyID == kPresetNameProperty)
    {
        presetPath = (CFStringRef)inData;
        return loadPreset();
    }
    
    // Default implementation for all non-custom properties
    return AUInstrumentBase::SetProperty(inPropertyID, inScope, inElement, inData, inDataSize);
}


OSStatus AKSampler_Plugin::GetParameterInfo(    AudioUnitScope          inScope,
                                                AudioUnitParameterID    inParameterID,
                                                AudioUnitParameterInfo& outParameterInfo)
{
	if (inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;

    outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable | kAudioUnitParameterFlag_IsReadable;

    switch (inParameterID) {
        case kMasterVolumeFraction:
            outParameterInfo.flags += SetAudioUnitParameterDisplayType (0, kAudioUnitParameterFlag_DisplaySquareRoot);
            AUBase::FillInParameterName (outParameterInfo, paramName[kMasterVolumeFraction], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 1.0;
            outParameterInfo.defaultValue = 1.0;
            break;
            
        case kPitchOffsetSemitones:
            AUBase::FillInParameterName (outParameterInfo, paramName[kPitchOffsetSemitones], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_RelativeSemiTones;
            outParameterInfo.minValue = -24.0;
            outParameterInfo.maxValue = 24.0;
            outParameterInfo.defaultValue = 0.0;
            break;
    
        case kVibratoDepthSemitones:
            AUBase::FillInParameterName (outParameterInfo, paramName[kVibratoDepthSemitones], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_RelativeSemiTones;
            outParameterInfo.minValue = 0.0;
            outParameterInfo.maxValue = 24.0;
            outParameterInfo.defaultValue = 0.0;
            break;
            
        case kFilterEnable:
            AUBase::FillInParameterName (outParameterInfo, paramName[kFilterEnable], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Boolean;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 1.0;
            outParameterInfo.defaultValue = 0.0;
            break;
            
        case kFilterCutoffHarmonic:
            outParameterInfo.flags += SetAudioUnitParameterDisplayType (0, kAudioUnitParameterFlag_DisplayLogarithmic);
            AUBase::FillInParameterName (outParameterInfo, paramName[kFilterCutoffHarmonic], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
            outParameterInfo.minValue = 1.0;
            outParameterInfo.maxValue = 1000.0;
            outParameterInfo.defaultValue = 1000.0;
            break;
    
        case kFilterResonanceDb:
            AUBase::FillInParameterName (outParameterInfo, paramName[kFilterResonanceDb], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Decibels;
            outParameterInfo.minValue = 0.0;
            outParameterInfo.maxValue = 10.0;
            outParameterInfo.defaultValue = 0.0;
            break;
            
        case kAmpEgAttackTimeSeconds:
            AUBase::FillInParameterName (outParameterInfo, paramName[kAmpEgAttackTimeSeconds], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 10.0;
            outParameterInfo.defaultValue = 0.0;
            break;
    
        case kAmpEgDecayTimeSeconds:
            AUBase::FillInParameterName (outParameterInfo, paramName[kAmpEgDecayTimeSeconds], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 10.0;
            outParameterInfo.defaultValue = 0.0;
            break;
            
        case kAmpEgSustainFraction:
            AUBase::FillInParameterName (outParameterInfo, paramName[kAmpEgSustainFraction], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 1.0;
            outParameterInfo.defaultValue = 1.0;
            break;
    
        case kAmpEgReleaseTimeSeconds:
            AUBase::FillInParameterName (outParameterInfo, paramName[kAmpEgReleaseTimeSeconds], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 10.0;
            outParameterInfo.defaultValue = 0.0;
            break;
    
        case kFilterEgAttackTimeSeconds:
            AUBase::FillInParameterName (outParameterInfo, paramName[kFilterEgAttackTimeSeconds], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 10.0;
            outParameterInfo.defaultValue = 0.0;
            break;
    
        case kFilterEgDecayTimeSeconds:
            AUBase::FillInParameterName (outParameterInfo, paramName[kFilterEgDecayTimeSeconds], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 10.0;
            outParameterInfo.defaultValue = 0.0;
            break;
    
        case kFilterEgSustainFraction:
            outParameterInfo.flags += SetAudioUnitParameterDisplayType (0, kAudioUnitParameterFlag_DisplayExponential);
            AUBase::FillInParameterName (outParameterInfo, paramName[kFilterEgSustainFraction], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_LinearGain;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 1.0;
            outParameterInfo.defaultValue = 1.0;
            break;
            
        case kFilterEgReleaseTimeSeconds:
            AUBase::FillInParameterName (outParameterInfo, paramName[kFilterEgReleaseTimeSeconds], false);
            outParameterInfo.unit = kAudioUnitParameterUnit_Seconds;
            outParameterInfo.minValue = 0;
            outParameterInfo.maxValue = 10.0;
            outParameterInfo.defaultValue = 0.0;
            break;
            
        default:
            return kAudioUnitErr_InvalidParameter;
    }
    
	return noErr;
}

OSStatus AKSampler_Plugin::GetParameter(    AudioUnitParameterID        inParameterID,
                                            AudioUnitScope              inScope,
                                            AudioUnitElement            inElement,
                                            AudioUnitParameterValue &   outValue)
{
    if (inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;
    
    switch (inParameterID)
    {
        case kMasterVolumeFraction:
            outValue = masterVolume;
            break;
            
        case kPitchOffsetSemitones:
            outValue = pitchOffset;
            break;
            
        case kVibratoDepthSemitones:
            outValue = vibratoDepth;
            break;
            
        case kFilterEnable:
            outValue = filterEnable ? 1.0f : 0.0f;
            break;
            
        case kFilterCutoffHarmonic:
            outValue = cutoffMultiple;
            break;
            
        case kFilterResonanceDb:
            outValue = resonanceDb;
            break;
            
        case kAmpEgAttackTimeSeconds:
            outValue = ampEGParams.getAttackTimeSeconds();
            break;
            
        case kAmpEgDecayTimeSeconds:
            outValue = ampEGParams.getDecayTimeSeconds();
            break;
            
        case kAmpEgSustainFraction:
            outValue = ampEGParams.sustainFraction;
            break;
            
        case kAmpEgReleaseTimeSeconds:
            outValue = ampEGParams.getReleaseTimeSeconds();
            break;
            
        case kFilterEgAttackTimeSeconds:
            outValue = filterEGParams.getAttackTimeSeconds();
            break;
            
        case kFilterEgDecayTimeSeconds:
            outValue = filterEGParams.getDecayTimeSeconds();
            break;
            
        case kFilterEgSustainFraction:
            outValue = filterEGParams.sustainFraction;
            break;
            
        case kFilterEgReleaseTimeSeconds:
            outValue = filterEGParams.getReleaseTimeSeconds();
            break;
            
        default:
            return kAudioUnitErr_InvalidParameter;
    }
    
    return noErr;
}

OSStatus AKSampler_Plugin::SetParameter(    AudioUnitParameterID        inParameterID,
                                            AudioUnitScope              inScope,
                                            AudioUnitElement            inElement,
                                            AudioUnitParameterValue     inValue,
                                            UInt32                      inBufferOffsetInFrames)
{
    if (inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;
    
    switch (inParameterID)
    {
        case kMasterVolumeFraction:
            masterVolume = inValue;
            break;
            
        case kPitchOffsetSemitones:
            pitchOffset = inValue;
            break;
            
        case kVibratoDepthSemitones:
            vibratoDepth = inValue;
            break;
            
        case kFilterEnable:
            filterEnable = (inValue > 0.5f);
            break;
            
        case kFilterCutoffHarmonic:
            cutoffMultiple = inValue;
            break;
            
        case kFilterResonanceDb:
            resonanceDb = inValue;
            break;
            
        case kAmpEgAttackTimeSeconds:
            ampEGParams.setAttackTimeSeconds(inValue);
            break;

        case kAmpEgDecayTimeSeconds:
            ampEGParams.setDecayTimeSeconds(inValue);
            break;
            
        case kAmpEgSustainFraction:
            ampEGParams.sustainFraction = inValue;
            break;
            
        case kAmpEgReleaseTimeSeconds:
            ampEGParams.setReleaseTimeSeconds(inValue);
            break;
            
        case kFilterEgAttackTimeSeconds:
            filterEGParams.setAttackTimeSeconds(inValue);
            break;
            
        case kFilterEgDecayTimeSeconds:
            filterEGParams.setDecayTimeSeconds(inValue);
            break;
            
        case kFilterEgSustainFraction:
            filterEGParams.sustainFraction = inValue;
            break;
            
        case kFilterEgReleaseTimeSeconds:
            filterEGParams.setReleaseTimeSeconds(inValue);
            break;

        default:
            return kAudioUnitErr_InvalidParameter;
    }
    
    return noErr;
}

OSStatus AKSampler_Plugin::Render(AudioUnitRenderActionFlags &ioActionFlags, const AudioTimeStamp &inTimeStamp, UInt32 nFrames)
{
    AUOutputElement* outputBus = GetOutput(0);
    outputBus->PrepareBuffer(nFrames); // prepare the output buffer list
    
    AudioBufferList& outputBufList = outputBus->GetBufferList();
    AUBufferList::ZeroBuffer(outputBufList);
    
    float* outBuffers[2];
    outBuffers[0] = (float*)(outputBufList.mBuffers[0].mData);
    outBuffers[1] = (float*)(outputBufList.mBuffers[1].mData);
    
    // process in chunks of maximum length CHUNKSIZE
    for (int frameIndex = 0; frameIndex < nFrames; frameIndex += CHUNKSIZE) {
        int chunkSize = nFrames - frameIndex;
        if (chunkSize > CHUNKSIZE) chunkSize = CHUNKSIZE;
        
        // Any ramping parameters would be updated here...
        
        unsigned channelCount = outputBufList.mNumberBuffers;
        AudioKitCore::Sampler::Render(channelCount, chunkSize, outBuffers);
        
        outBuffers[0] += CHUNKSIZE;
        outBuffers[1] += CHUNKSIZE;
    }
    
    return noErr;
}

OSStatus AKSampler_Plugin::HandleNoteOn(UInt8 inChannel, UInt8 inNoteNumber, UInt8 inVelocity, UInt32 inStartFrame)
{
    //printf("note on: ch%d nn%d vel%d\n", inChannel, inNoteNumber, inVelocity);
    playNote(inNoteNumber, inVelocity, 440.0f * pow(2.0f, (inNoteNumber - 69.0f)/12.0f));
    return noErr;
}

OSStatus AKSampler_Plugin::HandleNoteOff(UInt8 inChannel, UInt8 inNoteNumber, UInt8 inVelocity, UInt32 inStartFrame)
{
    //printf("note off: ch%d nn%d vel%d\n", inChannel, inNoteNumber, inVelocity);
    stopNote(inNoteNumber, false);
    return noErr;
}

OSStatus AKSampler_Plugin::HandleControlChange(UInt8 inChannel, UInt8 inController, UInt8 inValue, UInt32 inStartFrame)
{
    if (inController == kMidiController_Sustain)
    {
        sustainPedal(inValue != 0);
    }
    else if (inController == kMidiController_ModWheel)
    {
        vibratoDepth = 0.6f * inValue / 127.0f;
    }
    return noErr;
}

OSStatus AKSampler_Plugin::HandlePitchWheel(UInt8 inChannel, UInt8 inPitch1, UInt8 inPitch2, UInt32 inStartFrame)
{
    int intValue = ((inPitch2 << 7) | inPitch1) - (64 << 7);
    float floatValue = intValue / 8192.0f;
    pitchOffset = 2.0f * floatValue;
    return noErr;
}