# image-to-streaming
from opencv image data to h264 and stream out
## install gstreamer
**与官方安装有些不同，因为在一个ubuntu16.04电脑上面安装完死活没有gstreamer-app**
```cassandraql

sudo apt-get install libgstreamer-plugins-base1.0-0
sudo apt-get install libgstreamer1.0-0 gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa 
装完之后
pkg-config --list-all |grep gstreamer
需要有 
gstreamer-1.0>=1.4
gstreamer-sdp-1.0>=1.4
gstreamer-video-1.0>=1.4
gstreamer-app-1.0>=1.4

```
## install nanomsg
github 自行下载安装(由设备采集的图通过这个通讯框架发送过来, 由于你没有设备，所以取图过程自行解决)
## 代码说明
basic_gstreamer 是网上查找的一些例子，用于理解gstreamer
app 下面是我工程中正在使用的例子
- t3 : 将收到的RBG图片经过h264编码然后使用rtp封包然后udp发送出去
- 接收端 : gst-udp-client.sh 运行即可

## License (GLWT)
:)
