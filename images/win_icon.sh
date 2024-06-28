#!/bin/sh
# uses imagemagick convert to generate all the required resolutions from a nice big square design (1024x1024 works well)
# note that 'mogrify -strip image.png' can be used to get rid of some of the load errors in Qt
# note online converters like https://icoconvert.com/ may do a better job
# or use the javascript resize_for_icon.jsx file in adobe photoshop

# this one produces a white background
convert 'running_icon_white_bg.png' -bordercolor white -border 0 \( -clone 0 -resize 16x16 \) \( -clone 0 -resize 24x24 \) \( -clone 0 -resize 32x32 \) \( -clone 0 -resize 40x40 \) \( -clone 0 -resize 48x48 \) \( -clone 0 -resize 64x64 \) \( -clone 0 -resize 256x256 \) -delete 0 -alpha off -colors 256 icon_256x256.ico

# this one uses transparency from the png
#convert 'running_icon.png' -background transparent \( -clone 0 -resize 16x16 \) \( -clone 0 -resize 24x24 \) \( -clone 0 -resize 32x32 \) \( -clone 0 -resize 40x40 \) \( -clone 0 -resize 48x48 \) \( -clone 0 -resize 64x64 \) \( -clone 0 -resize 256x256 \) -delete 0 -colors 256 icon_256x256.ico

# I cannot get transparency to work. However the online converters work fine.
