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

#ifndef SOLAR_STANDALONE_FIDUCIAL_MARKER_MONO
#define SOLAR_STANDALONE_FIDUCIAL_MARKER_MONO

#include <memory>
#include <string>

namespace SolAR::standalone
{

class SolARStandAloneFiducialMono
{
    public:
    class Builder
    {
        public:

        Builder& selectPlaybackMode(const std::string& configFileName, int timeoutInS);
        Builder& selectLiveMode(const std::string& configFileName);
        std::shared_ptr<SolARStandAloneFiducialMono> build();

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
    };

    int main_impl();

    private:

    enum class Mode
    {
        unset,
        live,
        playback
    };

    SolARStandAloneFiducialMono() = default;
    void selectPlaybackMode(const std::string& configFileName, int timeoutInS);
    void selectLiveMode(const std::string& configFileName);

    Mode m_mode = Mode::unset;
    std::string m_configFileName = "";
    int m_timeoutInS = -1;

    bool m_poseDetected = false;

    friend class Builder;
};

} // namespace SolAR::standalone

#endif // SOLAR_STANDALONE_FIDUCIAL_MARKER_MONO
