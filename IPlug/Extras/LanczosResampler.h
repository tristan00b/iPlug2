
/*
 LanczosResampler derived from
 https://github.com/surge-synthesizer/sst-basic-blocks/blob/main/include/sst/basic-blocks/dsp/LanczosResampler.h
 
 * sst-basic-blocks - an open source library of core audio utilities
 * built by Surge Synth Team.
 *
 * Provides a collection of tools useful on the audio thread for blocks,
 * modulation, etc... or useful for adapting code to multiple environments.
 *
 * Copyright 2023, various authors, as described in the GitHub
 * transaction log. Parts of this code are derived from similar
 * functions original in Surge or ShortCircuit.
 *
 * sst-basic-blocks is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * All source in sst-basic-blocks available at
 * https://github.com/surge-synthesizer/sst-basic-blocks
 */

/*
 * A special note on licensing: This file (and only this file)
 * has Paul Walker (baconpaul) as the sole author to date.
 *
 * In order to make this handy small function based on public
 * information available to a set of open source projects
 * adapting hardware to software, but which are licensed under
 * MIT or BSD or similar licenses, this file and only this file
 * can be used in an MIT/BSD context as well as a GPL3 context, by
 * copying it and modifying it as you see fit.
 *
 * If you do that, you will need to replace the `sum_ps_to_float`
 * call below with either an hadd if you are SSE3 or higher or
 * an appropriate reduction operator from your toolkit.
 *
 * But basically: Need to resample 48k to variable rate with
 * a small window and want to use this? Go for it!
 *
 * For avoidance of doubt, this license exception only
 * applies to this file.
 */

#pragma once

#include <algorithm>
#include <utility>
#include <cmath>
#include <cstring>

#if defined(__arm64__)
#define SIMDE_ENABLE_NATIVE_ALIASES
#include "simde/x86/sse2.h"
#else
#include <emmintrin.h>
#endif

namespace iplug
{
/*
 * See https://en.wikipedia.org/wiki/Lanczos_resampling
 */

struct LanczosResampler
{
  inline __m128 sum_ps_to_ss(__m128 x) const
  {
    // FIXME: With SSE 3 this can be a dual hadd
    __m128 a = _mm_add_ps(x, _mm_movehl_ps(x, x));
    return _mm_add_ss(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(0, 0, 0, 1)));
  }

  inline float sum_ps_to_float(__m128 x) const
  {
    __m128 r = sum_ps_to_ss(x);
    float f;
    _mm_store_ss(&f, r);
    return f;
  }
  
  static constexpr size_t A = 4;
  static constexpr size_t BUFFER_SZ = 4096;
  static constexpr size_t filterWidth = A * 2;
  static constexpr size_t tableObs = 8192;
  static constexpr double dx = 1.0 / (tableObs);

  // Fixme: Make this static and shared
  static float lanczosTable alignas(16)[tableObs + 1][filterWidth], lanczosTableDX
    alignas(16)[tableObs + 1][filterWidth];
  static bool tablesInitialized;

  // This is a stereo resampler
  float input[2][BUFFER_SZ * 2];
  int wp = 0;
  float sri, sro;
  double phaseI, phaseO, dPhaseI, dPhaseO;

  inline double kernel(double x)
  {
    if (fabs(x) < 1e-7)
      return 1;
    return A * std::sin(M_PI * x) * std::sin(M_PI * x / A) / (M_PI * M_PI * x * x);
  }

  LanczosResampler(float inputRate, float outputRate) : sri(inputRate), sro(outputRate)
  {
    phaseI = 0;
    phaseO = 0;

    dPhaseI = 1.0;
    dPhaseO = sri / sro;

    memset(input, 0, 2 * BUFFER_SZ * sizeof(float));
    if (!tablesInitialized)
    {
      for (size_t t = 0; t < tableObs + 1; ++t)
      {
        double x0 = dx * t;
        for (size_t i = 0; i < filterWidth; ++i)
        {
          double x = x0 + i - A;
          lanczosTable[t][i] = kernel(x);
        }
      }
      for (size_t t = 0; t < tableObs; ++t)
      {
        for (size_t i = 0; i < filterWidth; ++i)
        {
          lanczosTableDX[t][i] =
            lanczosTable[t + 1][i] - lanczosTable[t][i];
        }
      }
      for (size_t i = 0; i < filterWidth; ++i)
      {
        // Wrap at the end - deriv is the same
        lanczosTableDX[tableObs][i] = lanczosTableDX[0][i];
      }
      tablesInitialized = true;
    }
  }

  inline void push(float fL, float fR)
  {
    input[0][wp] = fL;
    input[0][wp + BUFFER_SZ] = fL; // this way we can always wrap
    input[1][wp] = fR;
    input[1][wp + BUFFER_SZ] = fR;
    wp = (wp + 1) & (BUFFER_SZ - 1);
    phaseI += dPhaseI;
  }

  inline void read(double xBack, float &L, float &R) const
  {
    double p0 = wp - xBack;
    int idx0 = floor(p0);
    double off0 = 1.0 - (p0 - idx0);

    idx0 = (idx0 + BUFFER_SZ) & (BUFFER_SZ - 1);
    idx0 += (idx0 <= (int)A) * BUFFER_SZ;

    double off0byto = off0 * tableObs;
    int tidx = (int)(off0byto);
    double fidx = (off0byto - tidx);

    auto fl = _mm_set1_ps((float)fidx);
    auto f0 = _mm_load_ps(&lanczosTable[tidx][0]);
    auto df0 = _mm_load_ps(&lanczosTableDX[tidx][0]);

    f0 = _mm_add_ps(f0, _mm_mul_ps(df0, fl));

    auto f1 = _mm_load_ps(&lanczosTable[tidx][4]);
    auto df1 = _mm_load_ps(&lanczosTableDX[tidx][4]);
    f1 = _mm_add_ps(f1, _mm_mul_ps(df1, fl));

    auto d0 = _mm_loadu_ps(&input[0][idx0 - A]);
    auto d1 = _mm_loadu_ps(&input[0][idx0]);
    auto rv = _mm_add_ps(_mm_mul_ps(f0, d0), _mm_mul_ps(f1, d1));
    L = sum_ps_to_float(rv);

    d0 = _mm_loadu_ps(&input[1][idx0 - A]);
    d1 = _mm_loadu_ps(&input[1][idx0]);
    rv = _mm_add_ps(_mm_mul_ps(f0, d0), _mm_mul_ps(f1, d1));
    R = sum_ps_to_float(rv);
  }

  inline size_t inputsRequiredToGenerateOutputs(size_t desiredOutputs) const
  {
    /*
     * So (phaseI + dPhaseI * res - phaseO - dPhaseO * desiredOutputs) * sri > A + 1
     *
     * Use the fact that dPhaseI = sri and find
     * res > (A+1) - (phaseI - phaseO + dPhaseO * desiredOutputs) * sri
     */
    double res = A + 1 - (phaseI - phaseO - dPhaseO * desiredOutputs);

    return (size_t)std::max(res + 1, 0.0); // Check this calculation
  }

  size_t populateNext(float *fL, float *fR, size_t max);

  inline void advanceReadPointer(size_t n) { phaseO += n * dPhaseO; }
  inline void snapOutToIn()
  {
    phaseO = 0;
    phaseI = 0;
  }

  inline void renormalizePhases()
  {
    phaseI -= phaseO;
    phaseO = 0;
  }
};

float LanczosResampler::lanczosTable alignas(
  16)[LanczosResampler::tableObs + 1][LanczosResampler::filterWidth];
float LanczosResampler::lanczosTableDX alignas(
  16)[LanczosResampler::tableObs + 1][LanczosResampler::filterWidth];

bool LanczosResampler::tablesInitialized{false};

inline size_t LanczosResampler::populateNext(float *fL, float *fR, size_t max)
{
  int populated = 0;
  while (populated < max && (phaseI - phaseO) > A + 1)
  {
    read((phaseI - phaseO), fL[populated], fR[populated]);
    phaseO += dPhaseO;
    populated++;
  }
  return populated;
}

} // namespace iplug

