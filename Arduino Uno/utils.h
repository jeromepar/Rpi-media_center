void displayTime();

enum State_automate{
  S_INIT,
  S_DEFAULT,
  S_MUSIC,
  S_VIDEO,
  S_VOLUME,
  S_VOLUME_OMX,
  S_LINEIN,
  S_RADIO,
  S_MAX
};

enum Mode{
  M_DEFAULT,
  M_VIDEO,
  M_MUSIC,
  M_LINEIN,
  M_RADIO,
  M_MAX
};

enum Cmd_RasbPi{
  C_RASBPI_GET_TIME='0',
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
  
  C_PING,
  
  C_ALSA_LINEIN,
  C_ALSA_LINEIN_OFF
};

enum Cmd{
  CMD_PLUS,
  CMD_MINUS,
  CMD_PREVIOUS,
  CMD_NEXT,
  CMD_BACK,
  CMD_FORWARD,
  CMD_PLAY,
  CMD_PAUSE,
  CMD_STOP,
  CMD_CLEAR,
  CMD_ENTER,
  CMD_ALBUM_MINUS,
  CMD_ALBUM_PLUS,
  CMD_TUNER_MEM
};

enum Input_rasbPi{
  INP_TIME = '0',
  INP_VOLUME,
  INP_TEXT,
  INP_MODE,
  INP_END_MODE,
  INP_LINE_MODE,
  INP_OMX_DURATION,
  INP_PING_BACK
};

enum Symbol {
  SIMB_NOTE = 0,
  SIMB_CAMERA,
  SIMB_PLAY,
  SIMB_LINEIN,
  SIMB_RADIO,
  SIMB_PAUSE
};

void goState(State_automate s);
void set_time_state();
int is_time_state_over(unsigned int milli);
void update_volume_from_pot();
void update_IR();
void change_volume(int i);
void switch_mute();
void adjust_to_mode();
void interpret_cmd_music();
void interpret_cmd_video();
void interpret_cmd_default();
void send_cmd_to_rasbPi();
void process_rasbPi_serial();
