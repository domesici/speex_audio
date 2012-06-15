#include "ros/ros.h"
#include "std_msgs/String.h"
#include <boost/thread.hpp>


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <linux/soundcard.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>


#include <speex/speex.h>
#include "qos_audio.h"
#include <sstream>
#include <sys/ioctl.h>

#define USE_SPEEX
#define RATE 8000   /* the sampling rate */
#define SIZE 16      /* sample size: 8 or 16 bits */
#define CHANNELS 1

int monitor = 0;

bool log_enabled= true;
FILE *audio_in, *audio_out;
FILE *pkt_serial_in, *pkt_serial_out;

int outfd, infd;
unsigned long long last_pkt_id = 0;
int nPackets = 4;
int x = 160 * 2;
int pkt_idx = 0;


int audio_init(int type)
{
        int arg;		/* argument for ioctl calls */
        int status;		/* return status of system calls */

        /* open sound device */
        if (type == 0)
        {
                infd = open("/dev/dsp", O_RDONLY);
                outfd = open("/dev/dsp", O_WRONLY);
                if (infd < 0)
                {
                        perror("open of /dev/dsp failed");
                        return 0;
                }

                /* set sampling parameters */
                arg = SIZE; /* sample size */
                status = ioctl(infd,SNDCTL_DSP_SETFMT, &arg);
                status = ioctl(outfd,SNDCTL_DSP_SETFMT, &arg);
                if (status == -1)
                {
                        perror("SOUND_PCM_WRITE_BITS ioctl failed");
                        return 0;
                }
                if (arg != SIZE)
                {
                        perror("unable to set sample size");
                        return 0;
                }
                arg = CHANNELS; /* mono or stereo */
                status = ioctl(infd, SNDCTL_DSP_CHANNELS, &arg);
                status = ioctl(outfd, SNDCTL_DSP_CHANNELS, &arg);
                if (status == -1)
                {
                        perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
                        return 0;
                }
                if (arg != CHANNELS)
                {
                        perror("unable to set number of channels");
                        return 0;
                }

                int frag = (32 << 16) | 9;
                status = ioctl(infd, SNDCTL_DSP_SETFRAGMENT, &frag);
                status = ioctl(outfd,SNDCTL_DSP_SETFRAGMENT , &frag);


                if (status == -1)
                {
                        perror("SOUND_PCM_SUBDIVIDE ioctl failed");
                        return 0;
                }
                arg=RATE;
                status = ioctl(infd, SNDCTL_DSP_SPEED, &arg);
                status = ioctl(outfd, SNDCTL_DSP_SPEED, &arg);
                if (status == -1)
                {
                        perror("SOUND_PCM_WRITE_WRITE ioctl failed");
                        return 0;
                }
        }
        else
        {
                outfd = 1;
                infd = 0;
        }
        cmp_init(160, 8, 1);

        fprintf(stderr, "Audio initialized...\n");


        return 1;
}


void speexCallback(const std_msgs::String::ConstPtr& msg)
{
    int n1;

  ROS_INFO("I heard: [%d]", sizeof(msg->data.begin() -msg->data.end()));
  char coded[640], decoded[640];

  int i;
  for (i = 0; i < nPackets; i++){


        std::copy(msg->data.begin()+(42*i), msg->data.begin()+42*(i+1)-1, coded);
fprintf(stderr, "size is: %d\n",msg->data.size());
        n1 = cmp_decode(coded, decoded);
        write(outfd, decoded, n1);
        fprintf(stderr, "decoded buffer size is: %d\n",n1);

    }

}




int main(int argc, char **argv)
{

  ros::init(argc, argv, "speex_audio");
  audio_init(0);
  ros::NodeHandle n;
  ros::Publisher _pub = n.advertise<std_msgs::String>("speex_capture", 10, true);
  ros::Rate loop_rate(1000);

  //play part
  ros::Subscriber _sub = n.subscribe("speex_play", 10, speexCallback);


  ///capture part
  while (ros::ok())
  {
    std_msgs::String msg;
    ROS_INFO("%s", msg.data.c_str());

    char buff1[640], encoded[640];

    int  i = 0, j= 0,  sum = 0;
    int n1,n2;


    for (i = 0; i < nPackets; i++)
    {
            short *s1;

            n1 = read(infd, buff1, x);
            fprintf(stderr, "buffer size n1 is: %d\n",n1);


            while (n1 < x)
            {
                    n1 += read(infd, buff1 + n1, x - n1);
            }
            /* silencio > */
            s1 = (short *)buff1;
            for (j = 0; j < n1 / 2; j++)
            {
                    sum += abs(s1[j]);
            }
            /* silencio < */

            n2 = cmp_encode(buff1, encoded+n2*i);
            fprintf(stderr, "encoded buffer size is: %d\n",n2);

        }
    msg.data.resize(n2*nPackets);                                 //reservo memoria para msg.data
    std::copy(encoded,encoded + n2*nPackets, msg.data.begin());


    ROS_INFO("%d", msg.data.size());
    _pub.publish(msg);
    ros::spinOnce();
    loop_rate.sleep();

    }
  return 0;

}
