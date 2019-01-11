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

// ADD COMPONENTS HEADERS HERE, e.g #include "SolarComponent.h"

#include "PipelineManager.h"
#include "freeglut.h"

using namespace SolAR;
using namespace SolAR::PIPELINE;

namespace xpcf  = org::bcom::xpcf;

int main(){

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

    LOG_ADD_LOG_TO_CONSOLE();

    PipelineManager pipeline;
    if (pipeline.init("PipelineFiducialMarker.xml", "3898cc3b-3986-4edc-b7c8-f4fba0f6c22a"))
    {

        PipelineManager::CamParams calib = pipeline.getCameraParameters();

        GLuint textureHandle;
        glGenTextures(1, &textureHandle);
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, calib.width, calib.height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        if (pipeline.start(&textureHandle))
        {
            while (true)
            {
                PipelineManager::Pose pose;
                if (pipeline.udpate(pose))
                {
                    LOG_INFO("Camera Pose translation ({}, {}, {})", pose.translation(0), pose.translation(1), pose.translation(2));
                }
            }
        }
        pipeline.stop();
    }
}





