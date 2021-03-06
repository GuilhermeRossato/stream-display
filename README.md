# Stream Display

This repository contains a program that streams RGB data to the screen

## Interface

Running `stream.exe` without parameters yields:

```
This program takes RGB data from the display and write it to stdout
Usage:
Stream raw RGB binary data from screen in 3 channels of 1 byte each

        stream.exe <left> <top> <width> <height>

Get a single frame of RGB data from screen instead of a continous

        stream.exe -s <x> <y> <width> <height>
```

## Example

If you execute `stream.exe 0 0 1 1` you will get a stream of binary data:

1. The first byte will be the `Red` component of the pixel at `0, 0`.
2. The second byte will be the `Green` component of the pixel at `0, 0`.
3. The third byte wil be the `Blue` component of the pixel at `0, 0`.
4. Since the bytes ended, the process will repeat:
5. The fourth byte will be the `Red` component of the pixel at `0, 0`.
6. The fifth byte will be the `Green` component of the pixel at `0, 0`.
7. And so on until you kill the process.
8. ...

If you execute `stream.exe 2 2 2 2` you will get a stream of binary data:

1. The first 3 bytes will be the `RGB` components of the pixel at `2, 2`.
2. The fourth up to fifth byte will be the `RGB` components of the pixel at `3, 2`.
3. The sixth up to eight byte will be the `RGB` components of the pixel at `2, 3`.
4. The 9th up to 11th byte will be the `RGB` components of the pixel at `3, 3`.
5. Since the area has been described, the process will repeat:
6. the 12th byte up until 14th byte will be the `RGB` component of the pixel at `2, 2`
7. ...

If you execute `stream.exe -s 0 0 100 100` you will get a standard output with `100 * 100 * 3` bytes (`30000`) and then the process will close with exit code 0.

## How dos it work

1. Creates a DirectX9 object
2. Creates capture device from screen data
3. Get Buffer Data from capture device
4. Print buffer RGB to standard output
5. Repeat

## How fast can it capture

About 10FPS on a 1080p, much slower than FFMPEG, for example, but the data is very organized.