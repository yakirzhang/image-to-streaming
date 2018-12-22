#!/bin/sh
# Use Gstreamer to grab H.264 video and audio stream from four cameras
# Send video as RTP stream over UDP

#gst-launch-1.0 udpsrc port=5000 ! application/x-rtp,encoding-name=H264,payload=96 ! rtph264depay ! h264parse ! queue ! avdec_h264 ! xvimagesink sync=false async=false -e

# Here for demo purposes
SCALE="video/x-raw, width=1280, height=720" 
#SCALE="video/x-raw, width=1960, height=1080" 

# VIDEO_SINK is the preview window
#VIDEO_SINK=" autovideosink sync=false async=false -e"
VIDEO_SINK="videoconvert ! videoscale ! $SCALE ! autovideosink sync=false async=false -e"

# Address and port to serve the video stream
UDP_SRC="udpsrc port=5000"

#show gst-launch on the command line; can be useful for debugging
echo gst-launch-1.0 \
   $UDP_SRC \
   ! application/x-rtp,encoding-name=H264,payload=96     \
   ! rtph264depay					\
   ! h264parse						\
   ! queue ! avdec_h264	!				\
   $VIDEO_SINK
 
# sends H.264 rtp over TCP
gst-launch-1.0 \
   $UDP_SRC \
   ! application/x-rtp,encoding-name=H264,payload=96	\
   ! rtph264depay					\
   ! h264parse						\
   ! queue ! avdec_h264	!				\
   $VIDEO_SINK
