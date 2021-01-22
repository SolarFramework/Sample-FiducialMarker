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
#ifdef SolARPipeline_FiducialMarker_API_DLLEXPORT
#define SOLARPIPELINEFIDUCIALMARKER_EXPORT_API __declspec(dllexport)
#else //SOLARPIPELINEFIDUCIALMARKER_API_DLLEXPORT
#define SOLARPIPELINEFIDUCIALMARKER_EXPORT_API __declspec(dllimport)
#endif //SOLARPIPELINEFIDUCIALMARKER_API_DLLEXPORT
#else //_WIN32
#define SOLARPIPELINEFIDUCIALMARKER_EXPORT_API
#endif //_WIN32

#include "xpcf/core/traits.h"
#include "xpcf/component/ConfigurableBase.h"
#include "api/pipeline/IPoseEstimationPipeline.h"

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
#include "api/sink/ISinkPoseImage.h"
#include "api/source/ISourceImage.h"
#include "xpcf/threading/SharedBuffer.h"
#include "xpcf/threading/DropBuffer.h"
#include "xpcf/threading/BaseTask.h"

namespace SolAR {
namespace PIPELINES {

/**
 * @class PipelineFiducialMarker
 * @brief A pipeline to estimate the pose based on a squared fiducial marker.
 *
 * @SolARComponentInjectablesBegin
 * @SolARComponentInjectable{SolAR::api::input::devices::ICamera}
 * @SolARComponentInjectable{SolAR::api::input::files::IMarker2DSquaredBinary}
 * @SolARComponentInjectable{SolAR::api::image::IImageFilter}
 * @SolARComponentInjectable{SolAR::xpcf::IConfigurable}
 * @SolARComponentInjectable{SolAR::api::image::IImageConvertor}
 * @SolARComponentInjectable{SolAR::api::features::IContoursExtractor}
 * @SolARComponentInjectable{SolAR::api::features::IContoursFilter}
 * @SolARComponentInjectable{SolAR::api::image::IPerspectiveController}
 * @SolARComponentInjectable{SolAR::api::features::IDescriptorsExtractorSBPattern}
 * @SolARComponentInjectable{SolAR::api::features::IDescriptorMatcher}
 * @SolARComponentInjectable{SolAR::api::features::ISBPatternReIndexer}
 * @SolARComponentInjectable{SolAR::api::geom::IImage2WorldMapper}
 * @SolARComponentInjectable{SolAR::api::solver::pose::I3DTransformFinderFrom2D3D}
 * @SolARComponentInjectable{SolAR::api::sink::ISinkPoseImage}
 * @SolARComponentInjectable{SolAR::api::source::ISourceImage}
 * @SolARComponentInjectable{SolAR::api::image::IImageConvertor}
 * @SolARComponentInjectablesEnd
 *
 * @SolARComponentPropertiesBegin
 * @SolARComponentProperty{ minThreshold,
 *                          ,
 *                          @SolARComponentPropertyDescNum{ int, [0..MAX INT], 50 }}
 * @SolARComponentProperty{ maxThreshold,
 *                          ,
 *                          @SolARComponentPropertyDescNum{ int, [0..MAX INT], 220 }}
 * @SolARComponentProperty{ nbTestedThreshold,
 *                          ,
 *                          @SolARComponentPropertyDescNum{ int, [0..MAX INT], 6 }}
 * @SolARComponentPropertiesEnd
 *
 */

class SOLARPIPELINEFIDUCIALMARKER_EXPORT_API PipelineFiducialMarker : public org::bcom::xpcf::ConfigurableBase,
    public api::pipeline::IPoseEstimationPipeline
{
public:
    PipelineFiducialMarker();
    ~PipelineFiducialMarker();

    //// @brief Initialization of the pipeline
    /// @return FrameworkReturnCode::_SUCCESS if the init succeed, else FrameworkReturnCode::_ERROR_
    FrameworkReturnCode init() override;

    /// @brief Provide the camera parameters
    /// @return the camera parameters (its resolution and its focal)
    datastructure::CameraParameters getCameraParameters() const override;

    /// @brief Start the pipeline
    /// @return FrameworkReturnCode::_SUCCESS if the stard succeed, else FrameworkReturnCode::_ERROR_
    FrameworkReturnCode start() override { return FrameworkReturnCode::_SUCCESS; }

    /// @brief Starts the pipeline and provides a texture buffer which will be updated when required.
    /// @param[in] textureHandle a pointer to the texture buffer which will be updated at each call of the update method.
    FrameworkReturnCode start(void* imageDataBuffer) override;

    /// @brief Stop the pipeline.
    FrameworkReturnCode stop() override;

    /// @brief update the pipeline
    /// Get the new pose and update the texture buffer with the image that has to be displayed
    api::sink::SinkReturnCode update(datastructure::Transform3Df& pose) override;

    /// @brief load the source image
    api::source::SourceReturnCode loadSourceImage(void* sourceTextureHandle, int width, int height) override;
    org::bcom::xpcf::XPCFErrorCode onConfigured() override;
    void unloadComponent () override final;

private:
    // Decalaration of data structures shared between initialization and process thread
    SRef<datastructure::DescriptorBuffer> m_markerPatternDescriptor;

    // Declaration of the components used by the pipeline
    SRef<api::input::devices::ICamera> m_camera;
    SRef<api::input::files::IMarker2DSquaredBinary> m_binaryMarker;
    SRef<api::image::IImageFilter> m_imageFilterBinary;
    SRef<xpcf::IConfigurable> m_imageFilterBinaryConfigurable;
    SRef<api::image::IImageConvertor> m_imageConvertor;
    SRef<api::features::IContoursExtractor> m_contoursExtractor ;
    SRef<api::features::IContoursFilter> m_contoursFilter;
    SRef<api::image::IPerspectiveController> m_perspectiveController;
    SRef<api::features::IDescriptorsExtractorSBPattern> m_patternDescriptorExtractor;
    SRef<api::features::IDescriptorMatcher> m_patternMatcher;
    SRef<api::features::ISBPatternReIndexer> m_patternReIndexer;
    SRef<api::geom::IImage2WorldMapper> m_img2worldMapper;
    SRef<api::solver::pose::I3DTransformFinderFrom2D3D> m_PnP;
    SRef<api::sink::ISinkPoseImage> m_sink;
    SRef<api::source::ISourceImage> m_source;
    SRef<api::image::IImageConvertor> m_imageConvertorUnity;

    // State flag of the pipeline
    bool m_stopFlag, m_initOK, m_startedOK, m_haveToBeFlip;

    // Threads
    bool processCamImage();
    xpcf::DelegateTask* m_taskProcess;

    datastructure::Transform3Df m_pose;

    int m_minThreshold = 50;
    int m_maxThreshold = 220;
    int m_nbTestedThreshold = 6;

};

}
}

XPCF_DEFINE_COMPONENT_TRAITS(SolAR::PIPELINES::PipelineFiducialMarker,
                             "3898cc3b-3986-4edc-b7c8-f4fba0f6c22a",
                             "PipelineFiducialMarker",
                             "A pipeline to estimate the pose based on a squared fiducial marker");

#endif // PIPELINEFIDUCIALMARKER_H
