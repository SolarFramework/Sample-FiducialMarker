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

#include <boost/log/core.hpp>

#include <iostream>
#include <string>
#include <map>

// ADD COMPONENTS HEADERS HERE, e.g #include "SolarComponent.h"

#include "SolARModuleOpencv_traits.h"

#include "SolARModuleTools_traits.h"

#include "xpcf/xpcf.h"


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
#include "api/geom/IImage2WorldMapper.h"
#include "api/solver/pose/I3DTransformFinder.h"
#include "api/display/I3DOverlay.h"
#include "api/display/I2DOverlay.h"



using namespace std;
using namespace SolAR;
using namespace SolAR::MODULES::OPENCV;
using namespace SolAR::MODULES::TOOLS;
using namespace SolAR::api;
using namespace SolAR::datastructure;
namespace xpcf  = org::bcom::xpcf;

int marker_run(int argc,char** argv){

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

    LOG_ADD_LOG_TO_CONSOLE();

    /* instantiate component manager*/
    /* this is needed in dynamic mode */
    SRef<xpcf::IComponentManager> xpcfComponentManager = xpcf::getComponentManagerInstance();

    if(xpcfComponentManager->load(argv[1])!=org::bcom::xpcf::_SUCCESS)
    {
        LOG_ERROR("Failed to load the configuration file {}", argv[1])
        return -1;
    }

    // declare and create components
    LOG_INFO("Start creating components");

    auto camera =xpcfComponentManager->create<SolARCameraOpencv>()->bindTo<input::devices::ICamera>();
    auto binaryMarker =xpcfComponentManager->create<SolARMarker2DSquaredBinaryOpencv>()->bindTo<input::files::IMarker2DSquaredBinary>();

    auto imageViewer =xpcfComponentManager->create<SolARImageViewerOpencv>()->bindTo<display::IImageViewer>();
    auto imageViewerGrey =xpcfComponentManager->create<SolARImageViewerOpencv>("grey")->bindTo<display::IImageViewer>();
    auto imageViewerBinary =xpcfComponentManager->create<SolARImageViewerOpencv>("binary")->bindTo<display::IImageViewer>();
    auto imageViewerContours =xpcfComponentManager->create<SolARImageViewerOpencv>("contours")->bindTo<display::IImageViewer>();
    auto imageViewerFilteredContours =xpcfComponentManager->create<SolARImageViewerOpencv>("filteredContours")->bindTo<display::IImageViewer>();

    auto imageFilterBinary =xpcfComponentManager->create<SolARImageFilterBinaryOpencv>()->bindTo<image::IImageFilter>();
    auto imageConvertor =xpcfComponentManager->create<SolARImageConvertorOpencv>()->bindTo<image::IImageConvertor>();
    auto contoursExtractor =xpcfComponentManager->create<SolARContoursExtractorOpencv>()->bindTo<features::IContoursExtractor>();
    auto contoursFilter =xpcfComponentManager->create<SolARContoursFilterBinaryMarkerOpencv>()->bindTo<features::IContoursFilter>();
    auto perspectiveController =xpcfComponentManager->create<SolARPerspectiveControllerOpencv>()->bindTo<image::IPerspectiveController>();
    auto patternDescriptorExtractor =xpcfComponentManager->create<SolARDescriptorsExtractorSBPatternOpencv>()->bindTo<features::IDescriptorsExtractorSBPattern>();

    auto patternMatcher =xpcfComponentManager->create<SolARDescriptorMatcherRadiusOpencv>()->bindTo<features::IDescriptorMatcher>();
    auto patternReIndexer = xpcfComponentManager->create<SolARSBPatternReIndexer>()->bindTo<features::ISBPatternReIndexer>();

    auto img2worldMapper = xpcfComponentManager->create<SolARImage2WorldMapper4Marker2D>()->bindTo<geom::IImage2WorldMapper>();
    auto PnP =xpcfComponentManager->create<SolARPoseEstimationPnpOpencv>()->bindTo<solver::pose::I3DTransformFinder>();
    auto overlay3D =xpcfComponentManager->create<SolAR3DOverlayOpencv>()->bindTo<display::I3DOverlay>();
    auto overlay2DContours =xpcfComponentManager->create<SolAR2DOverlayOpencv>("contours")->bindTo<display::I2DOverlay>();
    auto overlay2DCircles =xpcfComponentManager->create<SolAR2DOverlayOpencv>("circles")->bindTo<display::I2DOverlay>();


    SRef<Image> inputImage;
    SRef<Image> greyImage;
    SRef<Image> binaryImage;
    SRef<Image> contoursImage;
    SRef<Image> filteredContoursImage;

    std::vector<SRef<Contour2Df>>              contours;
    std::vector<SRef<Contour2Df>>              filtered_contours;
    std::vector<SRef<Image>>                   patches;
    std::vector<SRef<Contour2Df>>              recognizedContours;
    SRef<DescriptorBuffer>                     recognizedPatternsDescriptors;
    SRef<DescriptorBuffer>                     markerPatternDescriptor;
    std::vector<DescriptorMatch>               patternMatches;
    std::vector<SRef<Point2Df>>                pattern2DPoints;
    std::vector<SRef<Point2Df>>                img2DPoints;
    std::vector<SRef<Point3Df>>                pattern3DPoints;
    Transform3Df                               pose;
   
    CamCalibration K;
  
    // components initialisation

    binaryMarker->loadMarker();
    patternDescriptorExtractor->extract(binaryMarker->getPattern(), markerPatternDescriptor);

#ifndef NDEBUG
    SquaredBinaryPatternMatrix patternMatrix = binaryMarker->getPattern()->getPatternMatrix();
    for (int i= 0; i < (int)patternMatrix.rows(); i++)
    {
        std::cout<<"[";
        for (int j = 0; j < (int)patternMatrix.cols(); j++)
        {
            if (patternMatrix(i,j))
                std::cout<<"w ";
            else
                std::cout<<"b ";
        }
        std::cout<<"]"<<std::endl;;
    }
#endif

    int patternSize = binaryMarker->getPattern()->getSize();

    patternDescriptorExtractor->bindTo<xpcf::IConfigurable>()->getProperty("patternSize")->setIntegerValue(patternSize);
    patternReIndexer->bindTo<xpcf::IConfigurable>()->getProperty("sbPatternSize")->setIntegerValue(patternSize);

    Sizei sbPatternSize;
    sbPatternSize.width = patternSize;
    sbPatternSize.height = patternSize;
    img2worldMapper->setParameters(sbPatternSize, binaryMarker->getSize());

    //int maximalDistanceToMatch = 0;
    //patternMatcher->setParameters(maximalDistanceToMatch);
    //PnP->bindTo<xpcf::IConfigurable>()->getProperty("intrinsicsParameters")->set
    PnP->setCameraParameters(camera->getIntrinsicsParameters(), camera->getDistorsionParameters());
    overlay3D->setCameraParameters(camera->getIntrinsicsParameters(), camera->getDistorsionParameters());

    std::string cameraArg=std::string(argv[2]);
    if(cameraArg.find("mp4")!=std::string::npos || cameraArg.find("wmv")!=std::string::npos || cameraArg.find("avi")!=std::string::npos )
    {
        if (camera->start(argv[2]) != FrameworkReturnCode::_SUCCESS) // videoFile
        {
            LOG_ERROR ("Video with url {} does not exist", argv[2]);
            return -1;
        }
    }
    else
    {
        if (camera->start(atoi(argv[2])) != FrameworkReturnCode::_SUCCESS) // Camera
        {
            LOG_ERROR ("Camera with id {} does not exist", argv[2]);
            return -1;
        }
    }

    // to count the average number of processed frames per seconds
    int count=0;
    clock_t start,end;

    count=0;
    start= clock();

    //cv::Mat img_temp;
    bool process = true;
    while (process){
        if(camera->getNextImage(inputImage)==SolAR::FrameworkReturnCode::_ERROR_)
            break;
        count++;

       // Convert Image from RGB to grey
       imageConvertor->convert(inputImage, greyImage, Image::ImageLayout::LAYOUT_GREY);

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
           std::cout << "Looking for the following descriptor:" << std::endl;
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
               for (int j = 0; j < recognizedContours[0]->size(); j++)
               {
                   for (int k = 0; k < 4; k++)
                   {
                       std::cout<<"[" << (*(recognizedContours[i*4+k]))[j][0] <<", "<< (*(recognizedContours[i*4+k]))[j][1] << "] ";
                   }
                   std::cout << std::endl;
               }
               std::cout << std::endl << std::endl;
           }
           std::cout << std::endl;
#endif

            // From extracted squared binary pattern, match the one corresponding to the squared binary marker
            if (patternMatcher->match(markerPatternDescriptor, recognizedPatternsDescriptors, patternMatches) == features::DescriptorMatcher::DESCRIPTORS_MATCHER_OK)
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
                std::cout << "2D Matched points :" << std::endl;
                for (int i = 0; i < img2DPoints.size(); i++)
                    std::cout << "[" << img2DPoints[i]->x() << "," << img2DPoints[i]->y() <<"],";
                std::cout << std::endl;
                for (int i = 0; i < pattern2DPoints.size(); i++)
                    std::cout << "[" << pattern2DPoints[i]->x() << "," << pattern2DPoints[i]->y() <<"],";
                std::cout << std::endl;
                overlay2DCircles->drawCircles(img2DPoints, inputImage);
#endif
                // Compute the 3D position of each corner of the marker
                img2worldMapper->map(pattern2DPoints, pattern3DPoints);
#ifndef NDEBUG
                std::cout << "3D Points position:" << std::endl;
                for (int i = 0; i < pattern3DPoints.size(); i++)
                    std::cout << "[" << pattern3DPoints[i]->x() << "," << pattern3DPoints[i]->y() <<"," << pattern3DPoints[i]->z() << "],";
                std::cout << std::endl;
#endif
                // Compute the pose of the camera using a Perspective n Points algorithm using only the 4 corners of the marker
                if (PnP->estimate(img2DPoints, pattern3DPoints, pose) == FrameworkReturnCode::_SUCCESS)
                {
#ifndef NDEBUG
                    std::cout << "Camera pose :" << std::endl;
                    std::cout << pose.matrix();
                    std::cout << std::endl;
#endif

                    // Display a 3D box over the marker
                    Transform3Df cubeTransform = Transform3Df::Identity();
                    overlay3D->drawBox(pose, binaryMarker->getWidth(), binaryMarker->getHeight(), binaryMarker->getHeight(), cubeTransform, inputImage);
                }
            }
       }

       // display images in viewers
       if (
         (imageViewer->display(inputImage) == FrameworkReturnCode::_STOP)
#ifndef NDEBUG
         ||(imageViewerGrey->display(greyImage) == FrameworkReturnCode::_STOP)
         ||(imageViewerBinary->display(binaryImage) == FrameworkReturnCode::_STOP)
         ||(imageViewerContours->display(contoursImage) == FrameworkReturnCode::_STOP)
         ||(imageViewerFilteredContours->display(filteredContoursImage) == FrameworkReturnCode::_STOP)

#endif
         )
       {
           process = false;
       }
    }
    end= clock();
    double duration=double(end - start) / CLOCKS_PER_SEC;
    printf ("\n\nElasped time is %.2lf seconds.\n",duration );
    printf (  "Number of processed frames per second : %8.2f\n\n",count/duration );

    return 0;

}

int printHelp(){
        printf(" usage :\n");
        printf(" exe ConfFile VideoFile|cameraId\n\n");
        printf(" Escape key to exit");
        return 1;
}

int main(int argc, char **argv){
    if(argc == 3){
        return marker_run(argc,argv);
    }
    else
        return(printHelp());
}





