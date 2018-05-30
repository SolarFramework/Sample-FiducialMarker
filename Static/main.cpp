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

// ADD COMPONENTS HEADERS HERE, e.g #include "SolarComponent.h"
#include "SolARCameraOpencv.h"
#include "SolARImageFilterOpencv.h"
#include "SolARImageViewerOpencv.h"
#include "SolARImageConvertorOpencv.h"
#include "SolARMarker2DSquaredBinaryOpencv.h"
#include "SolARImageFilterOpencv.h"
#include "SolARContoursExtractorOpencv.h"
#include "SolARContoursFilterBinaryMarkerOpencv.h"
#include "SolARPerspectiveControllerOpencv.h"
#include "SolARDescriptorsExtractorSBPatternOpencv.h"
#include "SolARDescriptorMatcherRadiusOpencv.h"
#include "SolARSBPatternReIndexer.h"
#include "SolARImage2WorldMapper4Marker2D.h"
#include "SolARPoseEstimationPnpOpencv.h"
#include "SolAR2DOverlayOpencv.h"
#include "SolAR3DOverlayOpencv.h"

#include <iostream>
#include <map>

using namespace std;
using namespace SolAR;
using namespace SolAR::datastructure;
using namespace SolAR::api;
using namespace SolAR::MODULES::OPENCV;
using namespace SolAR::MODULES::TOOLS;

namespace xpcf  = org::bcom::xpcf;

void marker_run(int argc,char** argv){

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif


    LOG_ADD_LOG_TO_CONSOLE();

    // declarations

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
    Transform3Df                                       pose;
    // The Intrinsic parameters of the camera
    CamCalibration K;
    // The escape key to exit the sample
    char escape_key = 27;
    // color used to draw contours
    std::vector<unsigned int> bgr{128, 128, 128};

    // component creation
    auto camera =xpcf::ComponentFactory::createComponent<SolARCameraOpencv>()->bindTo<input::devices::ICamera>();
    auto binaryMarker =xpcf::ComponentFactory::createComponent<SolARMarker2DSquaredBinaryOpencv>()->bindTo<input::files::IMarker2DSquaredBinary>();
    auto imageViewer =xpcf::ComponentFactory::createComponent<SolARImageViewerOpencv>()->bindTo<display::IImageViewer>();
    auto imageViewerGrey =xpcf::ComponentFactory::createComponent<SolARImageViewerOpencv>()->bindTo<display::IImageViewer>();
    auto imageViewerBinary =xpcf::ComponentFactory::createComponent<SolARImageViewerOpencv>()->bindTo<display::IImageViewer>();
    auto imageViewerContours =xpcf::ComponentFactory::createComponent<SolARImageViewerOpencv>()->bindTo<display::IImageViewer>();
    auto imageViewerFilteredContours =xpcf::ComponentFactory::createComponent<SolARImageViewerOpencv>()->bindTo<display::IImageViewer>();
    auto imageFilter =xpcf::ComponentFactory::createComponent<SolARImageFilterOpencv>()->bindTo<image::IImageFilter>();
    auto imageConvertor =xpcf::ComponentFactory::createComponent<SolARImageConvertorOpencv>()->bindTo<image::IImageConvertor>();
    auto contoursExtractor =xpcf::ComponentFactory::createComponent<SolARContoursExtractorOpencv>()->bindTo<features::IContoursExtractor>();
    auto contoursFilter =xpcf::ComponentFactory::createComponent<SolARContoursFilterBinaryMarkerOpencv>()->bindTo<features::IContoursFilter>();
    auto perspectiveController =xpcf::ComponentFactory::createComponent<SolARPerspectiveControllerOpencv>()->bindTo<image::IPerspectiveController>();
    auto patternDescriptorExtractor =xpcf::ComponentFactory::createComponent<SolARDescriptorsExtractorSBPatternOpencv>()->bindTo<features::IDescriptorsExtractorSBPattern>();
    auto patternMatcher =xpcf::ComponentFactory::createComponent<SolARDescriptorMatcherRadiusOpencv>()->bindTo<features::IDescriptorMatcher>();
    auto patternReIndexer = xpcf::ComponentFactory::createComponent<SolARSBPatternReIndexer>()->bindTo<features::ISBPatternReIndexer>();
    auto img2worldMapper = xpcf::ComponentFactory::createComponent<SolARImage2WorldMapper4Marker2D>()->bindTo<geom::IImage2WorldMapper>();
    auto PnP =xpcf::ComponentFactory::createComponent<SolARPoseEstimationPnpOpencv>()->bindTo<solver::pose::I3DTransformFinder>();
    auto overlay3D =xpcf::ComponentFactory::createComponent<SolAR3DOverlayOpencv>()->bindTo<display::I3DOverlay>();
    auto overlay2D =xpcf::ComponentFactory::createComponent<SolAR2DOverlayOpencv>()->bindTo<display::I2DOverlay>();



    // components initialisation

    binaryMarker->loadMarker(argv[1]);
    patternDescriptorExtractor->extract(binaryMarker->getPattern(), markerPatternDescriptor);

#ifdef DEBUG
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

    int minContourSize = 4;
    contoursExtractor->setParameters(minContourSize);

    int minContourLength = 20;
    contoursFilter->setParameters(minContourLength);

    Sizei CorrectedImagesSize = {640,480};
    perspectiveController->setParameters(CorrectedImagesSize);

    int patternSize = binaryMarker->getPattern()->getSize();
    patternDescriptorExtractor->setParameters(patternSize);

    patternReIndexer->setParameters(patternSize);

    Sizei sbPatternSize;
    sbPatternSize.width = patternSize;
    sbPatternSize.height = patternSize;
    img2worldMapper->setParameters(sbPatternSize, binaryMarker->getSize());

    //int maximalDistanceToMatch = 0;
    //patternMatcher->setParameters(maximalDistanceToMatch);

    //Load camera parameters and start it
    if (camera->loadCameraParameters(argv[2]) != FrameworkReturnCode::_SUCCESS){
        {
            LOG_ERROR ("camera calibration file {} does not exist", argv[2]);
            return ;
        }
    }

    PnP->setCameraParameters(camera->getIntrinsicsParameters(), camera->getDistorsionParameters());
    overlay3D->setCameraParameters(camera->getIntrinsicsParameters(), camera->getDistorsionParameters());

    std::string cameraArg=std::string(argv[3]);
    if(cameraArg.find("mp4")!=std::string::npos || cameraArg.find("wmv")!=std::string::npos || cameraArg.find("avi")!=std::string::npos )
    {
        if (camera->start(argv[3]) != FrameworkReturnCode::_SUCCESS) // videoFile
        {
            LOG_ERROR ("Video with url {} does not exist", argv[3]);
            return ;
        }
    }
    else
    {
        if (camera->start(atoi(argv[3])) != FrameworkReturnCode::_SUCCESS) // Camera
        {
            LOG_ERROR ("Camera with id {} does not exist", argv[3]);
            return ;
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
       imageFilter->binarize(greyImage,binaryImage,-1,255);

       // Extract contours from binary image
       contoursExtractor->extract(binaryImage,contours);
#ifdef DEBUG
       contoursImage = binaryImage->copy();
       overlay2D->drawContours(contours, 3, bgr, contoursImage);
#endif
       // Filter 4 edges contours to find those candidate for marker contours
       contoursFilter->filter(contours, filtered_contours);

#ifdef DEBUG
       filteredContoursImage = binaryImage->copy();
       overlay2D->drawContours(filtered_contours, 3, bgr, filteredContoursImage);
#endif
       // Create one warpped and cropped image by contour
       perspectiveController->correct(binaryImage, filtered_contours, patches);

       // test if this last image is really a squared binary marker, and if it is the case, extract its descriptor
       if (patternDescriptorExtractor->extract(patches, filtered_contours, recognizedPatternsDescriptors, recognizedContours) != FrameworkReturnCode::_ERROR_)
       {

#ifdef DEBUG
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
#ifdef DEBUG
                std::cout << "Matches :" << std::endl;
                for (int num_match = 0; num_match < patternMatches.size(); num_match++)
                    std::cout << "Match [" << patternMatches[num_match].getIndexInDescriptorA() << "," << patternMatches[num_match].getIndexInDescriptorB() << "], dist = " << patternMatches[num_match].getMatchingScore() << std::endl;
                std::cout << std::endl << std::endl;
#endif

                // Reindex the pattern to create two vector of points, the first one corresponding to marker corner, the second one corresponding to the poitsn of the contour
                patternReIndexer->reindex(recognizedContours, patternMatches, pattern2DPoints, img2DPoints);
#ifdef DEBUG
                std::cout << "2D Matched points :" << std::endl;
                for (int i = 0; i < img2DPoints.size(); i++)
                    std::cout << "[" << img2DPoints[i]->x() << "," << img2DPoints[i]->y() <<"],";
                std::cout << std::endl;
                for (int i = 0; i < pattern2DPoints.size(); i++)
                    std::cout << "[" << pattern2DPoints[i]->x() << "," << pattern2DPoints[i]->y() <<"],";
                std::cout << std::endl;
                overlay2D->drawCircles(img2DPoints, 6, 3, inputImage);
#endif
                // Compute the 3D position of each corner of the marker
                img2worldMapper->map(pattern2DPoints, pattern3DPoints);
#ifdef DEBUG
                std::cout << "3D Points position:" << std::endl;
                for (int i = 0; i < pattern3DPoints.size(); i++)
                    std::cout << "[" << pattern3DPoints[i]->x() << "," << pattern3DPoints[i]->y() <<"," << pattern3DPoints[i]->z() << "],";
                std::cout << std::endl;
#endif
                // Compute the pose of the camera using a Perspective n Points algorithm using only the 4 corners of the marker
                if (PnP->estimate(img2DPoints, pattern3DPoints, pose) == FrameworkReturnCode::_SUCCESS)
                {
#ifdef DEBUG
                    std::cout << "Camera pose :" << std::endl;
                    for(int ii = 0; ii < 4; ++ii){
                        for(int jj = 0; jj < 4; ++jj){
                            std::cout<<pose(ii,jj)<<" ";
                        }
                        std::cout<<std::endl;
                    }
                    std::cout << std::endl;
 #endif

                    // Display a 3D box over the marker
                    Transform3Df cubeTransform = Transform3Df::Identity();
                    overlay3D->drawBox(pose, binaryMarker->getWidth(), binaryMarker->getHeight(), binaryMarker->getHeight()*0.5, cubeTransform, inputImage);
                }
            }
       }

       // display images in viewers
       if (
         (imageViewer->display("original image", inputImage, &escape_key) == FrameworkReturnCode::_STOP)
#ifdef DEBUG
         ||(imageViewerGrey->display("Grey Image", greyImage, &escape_key) == FrameworkReturnCode::_STOP)
         ||(imageViewerBinary->display("Binary Image", binaryImage, &escape_key) == FrameworkReturnCode::_STOP)
         ||(imageViewerContours->display("Contours Image", contoursImage, &escape_key) == FrameworkReturnCode::_STOP)
         ||(imageViewerFilteredContours->display("Filtered Contours Image", filteredContoursImage, &escape_key) == FrameworkReturnCode::_STOP)

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

}

int printHelp(){
        printf(" usage :\n");
        printf(" exe exe FiducialMarkerFilename CameraCalibrationFile VideoFile|cameraId\n\n");
        printf(" Escape key to exit");
        return 1;
}

int main(int argc, char **argv){
    if(argc==3 || argc ==4){
        marker_run(argc,argv);
         return 1;
    }
    else
        return(printHelp());
}





