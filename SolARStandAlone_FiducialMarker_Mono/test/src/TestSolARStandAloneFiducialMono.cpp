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

#include "SolARStandAloneFiducialMono.h"

#include <gtest/gtest.h>

using SolAR::standalone::SolARStandAloneFiducialMono;

TEST(SolARStandAloneFiducialMono, testNominalPlayback)
{
    auto builder = SolARStandAloneFiducialMono::Builder()
                    .selectPlaybackMode("SolARStandalone_FiducialMarker_Mono_conf_test_001.xml",
                     /* timeoutInS = */ 2);

    std::shared_ptr<SolARStandAloneFiducialMono> prog;
    ASSERT_NO_THROW(prog = builder.build());

    EXPECT_EQ(prog->main_impl(), 0);
}


