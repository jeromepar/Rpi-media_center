#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctime>
#include <string.h>
#include <wiringPi.h>

#include "rs232.h"
#include "utils.h"
using namespace std;

int write_in_fifo(const char *s);

int main()
{
    int n = 0,
	cport_nr=16,        /* /dev/USB0 (COM9 on windows) */
	cport_nr2=17,        /* /dev/USB1 (COM10 on windows) */
	bdrate=9600;       /* 9600 baud */
    unsigned char *p_char;

    bool connected;
    int cpt_init; /* init à 0, attente des 2 msg d'init pour la scrutation*/

    unsigned char buf[4096];
    unsigned long current_time;
    unsigned char volume;
    time_t t;
    struct tm * p_now;

    FILE * output;
    FILE * pipe;

    char cmd_to_system[60];
    char cmd_mpc[15]="mpc -p 443 ";

    unsigned short arrow_right = 0x5b43;
    unsigned short arrow_left = 0x5b44;
    unsigned short arrow_up = 0x5b41;
    unsigned short arrow_down = 0x5b42;

    char mpd_title[21] = " ";
    char mpd_title_stopped[21] = " ";
    char mpd_title_received[21];
    char mpd_artist[21] = " ";
    char mpd_artist_received[21];
    char mpd_artist_stopped[21] = "Stopped";
    char omx_title[21] = " ";
    unsigned long omx_duration;

    bool video_was_playing = false;

    time_t last_ping_time;
    time_t temp_comp_time;

    /* init des PINs */
    int out = wiringPiSetup() ;
    if (out==-1){
	printf("unable to initialise wiringPy, exiting...\n");
	exit(0);
    }
    pinMode(0,1);
    digitalWrite(0,1);

    while(1) {

	if(RS232_OpenComport(cport_nr, bdrate) )
	{
	    cport_nr == 16 ? cport_nr =17 : cport_nr =16;
	    digitalWrite(0,1);
	    usleep(5000000);  /* sleep for 5 */
	} else {
	    connected = 1;
	    mpd_title[0]= 0;
	    mpd_title_stopped[0]= 0;
	    mpd_artist[0] = 0;
	    omx_title[0] = 0;
	    cpt_init = 0;
	    time(&last_ping_time);
	    while(connected)
	    {

		if(cpt_init == 2){
		    digitalWrite(0,0);
		    cpt_init++;
		}

		n = RS232_PollComport(cport_nr, buf, 4095);

		/* RECEPTION */
		for(int i=0; i<n; i++) {// data available
		    switch(buf[i]){
			case C_RASBPI_GET_TIME:
			    printf("demande de tps\n");
			    time(&t);   // get time now
			    p_now = localtime( &t );
			    current_time = (((p_now->tm_wday)*24+p_now->tm_hour)*60+p_now->tm_min)*60+p_now->tm_sec;
			    RS232_SendByte(cport_nr, INP_TIME);
			    RS232_SendBuf(cport_nr, TO_CHAR_POINTER(current_time),4);
			    cpt_init++;
			    break;
			case C_RASBPI_GET_VOLUME:
			    printf("demande de volume\n");
			    output = popen("amixer get PCM | egrep -o \"[0-9]+%\" | egrep -o \"[0-9]+\"","r");
			    if (output != NULL) {
				char s[10];
				fgets(s, 5, output);
				pclose(output);

				volume = atoi(s);
				RS232_SendByte(cport_nr, INP_VOLUME);
				RS232_SendByte(cport_nr, volume);
				cpt_init++;
			    }

			    break;
			case C_ALSA_VOLUME:
			    i++;
			    volume = buf[i];
			    printf("Volume recu: %d%%\n",volume);
			    sprintf(cmd_to_system, "amixer set PCM %d%% > /dev/null",volume);
			    system(cmd_to_system);
			    break;
			case C_ALSA_MUTE:
			    system("amixer sset PCM toggle > /dev/null");
			    break;
			case C_MPD_PREVIOUS:
			    strcpy(cmd_to_system, cmd_mpc);
			    strcat(cmd_to_system,"prev > /dev/null");
			    system(cmd_to_system);
			    break;
			case C_MPD_NEXT:
			    strcpy(cmd_to_system, cmd_mpc);
			    strcat(cmd_to_system,"next > /dev/null");
			    system(cmd_to_system);
			    break;
			case C_MPD_BACK:
			    break;
			case C_MPD_FORWARD:
			    break;
			case C_MPD_PLAY:
			    strcpy(cmd_to_system, cmd_mpc);
			    strcat(cmd_to_system,"play > /dev/null");
			    system(cmd_to_system);
			    break;
			case C_MPD_PAUSE:
			    strcpy(cmd_to_system, cmd_mpc);
			    strcat(cmd_to_system,"pause > /dev/null");
			    system(cmd_to_system);
			    break;
			case C_MPD_STOP:
			    strcpy(cmd_to_system, cmd_mpc);
			    strcat(cmd_to_system,"stop > /dev/null");
			    system(cmd_to_system);
			    break;
			case C_MPD_CLEAR:
			    strcpy(cmd_to_system, cmd_mpc);
			    strcat(cmd_to_system,"clear > /dev/null");
			    system(cmd_to_system);
			    break;
			case C_MPD_LAUNCH_RADIO:
			    strcpy(cmd_to_system, cmd_mpc);
			    strcat(cmd_to_system,"clear > /dev/null");
			    system(cmd_to_system);
			    strcpy(cmd_to_system, cmd_mpc);
			    strcat(cmd_to_system,"load radio > /dev/null");
			    system(cmd_to_system);
			    strcpy(cmd_to_system, cmd_mpc);
			    strcat(cmd_to_system,"play > /dev/null");
			    system(cmd_to_system);
			    break;
			case C_OMX_PREVIOUS:
			    if(write_in_fifo("\x1b\x5b\x42")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_NEXT:
			    if(write_in_fifo("\x1b\x5b\x41")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_BACK:
			    if(write_in_fifo("\x1b\x5b\x44")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_FORWARD:
			    if(write_in_fifo("\x1b\x5b\x43")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_PLAY:
			    if(write_in_fifo(" ")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_PAUSE:
			    if(write_in_fifo(" ")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_STOP:
			    if(write_in_fifo("q")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_SUB_MINUS:
			    if(write_in_fifo("d")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_SUB_PLUS:
			    if(write_in_fifo("f")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_V_PLUS:
			    if(write_in_fifo("+")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_OMX_V_MINUS:
			    if(write_in_fifo("-")==0){
				printf("FIFO not writeable\n");
			    }
			    break;
			case C_PING:
			    RS232_SendByte(cport_nr, INP_PING_BACK);
			    printf("pinged!\n");
			    time(&last_ping_time);
			    break;
			default:
			    break;
		    }
		} /* END RECEPTION */

		usleep(100000);  /* sleep for 100 milliSeconds */

		/* SCRUTATION */
		output = NULL;
		output = fopen("/home/pi/media_center/movie_playing", "r");
		if (output != NULL && !video_was_playing) {
		    video_was_playing = true;

		    for(int j=0;j<20;j++){
			omx_title[j]='o';
		    }
		    omx_title[20]=0;
		    fgets(omx_title, 21, output);
		    char duration_txt[10];
		    fgets(duration_txt,10, output);
		    int len = strlen(omx_title);
		    if (len > 0 && omx_title[len-1] == '\n') omx_title[--len] = '\0';
		    fclose(output);

		    unsigned long duration = atoi(duration_txt);

		    RS232_SendByte(cport_nr, INP_LINE_MODE);
		    RS232_SendByte(cport_nr, M_VIDEO);
		    RS232_SendByte(cport_nr, 3);
		    RS232_SendBuf(cport_nr, (unsigned char *)omx_title,20);
		    printf("Titre envoye: %s\n",omx_title);

		    RS232_SendByte(cport_nr, INP_OMX_DURATION);
		    RS232_SendBuf(cport_nr, TO_CHAR_POINTER(duration),4);

		    RS232_SendByte(cport_nr, INP_MODE);
		    RS232_SendByte(cport_nr, M_VIDEO);

		    printf("Movie started\n");

		} else if (output == NULL && video_was_playing){
		    video_was_playing = false;
		    RS232_SendByte(cport_nr, INP_END_MODE);
		    printf("Movie finnished\n");
		} else if(output != NULL){
		    fclose(output);
		}

		output = popen("mpc -p 443 -f %title%","r");
		if (output != NULL) {
		    fgets(mpd_title_received, 20, output);
		    int len = strlen(mpd_title_received);
		    if (len > 0 && mpd_title_received[len-1] == '\n') mpd_title_received[--len] = '\0';
		    pclose(output);

		    // if strings aren't identical
		    if(cpt_init>1 && strcmp(mpd_title, mpd_title_received) != 0){
			printf("titles differents -> %s\n",mpd_title_received);
			strcpy(mpd_title, mpd_title_received);

			RS232_SendByte(cport_nr, INP_LINE_MODE);
			RS232_SendByte(cport_nr, M_MUSIC);
			RS232_SendByte(cport_nr, 4);

			mpd_title_received[7]=0;
			if (strcmp("volume:", mpd_title_received)==0){
			    RS232_SendBuf(cport_nr, (unsigned char *)mpd_title_stopped,20);
			} else {
			    RS232_SendBuf(cport_nr, (unsigned char *)mpd_title,20);
			}
		    }

		}
		output = popen("mpc -p 443 -f \"[%artist%]|[%name%]\"","r");
		if (output != NULL) {
		    fgets(mpd_artist_received, 20, output);
		    int len = strlen(mpd_artist_received);
		    if (len > 0 && mpd_artist_received[len-1] == '\n') mpd_artist_received[--len] = '\0';
		    pclose(output);

		    // if strings aren't identical
		    if(cpt_init > 1 && strcmp(mpd_artist, mpd_artist_received) != 0){
			printf("artists differents -> %s\n", mpd_artist_received);
			strcpy(mpd_artist, mpd_artist_received);

			RS232_SendByte(cport_nr, INP_LINE_MODE);
			RS232_SendByte(cport_nr, M_MUSIC);
			RS232_SendByte(cport_nr, 3);

			mpd_artist_received[7]=0;
			if (strcmp("volume:", mpd_artist_received)==0){
			    RS232_SendBuf(cport_nr, (unsigned char *)mpd_artist_stopped,20);
			} else {
			    RS232_SendBuf(cport_nr, (unsigned char *)mpd_artist,20);
			}
		    }

		}


		usleep(100000);  /* sleep for 100 milliSeconds */

		/* no ping received? */
		time(&temp_comp_time);
		if(difftime(temp_comp_time,last_ping_time) > 10){
		    printf("Connexion perdue...\n");
		    connected = 0;
		}
	    } /*end while connected */
	    RS232_CloseComport(cport_nr);
	} /* end else opencomport */

    } /*end while (1) */
    printf("Sortie du while 0???\n");
    return(0);
}

int write_in_fifo(const char *s){
    int file;
    FILE *stream = NULL;

    file = open ("/home/pi/media_center/omx_fifo", O_WRONLY);
    if (file == -1){
	printf("Can't open the file\n");
	return 0;
    }
    stream = fdopen(file,"w");
    if (stream == NULL){
	close(file);
	printf("Can't open the stream\n");
	return 0;
    }
    fprintf(stream, s);
    fflush(stream);
    fclose (stream);

    close(file);
    return 1;
}
