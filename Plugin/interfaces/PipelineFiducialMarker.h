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

#ifndef PIPELINEFIDUCIALMARKER_H
#define PIPELINEFIDUCIALMARKER_H

#if _WIN32
#ifdef PipelineFiducialMarker_API_DLLEXPORT
#define SOLARPIPELINEFIDUCIALMARKER_EXPORT_API __declspec(dllexport)
#else //SOLARPIPELINEFIDUCIALMARKER_API_DLLEXPORT
#define SOLARPIPELINEFIDUCIALMARKER_EXPORT_API __declspec(dllimport)
#endif //SOLARPIPELINEFIDUCIALMARKER_API_DLLEXPORT
#else //_WIN32
#define SOLARPIPELINEFIDUCIALMARKER_EXPORT_API
#endif //_WIN32

#include "xpcf/core/traits.h"
#include "xpcf/component/ConfigurableBase.h"
#include "api/pipeline/IPipeline.h"

// Add the headers to datastructures and component interfaces used by the pipeline
#include "api/input/devices/ICamera.h"
#include "api/input/files/IMarker2DNaturalImage.h"
#include "api/input/files/IMarker2DSquaredBinary.h"
#include "api/image/IImageFilter.h"
#include "api/image/IImageConvertor.h"
#include "api/features/IContoursExtractor.h"
#include "api/features/IContoursFilter.h"
#include "api/image/IPerspectiveController.h"
#include "api/features/IDescriptorsExtractorSBPattern.h"
#include "api/features/IDescriptorMatcher.h"
#include "api/features/ISBPatternReIndexer.h"

#include "api/features/IKeypointDetector.h"
#include "api/features/IDescriptorsExtractor.h"
#include "api/features/IMatchesFilter.h"
#include "api/solver/pose/I2DTransformFinder.h"
#include "api/solver/pose/IHomographyValidation.h"
#include "api/features/IKeypointsReIndexer.h"
#include "api/solver/pose/I3DTransformFinderFrom2D3D.h"
#include "api/geom/IImage2WorldMapper.h"
#include "api/geom/I2DTransform.h"
#include "api/sink/ISinkPoseTextureBuffer.h"

#include "xpcf/threading/SharedBuffer.h"
#include "xpcf/threading/DropBuffer.h"
#include "xpcf/threading/BaseTask.h"

namespace SolAR {
using namespace datastructure;
using namespace api;
using namespace api::sink;
using namespace api::pipeline;
namespace PIPELINES {

/**
 * @class FiducialMarker
 * @brief A pipeline to estimate the pose based on a squared fiducial marker.
 */

class SOLARPIPELINEFIDUCIALMARKER_EXPORT_API PipelineFiducialMarker : public org::bcom::xpcf::ConfigurableBase,
    public api::pipeline::IPipeline
{
public:
    PipelineFiducialMarker();
    ~PipelineFiducialMarker();

    //// @brief Initialization of the pipeline
    /// Initialize the pipeline by providing a reference to the component manager loaded by the PipelineManager.
    /// @param[in] componentManager a shared reference to the component manager which has loaded the components and configuration in the pipleine manager
    FrameworkReturnCode init(SRef<xpcf::IComponentManager> xpcfComponentManager) override;

    /// @brief Provide the camera parameters
    /// @return the camera parameters (its resolution and its focal)
    CameraParameters getCameraParameters() override;

    /// @brief Starts the pipeline and provides a texture buffer which will be updated when required.
    /// @param[in] textureHandle a pointer to the texture buffer which will be updated at each call of the update method.
    FrameworkReturnCode start(void* textureHandle) override;

    /// @brief Stop the pipeline.
    FrameworkReturnCode stop() override;

    /// @brief update the pipeline
    /// Get the new pose and update the texture buffer with the image that has to be displayed
    SinkReturnCode update(Transform3Df& pose) override;

    void updateFrameDataOGL(int enventID) override;

    void unloadComponent () override final;

private:
    // Decalaration of data structures shared between initialization and process thread
    SRef<DescriptorBuffer> m_markerPatternDescriptor;

    // Declaration of the components used by the pipeline
    SRef<input::devices::ICamera> m_camera;
    SRef<input::files::IMarker2DSquaredBinary> m_binaryMarker;
    SRef<image::IImageFilter> m_imageFilterBinary;
    SRef<image::IImageConvertor> m_imageConvertor;
    SRef<features::IContoursExtractor> m_contoursExtractor ;
    SRef<features::IContoursFilter> m_contoursFilter;
    SRef<image::IPerspectiveController> m_perspectiveController;
    SRef<features::IDescriptorsExtractorSBPattern> m_patternDescriptorExtractor;
    SRef<features::IDescriptorMatcher> m_patternMatcher;
    SRef<features::ISBPatternReIndexer> m_patternReIndexer;
    SRef<geom::IImage2WorldMapper> m_img2worldMapper;
    SRef<solver::pose::I3DTransformFinderFrom2D3D> m_PnP;
    SRef<sink::ISinkPoseTextureBuffer> m_sink;

    // State flag of the pipeline
    bool m_stopFlag, m_initOK, m_startedOK;

    // Threads
    bool processCamImage();
    xpcf::DelegateTask* m_taskProcess;
};

}
}

XPCF_DEFINE_COMPONENT_TRAITS(SolAR::PIPELINES::PipelineFiducialMarker,
                             "3898cc3b-3986-4edc-b7c8-f4fba0f6c22a",
                             "PipelineFiducialMarker",
                             "A pipeline to estimate the pose based on a squared fiducial marker");

#endif // PIPELINEFIDUCIALMARKER_H
