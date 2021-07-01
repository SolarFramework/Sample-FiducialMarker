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

#include <gtest/gtest.h>

using SolAR::PIPELINES::runner::SolARPipelineFiducialMarkerRunner;

TEST(TestSolARPipelineFiducialMarkerRunner, testNominalPlayback)
{
    auto builder = SolARPipelineFiducialMarkerRunner::Builder()
                    .selectPlaybackMode("SolARPipeline_FiducialMarker_Runner_conf_test_001.xml",
                     /* timeoutInS = */ 2)
                    .disableDisplay();

    std::shared_ptr<SolARPipelineFiducialMarkerRunner> prog;
    ASSERT_NO_THROW(prog = builder.build());

    EXPECT_EQ(prog->run(), 0);

    EXPECT_TRUE(prog->isPoseDetected());
}

TEST(TestSolARPipelineFiducialMarkerRunner, testEmptyConfiguration)
{
    auto builder = SolARPipelineFiducialMarkerRunner::Builder()
                     .selectPlaybackMode("", 0)
                     .disableDisplay();

    ASSERT_THROW(builder.build(), std::runtime_error);
}

TEST(TestSolARPipelineFiducialMarkerRunner, testNonExistingConfiguration)
{
    auto builder = SolARPipelineFiducialMarkerRunner::Builder()
                     .selectPlaybackMode("bogus.xml", 0)
                     .disableDisplay();

    std::shared_ptr<SolARPipelineFiducialMarkerRunner> prog;
    ASSERT_NO_THROW(prog = builder.build());

    // Main has caught and handle the xpcf exception
    EXPECT_NE(prog->run(), 0);
}


// Add tests with other videos, configurations, ...








