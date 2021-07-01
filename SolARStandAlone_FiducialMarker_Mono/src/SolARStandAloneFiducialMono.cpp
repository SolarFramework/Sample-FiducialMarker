/**
 * @copyright Copyright (c) 2017 B-com http://www.b-com.com/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SolARStandAloneFiducialMono.h"

#include <boost/log/core.hpp>

#include <iostream>
#include <string>
#include <map>

// ADD COMPONENTS HEADERS HERE, e.g #include "SolarComponent.h"

#include "xpcf/xpcf.h"
#include "core/Log.h"
#include "api/input/devices/ICamera.h"
#include "api/input/files/IMarker2DSquaredBinary.h"
#include "api/display/IImageViewer.h"
#include "api/image/IImageFilter.h"
#include "api/image/IImageConvertor.h"
#include "api/features/IContoursExtractor.h"
#include "api/features/IContoursFilter.h"
#include "api/image/IPerspectiveController.h"
#include "api/features/IDescriptorsExtractorSBPattern.h"
#include "api/features/IDescriptorMatcher.h"
#include "api/features/ISBPatternReIndexer.h"
#include "api/features/ICornerRefinement.h"
#include "api/geom/IImage2WorldMapper.h"
#include "api/solver/pose/I3DTransformFinderFrom2D3D.h"
#include "api/display/I3DOverlay.h"
#include "api/display/I2DOverlay.h"


#define MIN_THRESHOLD -1
#define MAX_THRESHOLD 220
#define NB_THRESHOLD 8

using namespace std;
using namespace SolAR;
using namespace SolAR::api;
using namespace SolAR::datastructure;
namespace xpcf  = org::bcom::xpcf;

namespace SolAR::standalone
{

SolARStandAloneFiducialMono::Builder&
SolARStandAloneFiducialMono::Builder::selectPlaybackMode(const std::string& configFileName,
                                                         int timeoutInS)
{
    m_mode = Mode::playback;
    m_configFileName = configFileName;
    m_timeoutInS = timeoutInS;
    return *this;
}

SolARStandAloneFiducialMono::Builder&
SolARStandAloneFiducialMono::Builder::selectLiveMode(const std::string& configFileName)
{
    m_mode = Mode::live;
    m_configFileName = configFileName;
    return *this;
}

SolARStandAloneFiducialMono::Builder&
SolARStandAloneFiducialMono::Builder::disableDisplay()
{
    m_displayEnabled = false;
    return *this;
}

std::shared_ptr<SolARStandAloneFiducialMono> SolARStandAloneFiducialMono::Builder::build()
{
    auto result = std::shared_ptr<SolARStandAloneFiducialMono>(new SolARStandAloneFiducialMono());
    switch(m_mode)
    {
        case Mode::unset:
        {
            throw std::runtime_error("A mode must be selected");
        }
        case Mode::live:
        {
            if (m_configFileName.empty())
            {
                throw std::runtime_error("A configuration file must be provided");
            } 
            result->selectLiveMode(m_configFileName);
            break;
        }
        case Mode::playback:
        {
            if (m_configFileName.empty())
            {
                throw std::runtime_error("A configuration file must be provided");
            }
            result->selectPlaybackMode(m_configFileName, m_timeoutInS);
            break;
        }
        default:
        {
            throw std::runtime_error("Unknown mode selected");
        }

    }

    if (!m_displayEnabled)
    {
        if (m_mode == Mode::live)
        {
            LOG_WARNING ("Live mode with disabled display is discouraged, since you cannot exit the app properly");
        }
        result->disableDisplay();
    }

    return result;
}

void SolARStandAloneFiducialMono::selectPlaybackMode(const std::string& configFileName,
                                                     int timeoutInS)
{
    m_mode = Mode::playback;
    m_configFileName = configFileName;
    m_timeoutInS = timeoutInS;
}

void SolARStandAloneFiducialMono::selectLiveMode(const std::string& configFileName)
{
    m_mode = Mode::live;
    m_configFileName = configFileName;
}

void SolARStandAloneFiducialMono::disableDisplay()
{
    m_displayEnabled = false;
}

int SolARStandAloneFiducialMono::main_impl(){

    bool replayModeEnabled = m_mode == Mode::playback;

 #if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

    LOG_ADD_LOG_TO_CONSOLE();

    try {
    /* instantiate component manager*/
    /* this is needed in dynamic mode */
    SRef<xpcf::IComponentManager> xpcfComponentManager = xpcf::getComponentManagerInstance();

    // Required to run several tests with same mngr instance
    xpcfComponentManager->clear();

        if(xpcfComponentManager->load(m_configFileName.c_str())!=org::bcom::xpcf::_SUCCESS)
        {
            LOG_ERROR("Failed to load the configuration file '{}", m_configFileName);
            return -1;
        }

        // declare and create components
        LOG_INFO("Start creating components");

        auto camera =xpcfComponentManager->resolve<input::devices::ICamera>();
        auto binaryMarker =xpcfComponentManager->resolve<input::files::IMarker2DSquaredBinary>();

        auto imageViewer =xpcfComponentManager->resolve<display::IImageViewer>("original");
        auto imageViewerGrey =xpcfComponentManager->resolve<display::IImageViewer>("grey");
        auto imageViewerBinary =xpcfComponentManager->resolve<display::IImageViewer>("binary");
        auto imageViewerContours =xpcfComponentManager->resolve<display::IImageViewer>("contours");
        auto imageViewerFilteredContours =xpcfComponentManager->resolve<display::IImageViewer>("filteredContours");

        auto imageFilterBinary =xpcfComponentManager->resolve<image::IImageFilter>();
        auto imageConvertor =xpcfComponentManager->resolve<image::IImageConvertor>();
        auto contoursExtractor =xpcfComponentManager->resolve<features::IContoursExtractor>();
        auto contoursFilter =xpcfComponentManager->resolve<features::IContoursFilter>();
        auto perspectiveController =xpcfComponentManager->resolve<image::IPerspectiveController>();
        auto patternDescriptorExtractor =xpcfComponentManager->resolve<features::IDescriptorsExtractorSBPattern>();
		auto cornerRefinement = xpcfComponentManager->resolve<features::ICornerRefinement>();
        auto patternMatcher =xpcfComponentManager->resolve<features::IDescriptorMatcher>();
        auto patternReIndexer = xpcfComponentManager->resolve<features::ISBPatternReIndexer>();

        auto img2worldMapper = xpcfComponentManager->resolve<geom::IImage2WorldMapper>();
        auto PnP =xpcfComponentManager->resolve<solver::pose::I3DTransformFinderFrom2D3D>();
        auto overlay3D =xpcfComponentManager->resolve<display::I3DOverlay>();
        auto overlay2DContours =xpcfComponentManager->resolve<display::I2DOverlay>("contours");
        auto overlay2DCircles =xpcfComponentManager->resolve<display::I2DOverlay>("circles");


        SRef<Image> inputImage;
        SRef<Image> greyImage;
        SRef<Image> binaryImage;
        SRef<Image> contoursImage;
        SRef<Image> filteredContoursImage;

        std::vector<Contour2Df>              contours;
        std::vector<Contour2Df>              filtered_contours;
        std::vector<SRef<Image>>             patches;
        std::vector<Contour2Df>              recognizedContours;
        SRef<DescriptorBuffer>               recognizedPatternsDescriptors;
        SRef<DescriptorBuffer>               markerPatternDescriptor;
        std::vector<DescriptorMatch>         patternMatches;
        std::vector<Point2Df>                pattern2DPoints;
        std::vector<Point2Df>                img2DPoints;
        std::vector<Point3Df>                pattern3DPoints;
        Transform3Df                         pose;

        CamCalibration K;

        // components initialisation
        binaryMarker->loadMarker();
        patternDescriptorExtractor->extract(binaryMarker->getPattern(), markerPatternDescriptor);

        LOG_DEBUG ("Marker pattern:\n {}", binaryMarker->getPattern().getPatternMatrix())

        // Set the size of the box to display according to the marker size in world unit
        overlay3D->bindTo<xpcf::IConfigurable>()->getProperty("size")->setFloatingValue(binaryMarker->getSize().width,0);
        overlay3D->bindTo<xpcf::IConfigurable>()->getProperty("size")->setFloatingValue(binaryMarker->getSize().height,1);
        overlay3D->bindTo<xpcf::IConfigurable>()->getProperty("size")->setFloatingValue(binaryMarker->getSize().height/2.0f,2);


        int patternSize = binaryMarker->getPattern().getSize();

        patternDescriptorExtractor->bindTo<xpcf::IConfigurable>()->getProperty("patternSize")->setIntegerValue(patternSize);
        patternReIndexer->bindTo<xpcf::IConfigurable>()->getProperty("sbPatternSize")->setIntegerValue(patternSize);

        // NOT WORKING ! initialize image mapper with the reference image size and marker size
        img2worldMapper->bindTo<xpcf::IConfigurable>()->getProperty("digitalWidth")->setIntegerValue(patternSize);
        img2worldMapper->bindTo<xpcf::IConfigurable>()->getProperty("digitalHeight")->setIntegerValue(patternSize);
        img2worldMapper->bindTo<xpcf::IConfigurable>()->getProperty("worldWidth")->setFloatingValue(binaryMarker->getSize().width);
        img2worldMapper->bindTo<xpcf::IConfigurable>()->getProperty("worldHeight")->setFloatingValue(binaryMarker->getSize().height);

        PnP->setCameraParameters(camera->getIntrinsicsParameters(), camera->getDistortionParameters());
        overlay3D->setCameraParameters(camera->getIntrinsicsParameters(), camera->getDistortionParameters());

        if (camera->start() != FrameworkReturnCode::_SUCCESS) // Camera
        {
            LOG_ERROR ("Camera cannot start");
            return -1;
        }

        // to count the average number of processed frames per seconds
        int count=0;
        clock_t start,end;
        start= clock();
        clock_t timeoutInMs = m_timeoutInS * CLOCKS_PER_SEC;

        //cv::Mat img_temp;
        bool process = true;
        while (process){
            if(camera->getNextImage(inputImage)==SolAR::FrameworkReturnCode::_ERROR_)
                break;
            count++;

           // Convert Image from RGB to grey
           imageConvertor->convert(inputImage, greyImage, Image::ImageLayout::LAYOUT_GREY);

           bool marker_found = false;
           for (int num_threshold = 0; !marker_found && num_threshold < NB_THRESHOLD; num_threshold++)
           {
                // Compute the current Threshold valu for image binarization
                int threshold = MIN_THRESHOLD + (MAX_THRESHOLD - MIN_THRESHOLD)*((float)num_threshold/(float)(NB_THRESHOLD-1));

                // Convert Image from grey to black and white
                imageFilterBinary->bindTo<xpcf::IConfigurable>()->getProperty("min")->setIntegerValue(threshold);
                imageFilterBinary->bindTo<xpcf::IConfigurable>()->getProperty("max")->setIntegerValue(255);

               // Convert Image from grey to black and white
               imageFilterBinary->filter(greyImage,binaryImage);

               // Extract contours from binary image
               contoursExtractor->extract(binaryImage,contours);
    #ifndef NDEBUG
               contoursImage = binaryImage->copy();
               overlay2DContours->drawContours(contours, contoursImage);
    #endif
               // Filter 4 edges contours to find those candidate for marker contours
               contoursFilter->filter(contours, filtered_contours);

    #ifndef NDEBUG
               filteredContoursImage = binaryImage->copy();
               overlay2DContours->drawContours(filtered_contours, filteredContoursImage);
    #endif
               // Create one warpped and cropped image by contour
               perspectiveController->correct(binaryImage, filtered_contours, patches);

               // test if this last image is really a squared binary marker, and if it is the case, extract its descriptor
               if (patternDescriptorExtractor->extract(patches, filtered_contours, recognizedPatternsDescriptors, recognizedContours) != FrameworkReturnCode::_ERROR_)
               {
    #ifndef NDEBUG
                   LOG_DEBUG("Looking for the following descriptor:");
                   for (uint32_t i = 0; i < markerPatternDescriptor->getNbDescriptors()*markerPatternDescriptor->getDescriptorByteSize(); i++)
                   {

                       if (i%patternSize == 0)
                           std::cout<<"[";
                       if (i%patternSize != patternSize-1)
                           std::cout << (int)((unsigned char*)markerPatternDescriptor->data())[i] << ", ";
                       else
                            std::cout << (int)((unsigned char*)markerPatternDescriptor->data())[i] <<"]" << std::endl;
                   }
                   std::cout << std::endl;

                   std::cout << recognizedPatternsDescriptors->getNbDescriptors() <<" recognized Pattern Descriptors " << std::endl;
                   int desrciptorSize = recognizedPatternsDescriptors->getDescriptorByteSize();
                   for (uint32_t i = 0; i < recognizedPatternsDescriptors->getNbDescriptors()/4; i++)
                   {
                       for (int j = 0; j < patternSize; j++)
                       {
                           for (int k = 0; k < 4; k++)
                           {
                               std::cout<<"[";
                               for (int l = 0; l < patternSize; l++)
                               {
                                    std::cout << (int)((unsigned char*)recognizedPatternsDescriptors->data())[desrciptorSize*((i*4)+k) + j*patternSize + l];
                                    if (l != patternSize-1)
                                        std::cout << ", ";
                               }
                                std::cout <<"]";
                           }
                           std::cout << std::endl;
                       }
                       std::cout << std::endl << std::endl;
                   }

                   std::cout << recognizedContours.size() <<" Recognized Pattern contour " << std::endl;
                   for (int i = 0; i < recognizedContours.size()/4; i++)
                   {
                       for (int j = 0; j < recognizedContours[0].size(); j++)
                       {
                           for (int k = 0; k < 4; k++)
                           {
                               std::cout<<"[" << (recognizedContours[i*4+k])[j].getX() <<", "<< (recognizedContours[i*4+k])[j].getY() << "] ";
                           }
                           std::cout << std::endl;
                       }
                       std::cout << std::endl << std::endl;
                   }
                   std::cout << std::endl;
    #endif

                    // From extracted squared binary pattern, match the one corresponding to the squared binary marker
                    if (patternMatcher->match(markerPatternDescriptor, recognizedPatternsDescriptors, patternMatches) == features::IDescriptorMatcher::DESCRIPTORS_MATCHER_OK)
                    {
    #ifndef NDEBUG
                        std::cout << "Matches :" << std::endl;
                        for (int num_match = 0; num_match < patternMatches.size(); num_match++)
                            std::cout << "Match [" << patternMatches[num_match].getIndexInDescriptorA() << "," << patternMatches[num_match].getIndexInDescriptorB() << "], dist = " << patternMatches[num_match].getMatchingScore() << std::endl;
                        std::cout << std::endl << std::endl;
    #endif

                        // Reindex the pattern to create two vector of points, the first one corresponding to marker corner, the second one corresponding to the poitsn of the contour
                        patternReIndexer->reindex(recognizedContours, patternMatches, pattern2DPoints, img2DPoints);
    #ifndef NDEBUG
                        overlay2DCircles->drawCircles(img2DPoints, inputImage);
    #endif
                        // Compute the 3D position of each corner of the marker
                        img2worldMapper->map(pattern2DPoints, pattern3DPoints);
    #ifndef NDEBUG
                        std::cout << "3D Points position:" << std::endl;
                        for (int i = 0; i < pattern3DPoints.size(); i++)
                            LOG_DEBUG("{}", pattern3DPoints[i]);
    #endif
						// Refine corner locations
						cornerRefinement->refine(greyImage, img2DPoints);
                        // Compute the pose of the camera using a Perspective n Points algorithm using only the 4 corners of the marker
                        if (PnP->estimate(img2DPoints, pattern3DPoints, pose) == FrameworkReturnCode::_SUCCESS)
                        {
                            LOG_DEBUG("Camera pose : \n {}", pose.matrix());
                            // Display a 3D box over the marker
                            overlay3D->draw(pose,inputImage);
                            marker_found = true;
                        }
                    }
                }
           }

           // display images in viewers
           if ( ( m_displayEnabled &&
                   ((imageViewer->display(inputImage) == FrameworkReturnCode::_STOP)
    #ifndef NDEBUG
                   || (imageViewerGrey->display(greyImage) == FrameworkReturnCode::_STOP)
                   || (imageViewerBinary->display(binaryImage) == FrameworkReturnCode::_STOP)
                   || (imageViewerContours->display(contoursImage) == FrameworkReturnCode::_STOP)
                   || (imageViewerFilteredContours->display(filteredContoursImage) == FrameworkReturnCode::_STOP)
    #endif
                   )
                )
                || ( replayModeEnabled && timeoutInMs >= 0 && (clock() - start) > timeoutInMs )
             )
           {
               process = false;
           }
        }
        end= clock();
        double duration=double(end - start) / CLOCKS_PER_SEC;
        printf ("\n\nElasped time is %.2lf seconds.\n",duration );
        printf (  "Number of processed frames per second : %8.2f\n\n",count/duration );
    }
    catch (xpcf::Exception e)
    {
        LOG_ERROR ("The following exception has been catch : {}", e.what());
        return -1;
    }

    return 0;
}

} // namespace SolAR::standalone




