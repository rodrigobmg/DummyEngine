#include "DummyApp.h"

#include <fstream>

enum class eWaveFormat : uint16_t {
    PCM = 0x0001,
    IEEE_FLOAT = 0x0003,
    ALAW = 0x0006,
    MULAW = 0x0007,
    EXTENSIBLE = 0xFFFE
};

struct WaveChunk {
    char id[4];
    uint32_t size;
    eWaveFormat format;
    uint16_t channels_count;
    uint32_t samples_per_second;
    uint32_t bytes_per_second;
    uint16_t block_align;
    uint16_t bits_per_sample;
    uint16_t ext_size;
    uint16_t valid_bits_per_sample;
    uint32_t channel_mask;
    char sub_format[16];
};
static_assert(sizeof(WaveChunk) == 48, "!");

int LoadWAV(std::istream &in_data, int &channels, int &samples_per_second,
             int &bits_per_sample, std::unique_ptr<uint8_t[]> &samples) {
    { // check identifiers
        char chunk_id[4];
        if (!in_data.read(chunk_id, 4)) {
            return 0;
        }
        if (chunk_id[0] != 'R' || chunk_id[1] != 'I' || chunk_id[2] != 'F' ||
            chunk_id[3] != 'F') {
            return 0;
        }

        uint32_t chunk_size;
        if (!in_data.read((char *)&chunk_size, sizeof(uint32_t))) {
            return 0;
        }

        char wave_id[4];
        if (!in_data.read(wave_id, 4)) {
            return 0;
        }
        if (wave_id[0] != 'W' || wave_id[1] != 'A' || wave_id[2] != 'V' ||
            wave_id[3] != 'E') {
            return 0;
        }
    }

    WaveChunk chunk = {};
    if (!in_data.read((char *)&chunk, 8)) {
        return 0;
    }
    if (chunk.id[0] != 'f' || chunk.id[1] != 'm' || chunk.id[2] != 't' ||
        chunk.id[3] != ' ') {
        return 0;
    }

    if (!in_data.read((char *)&chunk.format, chunk.size)) {
        return 0;
    }

    if (chunk.format != eWaveFormat::PCM) {
        // not supported
        return 0;
    }

    char chunk_id[4];
    if (!in_data.read(chunk_id, 4)) {
        return 0;
    }

    if (chunk_id[0] == 'f' && chunk_id[1] == 'a' && chunk_id[2] == 'c' &&
        chunk_id[3] == 't') {
        uint32_t chunk_size;
        if (!in_data.read((char *)&chunk_size, sizeof(uint32_t))) {
            return 0;
        }

        // skip
        if (!in_data.seekg(chunk_size, std::ios::cur)) {
            return 0;
        }

        if (!in_data.read(chunk_id, 4)) {
            return 0;
        }
    }

    if (chunk_id[0] != 'd' || chunk_id[1] != 'a' || chunk_id[2] != 't' ||
        chunk_id[3] != 'a') {
        return 0;
    }

    uint32_t chunk_size;
    if (!in_data.read((char *)&chunk_size, sizeof(uint32_t))) {
        return 0;
    }

    samples.reset(new uint8_t[chunk_size]);
    if (!in_data.read((char *)&samples[0], chunk_size)) {
        return 0;
    }

    channels = (int)chunk.channels_count;
    samples_per_second = (int)chunk.samples_per_second;
    bits_per_sample = (int)chunk.bits_per_sample;

    return (int)chunk_size;
}

#include <Snd/OpenAL/include/al.h>
#include <Snd/OpenAL/include/alc.h>

#include <Snd/Context.h>
#include <Eng/Log.h>

#undef main
int main(int argc, char *argv[]) {
    LogStdout log;

    Snd::Context ctx;
    ctx.Init(&log);

    /////////////////////////////////////////////////////////////////////////

    std::ifstream in_file("assets/sounds/africa-toto.wav", std::ios::binary);
    if (!in_file) {
        return -1;
    }

    int channels, samples_per_second, bits_per_sample;
    std::unique_ptr<uint8_t[]> samples;
    const int size = LoadWAV(in_file, channels, samples_per_second, bits_per_sample, samples);
    if (!size) {
        return -1;
    }

    /////////////////////////////////////////////////////////////////////////

    ALuint buf;
    alGenBuffers(1, &buf);

    ALenum format;
    if (channels == 1 && bits_per_sample == 8) {
        format = AL_FORMAT_MONO8;
    } else if (channels == 1 && bits_per_sample == 16) {
        format = AL_FORMAT_MONO16;
    } else if (channels == 2 && bits_per_sample == 8) {
        format = AL_FORMAT_STEREO8;
    } else if (channels == 2 && bits_per_sample == 16) {
        format = AL_FORMAT_STEREO16;
    } else {
        return -1;
    }

    alBufferData(buf, format, &samples[0], size, samples_per_second);

    ALuint source;
    alGenSources(1, &source);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1.0f);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSourcei(source, AL_BUFFER, buf);

    alSourcePlay(source);

    ALint state = AL_PLAYING;
    while (state == AL_PLAYING) {
        alGetSourcei(source, AL_SOURCE_STATE, &state);
    }

    return 0;

    return DummyApp().Run(argc, argv);
}

// TODO:
// refactor probe cache loading
// fix exposure flicker
// use texture array for lightmaps
// texture streaming
// use stencil to distinguich ssr/nossr regions
// velocities for skinned meshes
// use GL_EXT_shader_group_vote
// refactor msaa (resolve once, remove permutations)
// refactor file read on android
// start with scene editing
// use direct state access extension
// add assetstream
// get rid of SDL in Modl app
// make full screen quad passes differently
// refactor repetitive things in shaders
// use frame graph approach in renderer
// check GL_QCOM_alpha_test extension (for depth prepass and shadow rendering)
// check GL_QCOM_tiled_rendering extension
// try to use texture views to share framebuffers texture memory (GL_OES_texture_view)
// use one big array for instance indices
// get rid of SOIL in Ren (??? png loading left)
