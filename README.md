# Image Steganography using LSB Encoding and Decoding

## Overview

 **Least Significant Bit (LSB) Image Digital Steganography is a project in C which is used to securely hide secret text inside a BMP image. The application supports both ****encoding** and **decoding** operations.

The encoding process hides the contents of a secret text file into an image without any visible affect on the image quality. The decoding process extracts the hidden data from the encoded image.

## Features

* Encode secret text into BMP image
* Decode hidden text from encoded image
* File-based input/output handling
* Supports BMP image format
* Command line argument-based execution

## Technologies & Concepts Used

* C Programming
* File Handling
* Bitwise Operations
* Structures
* Command Line Arguments

## Project Structure

* `main.c` – Main execution flow
* `encode.c` – Encoding logic
* `decode.c` – Decoding logic
* `common.h` / `types.h` – Common definitions and structures

## How to Run

### Compile

```bash
gcc *.c -o steganography
```

### Encode

```bash
./steganography -e beautiful.bmp secret.txt output.bmp
```

### Decode

```bash
./steganography -d output.bmp decoded.txt
```
