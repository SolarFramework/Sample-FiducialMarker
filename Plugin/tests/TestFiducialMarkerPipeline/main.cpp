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
#include "core/Log.h"
#include "xpcf/xpcf.h"


// ADD COMPONENTS HEADERS HERE, e.g #include "SolarComponent.h"

#include "PipelineManager.h"
#include "freeglut.h"

using namespace SolAR;
using namespace SolAR::PIPELINE;

namespace xpcf  = org::bcom::xpcf;

#define MARKER_CONFIGFILE "fiducialMarker.yml"

#include "SolARModuleOpencv_traits.h"
#include "SolARImageViewerOpencv.h"
#include "SolAR3DOverlayBoxOpencv.h"

using namespace SolAR;
using namespace SolAR::MODULES::OPENCV;
using namespace SolAR::api;

int ParseMarkerConfig(const char* fileName,float& width,float& height)
{
    cv::FileStorage fs(fileName, cv::FileStorage::READ);

    if (fs.isOpened())
    {
        fs["MarkerWidth"] >> width;
        fs["MarkerHeight"] >> height;
    }
    else
    {
        LOG_ERROR("SolARCameraOpencv::loadCameraParameters: Cannot open marker calibration file ")
        return 1;
    }
    return 0;
}

int main(){

     auto imageViewerResult =xpcf::ComponentFactory::createInstance<SolARImageViewerOpencv>()->bindTo<display::IImageViewer>();
     auto overlay3DComponent =xpcf::ComponentFactory::createInstance<SolAR3DOverlayBoxOpencv>()->bindTo<display::I3DOverlay>();

     overlay3DComponent->bindTo<xpcf::IConfigurable>()->getProperty("position")->setFloatingValue(0,0);
     overlay3DComponent->bindTo<xpcf::IConfigurable>()->getProperty("position")->setFloatingValue(0,1);
     overlay3DComponent->bindTo<xpcf::IConfigurable>()->getProperty("position")->setFloatingValue(0,2);

     overlay3DComponent->bindTo<xpcf::IConfigurable>()->getProperty("orientation")->setFloatingValue(0,0);
     overlay3DComponent->bindTo<xpcf::IConfigurable>()->getProperty("orientation")->setFloatingValue(0,1);
     overlay3DComponent->bindTo<xpcf::IConfigurable>()->getProperty("orientation")->setFloatingValue(0,2);

     float marker_width,marker_height;
     if(ParseMarkerConfig(MARKER_CONFIGFILE,marker_width,marker_height)!=0){
         LOG_INFO("pb parsing config file {} \n",MARKER_CONFIGFILE);
         return 1;
     }

     overlay3DComponent->bindTo<xpcf::IConfigurable>()->getProperty("size")->setFloatingValue(marker_width,0);
     overlay3DComponent->bindTo<xpcf::IConfigurable>()->getProperty("size")->setFloatingValue(marker_height,1);
     overlay3DComponent->bindTo<xpcf::IConfigurable>()->getProperty("size")->setFloatingValue(0.1,2);

     overlay3DComponent->bindTo<xpcf::IConfigurable>()->onConfigured();




#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

    LOG_ADD_LOG_TO_CONSOLE();

    PipelineManager pipeline;
    if (pipeline.init("PipelineFiducialMarker.xml", "3898cc3b-3986-4edc-b7c8-f4fba0f6c22a"))
    {

        PipelineManager::CamParams calib = pipeline.getCameraParameters();

        unsigned char* r_imageData=new unsigned char[calib.width*calib.height*3];
        SRef<Image> camImage;
        Transform3Df s_pose;

        if (pipeline.start(r_imageData))
        {
            while (true)
            {
                PipelineManager::Pose pose;

                camImage=xpcf::utils::make_shared<Image>(r_imageData,calib.width,calib.height,SolAR::Image::LAYOUT_BGR,SolAR::Image::INTERLEAVED,SolAR::Image::TYPE_8U);

                if (pipeline.udpate(pose))
                {
                    LOG_INFO("Camera Pose translation ({}, {}, {})", pose.translation(0), pose.translation(1), pose.translation(2));
                    for(int i=0;i<3;i++)
                         for(int j=0;j<3;j++)
                             s_pose(j,i)=pose.rotation(i,j);
                    for(int j=0;j<3;j++)
                             s_pose(3,j)=pose.translation(j);
                    for(int i=0;i<3;i++)
                        s_pose(i,3)=0;
                    s_pose(3,3)=1;
                    LOG_INFO("pose.matrix():\n {} \n",s_pose.matrix())
                    overlay3DComponent->draw(s_pose, camImage);
                }
                if (imageViewerResult->display(camImage) == SolAR::FrameworkReturnCode::_STOP){
                    pipeline.stop();
                    break;
                }
             }
        }
    }
}





