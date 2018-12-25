#include <fcntl.h>
#include <glog/logging.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <linux/fb.h>
#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;
bool running = true;
void sigHandler(int s) { running = false; }
typedef struct _GstDataStruct {
  GstElement *pipeline;
  GstElement *app_source;
  GstElement *video_convert;
  GstElement *auto_video_sink;
  GstElement *rtph264pay;
  GstElement *udpsink;
  GstElement *fakesink;
  GstElement *auto_video_convert;
  GstElement *h264_encoder;
  GstElement *jpgenc;
  GstElement *mp4mux;
  GstElement *flvmux;
  GstElement *rtmpsink;
  GstElement *file_sink;
  GstElement *app_sink;
  guint sourceid; /* To control the GSource */
  guint app_src_index;
  guint app_sink_index;
  GMainLoop *loop; /* GLib's Main Loop */
} GstDataStruct;

typedef struct {
  struct fb_var_screeninfo vinfo;  // 可变的显示屏幕信息
  int width;                       // width 宽度
  int height;                      // height 高度
  int bpp;                         // bit per pixel 像素的位数
  int rowsize;                     // 屏幕一行所占字节数
  int real_len;                    // 实际显示区域的字节数
  int total_len;                   // 显示区域总字节数，即长度
  int offset;                      // 实际显示位置在总长度中的偏移量
  int fd;                          // 打开fb节点时的文件句柄
  void *data;                      // fb0 节点内存映射后数据指针
} FbInfoStruct;

static void new_h264_sample_on_appsink(GstElement *sink, GstDataStruct *pGstData);
static void eos_on_appsink(GstElement *sink, GstDataStruct *pGstData);
static gboolean read_data(GstDataStruct *pGstData);
static void start_feed(GstElement *pipeline, guint size, GstDataStruct *pGstData);
static void stop_feed(GstElement *pipeline, GstDataStruct *pGstData);
gboolean bus_msg_call(GstBus *bus, GstMessage *msg, GstDataStruct *pGstData);

static FILE *fp;  // 打开或保存文件的的指针
static GstBus *bus;
static GstDataStruct GstData;
static FbInfoStruct FbInfo;

vector<char> image_buff(1280 * 720 * 3);
condition_variable cv;
mutex mtx;

void recv_data() {
  auto socket = nn_socket(AF_SP, NN_SUB);
  nn_setsockopt(socket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);
  int recv_max = 5 * 1024 * 1024;
  nn_setsockopt(socket, NN_SOL_SOCKET, NN_RCVMAXSIZE, &recv_max, 4);
  nn_connect(socket, "tcp://192.168.1.10:4445");
  while (running) {
    char *recv_msg = nullptr;
    auto recv_size = nn_recv(socket, &recv_msg, NN_MSG, 0);
    if (recv_size > 0) {
      DLOG(INFO) << "Update Image";
      std::lock_guard<mutex> lck(mtx);
      memcpy(image_buff.data(), recv_msg, recv_size);
      cv.notify_one();
      nn_freemsg(recv_msg);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
}
int main(int argc, char *argv[]) {
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = true;

  thread recv_thread(recv_data);

  printf("================ imx60 360 main start ==============\n");
  memset(&GstData, 0, sizeof(GstDataStruct));
  memset(&FbInfo, 0, sizeof(FbInfoStruct));

  /* Initialize cumstom data structure */
  printf("============= imx60 360 gst init start ============\n");
  gst_init(&argc, &argv);

  /* Create gstreamer elements */
  printf("=========== create imx60 360 pipeline =============\n");
  GstData.pipeline = gst_pipeline_new("imx60_360");
  GstData.app_source = gst_element_factory_make("appsrc", "appsrc");
  GstData.video_convert = gst_element_factory_make("videoconvert", "videoconvert");
  GstData.h264_encoder = gst_element_factory_make("x264enc", "x264enc");
  GstData.mp4mux = gst_element_factory_make("mp4mux", "mp4mux");
  GstData.file_sink = gst_element_factory_make("filesink", "filesink");
  GstData.rtph264pay = gst_element_factory_make("rtph264pay", "rtph264pay");
  GstData.udpsink = gst_element_factory_make("udpsink", "udpsink");
  GstData.auto_video_sink = gst_element_factory_make("autovideosink", "autovideosink");
  GstData.flvmux = gst_element_factory_make("flvmux", "flvmux");
  GstData.rtmpsink = gst_element_factory_make("rtmpsink", "rtmpsink");
  GstData.fakesink = gst_element_factory_make("fakesink", "fakesink");

  //  GstData.h264_encoder = gst_element_factory_make("x264enc", "x264enc");
  //  GstData.app_sink = gst_element_factory_make("appsink", "fake");

  if (!GstData.pipeline || !GstData.app_source || !GstData.auto_video_sink || !GstData.video_convert) {
    g_printerr("One element could not be created... Exit\n");
    munmap(FbInfo.data, FbInfo.total_len);
    close(FbInfo.fd);
    fclose(fp);
    return -1;
  }

  printf("============ link imx60 360 pipeline ==============\n");
  //  char szTemp[64];
  //  sprintf(szTemp, "%d", FbInfo.width * FbInfo.height * (FbInfo.bpp >> 3));
  //  g_object_set(G_OBJECT(GstData.app_source), "blocksize", szTemp, NULL);
  g_object_set(G_OBJECT(GstData.app_source), "is-live", TRUE, NULL);
  g_object_set(G_OBJECT(GstData.app_source), "do-timestamp", TRUE, NULL);
  g_object_set(G_OBJECT(GstData.app_source), "stream-type", 0, "format", GST_FORMAT_TIME, NULL);

  //  g_object_set(G_OBJECT(GstData.h265_encoder), "byte-stream", true, NULL);
  g_object_set(G_OBJECT(GstData.h264_encoder), "speed-preset", 1, NULL);
  g_object_set(G_OBJECT(GstData.h264_encoder), "tune", 4, NULL);
  //  g_object_set(G_OBJECT(GstData.h264_encoder), "qp-min", 1, NULL);
  //  g_object_set(G_OBJECT(GstData.h264_encoder), "qp-max", 10, NULL);
  //  g_object_set(G_OBJECT(GstData.h264_encoder), "bitrate", 1024000, NULL);

  g_object_set(G_OBJECT(GstData.rtph264pay), "pt", 96, "config interval", 1, NULL);
  g_object_set(G_OBJECT(GstData.udpsink), "host", "192.168.1.100", "port", 5000, "sync", false, "async", FALSE, NULL);
  g_object_set(G_OBJECT(GstData.file_sink), "location", "222.mp4", NULL);

  g_object_set(G_OBJECT(GstData.flvmux), "streamable", true, NULL);

  g_object_set(G_OBJECT(GstData.rtmpsink), "location", "rtmp://127.0.0.1/live", NULL);
  g_object_set(G_OBJECT(GstData.rtmpsink), "sync", false, "async", false, NULL);
  g_object_set(G_OBJECT(GstData.rtmpsink), "max-lateness", 1000000000, NULL);

  //  g_object_set(G_OBJECT(GstData.app_source), "min-percent", 3, NULL);
  //  GstCaps *appsrc_caps;
  auto appsrc_caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT, 1280,
                                         "height", G_TYPE_INT, 720, NULL);
  g_object_set(G_OBJECT(GstData.app_source), "caps", appsrc_caps, NULL);
  //  GstCaps *caps_app_sink;
  //  caps_app_sink = gst_caps_new_simple("video/x-h264", "stream-format", G_TYPE_STRING, "byte-stream", "alignment",
  //                                      G_TYPE_STRING, "au", NULL);
  //  //  g_object_set(G_OBJECT(GstData.h264_encoder), "bitrate", 30*1280*720*8, NULL);
  //  //  g_object_set(G_OBJECT(GstData.h264_encoder), "quant", 51, NULL);
  //  //  g_object_set(G_OBJECT(GstData.h264_encoder), "gop-size", gop_size, NULL);
  //  g_object_set(G_OBJECT(GstData.app_sink), "emit-signals", TRUE, "caps", caps_app_sink, "sync", FALSE, NULL);

  bus = gst_pipeline_get_bus(GST_PIPELINE(GstData.pipeline));
  auto bus_watch_id = gst_bus_add_watch(bus, (GstBusFunc)bus_msg_call, (gpointer)&GstData);
  gst_object_unref(bus);

  g_signal_connect(GstData.app_source, "need-data", G_CALLBACK(start_feed), &GstData);
  //  g_signal_connect(GstData.app_source, "enough-data", G_CALLBACK(stop_feed), &GstData);
  //  g_signal_connect(GstData.app_sink, "new-sample", G_CALLBACK(new_h264_sample_on_appsink), &GstData);
  //  g_signal_connect(GstData.app_sink, "eos", G_CALLBACK(eos_on_appsink), &GstData);
  gst_bin_add_many(GST_BIN(GstData.pipeline), GstData.app_source, GstData.video_convert, GstData.h264_encoder,
                   GstData.flvmux, GstData.rtmpsink, NULL);
  gst_element_link_many(GstData.app_source, GstData.video_convert, GstData.h264_encoder, GstData.flvmux,
                        GstData.rtmpsink, nullptr);

  GstData.app_src_index = 0;
  GstData.app_sink_index = 0;
  gst_element_set_state(GstData.pipeline, GST_STATE_PLAYING);
  GstData.loop = g_main_loop_new(NULL, FALSE);  // Create gstreamer loop
  g_main_loop_run(GstData.loop);                // Loop will run until receiving EOS (end-of-stream), will block here

  // Free resources
  gst_element_set_state(GstData.pipeline, GST_STATE_NULL);  // Stop pipeline to be released
  fprintf(stderr, "Deleting pipeline\n");
  gst_object_unref(GstData.pipeline);  // THis will also delete all pipeline elements
  g_source_remove(bus_watch_id);
  g_main_loop_unref(GstData.loop);
  munmap(FbInfo.data, FbInfo.total_len);
  close(FbInfo.fd);
  fclose(fp);
  recv_thread.join();
  return 0;
}

static void eos_on_appsink(GstElement *sink, GstDataStruct *pGstData) { printf("appsink get signal eos !!!\n"); }

static void new_h264_sample_on_appsink(GstElement *sink, GstDataStruct *pGstData) {
  GstSample *sample = NULL;
  g_signal_emit_by_name(sink, "pull-sample", &sample);
  if (sample) {
    pGstData->app_sink_index++;
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstMapInfo info;
    if (gst_buffer_map((buffer), &info, GST_MAP_READ)) {
      g_print("h264 Streaming Buffer is Comming\n");
      // Here to get h264 buffer data with info.data and get h264 buffer size with info.size
      // gst_util_dump_mem (info.data, info.size);
      fwrite(info.data, info.size, 1, fp);
      g_print("h264 Streaming Buffer is wrote, len:%d, index:%d\n", (int)info.size, pGstData->app_sink_index);
      gst_buffer_unmap(buffer, &info);
      gst_sample_unref(sample);
    }
  }
}

/* This method is called by the idle GSource in the mainloop. We feed CHUNK_SIZE
 * bytes into appsrc.
 * The ide handler is added to the mainloop when appsrc requests us to start
 * sending data (need-data signal) and is removed when appsrc has enough data
 * (enough-data signal).
 */
static gboolean read_data(GstDataStruct *pGstData) {
  static GstClockTime timestamp = 0;
  GstFlowReturn ret;
  GstBuffer *buffer;
  GstMemory *memory;

  //  pGstData->app_src_index++;
  //  if (pGstData->app_src_index > 500) {
  //    g_signal_emit_by_name(pGstData->app_source, "end-of-stream", &ret);
  //    // ret = gst_app_src_end_of_stream(GST_APP_SRC(pGstData->app_source));
  //    g_debug("eos returned %d at %d\n", ret, __LINE__);
  //    return FALSE;
  //  }
  DLOG(INFO) << "read data";
  unique_lock<mutex> lck(mtx);
  cv.wait(lck);
  buffer = gst_buffer_new();
  memory = gst_memory_new_wrapped(GST_MEMORY_FLAG_READONLY, image_buff.data(), image_buff.size(), 0, image_buff.size(),
                                  NULL, NULL);

  gst_buffer_append_memory(buffer, memory);
  //  GST_BUFFER_PTS(buffer) = timestamp;
  //  GST_BUFFER_DURATION(buffer) = gst_util_uint64_scale_int(1, GST_SECOND, 5);
  //  timestamp += GST_BUFFER_DURATION(buffer);

  g_signal_emit_by_name(pGstData->app_source, "push-buffer", buffer, &ret);

  gst_buffer_unref(buffer);

  if (ret != GST_FLOW_OK) {
    g_debug("push buffer returned %d for %d bytes \n", ret, FbInfo.real_len);
    return FALSE;
  }
  return TRUE;
}

static void start_feed(GstElement *pipeline, guint size, GstDataStruct *pGstData) {
  g_print("start feed...................\n");
  if (pGstData->sourceid == 0) {
    // GST_DEBUG ("start feeding");
    pGstData->sourceid = g_idle_add((GSourceFunc)read_data, pGstData);
  }
}

static void stop_feed(GstElement *pipeline, GstDataStruct *pGstData) {
  g_print("stop feed...................\n");
  if (pGstData->sourceid != 0) {
    // GST_DEBUG ("stop feeding");
    g_source_remove(pGstData->sourceid);
    pGstData->sourceid = 0;
  }
}

// Bus messages processing, similar to all gstreamer examples
gboolean bus_msg_call(GstBus *bus, GstMessage *msg, GstDataStruct *pGstData) {
  gchar *debug;
  GError *error;
  GMainLoop *loop = pGstData->loop;
  GST_DEBUG("got message %s", gst_message_type_get_name(GST_MESSAGE_TYPE(msg)));
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
      fprintf(stderr, "End of stream\n");
      g_main_loop_quit(loop);
      break;
    case GST_MESSAGE_ERROR:
      gst_message_parse_error(msg, &error, &debug);
      g_free(debug);
      g_printerr("Error: %s\n", error->message);
      g_error_free(error);
      g_main_loop_quit(loop);
      break;
    default:
      break;
  }
  return TRUE;
}
