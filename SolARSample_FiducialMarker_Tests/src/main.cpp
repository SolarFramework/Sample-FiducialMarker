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

//#include <core/Log.h>

//#include <boost/log/core.hpp>
#include <gtest/gtest.h>

// #include <filesystem>

//namespace fs = std::filesystem;

// using SolAR::Log;

// Experimental, commented out because gcc on CI does not implement filesystem
/*
class TestOutputFileLogger : public testing::EmptyTestEventListener {

  void OnTestStart(const testing::TestInfo& test_info) override {
      std::string stdOutFileName =
              fs::current_path().string() + "/"
               + std::string(test_info.test_suite_name()) + "_"
               + std::string(test_info.name()) + "_output.txt";

      LOG_ADD_LOG_TO_FILE(stdOutFileName.c_str(), "r");
      LOG_SET_DEBUG_LEVEL();
  }

  // Called after a failed assertion or a SUCCESS().
  void OnTestPartResult(const testing::TestPartResult& test_part_result) override {

  }

  // Called after a test ends.
  void OnTestEnd(const testing::TestInfo& test_info) override {
        LOG_RELEASE;
  }
};

*/

int main(int argc, char **argv) {

    ::testing::InitGoogleTest(&argc, argv);

//    Experimental
//    testing::TestEventListeners& listeners =
//        testing::UnitTest::GetInstance()->listeners();
//    listeners.Append(new TestOutputFileLogger());

    return RUN_ALL_TESTS();
}
