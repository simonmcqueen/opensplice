/*
 *  DDSTouchStone: a scenario-driven Open Source benchmarking framework
 *  for evaluating the performance of OMG DDS compliant implementations.
 *
 *  Copyright (C) 2008-2009 PrismTech Ltd.
 *  ddstouchstone@prismtech.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License Version 3 dated 29 June 2007, as published by the
 *  Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with DDSTouchStone; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <stdlib.h>

#include "metrics.h"

#define MAX_SAMPLES 10000

/* --------------------------- private functions --------------------- */

struct metrics_t
{
    int    count;
    double stamps[MAX_SAMPLES];
};

static int
compare_stamp(
    const void *v1,
    const void *v2)
{
    double d1, d2;
    int result = 0;

    assert(v1 != NULL);
    assert(v2 != NULL);

    d1 = *((double *)v1);
    d2 = *((double *)v2);

    if (d1 > d2) result = 1;
    if (d2 > d1) result = -1;
    return result;
}

static void
metrics_report (
    metrics _this,
    int cnt,
    double prc,
    DDSTouchStone_metricsReport *report )
{
    double *lst = _this->stamps;
    double sum, min, avg, max, std;
    int i, rng;

    if (cnt <= 0) {
        report->sample_count = 0;
        report->minimum = 0;
        report->average = 0;
        report->maximum = 0;
        report->deviation = 0;
    } else {
        rng = (cnt*prc)/100;
        min = lst[0];
        max = lst[rng-1];
        sum = 0.0;
        for (i=0; i<rng; i++) {
            sum += lst[i];
        }
        avg = sum / (double)rng;
        sum = 0.0;
        for (i=0; i<rng; i++) {
            sum += ((lst[i]-avg)*(lst[i]-avg));
        }
        std = sqrt(sum/(double)rng);
        report->percentile = prc;
        report->sample_count = rng;
        report->minimum = min;
        report->average = avg;
        report->maximum = max;
        report->deviation = std;
    }
}



/* -------------------------- public functions ----------------- */

void
metrics_analyse (
    metrics _this,
    DDSTouchStone_metricsReport *report)
{
    int count,size;

    count = _this->count;
    size = sizeof(_this->stamps[0]);

    qsort(&_this->stamps,count,size,compare_stamp);

    metrics_report(_this, count, 100,   &report[0]);
    metrics_report(_this, count,  99.9, &report[1]);
    metrics_report(_this, count,  99.0, &report[2]);
    metrics_report(_this, count,  90.0, &report[3]);
}


void
metrics_add_stamp (
    metrics _this,
    double stamp )
{
    if (_this->count < MAX_SAMPLES) {
        _this->stamps[_this->count++] = stamp;
    }
}

static void
metrics_init (
    metrics _this )
{
    _this->count = 0;
}

metrics
metrics_new(
    void)
{
    metrics result;

    result = malloc(sizeof(*result));
    if (result) {
        metrics_init(result);
    }

    return result;
}

void
metrics_free(
    metrics _this)
{
    if (_this != NULL) {
        free(_this);
    }
}
