#pragma once
#include <cstdint>


class CGlobalVarsBase {
public:
    float m_realtime; //0x0000
    int m_framecount; //0x0004
    float m_absolute_frame_time; //0x0008
    float m_absolute_frame_start_time_std_dev; //0x000C
    int m_max_clients; //0x0010
    pad(0x1C);
    float m_frame_time; //0x0030
    float m_curtime; //0x0034
    float m_curtime2; //0x0038
    pad(0xC);
    int m_tickcount; //0x0050
    pad(292);
    char* m_current_map; 
    char* m_current_map_name;
};