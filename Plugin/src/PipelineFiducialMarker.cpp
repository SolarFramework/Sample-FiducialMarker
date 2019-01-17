/**
 * @copyright Copyright (c) 2015 All Right Reserved, B-com http://www.b-com.com/
 *
 * This file is subject to the B<>Com License.
 * All other rights reserved.
 *
 * THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 */

#include "xpcf/module/ModuleFactory.h"
#include "PipelineFiducialMarker.h"

#include "SolARModuleOpencv_traits.h"
#include "SolARModuleTools_traits.h"
#include "SolARModuleOpengl_traits.h"

#include "core/Log.h"

namespace xpcf=org::bcom::xpcf;

// Declaration of the module embedding the fiducial marker pipeline
XPCF_DECLARE_MODULE("63b4282f-94cf-44d0-8ec0-9e8b0639fff6", "FiducialMarkerModule", "The module embedding a pipeline to estimate the pose based on a squared fiducial marker")

extern "C" XPCF_MODULEHOOKS_API xpcf::XPCFErrorCode XPCF_getComponent(const boost::uuids::uuid& componentUUID,SRef<xpcf::IComponentIntrospect>& interfaceRef)
{
    xpcf::XPCFErrorCode errCode = xpcf::XPCFErrorCode::_FAIL;
    errCode = xpcf::tryCreateComponent<SolAR::PIPELINES::PipelineFiducialMarker>(componentUUID,interfaceRef);

    return errCode;
}

XPCF_BEGIN_COMPONENTS_DECLARATION
XPCF_ADD_COMPONENT(SolAR::PIPELINES::PipelineFiducialMarker)
XPCF_END_COMPONENTS_DECLARATION

// The pipeline component for the fiducial marker

XPCF_DEFINE_FACTORY_CREATE_INSTANCE(SolAR::PIPELINES::PipelineFiducialMarker)

namespace SolAR {
using namespace datastructure;
using namespace api::pipeline;
namespace PIPELINES {

PipelineFiducialMarker::PipelineFiducialMarker():ConfigurableBase(xpcf::toUUID<PipelineFiducialMarker>())
{
   addInterface<api::pipeline::IPipeline>(this);
   m_initOK = false;
   m_startedOK = false;
   m_stopFlag = false;
   m_taskProcess = nullptr;
   LOG_DEBUG(" Pipeline constructor");
}

PipelineFiducialMarker::~PipelineFiducialMarker()
{
    if(m_taskProcess != nullptr)
        delete m_taskProcess;
    LOG_DEBUG(" Pipeline destructor")
}

FrameworkReturnCode PipelineFiducialMarker::init(SRef<xpcf::IComponentManager> xpcfComponentManager)
{
    LOG_INFO("Start init")
    m_camera = xpcfComponentManager->create<MODULES::OPENCV::SolARCameraOpencv>()->bindTo<input::devices::ICamera>();
    if (m_camera)
        LOG_INFO("Camera component loaded");
    m_binaryMarker =xpcfComponentManager->create<MODULES::OPENCV::SolARMarker2DSquaredBinaryOpencv>()->bindTo<input::files::IMarker2DSquaredBinary>();
    if (m_binaryMarker)
        LOG_INFO("Binary Marker component loaded");
    m_imageFilterBinary =xpcfComponentManager->create<MODULES::OPENCV::SolARImageFilterBinaryOpencv>()->bindTo<image::IImageFilter>();
    if (m_imageFilterBinary)
        LOG_INFO("Image Filter component loaded");
    m_imageConvertor =xpcfComponentManager->create<MODULES::OPENCV::SolARImageConvertorOpencv>()->bindTo<image::IImageConvertor>();
    if (m_imageConvertor)
        LOG_INFO("Image Convertor component loaded");
    m_contoursExtractor =xpcfComponentManager->create<MODULES::OPENCV::SolARContoursExtractorOpencv>()->bindTo<features::IContoursExtractor>();
    if (m_contoursExtractor)
        LOG_INFO("Controus Extracor component loaded");
    m_contoursFilter =xpcfComponentManager->create<MODULES::OPENCV::SolARContoursFilterBinaryMarkerOpencv>()->bindTo<features::IContoursFilter>();
    if (m_contoursFilter)
        LOG_INFO("Contours Filter component loaded");
    m_perspectiveController =xpcfComponentManager->create<MODULES::OPENCV::SolARPerspectiveControllerOpencv>()->bindTo<image::IPerspectiveController>();
    if (m_perspectiveController)
        LOG_INFO("Perspective Controller component loaded");
    m_patternDescriptorExtractor =xpcfComponentManager->create<MODULES::OPENCV::SolARDescriptorsExtractorSBPatternOpencv>()->bindTo<features::IDescriptorsExtractorSBPattern>();
    if (m_patternDescriptorExtractor)
        LOG_INFO("Descriptor Extractor component loaded");
    m_patternMatcher =xpcfComponentManager->create<MODULES::OPENCV::SolARDescriptorMatcherRadiusOpencv>()->bindTo<features::IDescriptorMatcher>();
    if (m_patternMatcher)
        LOG_INFO("Pattern Matcher component loaded");
    m_patternReIndexer = xpcfComponentManager->create<MODULES::TOOLS::SolARSBPatternReIndexer>()->bindTo<features::ISBPatternReIndexer>();
    if (m_patternReIndexer)
        LOG_INFO("Pattern Reindexer component loaded");
    m_img2worldMapper = xpcfComponentManager->create<MODULES::TOOLS::SolARImage2WorldMapper4Marker2D>()->bindTo<geom::IImage2WorldMapper>();
    if (m_img2worldMapper)
        LOG_INFO("Image To World Mapper component loaded");
    m_PnP =xpcfComponentManager->create<MODULES::OPENCV::SolARPoseEstimationPnpOpencv>()->bindTo<solver::pose::I3DTransformFinderFrom2D3D>();
    if (m_PnP)
        LOG_INFO("PnP component loaded");
#ifdef USE_OPENGL
    m_sink = xpcfComponentManager->create<MODULES::OPENGL::SinkPoseTextureBuffer>()->bindTo<sink::ISinkPoseTextureBuffer>();
#else
    m_sink = xpcfComponentManager->create<MODULES::TOOLS::SolARBasicSink>()->bindTo<sink::ISinkPoseImage>();
#endif
    if (m_sink)
        LOG_INFO("Pose Texture Buffer Sink component loaded");

    if (m_camera && m_binaryMarker && m_imageFilterBinary && m_imageConvertor && m_contoursExtractor && m_contoursFilter && m_perspectiveController &&
        m_patternDescriptorExtractor && m_patternMatcher && m_patternReIndexer && m_img2worldMapper && m_PnP && m_sink)
    {
        LOG_INFO("All components have been created");
    }
    else
    {
        LOG_INFO("One or more components cannot be created");
        return FrameworkReturnCode::_ERROR_;
    }

    // load marker
    LOG_INFO("LOAD MARKER IMAGE ");
    m_binaryMarker->loadMarker();
    LOG_INFO("MARKER IMAGE LOADED");
    m_patternDescriptorExtractor->extract(m_binaryMarker->getPattern(), m_markerPatternDescriptor);
    LOG_INFO ("Marker pattern:\n {}", m_binaryMarker->getPattern()->getPatternMatrix())

    int patternSize = m_binaryMarker->getPattern()->getSize();

    m_patternDescriptorExtractor->bindTo<xpcf::IConfigurable>()->getProperty("patternSize")->setIntegerValue(patternSize);
    m_patternReIndexer->bindTo<xpcf::IConfigurable>()->getProperty("sbPatternSize")->setIntegerValue(patternSize);

    m_img2worldMapper->bindTo<xpcf::IConfigurable>()->getProperty("digitalWidth")->setIntegerValue(patternSize);
    m_img2worldMapper->bindTo<xpcf::IConfigurable>()->getProperty("digitalHeight")->setIntegerValue(patternSize);
    m_img2worldMapper->bindTo<xpcf::IConfigurable>()->getProperty("worldWidth")->setFloatingValue(m_binaryMarker->getSize().width);
    m_img2worldMapper->bindTo<xpcf::IConfigurable>()->getProperty("worldHeight")->setFloatingValue(m_binaryMarker->getSize().height);

    m_PnP->setCameraParameters(m_camera->getIntrinsicsParameters(), m_camera->getDistorsionParameters());

    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            m_pose(i,j)=0.f;

    m_initOK = true;
    return FrameworkReturnCode::_SUCCESS;
}

CameraParameters PipelineFiducialMarker::getCameraParameters()
{
    CameraParameters camParam;
    if (m_camera)
    {
        Sizei resolution = m_camera->getResolution();
        CamCalibration calib = m_camera->getIntrinsicsParameters();
        camParam.width = resolution.width;
        camParam.height = resolution.height;
        camParam.focalX = calib(0,0);
        camParam.focalY = calib(1,1);
        camParam.centerX = calib(2,0);
        camParam.centerY = calib(2,1);
    }
    return camParam;
}

bool PipelineFiducialMarker::processCamImage()
{
    SRef<Image>                     camImage, greyImage, binaryImage;
    std::vector<SRef<Contour2Df>>   contours;
    std::vector<SRef<Contour2Df>>   filtered_contours;
    std::vector<SRef<Image>>        patches;
    std::vector<SRef<Contour2Df>>   recognizedContours;
    SRef<DescriptorBuffer>          recognizedPatternsDescriptors;
    std::vector<DescriptorMatch>    patternMatches;
    std::vector<SRef<Point2Df>>     pattern2DPoints;
    std::vector<SRef<Point2Df>>     img2DPoints;
    std::vector<SRef<Point3Df>>     pattern3DPoints;

    if (m_stopFlag || !m_initOK || !m_startedOK)
        return false;

    bool poseComputed = false;

    if (m_camera->getNextImage(camImage) == SolAR::FrameworkReturnCode::_ERROR_LOAD_IMAGE)
    {
        LOG_WARNING("The camera cannot load any image");
        m_stopFlag = true;
        return false;
    }
    // Convert Image from RGB to grey
    m_imageConvertor->convert(camImage, greyImage, Image::ImageLayout::LAYOUT_GREY);

    // Convert Image from grey to black and white
    m_imageFilterBinary->filter(greyImage, binaryImage);

    // Extract contours from binary image
    m_contoursExtractor->extract(binaryImage, contours);

     // Filter 4 edges contours to find those candidate for marker contours
    m_contoursFilter->filter(contours, filtered_contours);

    // Create one warpped and cropped image by contour
    m_perspectiveController->correct(binaryImage, filtered_contours, patches);

    // test if this last image is really a squared binary marker, and if it is the case, extract its descriptor
    if (m_patternDescriptorExtractor->extract(patches, filtered_contours, recognizedPatternsDescriptors, recognizedContours) != FrameworkReturnCode::_ERROR_)
    {
        // From extracted squared binary pattern, match the one corresponding to the squared binary marker
        if (m_patternMatcher->match(m_markerPatternDescriptor, recognizedPatternsDescriptors, patternMatches) == features::DescriptorMatcher::DESCRIPTORS_MATCHER_OK)
        {
            // Reindex the pattern to create two vector of points, the first one corresponding to marker corner, the second one corresponding to the poitsn of the contour
            m_patternReIndexer->reindex(recognizedContours, patternMatches, pattern2DPoints, img2DPoints);

            // Compute the 3D position of each corner of the marker
            m_img2worldMapper->map(pattern2DPoints, pattern3DPoints);

            // Compute the pose of the camera using a Perspective n Points algorithm using only the 4 corners of the marker
            if (m_PnP->estimate(img2DPoints, pattern3DPoints, m_pose) == FrameworkReturnCode::_SUCCESS)
            {
                poseComputed = true;
            }
        }
    }

    if (poseComputed)
        m_sink->set(m_pose, camImage);
    else
        m_sink->set(camImage);

    return true;
}

#ifdef USE_OPENGL
FrameworkReturnCode PipelineFiducialMarker::start(void* textureHandle)
#else
FrameworkReturnCode PipelineFiducialMarker::start(void* imageDataBuffer)
#endif
{
    if (m_initOK==false)
    {
        LOG_WARNING("Try to start the Fiducial marker pipeline without initializing it");
        return FrameworkReturnCode::_ERROR_;
    }
    m_stopFlag=false;
#ifdef USE_OPENGL
    m_sink->setTextureBuffer(textureHandle);
#else
    m_sink->setImageBuffer((unsigned char*)imageDataBuffer);
#endif
    if (m_camera->start() != FrameworkReturnCode::_SUCCESS)
    {
        LOG_ERROR("Camera cannot start")
        return FrameworkReturnCode::_ERROR_;
    }

    // create and start a thread to process the images
    auto processCamImageThread = [this](){;processCamImage();};

    m_taskProcess = new xpcf::DelegateTask(processCamImageThread);
    m_taskProcess->start();
    LOG_INFO("Fiducial marker pipeline has started");
    m_startedOK = true;
    return FrameworkReturnCode::_SUCCESS;
}

FrameworkReturnCode PipelineFiducialMarker::stop()
{    
    m_camera->stop();
    if (m_taskProcess != nullptr)
        m_taskProcess->stop();

    if(!m_initOK)
    {
        LOG_WARNING("Try to stop a pipeline that has not been initialized");
        return FrameworkReturnCode::_ERROR_;
    }
    if (!m_startedOK)
    {
        LOG_WARNING("Try to stop a pipeline that has not been started");
        return FrameworkReturnCode::_ERROR_;
    }
    m_stopFlag=true;

    LOG_INFO("Pipeline has stopped: \n");

    return FrameworkReturnCode::_SUCCESS;
}

void PipelineFiducialMarker::updateFrameDataOGL(int enventID)
{
    return m_sink->updateFrameDataOGL(enventID);
}

SinkReturnCode PipelineFiducialMarker::update(Transform3Df& pose)
{
    return m_sink->tryUpdate(pose);
}



}
}
