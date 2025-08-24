# cget
a very small, memory efficient wget-like C-based downloader made for unix systems


## Building
1. compile the code using the included makefile
```console
make
```
2. the resulting executable will be in the build folder
```console
./build/cget
```

## Usage

1. get the download link without www at the start and use it like this
```console
./build/cget LINK
```
additionally you can assign an output file
```console
./build/cget LINK -o output.format
```


