#include <PI_Dragonfly.h>
#include <gst/gst.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// gstreamer main loop
static GMainLoop *loop;

uint64_t getCurrentTicks();

bool isExit = false;
bool SAVEYUV = false;

struct FrameData fd[4];
CameraAPI *camapi_p;

void intSigintHandler(int sig) {
  isExit = true;
  cout << "this is a test" << endl;
}

void remapY(unsigned char *imgPtr, struct FrameData *fd) {
  for (int k = 0; k < fd[0].para.height[0]; k++) {
    memcpy(imgPtr + k * 2560, fd[0].data_y + k * 1280, 1280);
  }

  for (int k = 0; k < fd[1].para.height[0]; k++) {
    memcpy(imgPtr + k * 2560 + 1280, fd[1].data_y + k * 1280, 1280);
  }

  for (int k = 0; k < fd[2].para.height[0]; k++) {
    memcpy(imgPtr + (k + 720) * 2560, fd[2].data_y + k * 1280, 1280);
  }

  for (int k = 0; k < fd[3].para.height[0]; k++) {
    memcpy(imgPtr + (k + 720) * 2560 + 1280, fd[3].data_y + k * 1280, 1280);
  }
}

void remapU(unsigned char *imgPtr, struct FrameData *fd) {
  for (int k = 0; k < fd[0].para.height[1]; k++) {
    memcpy(imgPtr + k * 1280, fd[0].data_u + k * 768, 640);
    memcpy(imgPtr + k * 1280 + 640, fd[1].data_u + k * 768, 640);
  }

  for (int k = 0; k < fd[2].para.height[1]; k++) {
    memcpy(imgPtr + (k + 360) * 1280, fd[2].data_u + k * 768, 640);
    memcpy(imgPtr + (k + 360) * 1280 + 640, fd[3].data_u + k * 768, 640);
  }
}

void remapV(unsigned char *imgPtr, struct FrameData *fd) {
  for (int k = 0; k < fd[0].para.height[2]; k++) {
    memcpy(imgPtr + k * 1280, fd[0].data_v + k * 768, 640);
    memcpy(imgPtr + k * 1280 + 640, fd[1].data_v + k * 768, 640);
  }

  for (int k = 0; k < fd[2].para.height[2]; k++) {
    memcpy(imgPtr + (k + 360) * 1280, fd[2].data_v + k * 768, 640);
    memcpy(imgPtr + (k + 360) * 1280 + 640, fd[3].data_v + k * 768, 640);
  }
}

uint64_t getCurrentTicks() {
  timeval t;
  gettimeofday(&t, NULL);
  return static_cast<uint64_t>(t.tv_sec) * 1000000ull + static_cast<uint64_t>(t.tv_usec);
}

// need data to appsrc gstreamer
static void cb_need_data(GstElement *appsrc, guint unused_size, CameraAPI *camapi_p) {
  static gboolean white = FALSE;
  static GstClockTime timestamp = 0;
  GstBuffer *buffer;
  guint size;
  GstFlowReturn ret;

  camapi_p > CaptureFrame(fd);
  int RES_WIDTH = 1280;
  int RES_HEIGHT = 720;

  size = RES_WIDTH * RES_HEIGHT * 3 / 2 * 4;
  int offsize_y = RES_WIDTH * RES_HEIGHT * 4;
  int offsize_u = RES_WIDTH * RES_HEIGHT;
  int offsize_v = RES_WIDTH * RES_HEIGHT;

  unsigned char *imageData = (unsigned char *)malloc(size);
  unsigned char *p = imageData;

  remapY(p, fd);
  p = p + offsize_y;
  remapU(p, fd);
  p = p + offsize_u;
  remapV(p, fd);

  if (SAVEYUV) {
    // print 4 frame size
    for (int i = 0; i < 4; i++) {
      cout << (long)fd[i].para.height[0] << endl;
      cout << (long)fd[i].para.width[0] << endl;
      cout << (long)fd[i].para.pitch[0] << endl;
      cout << (long)fd[i].para.height[1] << endl;
      cout << (long)fd[i].para.width[1] << endl;
      cout << (long)fd[i].para.pitch[1] << endl;
      cout << (long)fd[i].para.height[2] << endl;
      cout << (long)fd[i].para.width[2] << endl;
      cout << (long)fd[i].para.pitch[2] << endl;
    }
    FILE *file;
    file = fopen("test_yuv.raw", "a+");
    fwrite(imageData, 1, size, file);
    SAVEYUV = false;
    free(file);
  }

  buffer = gst_buffer_new_allocate(NULL, size, NULL);
  gst_buffer_fill(buffer, 0, imageData, size);

  GST_BUFFER_PTS(buffer) = timestamp;
  GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 30);

  timestamp += GST_BUFFER_DURATION(buffer);

  g_signal_emit_by_name(appsrc, "push buffer", buffer, &ret);
  gst_buffer_unref(buffer);
  free(imageData);

  if (ret != GST_FLOW_OK || isExit) {
    /* something wrong, stop pushing */
    g_main_loop_quit(loop);
  }
}

gint main(gint argc, gchar *argv[]) {
  signal(SIGINT, intSigintHandler);
  GstElement *pipeline, *appsrc, *videoconvert, *videorate, *queue, *omxh264enc, *rtph264pay, *udpsink;
  GstCaps *capture_caps, *converted_caps, *h264_caps;
  int bitrate = 8000000;
  char *gst_server_ip = "192.168.0.113";
  int gst_udp_port = 5000, gst_xres = 1280 * 2, gst_yres = 720 * 2;

  /* init GStreamer */
  gst_init(&argc, &argv);
  loop = g_main_loop_new(NULL, FALSE);

  // open camera to update the framebuf
  CameraAPI camapi;
  camapi_p = &camapi;
  memset(fd, 0, sizeof(struct FrameData) * 4);

  /* setup pipeline */
  pipeline = gst_pipeline_new("pipeline");
  appsrc = gst_element_factory_make("appsrc", "source");
  videorate = gst_element_factory_make("videorate", "videorate");
  queue = gst_element_factory_make("queue", "queue");
  videoconvert = gst_element_factory_make("videoconvert", "conv");
  omxh264enc = gst_element_factory_make("omxh264enc", "omxh264enc");
  rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
  udpsink = gst_element_factory_make("udpsink", "udpsink");

  converted_caps = gst_caps_new_simple("video/x raw", "width", G_TYPE_INT, gst_xres, "height", G_TYPE_INT, gst_yres,
                                       "framerate", GST_TYPE_FRACTION, 30, 1, NULL);
  h264_caps = gst_caps_new_simple("video/x h264", "stream format", G_TYPE_STRING, "byte stream", NULL);

  /* setup */
  g_object_set(G_OBJECT(appsrc), "caps",
               gst_caps_new_simple("video/x raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT, gst_xres,
                                   "height", G_TYPE_INT, gst_yres, "framerate", GST_TYPE_FRACTION, 30, 1, NULL),
               NULL);
  // gst_bin_add_many (GST_BIN (pipeline), appsrc, conv, udpsink, NULL);
  // gst_element_link_many (appsrc, conv, videosink, NULL);

  /* setup appsrc */
  g_object_set(G_OBJECT(appsrc), "stream type", 0, "format", GST_FORMAT_TIME, NULL);
  g_signal_connect(appsrc, "need data", G_CALLBACK(cb_need_data), camapi_p);

  /* set properties */
  // better paras: control rate=2,
  g_object_set(G_OBJECT(omxh264enc), "bitrate", bitrate, "control rate", 2, "EnableTwopassCBR", false, "iframeinterval",
               0, "EnableMVBufferMeta", false, "preset level", 0, "quality level", 0, "vbv size", 10, NULL);
  // g_object_set( G_OBJECT(omxh264enc), "bitrate", bitrate, "control rate", 2,"EnableTwopassCBR", true, "qp range",
  // "10,30:10,35:10,35", "iframeinterval", 100,  "EnableMVBufferMeta", true,"preset level", 3, "quality level",
  // 2,"vbv size", 10, NULL);
  g_object_set(G_OBJECT(rtph264pay), "pt", 96, "config interval", 1, NULL);
  g_object_set(G_OBJECT(udpsink), "host", gst_server_ip, "port", gst_udp_port, "sync", FALSE, "async", FALSE, NULL);
  g_object_set(G_OBJECT(queue), "max size buffers", 0, NULL);
  g_object_set(G_OBJECT(videorate), "drop only", TRUE, NULL);

  // videosink = gst_element_factory_make ("xvimagesink", "videosink");

  if (!pipeline || !appsrc || !videoconvert || !omxh264enc || !rtph264pay || !udpsink) {
    g_printerr("Not all elements could be created.\n");
    return 1;
  }

  /* Build the pipeline */
  gst_bin_add_many(GST_BIN(pipeline), appsrc, videoconvert, omxh264enc, rtph264pay, udpsink, NULL);
  fprintf(stderr, "Linking pipeline..\n");

#if 0
                  if (gst_element_link_filtered(appsrc , videorate, capture_caps) !=TRUE ) {
         g_printerr ("Could not connect appsrc to videorate.\n");
         gst_element_set_state (pipeline, GST_STATE_NULL);
         gst_object_unref (pipeline);
         pipeline=NULL;
         return  1;
       }
     if (gst_element_link_filtered(videorate , queue, converted_caps) !=TRUE ) {
         g_printerr ("Could not connect videorate to queue.\n");
         gst_element_set_state (pipeline, GST_STATE_NULL);
         gst_object_unref (pipeline);
         pipeline=NULL;
         return  1;
       }
#endif

  if (gst_element_link(appsrc, videoconvert) != TRUE) {
    g_printerr("Could not connect queue to videoconvert.\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = NULL;
    return 1;
  }
  if (gst_element_link(videoconvert, omxh264enc) != TRUE) {
    g_printerr("Could not connect videoconvert to omxh264enc.\n");
    gst_object_unref(pipeline);
    pipeline = NULL;
    return 1;
  }
  // if (gst_element_link(omxh264enc , rtph264pay ) !=TRUE ) {
  if (gst_element_link_filtered(omxh264enc, rtph264pay, h264_caps) != TRUE) {
    g_printerr("Could not connect omxh264enc to rtph264pay.\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = NULL;
    return 1;
  }
  if (gst_element_link(rtph264pay, udpsink) != TRUE) {
    g_printerr("Could not connect rpth264pay to udpsink.\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = NULL;
    return 1;
  }

  /* play */
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  g_main_loop_run(loop);

  /* clean up */
  gst_element_set_state(pipeline, GST_STATE_NULL);
  gst_object_unref(GST_OBJECT(pipeline));
  g_main_loop_unref(loop);

  return 0;
}
