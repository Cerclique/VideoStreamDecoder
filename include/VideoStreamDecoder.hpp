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
  AVFormatContext* pFormatCtx = nullptr;
  AVCodec* pCodec = nullptr;
  AVCodecParameters* pCodecParameters = nullptr;
  AVCodecContext* pCodecCtx = nullptr;
  uint8_t* pictureBuffer = nullptr;
  AVFrame* pFrameRaw = nullptr;
  AVFrame* pFrameBGR = nullptr;
  SwsContext* pScalerCtx = nullptr;
  AVPixelFormat pixelFormat = AV_PIX_FMT_BGR24;
  int streamFramerate = -1;
  int streamWidth = -1;
  int streamHeight = -1;
  int videoStreamIndex = -1;
  bool isStreamOpened = false;
  AVPacket framePacket;

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
