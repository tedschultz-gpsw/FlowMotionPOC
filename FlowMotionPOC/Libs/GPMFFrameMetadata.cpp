#include "GPMFFrameMetadata.h"

#include <iostream>
#include <fstream>
#include "GPMFAssetSourceVideo.h"
#include "GPMFTimeline.h"
#define USECSV 0

#include <fstream>
#include <math.h>

#define MAXW 128

using namespace std;

GPMFFrameMetadata::Status GPMFFrameMetadata::extract_cori_iori_grav(const char *filepath, std::vector<FrameData> &output)
{
    return extract_cori_iori_grav(filepath, output, nullptr, nullptr, nullptr);
}

GPMFFrameMetadata::Status GPMFFrameMetadata::extract_cori_iori_grav(const char *filepath, std::vector<FrameData> &output, bool *sphericalTimeLapse, bool *superView, unsigned char *eisType)
{
    GPMF_LIB_ERROR err = GPMF_LIB_FAIL;
    GPMFSampleptr sample = NULL;
    uint32_t coriSampleCount = 0;
    uint32_t ioriSampleCount = 0;
    uint32_t gravSampleCount = 0;

    GPMFTimeline gpmf;
    GPMFRational rational;

    gpmf.OpenURI(filepath, err);
    
    gpmf.Parse(err, GPMF_FILE_TYPE_ALL, std::vector<std::string> {"RATE", "PRJT", "CORI", "IORI", "GRAV", "MTYP", "VFOV", "EISA"});
    
    gpmf.GetSampleCount("CORI", &coriSampleCount);
    gpmf.GetSampleCount("IORI", &ioriSampleCount);
    gpmf.GetSampleCount("GRAV", &gravSampleCount);

    uint32_t frameCount;
    rational = gpmf.GetVideoFrameRateAndCount(err, &frameCount);
    if (err != GPMF_LIB_OK)
    {
        if (err != GPMF_LIB_ERROR_NOT_VALID_FOR_TYPE)
            return Status::FAILED;

        // support image
        rational.numerator = 1;
        rational.denominator = 1;
        frameCount = 1;
    }

    if(gravSampleCount == 0)
    {
        //No Grav
        return Status::FAILED;
        
    }
    
    uint32_t sampleCount = std::min({coriSampleCount, ioriSampleCount, gravSampleCount});
    
    //Get largest value
    uint32_t largestCount = std::max({coriSampleCount, ioriSampleCount, gravSampleCount});
    //Error case if tracks differ by more than 2
    if (largestCount - sampleCount > 2)
    {
        return Status::FAILED;
    }
    
    // The camera can miss up to 1gop of samples
    double fps = rational.numerator / (double)rational.denominator;
    uint32_t maxMissingSample = (uint32_t)fps;
    if (fps > maxMissingSample)
    {
        ++maxMissingSample;
    }

    uint32_t frameCountCompensated = frameCount < maxMissingSample ? 0 : frameCount - maxMissingSample;
    if (coriSampleCount < frameCountCompensated || ioriSampleCount < frameCountCompensated || gravSampleCount < frameCountCompensated)
    {
        return Status::FAILED;
    }
    
    sample = gpmf.GetSample("RATE", 0, err);
    if(!sample)
    {
        return Status::FAILED;
    }
    
    std::string rate((char *)sample->sampleBuffer);
    //2X", "5X", "10X", "15X", "30X
    std::string x = "X";
    std::string rateString;
    int rateValue = -1;
    
    if (rate.find(x) != std::string::npos) {
        std::cout << "found X in string!" << '\n';
        rateString = rate.substr(0, rate.size()-1);
        rateValue = std::stoi( rateString );
    }
    
    sample = gpmf.GetSample("MTYP", 0, err);
    
    bool isSphericalTimeLapse = false;
    
    if(sample)
    {
        int mTypeVal = ((int *)sample->sampleBuffer)[0];
        if (mTypeVal == 7) //SPHERICAL TIME LAPSE
        {
            isSphericalTimeLapse = true;
        }
    }
    
    if (sphericalTimeLapse != nullptr)
    {
        *sphericalTimeLapse = isSphericalTimeLapse;
    }
    
    sample = gpmf.GetSample("VFOV", 0, err);
    
    bool isSuperView = false;
    
    if(sample)
    {
        std::string vFovType((char *)sample->sampleBuffer);
        
        if (vFovType == "S")
        {
            isSuperView = true;
        }
    }
    
    if (superView != nullptr)
    {
        *superView = isSuperView;
    }
    
    sample = gpmf.GetSample("EISA", 0, err);
    
    unsigned char eisTypeFound = 0;
        
    if(sample)
    {
        std::string eisActiveType((char *)sample->sampleBuffer);
        
        /*
         "N/A" - User disable stabilization
         "HS EIS" - Hypersmooth STD is active
         "HS High" - Hypersmooth High is active
         "HS Boost" - Hypersmooth Boost is active
         "HS HLevel" - Horizon Leveled
         */
        
        if (eisActiveType.find("HS") != std::string::npos)
        {
            if (eisActiveType.find("EIS") != std::string::npos)
            {
                eisTypeFound = 1;
            }
            else if (eisActiveType.find("High") != std::string::npos)
            {
                eisTypeFound = 2;
            }
            else if (eisActiveType.find("Boost") != std::string::npos)
            {
                eisTypeFound = 3;
            }
            else //HLevel
            {
                eisTypeFound = 4;
            }
        }
    }
    
    if (eisType != nullptr)
    {
        *eisType = eisTypeFound;
    }
        
    sample = gpmf.GetSample("PRJT", 0, err);
    
    if(!sample)
    {
        return Status::FAILED;
    }
    std::string projectionType((char *)sample->sampleBuffer);

    bool allCoriAreZero = true;
    bool allIORIAreZero = true;
    bool allGravAreZero = true;
    
    // assume equal counts of cori, iori and grav
    for(uint32_t i = 0; i < sampleCount; i++)
    {
        // CORI
        if((sample = gpmf.GetSample("CORI", i, err)))
        {
            FrameData frameData = {0};
            frameData.timestamp = (int64_t)(sample->sampleTime * 1000000000);
            
            if(rateValue > -1)
            {
                frameData.frameRate = (rational.numerator / rational.denominator) / (float)rateValue;
            }
            else
            {
                frameData.frameRate = -1;
            }
            
            float *ptr = (float *)sample->sampleBuffer;
            
            frameData.cori.setXYZW(ptr[1], ptr[2], ptr[3], ptr[0]);
            output.push_back(frameData);
            
            if (allCoriAreZero && (ptr[0] != 0 || ptr[1] != 0 || ptr[2] != 0 || ptr[3] != 0))
            {
                allCoriAreZero = false;
            }
        }
    }
    
    if (allCoriAreZero)
    {
        return Status::FAILED;
    }
    
    for(uint32_t i = 0; i < sampleCount; i++)
    {
        // IORI
        if((sample = gpmf.GetSample("IORI", i, err)))
        {
            auto& frameData = output[i];
            std::string filePathString = std::string(filepath);
            if (projectionType == "FSFB" || filePathString.find("GPMF.mp4") != std::string::npos)
            {
                //LRV IORI should not be used for fisheyes!
                frameData.iori.setXYZW(0, 0, 0, 1);
                
                allIORIAreZero = false;
            }
            else
            {
                float *ptr = (float *)sample->sampleBuffer;

                frameData.iori.setXYZW(ptr[1], ptr[2], ptr[3], ptr[0]);
                
                if (allIORIAreZero && (ptr[0] != 0 || ptr[1] != 0 || ptr[2] != 0 || ptr[3] != 0))
                {
                    allIORIAreZero = false;
                }
            }

        }
    }
    
    if (allIORIAreZero)
    {
        return Status::FAILED;
    }
    
    for(uint32_t i = 0; i < sampleCount; i++)
    {
        // GRAV
        if((sample = gpmf.GetSample("GRAV", i, err)))
        {
            float *ptr = (float *)sample->sampleBuffer;
            auto& frameData = output[i];
            frameData.grav.setXYZ(ptr[0], ptr[1], ptr[2]);
            
            if (allGravAreZero && (ptr[0] != 0 || ptr[1] != 0 || ptr[2] != 0))
            {
                allGravAreZero = false;
            }
        }
    }
    
    if (allGravAreZero)
    {
        return Status::METADATA_GRAV_INVALID;
    }
    
    if(output.empty())
    {
        return Status::FAILED;
    }
    
    return Status::OK;
}


GPMFFrameMetadata::Status GPMFFrameMetadata::extract_isog_shut(const char *frontFilepath, const char *backFilepath, std::vector<FrameIQData> &output)
{
    GPMF_LIB_ERROR err = GPMF_LIB_FAIL;
    GPMFSampleptr sample = NULL;
    uint32_t shutSampleCount = 0;
    uint32_t isogSampleCount = 0;
    
    GPMFTimeline gpmfFront;
    GPMFTimeline gpmfBack;
    
    gpmfFront.OpenURI(frontFilepath, err);
    gpmfBack.OpenURI(backFilepath, err);
    
    gpmfFront.Parse(err, GPMF_FILE_TYPE_ALL, std::vector<std::string> {"SHUT", "ISOG"});
    gpmfBack.Parse(err, GPMF_FILE_TYPE_ALL, std::vector<std::string> {"SHUT", "ISOG"});
    
    gpmfFront.GetSampleCount("SHUT", &shutSampleCount);
    gpmfFront.GetSampleCount("ISOG", &isogSampleCount);
    
    if (shutSampleCount != isogSampleCount)
    {
        return Status::FAILED;
    }
    
    // SHUT
    // assume equal counts of shut, and isog
    for(uint32_t i = 0; i < shutSampleCount; i++)
    {
        FrameIQData frameIQData = {0};
        
        // SHUT
        if((sample = gpmfFront.GetSample("SHUT", i, err)))
        {
            frameIQData.timestamp = (int64_t)(sample->sampleTime * 1000000000);
            
            float *ptr = (float *)sample->sampleBuffer;
            frameIQData.frontShut = ptr[0];
        }
        
        if((sample = gpmfBack.GetSample("SHUT", i, err)))
        {
            float *ptr = (float *)sample->sampleBuffer;
            frameIQData.backShut = ptr[0];
        }
        
        output.push_back(frameIQData);
    }
    
    // ISOG
    // assume equal counts of shut, and isog
    for(uint32_t i = 0; i < shutSampleCount; i++)
    {
        auto& frameIQData = output[i];
        if((sample = gpmfFront.GetSample("ISOG", i, err)))
        {
            float *ptr = (float *)sample->sampleBuffer;
            frameIQData.frontIsog = ptr[0];
        }
        
        if((sample = gpmfBack.GetSample("ISOG", i, err)))
        {
            float *ptr = (float *)sample->sampleBuffer;
            frameIQData.backIsog = ptr[0];
        }
    }
    
    if(output.empty())
    {
        return Status::FAILED;
    }
    
    return Status::OK;
}
