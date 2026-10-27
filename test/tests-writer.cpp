
#include <array>
#include <chrono>
#include <filesystem>
#include <sndfile.h>
#include <string>

#include <sndfile_wrapper/sound_file_writer.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

std::filesystem::path make_temp_path() {
  const auto id = std::chrono::steady_clock::now().time_since_epoch().count();

  return std::filesystem::temp_directory_path() /
         ("sndfile_writer_" + std::to_string(id) + ".wav");
}

struct TempFile {
  std::filesystem::path path = make_temp_path();

  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path, ec);
  }
};

}  // namespace

TEST_CASE("SoundFileWriter writes float samples", "[sndfile_writer][float]") {
  TempFile file;

  const std::array<float, 8> samples{-1.0f, -0.75f, -0.5f, -0.25f,
                                     0.0f,  0.25f,  0.5f,  0.75f};

  {
    SoundFileWriter<float, SF_FORMAT_WAV> writer(file.path.string(), 16000, 1);

    CHECK(writer.write(samples.data(), samples.size()) ==
          static_cast<sf_count_t>(samples.size()));
  }

  REQUIRE(std::filesystem::exists(file.path));
  CHECK(std::filesystem::file_size(file.path) > 0);
}

TEST_CASE("SoundFileWriter writes double samples", "[sndfile_writer][double]") {
  TempFile file;

  const std::array<double, 4> samples{-0.5, 0.0, 0.5, 0.75};

  {
    SoundFileWriter<double, SF_FORMAT_WAV> writer(file.path.string(), 44100, 1);

    CHECK(writer.write(samples.data(), samples.size()) ==
          static_cast<sf_count_t>(samples.size()));
  }

  REQUIRE(std::filesystem::exists(file.path));
  CHECK(std::filesystem::file_size(file.path) > 0);
}

TEST_CASE("SoundFileWriter writes integer PCM samples",
          "[sndfile_writer][int]") {
  TempFile file;

  const std::array<int, 4> samples{-1073741824, -536870912, 0, 1073741824};

  {
    SoundFileWriter<int, SF_FORMAT_WAV, SF_FORMAT_PCM_24> writer(
        file.path.string(), 16000, 1);

    CHECK(writer.write(samples.data(), samples.size()) ==
          static_cast<sf_count_t>(samples.size()));
  }

  REQUIRE(std::filesystem::exists(file.path));
  CHECK(std::filesystem::file_size(file.path) > 0);
}

TEST_CASE("SoundFileWriter writes multichannel audio",
          "[sndfile_writer][channels]") {
  TempFile file;

  // Four frames of stereo audio, interleaved L/R.
  const std::array<float, 8> samples{0.1f, -0.1f, 0.2f, -0.2f,
                                     0.3f, -0.3f, 0.4f, -0.4f};

  {
    SoundFileWriter<float, SF_FORMAT_WAV> writer(file.path.string(), 48000, 2);

    CHECK(writer.write(samples.data(), samples.size()) ==
          static_cast<sf_count_t>(samples.size()));
  }

  REQUIRE(std::filesystem::exists(file.path));

  SF_INFO info{};
  SNDFILE* handle = sf_open(file.path.c_str(), SFM_READ, &info);

  REQUIRE(handle != nullptr);

  CHECK(info.samplerate == 48000);
  CHECK(info.channels == 2);
  CHECK(info.frames == 4);

  sf_close(handle);
}
