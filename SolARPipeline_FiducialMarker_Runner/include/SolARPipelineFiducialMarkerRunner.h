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

#ifndef SOLAR_PIPELINE_FIDUCIAL_MARKER_RUNNER
#define SOLAR_PIPELINE_FIDUCIAL_MARKER_RUNNER

#include <memory>
#include <string>

namespace SolAR::PIPELINES::runner
{

class SolARPipelineFiducialMarkerRunner
{
    public:
    class Builder
    {
        public:

        Builder& selectPlaybackMode(const std::string& configFileName, int timeoutInS);
        Builder& selectLiveMode(const std::string& configFileName);
        Builder& disableDisplay();
        std::shared_ptr<SolARPipelineFiducialMarkerRunner> build();

        private:

        enum class Mode
        {
            unset,
            live,
            playback
        };

        Mode m_mode = Mode::unset ;
        std::string m_configFileName = "";
        int m_timeoutInS = -1;
        bool m_displayEnabled = true;
    };

    int run();
    bool isPoseDetected();

    private:

    enum class Mode
    {
        unset,
        live,
        playback
    };

    SolARPipelineFiducialMarkerRunner() = default;
    void selectPlaybackMode(const std::string& configFileName, int timeoutInS);
    void selectLiveMode(const std::string& configFileName);
    void disableDisplay();

    Mode m_mode = Mode::unset;
    std::string m_configFileName = "";
    int m_timeoutInS = -1;

    bool m_poseDetected = false;

    bool m_displayEnabled = true;

    friend class Builder;
};

} // namespace SolAR::PIPELINES::runner

#endif // SOLAR_PIPELINE_FIDUCIAL_MARKER_RUNNER
