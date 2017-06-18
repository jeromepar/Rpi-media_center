//MACRO 
#define TO_CHAR_POINTER(a) (unsigned char *)&a

enum Cmd_RasbPi{
  C_RASBPI_GET_TIME = '0',
  C_RASBPI_GET_VOLUME,
  
  C_ALSA_VOLUME,
  C_ALSA_MUTE,
  
  C_MPD_PREVIOUS,
  C_MPD_NEXT,
  C_MPD_BACK,
  C_MPD_FORWARD,
  C_MPD_PLAY,
  C_MPD_PAUSE,
  C_MPD_STOP,
  C_MPD_CLEAR,
  C_MPD_LAUNCH_RADIO,

  C_OMX_PREVIOUS,
  C_OMX_NEXT,
  C_OMX_BACK,
  C_OMX_FORWARD,
  C_OMX_PLAY,
  C_OMX_PAUSE,
  C_OMX_STOP,
  C_OMX_SUB_MINUS,
  C_OMX_SUB_PLUS,
  C_OMX_V_PLUS,
  C_OMX_V_MINUS,

  C_PING
};

enum Input_rasbPi{
  INP_TIME='0',
  INP_VOLUME,
  INP_TEXT,
  INP_MODE,
  INP_END_MODE,
  INP_LINE_MODE,
  INP_OMX_DURATION,
  INP_PING_BACK
};

enum Mode{
    M_DEFAULT,
    M_VIDEO,
    M_MUSIC,
    M_MAX
};
