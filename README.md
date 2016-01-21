# Thumbler

## Dependencies
* Graphics Library (gd), [libgd](http://www.libgd.org/)

## What?
Create thumbnail by using a gd graphics library.

## Why?
Since ImageMagick was way too bloated for my use.

## Usage
At first, generate a file list, for example this way

	`find . -type f \( -iname "*.jpg" -or -iname "*.png" \) >images.lst`

If you want to get more information `-v` verbose switch can be switched on.

### Create thumbnails
Then, execute `thumbler`:

	./thumbler -t images.lst

Now, the `image.jpg` should have thumb named `image_thmb.jpg`.

### Pack images by width wise
Packing images regarding the width, can be done by executing:

	./thumbler -p thumbs.lst

