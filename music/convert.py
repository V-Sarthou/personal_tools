#!/usr/bin/env python

import sys
import os
import subprocess
#import eyed3

src_root = os.getcwd()

dst_root = os.path.join("C:\\", "Users", "Valentin", "Music", "forUSB")
g_convert_to_mp3 = True
g_gen_covers = True
g_overwrite_covers = True
g_resize_only = False
g_use_old_convert_method = False
fixed_dims = [250, 232]
front_cover_filename = "Folder.jpg"

fixed_ratio = float(fixed_dims[0]) / float(fixed_dims[1])
fixed_ratio_sqrd = fixed_ratio * fixed_ratio


def image_dims(file):
    return list(map(int, subprocess.check_output(["magick", "identify", "-format", "%w %h", file]).split(b' ')))


def get_new_dims(old_dims):
    ratio = float(old_dims[0]) / float(old_dims[1])
    new_dims = list(fixed_dims)
    if ratio <= fixed_ratio_sqrd:
        fixed_ratio_inv = 1.0 / fixed_ratio
        new_dims[0] = int(ratio * fixed_ratio_inv * float(fixed_dims[1]) + 0.5)
    else:
        ratio_inv = 1.0 / ratio
        new_dims[1] = int(ratio_inv * fixed_ratio * float(fixed_dims[0]) + 0.5)
    return new_dims


def init_dir(dir):
    try:
        os.stat(dir)
    except:
        os.makedirs(dir)


def file_exists(file):
    try:
        os.stat(file)
    except:
        return False
    return True


# Not used anymore (for previous MediaNav version)
corrected_height = int((float(fixed_dims[0]) * float(fixed_dims[0]) / float(fixed_dims[1])) + 0.5)


def resize_image_OLD(file, dest_file):
    init_dir(os.path.dirname(dest_file))
    if not g_overwrite_covers and file_exists(dest_file):
        print("{f} already exists".format(f=dest_file).encode('utf8'))
    else:
        print("Writing {f}".format(f=dest_file).encode('utf8'))
        sys.stdout.flush()
        subprocess.call(["magick", "convert", file,
                         "-resize", "%dx%d!" % (fixed_dims[1], fixed_dims[1]),
                         "-gravity", "northwest",
                         "-background", "black",
                         "-extent", "%dx%d" % (fixed_dims[1], corrected_height),
                         "-quality", "100",
                         dest_file])


def resize_image(file, dest_file, new_dims):
    init_dir(os.path.dirname(dest_file))
    if not g_overwrite_covers and file_exists(dest_file):
        print("{f} already exists".format(f=dest_file).encode('utf8'))
    else:
        print("Writing {f}".format(f=dest_file).encode('utf8'))
        sys.stdout.flush()
        subprocess.call(["magick", "convert", file,
                         "-resize", "%dx%d!" % (new_dims[0], new_dims[1]),
                         "-gravity", "center",
                         "-background", "black",
                         "-extent", "%dx%d" % (fixed_dims[0], fixed_dims[1]),
                         "-quality", "100",
                         dest_file])


def resize_only_image(file, dest_file):
    init_dir(os.path.dirname(dest_file))
    if not g_overwrite_covers and file_exists(dest_file):
        print("{f} already exists".format(f=dest_file).encode('utf8'))
    else:
        print("Writing {f}".format(f=dest_file).encode('utf8'))
        sys.stdout.flush()
        subprocess.call(["magick", "convert", file,
                         "-resize", "%dx%d!" % (300, 300),
                         "-quality", "100",
                         dest_file])


def all_covers_in(path):
    for root, dirs, files in os.walk(path):
        for filename in files:
            if filename.endswith(front_cover_filename):
                yield os.path.join(root, filename)


def get_dest_cover_file(file):
    if not file.startswith(src_root):
        raise ValueError('Source file %s is not in %s' % (file, src_root))
    return os.path.join(dst_root, file[1 + len(src_root):])  # +1 to get the separator


def convert_to_mp3(file, dest_file):
    init_dir(os.path.dirname(dest_file))
    if file_exists(dest_file):
        print("{f} already exists".format(f=dest_file).encode('utf8'))
    else:
        print("Writing {f}".format(f=dest_file).encode('utf8'))
        sys.stdout.flush()
        subprocess.call(["ffmpeg", "-i", file, "-b:a", "320k", dest_file])


def get_dest_song_file(file):
    if not file.startswith(src_root):
        raise ValueError('Source file %s is not in %s' % (file, src_root))
    file_mp3 = "%s.mp3" % os.path.splitext(file)[0]
    return os.path.join(dst_root, file_mp3[1 + len(src_root):])  # +1 to get the separator


def all_songs_in(path):
    for root, dirs, files in os.walk(path):
        for filename in files:
            if filename.endswith(".flac"):
                yield os.path.join(root, filename)


def setup_tags(mp3_path):
    audiofile = eyed3.load(mp3_path)

    artist = audiofile.tag.artist
    album = audiofile.tag.album
    title = audiofile.tag.title
    track_num = audiofile.tag.track_num[0]
    date = audiofile.tag.getBestDate()
    genre = audiofile.tag.genre

    audiofile.tag.clear()

    # Setup ID3V1 tags
    audiofile.initTag(version=(1, 1, 0))
    audiofile.tag.artist = artist
    audiofile.tag.album = album
    audiofile.tag.title = title
    audiofile.tag.track_num = int(track_num)
    audiofile.tag.release_date = date
    audiofile.tag.genre = genre
    audiofile.tag.save()

    # Setup ID3V2.3 tags
    audiofile.initTag(version=(2, 3, 0))
    audiofile.tag.artist = artist
    audiofile.tag.album = album
    audiofile.tag.title = title
    audiofile.tag.track_num = track_num
    audiofile.tag.recording_date = date
    audiofile.tag.genre = genre
    audiofile.tag.images.set(eyed3.id3.frames.ImageFrame.FRONT_COVER,
                             open(os.path.join(os.path.dirname(mp3_path), front_cover_filename), 'rb').read(),
                             'image/jpeg')
    audiofile.tag.save()


search_dir = os.path.abspath(sys.argv[1])

# Generate thumbnails
if g_gen_covers:
    for cover in all_covers_in(search_dir):
        dest_cover_file = get_dest_cover_file(cover)
        if g_resize_only:
            resize_only_image(cover, dest_cover_file)
        else:
            if g_use_old_convert_method:
                resize_image_OLD(cover, dest_cover_file)
            else:
                resize_image(cover, dest_cover_file, get_new_dims(image_dims(cover)))

# Convert to mp3
if g_convert_to_mp3:
    for song in all_songs_in(search_dir):
        dest_song = get_dest_song_file(song)
        convert_to_mp3(song, dest_song)
        #setup_tags(dest_song)
