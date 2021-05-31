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


#include "api/pipeline/IPoseEstimationPipeline.h"
#include "api/display/IImageViewer.h"
#include "api/display/I3DOverlay.h"
#include "datastructure/CameraDefinitions.h"

#include <gtest/gtest.h>

namespace xpcf  = org::bcom::xpcf;

using namespace SolAR;
using namespace SolAR::datastructure;
using namespace SolAR::api;

#ifdef USE_AS_GTEST
//int main(int argc, char **argv) {
//  ::testing::InitGoogleTest(&argc, argv);0
//  return RUN_ALL_TESTS();
//}

TEST(Toto, pipeline) {

#else
// TODO(jmhenaff): add this in a common header?
// Disable GTest MACROs
#define FAIL();
#define SUCCEED();
#define EXPECT_TRUE(X);
#define ASSERT_TRUE(X); if(!(X)) return -1;

int main(){
#endif

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

    try{
        LOG_ADD_LOG_TO_CONSOLE();
        SRef<xpcf::IComponentManager> componentMgr = xpcf::getComponentManagerInstance();
        componentMgr->load("SolARPipelineTest_FiducialMarker_conf.xml");
        auto pipeline = componentMgr->resolve<pipeline::IPoseEstimationPipeline>();

        if (pipeline->init(componentMgr) == FrameworkReturnCode::_SUCCESS)
        {
            auto imageViewerResult = componentMgr->resolve<display::IImageViewer>();
            auto overlay3DComponent = componentMgr->resolve<display::I3DOverlay>();

            // Set camera parameters
            CameraParameters camParam = pipeline->getCameraParameters();
            overlay3DComponent->setCameraParameters(camParam.intrinsic, camParam.distortion);

            unsigned char* r_imageData=new unsigned char[camParam.resolution.width * camParam.resolution.height * 3];
            SRef<Image> camImage = xpcf::utils::make_shared<Image>(r_imageData, camParam.resolution.width, camParam.resolution.height, Image::LAYOUT_BGR, Image::INTERLEAVED, Image::TYPE_8U);

            Transform3Df s_pose;
            int count(0);
            if (pipeline->start(camImage->data()) == FrameworkReturnCode::_SUCCESS)
            {
#ifdef USE_AS_GTEST
                bool poseFound = false;
#endif
                while (true)
                {
                    Transform3Df pose;

                    sink::SinkReturnCode returnCode = pipeline->update(pose);

                    if (returnCode == sink::SinkReturnCode::_NOTHING)
                        continue;

                    if ((returnCode == sink::SinkReturnCode::_NEW_POSE) || (returnCode == sink::SinkReturnCode::_NEW_POSE_AND_IMAGE))
                    {
#ifdef USE_AS_GTEST
                        poseFound = true;
#endif

                        for(int i=0;i<3;i++)
                             for(int j=0;j<3;j++)
                                 s_pose(i,j)=pose(i,j);
                        for(int i=0;i<3;i++)
                                 s_pose(i,3)=pose(i,3);
                        for(int j=0;j<3;j++)
                            s_pose(3,j)=0;
                        s_pose(3,3)=1;
                        overlay3DComponent->draw(s_pose, camImage);
                    }

                    if (imageViewerResult->display(camImage) == SolAR::FrameworkReturnCode::_STOP
#ifdef USE_AS_GTEST
                         || ++count > 20
#endif

                            ){
                        pipeline->stop();
                        break;
                    }

#ifdef USE_AS_GTEST
                    if (++count > 20)
                    {
                        pipeline->stop();
                        break;
                    }
#endif

                 }

               ASSERT_TRUE(poseFound);

            }
            delete[] r_imageData;
        }

        SUCCEED();
    }
    catch (xpcf::Exception e)
    {
        LOG_ERROR ("The following exception has been catch : {}", e.what());
        FAIL();//return -1;
    }

    SUCCEED();
}





