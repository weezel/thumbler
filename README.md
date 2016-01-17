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

then, let the thumbler do the job

	./thumbler -t images.lst

and tha da, mypic.jpg has mypic_thmb.jpg adjacent!

