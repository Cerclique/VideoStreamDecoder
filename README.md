# VideoStreamDecoder

This project offer the possibily to open a video file and retrieve each frame one by one.

## Dependencies
- libavcodec
- libavformat
- libavutil
- libswscale
- OpenCV4 (for the example)

## Compilation

The makefile provided is using `clang++` to compile. You're free to use another compiler.

For more information about the Makefile, please refer to the documentation of my [MakefileTemplate](https://github.com/Cerclique/MakefileTemplate) repository

## Usage

This project take the input file as argument.

```./VideoStreamDecoder <path_to_file>```

