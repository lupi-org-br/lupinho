#include "fur_player.h"

#include <climits>
#include <cstring>
#include "engine.h"
#include "ta-log.h"

#include <mutex>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static DivEngine *engine = NULL;
static std::recursive_mutex engine_mu;
static int engine_ready = 0;
static int sample_rate = 44100;
static float *chan_l = NULL;
static float *chan_r = NULL;
static unsigned int chan_cap = 0;

void reportError(String what) {
    fprintf(stderr, "furnace: %s\n", what.c_str());
}

static void ensure_config_dir(void) {
#if !defined(_WIN32)
    const char *dir = "/tmp/lupinho-furnace";
    setenv("XDG_CONFIG_HOME", dir, 1);
    mkdir(dir, 0755);
#endif
}

static int ensure_channels(unsigned int frames) {
    if (frames <= chan_cap) return 1;
    float *nl = new (std::nothrow) float[frames];
    float *nr = new (std::nothrow) float[frames];
    if (nl == NULL || nr == NULL) {
        delete[] nl;
        delete[] nr;
        return 0;
    }
    delete[] chan_l;
    delete[] chan_r;
    chan_l = nl;
    chan_r = nr;
    chan_cap = frames;
    return 1;
}

int fur_player_init(void) {
    std::lock_guard<std::recursive_mutex> lock(engine_mu);
    if (engine != NULL) return 1;

    ensure_config_dir();
    initLog(stdout);
    logLevel = LOGLEVEL_ERROR;

    engine = new (std::nothrow) DivEngine();
    if (engine == NULL) return 0;

    engine->setConsoleMode(true, false);
    engine->setAudio(DIV_AUDIO_DUMMY);
    // preInit returns true only when the engine wants safe mode, which we ignore.
    engine->preInit(true);
    engine->setConf("audioRate", 44100);
    engine->setConf("audioBufSize", 1024);
    engine->setConf("audioChans", 2);
    return 1;
}

void fur_player_shutdown(void) {
    std::lock_guard<std::recursive_mutex> lock(engine_mu);
    if (engine != NULL) {
        engine->quit(false);
        delete engine;
        engine = NULL;
    }
    delete[] chan_l;
    delete[] chan_r;
    chan_l = NULL;
    chan_r = NULL;
    chan_cap = 0;
    engine_ready = 0;
}

int fur_player_sample_rate(void) {
    return sample_rate;
}

int fur_player_load(const unsigned char *data, size_t len, const char *name) {
    if (data == NULL || len == 0) return 0;
    std::lock_guard<std::recursive_mutex> lock(engine_mu);
    if (engine == NULL && !fur_player_init()) return 0;
    if (engine == NULL) return 0;

    unsigned char *copy = new (std::nothrow) unsigned char[len];
    if (copy == NULL) return 0;
    memcpy(copy, data, len);

    if (!engine->load(copy, len, name != NULL ? name : "song.fur")) {
        fprintf(stderr, "music: %s\n", engine->getLastError().c_str());
        return 0;
    }

    if (!engine_ready) {
        if (!engine->init()) {
            fprintf(stderr, "music: furnace init failed (%s)\n", engine->getLastError().c_str());
            return 0;
        }
        double rate = engine->getAudioDescGot().rate;
        if (rate >= 8000.0 && rate <= 192000.0) sample_rate = (int)rate;
        engine_ready = 1;
    }

    engine->setLoops(-1);
    if (!engine->play()) {
        fprintf(stderr, "music: furnace play failed\n");
        return 0;
    }
    return 1;
}

void fur_player_stop(void) {
    std::lock_guard<std::recursive_mutex> lock(engine_mu);
    if (engine != NULL && engine_ready) engine->stop();
}

int fur_player_playing(void) {
    std::lock_guard<std::recursive_mutex> lock(engine_mu);
    if (engine == NULL || !engine_ready) return 0;
    return engine->isPlaying() ? 1 : 0;
}

void fur_player_render(float *interleaved, unsigned int frames) {
    if (interleaved == NULL || frames == 0) return;
    // Furnace sizes some internal mix buffers for one audio period (export uses 2048).
    const unsigned int chunk = 2048;
    unsigned int done = 0;
    while (done < frames) {
        unsigned int n = frames - done;
        if (n > chunk) n = chunk;
        float *dst = interleaved + done * 2;

        std::lock_guard<std::recursive_mutex> lock(engine_mu);
        if (engine == NULL || !engine_ready || !engine->isPlaying() || !ensure_channels(n)) {
            memset(dst, 0, (size_t)n * 2 * sizeof(float));
        } else {
            float *out[2] = { chan_l, chan_r };
            engine->nextBuf(NULL, out, 0, 2, n, true);
            for (unsigned int i = 0; i < n; i++) {
                float l = chan_l[i];
                float r = chan_r[i];
                if (l > 1.0f) l = 1.0f;
                if (l < -1.0f) l = -1.0f;
                if (r > 1.0f) r = 1.0f;
                if (r < -1.0f) r = -1.0f;
                dst[i * 2] = l;
                dst[i * 2 + 1] = r;
            }
        }
        done += n;
    }
}
