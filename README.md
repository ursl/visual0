# visual0

## install dependencies on Linux
Cf. https://docs.opencv.org/4.x/d7/d9f/tutorial_linux_install.html
```
cd somewhere
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.x.zip
unzip opencv.zip
cd opencv-4.x
mkdir _build && cd _build
cmake ..
cmake --build . --parallel 10
```

## analysis code
Get the source code 
```
git clone https://github.com/ursl/visual0
```
Adapt the Makefile to find your installation of opencv

Example first run
```
bin/opencvRun -m test0 -f DSC_3084.JPG 
```
