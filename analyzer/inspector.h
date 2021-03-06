/*

  Copyright (C) 2017 Gonzalo José Carracedo Carballal

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this program.  If not, see
  <http://www.gnu.org/licenses/>

*/

#ifndef _INSPECTOR_H
#define _INSPECTOR_H

#include <sigutils/sigutils.h>
#include <sigutils/agc.h>
#include <sigutils/pll.h>
#include <sigutils/clock.h>
#include <sigutils/detect.h>

#define SUHANDLE int32_t

#define SUSCAN_ANALYZER_CPU_USAGE_UPDATE_ALPHA .025

enum suscan_aync_state {
  SUSCAN_ASYNC_STATE_CREATED,
  SUSCAN_ASYNC_STATE_RUNNING,
  SUSCAN_ASYNC_STATE_HALTING,
  SUSCAN_ASYNC_STATE_HALTED
};

struct suscan_baud_det_result {
  SUFLOAT fac;
  SUFLOAT nln;
};

enum suscan_inspector_gain_control {
  SUSCAN_INSPECTOR_GAIN_CONTROL_MANUAL,
  SUSCAN_INSPECTOR_GAIN_CONTROL_AUTOMATIC
};

enum suscan_inspector_carrier_control {
  SUSCAN_INSPECTOR_CARRIER_CONTROL_MANUAL,
  SUSCAN_INSPECTOR_CARRIER_CONTROL_COSTAS_2,
  SUSCAN_INSPECTOR_CARRIER_CONTROL_COSTAS_4,
  SUSCAN_INSPECTOR_CARRIER_CONTROL_COSTAS_8,
};

enum suscan_inspector_matched_filter {
  SUSCAN_INSPECTOR_MATCHED_FILTER_BYPASS,
  SUSCAN_INSPECTOR_MATCHED_FILTER_MANUAL
};

enum suscan_inspector_baudrate_control {
  SUSCAN_INSPECTOR_BAUDRATE_CONTROL_MANUAL,
  SUSCAN_INSPECTOR_BAUDRATE_CONTROL_GARDNER
};

enum suscan_inspector_psd_source {
  SUSCAN_INSPECTOR_PSD_SOURCE_NONE,
  SUSCAN_INSPECTOR_PSD_SOURCE_FAC,
  SUSCAN_INSPECTOR_PSD_SOURCE_NLN
};

struct suscan_inspector_params {
  uint32_t inspector_id;

  /* Gain control parameters */
  enum suscan_inspector_gain_control gc_ctrl;
  SUFLOAT gc_gain;    /* Positive gain (linear) */

  /* Frequency control parameters */
  enum suscan_inspector_carrier_control fc_ctrl;
  SUFLOAT fc_off;     /* Offset frequency */
  SUFLOAT fc_phi;     /* Carrier phase */

  /* Matched filter parameters */
  enum suscan_inspector_matched_filter mf_conf;
  SUFLOAT mf_rolloff; /* Roll-off factor */

  /* Baudrate control parameters */
  enum suscan_inspector_baudrate_control br_ctrl;
  SUFLOAT br_alpha;   /* Baudrate control alpha (linear) */
  SUFLOAT br_beta;    /* Baudrate control beta (linear) */

  /* Spectrum source configuration */
  enum suscan_inspector_psd_source psd_source; /* Spectrum source */
  SUFLOAT sym_phase;  /* Symbol phase */
  SUFLOAT baud;       /* Baudrate */
};

/* TODO: protect baudrate access with mutexes */
struct suscan_inspector {
  struct sigutils_channel channel;
  SUFLOAT                 equiv_fs; /* Equivalent sample rate */
  su_channel_detector_t  *fac_baud_det; /* FAC baud detector */
  su_channel_detector_t  *nln_baud_det; /* Non-linear baud detector */
  su_agc_t                agc;      /* AGC, for sampler */
  su_costas_t             costas_2; /* 2nd order Costas loop */
  su_costas_t             costas_4; /* 4th order Costas loop */
  su_costas_t             costas_8; /* 8th order Costas loop */
  su_iir_filt_t           mf;       /* Matched filter (Root Raised Cosine) */
  su_clock_detector_t     cd;       /* Clock detector */
  su_ncqo_t               lo;       /* Oscillator for manual carrier offset */
  SUCOMPLEX               phase;    /* Local oscillator phase */

  /* Spectrum state */
  SUFLOAT                 interval_psd;
  SUSCOUNT                per_cnt_psd;
  SUBOOL                  pending;

  /* Inspector parameters */
  pthread_mutex_t params_mutex;
  struct suscan_inspector_params params;
  struct suscan_inspector_params params_request;
  SUBOOL    params_requested;
  SUBOOL    sym_new_sample;     /* New sample flag */
  SUCOMPLEX sym_last_sample;    /* Last sample fed to inspector */
  SUCOMPLEX sym_sampler_output; /* Sampler output */
  SUFLOAT   sym_phase;          /* Current sampling phase, in samples */
  SUFLOAT   sym_period;         /* In samples */

  enum suscan_aync_state state; /* Used to remove analyzer from queue */
};

typedef struct suscan_inspector suscan_inspector_t;


void suscan_inspector_destroy(suscan_inspector_t *chanal);

void suscan_inspector_params_initialize(
    struct suscan_inspector_params *params);

suscan_inspector_t *suscan_inspector_new(
    SUSCOUNT fs,
    const struct sigutils_channel *channel);

int suscan_inspector_feed_bulk(
    suscan_inspector_t *insp,
    const SUCOMPLEX *x,
    int count);

void suscan_inspector_request_params(
    suscan_inspector_t *insp,
    struct suscan_inspector_params *params_request);

void suscan_inspector_assert_params(suscan_inspector_t *insp);

#endif /* _INSPECTOR_H */
