#!/bin/bash -e

if [ ! -d MultiCraft/MultiCraft.xcodeproj ]; then
	echo "Run this in build/iOS"
	exit 1
fi

FOLDER=$(pwd)
DEST=$(mktemp -d)

for dir in builtin textures; do
	cp -r ../../$dir $DEST/$dir
done
mkdir -p $DEST/fonts
cp ../../fonts/Retron2000.ttf $DEST/fonts/ # no PNG fonts because freetype
mkdir -p $DEST/games
cp -r ../../games/default $DEST/games/default
pushd ../../po
for lang in *; do
	[ ${#lang} -ne 2 ] && continue
	mopath=$DEST/locale/$lang/LC_MESSAGES
	mkdir -p $mopath
	pushd $lang
	for fn in *.po; do
		# brew install gettext
		/usr/local/Cellar/gettext/*/bin/msgfmt -o $mopath/${fn/.po/.mo} $fn
	done
	popd
done
popd

find $DEST -type d -name '.git' -print0 | xargs -0 -- rm -r
find $DEST -type f -name '.git*' -delete
find $DEST -type f -name '.DS_Store' -delete

# remove broken languages
for broken_lang in ja ko he; do
	find $DEST -type d -name $broken_lang -print0 | xargs -0 -- rm -r
done

# remove unnecessary mods
MODS=$DEST/games/default/files
for mods in compatibility experience; do
	find $DEST/games/default/files -type d -name $mods -print0 | xargs -0 -- rm -r
done

echo "Creating assets.zip"
ZIPDEST=$FOLDER/assets.zip
rm -f $ZIPDEST

cd $DEST; zip -1r $ZIPDEST -- *
cd $FOLDER; rm -rf $DEST

###########

echo "Creating worlds.zip"
ZIPDEST=$FOLDER/worlds.zip
rm -f $ZIPDEST

cd ../..; zip -0r $ZIPDEST -- worlds
