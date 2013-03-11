//
//  decoder_alac_mac.c
//  AirFloat
//
//  Created by Kristian Trenskow on 2/25/13.
//
//

#if (defined(__APPLE__) && !defined(ALAC_SOFTWARE))

#include <stdint.h>

#include <AudioToolbox/AudioToolbox.h>

#include "log.h"
#include "endian.h"
#include "decoder.h"
#include "alac_format.h"

#define MIN(x, y) (x < y ? x : y)
#define ca_assert(error) assert((error) == noErr)

struct decoder_alac_mac_t {
    struct alac_magic_cookie_t magic_cookie;
    struct decoder_output_format_t output_format;
    AudioStreamBasicDescription out_desc;
    AudioConverterRef converter_ref;
    AudioStreamPacketDescription packet_description;
    AudioBuffer audio_buffer;
    void* buffer_p;
    size_t buffer_size;
};

void* decoder_alac_create(const char* rtp_fmtp) {
    
    struct decoder_alac_mac_t* d = (struct decoder_alac_mac_t*)malloc(sizeof(struct decoder_alac_mac_t));
    bzero(d, sizeof(struct decoder_alac_mac_t));
    
    d->magic_cookie = alac_format_parse(rtp_fmtp);
    
    struct alac_specific_config_t config = d->magic_cookie.alac_specific_info.config;
    
    AudioStreamBasicDescription in_desc;
    bzero(&in_desc, sizeof(AudioStreamBasicDescription));
    
    in_desc.mFormatID = kAudioFormatAppleLossless;
    in_desc.mSampleRate = btml(config.sample_rate);
    in_desc.mFramesPerPacket = d->output_format.frames_per_packet = btml(config.frame_length);
    in_desc.mChannelsPerFrame = config.num_channels;
    
    d->out_desc.mFormatID = kAudioFormatLinearPCM;
    d->out_desc.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    d->out_desc.mSampleRate = d->output_format.sample_rate = btml(config.sample_rate);
    d->out_desc.mChannelsPerFrame = d->output_format.channels = config.num_channels;
    d->out_desc.mFramesPerPacket = 1;
    d->out_desc.mBitsPerChannel = d->output_format.bit_depth = config.bit_depth;
    
    d->output_format.frame_size = d->output_format.channels * (d->output_format.bit_depth / 8);
    
    /* Make Core Audio fill out rest of outDesc. */
    UInt32 size = sizeof(AudioStreamBasicDescription);
    ca_assert(AudioFormatGetProperty(kAudioFormatProperty_FormatInfo, 0, NULL, &size, &d->out_desc));
    
    ca_assert(AudioConverterNew(&in_desc, &d->out_desc, &d->converter_ref));
    
    AudioChannelLayout channelLayout;
    bzero(&channelLayout, sizeof(channelLayout));
    channelLayout.mChannelLayoutTag = (config.num_channels == 2 ? kAudioChannelLayoutTag_Stereo : kAudioChannelLayoutTag_Mono);
    
    ca_assert(AudioConverterSetProperty(d->converter_ref, kAudioConverterDecompressionMagicCookie, sizeof(struct alac_magic_cookie_t), &d->magic_cookie));
    ca_assert(AudioConverterSetProperty(d->converter_ref, kAudioConverterInputChannelLayout, sizeof(channelLayout), &channelLayout));
    
    return d;
    
}

void decoder_alac_destroy(void* data) {
    
    struct decoder_alac_mac_t* d = (struct decoder_alac_mac_t*)data;
    
    ca_assert(AudioConverterDispose(d->converter_ref));
    
    free(d);
    
}

struct decoder_output_format_t decoder_alac_get_output_format(void* data) {
    
    return ((struct decoder_alac_mac_t*)data)->output_format;
    
}

OSStatus _decoder_alac_mac_input_data_proc(AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription  **outDataPacketDescription, void *inUserData) {
    
    struct decoder_alac_mac_t* d = (struct decoder_alac_mac_t*)inUserData;
    
    (*ioData).mBuffers[0].mData = d->buffer_p;
    (*ioData).mBuffers[0].mDataByteSize = (UInt32)d->buffer_size;
    (*ioData).mBuffers[0].mNumberChannels = d->magic_cookie.alac_specific_info.config.num_channels;
    
    d->packet_description.mDataByteSize = (UInt32)d->buffer_size;
    
    if (outDataPacketDescription)
        *outDataPacketDescription = &d->packet_description;
    
    return noErr;
    
}

size_t decoder_alac_decode(void* data, void* in_audio_data, size_t in_audio_data_size, void* out_audio_data, size_t out_audio_data_size) {
    
    struct decoder_alac_mac_t* d = (struct decoder_alac_mac_t*)data;
    
    assert(in_audio_data != NULL && out_audio_data != NULL && out_audio_data_size > 0);
    
    d->audio_buffer.mData = in_audio_data;
    d->audio_buffer.mDataByteSize = (UInt32)in_audio_data_size;
    d->audio_buffer.mNumberChannels = d->magic_cookie.alac_specific_info.config.num_channels;
    
    AudioBufferList out_buffer_list;
    out_buffer_list.mNumberBuffers = 1;
    out_buffer_list.mBuffers[0].mData = out_audio_data;
    out_buffer_list.mBuffers[0].mDataByteSize = (UInt32)out_audio_data_size;
    
    d->buffer_size = in_audio_data_size;
    d->buffer_p = in_audio_data;
    
    UInt32 io_output_data_packets = (UInt32)out_audio_data_size / d->output_format.frame_size;
    ca_assert(AudioConverterFillComplexBuffer(d->converter_ref, _decoder_alac_mac_input_data_proc, d, &io_output_data_packets, &out_buffer_list, NULL));
    
    return d->output_format.frames_per_packet * d->out_desc.mBytesPerFrame;
    
}

void decoder_alac_reset(void* data) {
    
    struct decoder_alac_mac_t* d = (struct decoder_alac_mac_t*)data;
    
    ca_assert(AudioConverterReset(d->converter_ref));
    
}

#endif
