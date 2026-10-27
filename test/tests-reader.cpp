
#include <array>
#include <chrono>
#include <filesystem>
#include <sndfile.h>
#include <string>

#include <sndfile_wrapper/sndfile_wrapper.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

std::filesystem::path make_temp_path() {
  const auto id = std::chrono::steady_clock::now().time_since_epoch().count();

  return std::filesystem::temp_directory_path() /
         ("sndfile_reader_" + std::to_string(id) + ".wav");
}

struct TempFile {
  std::filesystem::path path = make_temp_path();

  ~TempFile() {
    std::error_code ec;
    std::filesystem::remove(path, ec);
  }
};

}  // namespace

TEST_CASE("SoundFileReader reads float samples", "[sndfile_reader][float]") {
  TempFile file;

  const std::array<float, 6> expected{-0.75f, -0.25f, 0.0f, 0.25f, 0.5f, 0.75f};

  {
    SoundFileWriter<float, SF_FORMAT_WAV> writer(file.path.string(), 16000, 1);

    REQUIRE(writer.write(expected.data(), expected.size()) ==
            static_cast<sf_count_t>(expected.size()));
  }

  SoundFileReader<float, SF_FORMAT_WAV> reader(file.path.string(), 16000, 1);

  CHECK(reader.frames() == 6);
  CHECK(reader.channels() == 1);

  std::array<float, 6> actual{};

  REQUIRE(reader.read(actual.data(), actual.size()) ==
          static_cast<sf_count_t>(actual.size()));

  for (std::size_t i = 0; i < expected.size(); ++i) {
    CHECK(actual[i] == Catch::Approx(expected[i]).margin(1e-6));
  }
}

TEST_CASE("SoundFileReader reads double samples", "[sndfile_reader][double]") {
  TempFile file;

  const std::array<double, 4> expected{-0.5, 0.0, 0.5, 0.75};

  {
    SoundFileWriter<double, SF_FORMAT_WAV> writer(file.path.string(), 44100, 1);

    REQUIRE(writer.write(expected.data(), expected.size()) ==
            static_cast<sf_count_t>(expected.size()));
  }

  SoundFileReader<double, SF_FORMAT_WAV> reader(file.path.string(), 44100, 1);

  std::array<double, 4> actual{};

  REQUIRE(reader.read(actual.data(), actual.size()) ==
          static_cast<sf_count_t>(actual.size()));

  for (std::size_t i = 0; i < expected.size(); ++i) {
    CHECK(actual[i] == Catch::Approx(expected[i]).margin(1e-6));
  }
}

TEST_CASE("SoundFileReader reads integer PCM samples",
          "[sndfile_reader][int]") {
  TempFile file;

  const std::array<int, 4> expected{-1073741824, -536870912, 0, 1073741824};

  {
    SoundFileWriter<int, SF_FORMAT_WAV, SF_FORMAT_PCM_24> writer(
        file.path.string(), 16000, 1);

    REQUIRE(writer.write(expected.data(), expected.size()) ==
            static_cast<sf_count_t>(expected.size()));
  }

  SoundFileReader<int, SF_FORMAT_WAV, SF_FORMAT_PCM_24> reader(
      file.path.string(), 16000, 1);

  std::array<int, 4> actual{};

  REQUIRE(reader.read(actual.data(), actual.size()) ==
          static_cast<sf_count_t>(actual.size()));

  for (std::size_t i = 0; i < expected.size(); ++i) {
    CHECK(actual[i] == expected[i]);
  }
}

TEST_CASE("SoundFileReader reports multichannel metadata",
          "[sndfile_reader][channels]") {
  TempFile file;

  // Four frames, two channels, eight interleaved samples.
  const std::array<float, 8> samples{0.1f, -0.1f, 0.2f, -0.2f,
                                     0.3f, -0.3f, 0.4f, -0.4f};

  {
    SoundFileWriter<float, SF_FORMAT_WAV> writer(file.path.string(), 48000, 2);

    REQUIRE(writer.write(samples.data(), samples.size()) ==
            static_cast<sf_count_t>(samples.size()));
  }

  SoundFileReader<float, SF_FORMAT_WAV> reader(file.path.string(), 48000, 2);

  CHECK(reader.frames() == 4);
  CHECK(reader.channels() == 2);
}

TEST_CASE("SoundFileReader can read fewer samples than the file contains",
          "[sndfile_reader][partial]") {
  TempFile file;

  const std::array<float, 6> samples{0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f};

  {
    SoundFileWriter<float, SF_FORMAT_WAV> writer(file.path.string(), 16000, 1);

    REQUIRE(writer.write(samples.data(), samples.size()) ==
            static_cast<sf_count_t>(samples.size()));
  }

  SoundFileReader<float, SF_FORMAT_WAV> reader(file.path.string(), 16000, 1);

  std::array<float, 2> actual{};

  CHECK(reader.read(actual.data(), actual.size()) == 2);
  CHECK(actual[0] == Catch::Approx(0.1f).margin(1e-6));
  CHECK(actual[1] == Catch::Approx(0.2f).margin(1e-6));

  // The reader should continue from the current file position.
  CHECK(reader.read(actual.data(), actual.size()) == 2);
  CHECK(actual[0] == Catch::Approx(0.3f).margin(1e-6));
  CHECK(actual[1] == Catch::Approx(0.4f).margin(1e-6));
}
