/**
 * @copyright Copyright (c) 2021 B-com http://www.b-com.com/
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

#include "SolARPipelineFiducialMarkerRunner.h"

#include <core/Log.h>
#include <xpcf/xpcf.h>


#include <api/pipeline/IPoseEstimationPipeline.h>
#include <api/display/IImageViewer.h>
#include <api/display/I3DOverlay.h>
#include <datastructure/CameraDefinitions.h>

#include <boost/log/core.hpp>

namespace xpcf  = org::bcom::xpcf;

using namespace SolAR;
using namespace SolAR::datastructure;
using namespace SolAR::api;

namespace SolAR::PIPELINES::runner
{

SolARPipelineFiducialMarkerRunner::Builder&
SolARPipelineFiducialMarkerRunner::Builder::selectPlaybackMode(const std::string& configFileName,
                                                               int timeoutInS)
{
    m_mode = Mode::playback;
    m_configFileName = configFileName;
    m_timeoutInS = timeoutInS;
    return *this;
}

SolARPipelineFiducialMarkerRunner::Builder&
SolARPipelineFiducialMarkerRunner::Builder::selectLiveMode(const std::string& configFileName)
{
    m_mode = Mode::live;
    m_configFileName = configFileName;
    return *this;
}

SolARPipelineFiducialMarkerRunner::Builder&
SolARPipelineFiducialMarkerRunner::Builder::disableDisplay()
{
    m_displayEnabled = false;
    return *this;
}

std::shared_ptr<SolARPipelineFiducialMarkerRunner> SolARPipelineFiducialMarkerRunner::Builder::build()
{
    auto result = std::shared_ptr<SolARPipelineFiducialMarkerRunner>(new SolARPipelineFiducialMarkerRunner());
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
            if (m_timeoutInS < 0)
            {
                throw std::runtime_error("Timeout value must be positive");
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

void SolARPipelineFiducialMarkerRunner::selectPlaybackMode(const std::string& configFileName,
                                                           int timeoutInS)
{
    m_mode = Mode::playback;
    m_configFileName = configFileName;
    m_timeoutInS = timeoutInS;
}

void SolARPipelineFiducialMarkerRunner::selectLiveMode(const std::string& configFileName)
{
    m_mode = Mode::live;
    m_configFileName = configFileName;
}

void SolARPipelineFiducialMarkerRunner::disableDisplay()
{
    m_displayEnabled = false;
}

int SolARPipelineFiducialMarkerRunner::run(){

    bool replayModeEnabled = m_mode == Mode::playback;

    m_poseDetected = false;

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

#if NDEBUG
    boost::log::core::get()->set_logging_enabled(false);
#endif

    try{
        LOG_ADD_LOG_TO_CONSOLE();
        SRef<xpcf::IComponentManager> componentMgr = xpcf::getComponentManagerInstance();

        // Required to run several tests with same mngr instance
        componentMgr->clear();

        componentMgr->load(m_configFileName.c_str());
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
            if (pipeline->start(camImage->data()) == FrameworkReturnCode::_SUCCESS)
            {
                clock_t timeoutInMs = m_timeoutInS * CLOCKS_PER_SEC;
                clock_t start = clock();

                while (true)
                {
                    Transform3Df pose;

                    sink::SinkReturnCode returnCode = pipeline->update(pose);

                    if (returnCode == sink::SinkReturnCode::_NOTHING)
                        continue;

                    if ((returnCode == sink::SinkReturnCode::_NEW_POSE) || (returnCode == sink::SinkReturnCode::_NEW_POSE_AND_IMAGE))
                    {
                        m_poseDetected = true;

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

                    if ((m_displayEnabled && imageViewerResult->display(camImage) == SolAR::FrameworkReturnCode::_STOP)
                         || ( replayModeEnabled && timeoutInMs >= 0 && (clock() - start) > timeoutInMs ))
                    {
                        pipeline->stop();
                        break;
                    }
                 }

            }
            delete[] r_imageData;
        }
    }
    catch (xpcf::Exception e)
    {
        LOG_ERROR ("The following exception has been catch : {}", e.what());
        return -1;
    }
    return 0;
}

bool SolARPipelineFiducialMarkerRunner::isPoseDetected()
{
    return m_poseDetected;
}

} // namespace SolAR::PIPELINES::runner



