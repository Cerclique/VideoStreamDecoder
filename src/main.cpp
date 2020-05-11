#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "VideoStreamDecoder.hpp"

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  std::cout << "Hello World!" << std::endl;

  int errorCode = 0;
  VideoStreamDecoder decoder("star_trails.mkv");

  errorCode = decoder.open();
  if (errorCode < 0) {
    std::cerr << decoder.getErrorMessage(errorCode) << std::endl;
  }

  uint8_t* frameBuffer;
  while (decoder.isFrameAvailable()) {
    errorCode = decoder.getFrame(&frameBuffer);
    if (errorCode < 0) {
      std::cerr << decoder.getErrorMessage(errorCode) << std::endl;
    }

    if (frameBuffer != nullptr) {
      int width = decoder.getWidth();
      int height = decoder.getHeight();

      cv::Mat imageMat(width, height, CV_8UC3, frameBuffer);
      cv::imshow("Display", imageMat);
    }
    cv::waitKey(33);
  }
  return 0;
}
