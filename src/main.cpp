#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "VideoStreamDecoder.hpp"

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  std::cout << "Hello World!" << std::endl;

  int errorCode = 0;
  VideoStreamDecoder decoder("samples/star_trails.mkv");

  errorCode = decoder.open();
  if (errorCode < 0) {
    std::cerr << decoder.getErrorMessage(errorCode) << std::endl;
  }

  uint8_t* frameBuffer;
  int width = decoder.getWidth();
  int height = decoder.getHeight();
  int fps = decoder.getFPS();

  while (decoder.isFrameAvailable()) {
    errorCode = decoder.getFrame(&frameBuffer);
    if (errorCode < 0) {
      std::cerr << decoder.getErrorMessage(errorCode) << std::endl;
    }

    if (frameBuffer != nullptr) {

      cv::Mat imageMat(height, width, CV_8UC3, frameBuffer);
      cv::imshow("Display", imageMat);

      int delay = (static_cast<double>(1) / fps) * 1e3;
      cv::waitKey(delay);
    }
  }
  return 0;
}
