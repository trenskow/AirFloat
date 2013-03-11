//
//  audiooutput.h
//  AirFloat
//
//  Created by Kristian Trenskow on 1/21/12.
//  Copyright (c) 2012 The Famous Software Company. All rights reserved.
//

#ifndef audiooutput_h
#define audiooutput_h

#include <stdint.h>

#include "decoder.h"

typedef struct audio_output_t *audio_output_p;

void audio_output_set_volume(audio_output_p ao, double volume);
void audio_output_set_muted(audio_output_p ao, bool muted);

#endif
