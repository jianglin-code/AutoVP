#ifndef _RK_MULTIAUDIO_H_
#define _RK_MULTIAUDIO_H_

#include <system/audio.h>

    void multiaudio_A(audio_session_t sessionid, audio_port_handle_t *deviceid, audio_stream_type_t type, bool *boo, audio_devices_t *device);
    void multiaudio_B(audio_session_t sessionid, audio_port_handle_t *deviceid, audio_stream_type_t type, bool *boo, audio_devices_t *device);
    void multiaudio_C(audio_session_t sessionid, audio_port_handle_t *deviceid, audio_stream_type_t type, bool *boo, audio_devices_t *device);
    void multiaudio_D(audio_session_t sessionid, audio_port_handle_t *deviceid, audio_stream_type_t type, bool *boo, audio_devices_t *device);

#endif //_RK_MULTIAUDIO_H_
