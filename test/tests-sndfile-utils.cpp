
#include <sndfile.h>

#include <sndfile_wrapper/sndfile_utils.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("type_to_sf_type maps float to SF_FORMAT_FLOAT",
          "[sndfile_utils][types]") {
  CHECK(type_to_sf_type<float>() == SF_FORMAT_FLOAT);
}

TEST_CASE("type_to_sf_type maps double to SF_FORMAT_DOUBLE",
          "[sndfile_utils][types]") {
  CHECK(type_to_sf_type<double>() == SF_FORMAT_DOUBLE);
}

TEST_CASE("type_to_sf_type maps int to PCM_24 by default",
          "[sndfile_utils][types]") {
  CHECK(type_to_sf_type<int>() == SF_FORMAT_PCM_24);
}

TEST_CASE("type_to_sf_type supports custom integer subtypes",
          "[sndfile_utils][types]") {
  CHECK(type_to_sf_type<int, SF_FORMAT_PCM_S8>() == SF_FORMAT_PCM_S8);

  CHECK(type_to_sf_type<int, SF_FORMAT_PCM_16>() == SF_FORMAT_PCM_16);

  CHECK(type_to_sf_type<int, SF_FORMAT_PCM_32>() == SF_FORMAT_PCM_32);
}
