#include <iostream>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "VideoStreamDecoder.hpp"

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  int errorCode = 0;

  if (argc < 2) {
    std::cout << "Usage : \n\t VideoStreamDecoder <PATH_TO_FILE>" << std::endl;
    return 0;
  }

  VideoStreamDecoder decoder(argv[1]);

  errorCode = decoder.open();
  if (errorCode < 0) {
    std::cerr << decoder.getErrorMessage(errorCode) << std::endl;
  }

  decoder.printInfos();

  uint8_t* frameBuffer;
  int width = decoder.getWidth();
  int height = decoder.getHeight();
  int fps = decoder.getFPS();
  int delay = (static_cast<double>(1) / fps) * 1e3;

  while (decoder.isFrameAvailable()) {
    errorCode = decoder.getFrame(frameBuffer);
    if (errorCode < 0) {
      std::cerr << decoder.getErrorMessage(errorCode) << std::endl;
    }

    if (frameBuffer != nullptr) {
      cv::Mat imageMat(height, width, CV_8UC3, frameBuffer);
      cv::imshow("Display", imageMat);

      cv::waitKey(delay);
    }

  }

  decoder.close();

  return 0;
}
