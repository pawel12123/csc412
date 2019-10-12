#!/bin/sh

chmod +x split.c
chmod +x dimensions.c
chmod +x crop.c
chmod +x rotate.c

gcc split.c imageIO/imageIO_TGA.c -o split.sh
gcc crop.c imageIO/imageIO_TGA.c -o crop.sh

for image in $1*.tga
do
	echo $image
	echo ""
	W=`identify ./$image | cut -f 3 -d " " | sed s/x.*//`
	H=`identify ./$image | cut -f 3 -d " " | sed s/.*x//`
	./split.sh $image $2
	./rotate.sh $image $2 L
done
