#include "VideoStreamDecoder.hpp"

VideoStreamDecoder::VideoStreamDecoder(const std::string _streamPath) {
  this->streamPath = _streamPath;
}

int VideoStreamDecoder::open() {

  int errorCode = 0;

  errorCode = avformat_open_input(&this->pFormatCtx, this->streamPath.c_str(), nullptr, nullptr);
  if (errorCode < 0) {
    return errorCode;
  }

  errorCode = avformat_find_stream_info(this->pFormatCtx, nullptr);
  if (errorCode < 0) {
    return errorCode;
  }

  /** Find video stream based on user need. Here AVMEDIA_TYPE_VIDEO  **/
  errorCode = av_find_best_stream(this->pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, &this->pCodec, 0);
  if (errorCode == AVERROR_STREAM_NOT_FOUND || errorCode == AVERROR_DECODER_NOT_FOUND) {
    return errorCode;
  }

  /** A positive errorCode of av_find_best_stream refer to the stream index found */
  this->videoStreamIndex = errorCode;

  /** Retrieve parameter of the codec from stream context **/
  this->pCodecParameters = this->pFormatCtx->streams[this->videoStreamIndex]->codecpar;

  /** Get framerate **/
  AVRational rationalFPS = this->pFormatCtx->streams[this->videoStreamIndex]->r_frame_rate;
  this->streamFramerate = rationalFPS.num / rationalFPS.den;

  /** Allocate codec context based on codec found **/
  this->pCodecCtx = avcodec_alloc_context3(this->pCodec);
  assert(this->pCodecCtx != nullptr);

  /** Initialize codec context from codec paramaters **/
  errorCode = avcodec_parameters_to_context(this->pCodecCtx, this->pCodecParameters);
  if (errorCode < 0) {
    return errorCode;
  }

  /** Open codec **/
  errorCode = avcodec_open2(this->pCodecCtx, this->pCodec, nullptr);
  if (errorCode < 0) {
    return errorCode;
  }

  /** Once everything is open, we can allocate buffers that will receive frames **/
  this->allocateFrameBuffer();

  return errorCode;
}

void VideoStreamDecoder::close() {
  /**
      - avformat/avcodec functions automatically set pointer to nullptr.
      - For swsd_freeContext(), we need to do it manually.
      - pCodec and pCodecParameters are not dynamically allocated so we just have to make them point to nullptr at the end
  **/
  if (this->pScalerCtx != nullptr) {
    sws_freeContext(this->pScalerCtx);
    this->pScalerCtx = nullptr;
  }

  if (this->pFrameBGR != nullptr) {
    av_frame_free(&this->pFrameBGR);
  }

  if (this->pFrameRaw != nullptr) {
    av_frame_free(&this->pFrameRaw);
  }

  if (this->pictureBuffer != nullptr) {
    av_free(this->pictureBuffer);
  }

  if (this->pCodecParameters != nullptr) {
    this->pCodecParameters = nullptr;
  }

  if (this->pCodec != nullptr) {
    this->pCodec = nullptr;
  }

  if (this->pCodecCtx != nullptr) {
    avcodec_free_context(&this->pCodecCtx);
  }

  avformat_close_input(&this->pFormatCtx);

  /** Set boolean to false **/
  this->isStreamOpened = false;
}

int VideoStreamDecoder::allocateFrameBuffer() {
  int errorCode = 0;

  /** Get width and height from codec paramaters **/
  this->streamWidth = this->pCodecParameters->width;
  this->streamHeight = this->pCodecParameters->height;

  /** Compure size (in bytes) needed to store an image **/
  const int bufferSize = av_image_get_buffer_size(this->pixelFormat, this->streamWidth, this->streamHeight, 1);
  this->pictureBuffer = static_cast<uint8_t*>(av_malloc(bufferSize * sizeof(uint8_t)));
  assert(pictureBuffer != nullptr);

  /** Allocate frames **/
  this->pFrameRaw = av_frame_alloc();
  assert(pFrameRaw != nullptr);
  this->pFrameBGR = av_frame_alloc();
  assert(pFrameBGR != nullptr);

  /** Fill pFrameBGR with previously allocated pictureBuffer **/
  errorCode = av_image_fill_arrays(pFrameBGR->data, pFrameBGR->linesize, this->pictureBuffer, pixelFormat, this->streamWidth, this->streamHeight, 1);

  /** Allocate scaler context to convert raw frame to BGR frame  **/
  this->pScalerCtx = sws_getCachedContext(nullptr, this->streamWidth, this->streamHeight, this->pCodecCtx->pix_fmt, this->streamWidth, this->streamHeight, this->pixelFormat, SWS_BICUBIC, nullptr, nullptr, nullptr);

  /** Once everything is opended/allocated, set the boolean to true. **/
  this->isStreamOpened = true;

  return errorCode;
}

bool VideoStreamDecoder::isFrameAvailable() {
  int errorCode = 0;
  bool frameAvailable = false;

  while(true) {
    errorCode = av_read_frame(this->pFormatCtx, &this->framePacket);
    if (errorCode < 0)  {
      break;
    }

    if (this->framePacket.stream_index ==  this->videoStreamIndex) {
      frameAvailable = true;
      break;
    }
  }

  if (errorCode < 0 && errorCode != AVERROR_EOF) {
    std::cerr << this->getErrorMessage(errorCode) << std::endl;
  }

  return frameAvailable;
}

int VideoStreamDecoder::getFrame(uint8_t** frameBuffer) {
  int errorCode = 0;

  /** Clear output pointer **/
  *frameBuffer = nullptr;
  /** Since avcodec_decode_video2 is deprecated, we need to use send_packet/receive_frame to get frame buffer **/
  errorCode = avcodec_send_packet(this->pCodecCtx, &this->framePacket);
  if (errorCode < 0) {
    return errorCode;
  }

  errorCode = avcodec_receive_frame(this->pCodecCtx, this->pFrameRaw);
  if (errorCode < 0) {
    return errorCode;
  }

  /** Convert raw frame to BGR frame. **/
  this->convertFrameToBGR();

  *frameBuffer = this->pFrameBGR->data[0];

  /** Packet need to be unref/reset to be able to get next frame **/
  av_packet_unref(&this->framePacket);

  return errorCode;
}

void VideoStreamDecoder::convertFrameToBGR() {
  assert(pScalerCtx != nullptr);

  sws_scale(this->pScalerCtx, this->pFrameRaw->data, this->pFrameRaw->linesize, 0, this->streamHeight, this->pFrameBGR->data, this->pFrameBGR->linesize);
}

int VideoStreamDecoder::getWidth() const {
  return this->streamWidth;
}

int VideoStreamDecoder::getHeight() const {
  return this->streamHeight;
}

int VideoStreamDecoder::getFPS() const {
  return this->streamFramerate;
}

std::string VideoStreamDecoder::getErrorMessage(int errorCode) {
  char errorBuffer[ERROR_BUFFER_SIZE];
  av_strerror(errorCode, errorBuffer, ERROR_BUFFER_SIZE);
  return std::string(errorBuffer);
}

void VideoStreamDecoder::printInfos() {
  if (this->isStreamOpened) {
    av_dump_format(this->pFormatCtx, 0, this->streamPath.c_str(), 0);
  }
}
