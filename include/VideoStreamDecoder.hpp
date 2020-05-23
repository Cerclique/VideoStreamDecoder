#include <iostream>
#include <string>
#include <cassert>

extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
  #include <libavutil/imgutils.h>
}

#define ERROR_BUFFER_SIZE 1024

class VideoStreamDecoder {
private:
  std::string streamPath;
  bool isStreamOpened = false;
  AVFormatContext* pFormatCtx = nullptr;
  AVCodec* pCodec = nullptr;
  AVCodecParameters* pCodecParameters = nullptr;
  AVCodecContext* pCodecCtx = nullptr;
  int videoStreamIndex = -1;
  uint8_t* pictureBuffer = nullptr;
  AVFrame* pFrameRaw = nullptr;
  AVFrame* pFrameBGR = nullptr;
  AVPacket framePacket;
  AVPixelFormat pixelFormat = AV_PIX_FMT_BGR24;
  SwsContext* pScalerCtx = nullptr;
  int streamFramerate = -1;
  int streamWidth = 0;
  int streamHeight = 0;

  int allocateFrameBuffer();
  void convertFrameToBGR();

public:
  VideoStreamDecoder(const std::string _streamPath);

  int open();
  void close();
  bool isFrameAvailable();
  int getFrame(uint8_t* &frameBuffer);

  int getWidth() const;
  int getHeight() const;
  int getFPS() const;

  std::string getErrorMessage(int errorCode);
  void printInfos();
};
