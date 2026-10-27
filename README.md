# SoundFile Utilities

A lightweight, header-only C++ library for reading and writing audio files using [libsndfile](https://libsndfile.github.io/libsndfile/).

The library provides simple template-based wrappers around libsndfile, allowing audio data to be read from and written to files using familiar C++ types.

## Features

- **Type mapping:** Maps C++ sample types to libsndfile subtypes.
- **Audio writing:** Write audio samples to files using `SoundFileWriter`.
- **Audio reading:** Read audio samples from files using `SoundFileReader`.
- **Multiple sample types:** Supports `float`, `double`, and `int`.
- **Configurable formats:** Specify the audio container format and integer PCM subtype at compile time.
- **Multichannel audio:** Supports interleaved audio samples for mono, stereo, and other channel configurations.
- **Metadata access:** Retrieve the number of frames and channels from a reader.

## Requirements

- C++20 or later
- [libsndfile](https://libsndfile.github.io/libsndfile/) development headers and library

## Installation

This is a header-only library. Add the headers to your project and link against libsndfile.

Example project structure:

```text
your_project/
├── include/
│   ├── sndfile_utils.hpp
│   ├── SoundFileWriter.hpp
│   └── SoundFileReader.hpp
└── main.cpp
```

The header filenames above are examples; adjust them to match your project.

### CMake

```cmake
FetchContent_Declare(
  sndfile_wrapper
  GIT_REPOSITORY https://github.com/grybouilli/sndfile_wrapper.git
  GIT_TAG main)
FetchContent_MakeAvailable(sndfile_wrapper)
# ...

target_link_libraries(my_app PRIVATE sndfile_wrapper)
```

## Usage

### 1. Mapping C++ types to libsndfile subtypes

The `type_to_sf_type<T, IntType>()` function maps a C++ sample type to its corresponding libsndfile subtype.

```cpp
#include <sndfile_utils.hpp>
#include <sndfile.h>

int float_type = type_to_sf_type<float>();
// SF_FORMAT_FLOAT

int double_type = type_to_sf_type<double>();
// SF_FORMAT_DOUBLE

int int_type = type_to_sf_type<int>();
// SF_FORMAT_PCM_24
```

For integer samples, the subtype can be selected explicitly:

```cpp
int type = type_to_sf_type<int, SF_FORMAT_PCM_16>();
// SF_FORMAT_PCM_16
```

The default integer subtype is `SF_FORMAT_PCM_24`.

The returned value represents the **subtype**, not a complete file format. It is combined with a container format, such as `SF_FORMAT_WAV`, when opening a file.

Supported C++ types:

| C++ type | libsndfile subtype                                           |
| -------- | ------------------------------------------------------------ |
| `float`  | `SF_FORMAT_FLOAT`                                            |
| `double` | `SF_FORMAT_DOUBLE`                                           |
| `int`    | Configurable integer subtype; defaults to `SF_FORMAT_PCM_24` |

### 2. Writing audio files

`SoundFileWriter<T, Format, IntType>` writes audio samples to a file.

Template parameters:

| Parameter | Description                                   |
| --------- | --------------------------------------------- |
| `T`       | C++ sample type: `float`, `double`, or `int`  |
| `Format`  | Container format, such as `SF_FORMAT_WAV`     |
| `IntType` | Optional integer PCM subtype; defaults to `0` |

The `IntType` template parameter is passed to `type_to_sf_type`. For `int`, choose a valid integer subtype explicitly when needed. For `float` and `double`, the subtype is determined by the sample type.

#### Example

```cpp
#include <array>
#include <sndfile.h>
#include <SoundFileWriter.hpp>

int main() {
    const std::array<float, 8> samples{
        -0.8f, -0.5f, -0.2f, 0.0f,
         0.2f,  0.5f,  0.8f, 0.0f
    };

    SoundFileWriter<float, SF_FORMAT_WAV> writer(
        "output.wav",
        16000,  // Sample rate in Hz
        1       // Number of channels
    );

    sf_count_t written =
        writer.write(samples.data(), samples.size());

    // written is the number of sample items written.
}
```

The file is closed when the writer is destroyed.

#### Multichannel audio

Samples must be provided in interleaved order. For stereo audio, the data layout is:

```text
Left 0, Right 0, Left 1, Right 1, ...
```

For example:

```cpp
const std::array<float, 8> stereo_samples{
    0.1f, -0.1f,
    0.2f, -0.2f,
    0.3f, -0.3f,
    0.4f, -0.4f
};

SoundFileWriter<float, SF_FORMAT_WAV> writer(
    "stereo.wav", 48000, 2);

writer.write(stereo_samples.data(), stereo_samples.size());
```

This represents four frames of stereo audio.

### 3. Reading audio files

`SoundFileReader<T, Format, IntType>` reads samples from an existing audio file.

#### Example

```cpp
#include <array>
#include <iostream>
#include <sndfile.h>
#include <SoundFileReader.hpp>

int main() {
    SoundFileReader<float, SF_FORMAT_WAV> reader(
        "output.wav",
        16000,
        1
    );

    std::cout << "Frames: " << reader.frames() << '\n';
    std::cout << "Channels: " << reader.channels() << '\n';

    std::array<float, 8> samples{};

    sf_count_t count =
        reader.read(samples.data(), samples.size());

    std::cout << "Samples read: " << count << '\n';
}
```

The reader exposes two metadata accessors:

| Method       | Description                                   |
| ------------ | --------------------------------------------- |
| `frames()`   | Number of audio frames reported by libsndfile |
| `channels()` | Number of channels in the opened file         |

The sample rate and channel count supplied to the constructor are used as part of the `SF_INFO` structure passed to `sf_open`. When reading, libsndfile uses this structure to receive the file metadata.

### 4. Reading audio in blocks

The `read()` method can be used repeatedly to process audio in blocks instead of loading an entire file into memory.

```cpp
#include <array>
#include <sndfile.h>
#include <SoundFileReader.hpp>

int main() {
    SoundFileReader<float, SF_FORMAT_WAV> reader(
        "input.wav", 16000, 1);

    constexpr std::size_t block_size = 512;
    std::array<float, block_size> buffer{};

    sf_count_t count;

    while ((count = reader.read(
                buffer.data(), buffer.size())) > 0) {
        // Process buffer[0] through buffer[count - 1].
    }
}
```

The return value of `read()` is the number of sample items read. For multichannel audio, this count includes samples from all channels, not just the number of frames.

## API reference

### `type_to_sf_type<T, IntType>()`

Returns the libsndfile subtype associated with the given C++ sample type.

### `SoundFileWriter<T, Format, IntType>`

Constructor:

```cpp
SoundFileWriter(
    const std::string filepath,
    int sample_rate,
    int channels);
```

Method:

```cpp
sf_count_t write(const T* data, std::size_t length);
```

Writes up to `length` sample items and returns the number written.

### `SoundFileReader<T, Format, IntType>`

Constructor:

```cpp
SoundFileReader(
    const std::string filepath,
    int sample_rate,
    int channels);
```

Methods:

```cpp
sf_count_t read(T* data, std::size_t length);

int frames() const&;

int channels() const&;
```

`read()` returns the number of sample items read. `frames()` and `channels()` return the corresponding metadata.

## Testing

The project can be tested using [Catch2](https://github.com/catchorg/Catch2) and libsndfile.

Suggested test coverage includes:

- C++ type-to-subtype mapping
- Writing and reading `float`, `double`, and integer PCM samples
- Sample-rate and channel metadata
- Mono and multichannel audio
- Partial reads and sequential block reading
- Handling invalid paths and file-opening failures

Temporary WAV files are suitable for round-trip tests without requiring external audio assets.

## Limitations and considerations

- **Supported sample types:** Only `float`, `double`, and `int` are implemented.
- **Sample count:** Read and write lengths are sample-item counts, not frame counts. Multiply the number of frames by the number of channels when calculating the required number of interleaved samples.
- **File format:** The container format and sample subtype must be compatible. libsndfile may reject unsupported combinations.
- **Error handling:** The current implementation does not explicitly check whether `sf_open()` succeeds. Callers should not use a reader or writer if the file failed to open; adding constructor-level error handling is recommended.
- **Resource ownership:** The reader and writer close their file handles in their destructors. They should not be copied unless ownership semantics are implemented explicitly.
- **Integer conversion:** libsndfile converts between the supplied C++ sample type and the file's stored sample representation. Integer PCM formats have different bit depths and representable ranges.

## License

Add the license applicable to your project here.
