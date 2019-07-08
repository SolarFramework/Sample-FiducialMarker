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
XPCF_ADD_COMPONENT(SolAR::PIPELINES::PipelineFiducialMarker);
XPCF_END_COMPONENTS_DECLARATION

// The pipeline component for the fiducial marker
