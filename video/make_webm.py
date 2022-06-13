#!/usr/bin/env python

import tkinter as tk
import tkinter.filedialog
import multiprocessing
import sys
import os
import subprocess
import glob
from threading import Thread

advanced_options = False

class WebmMakerApp(tk.Tk):
  def __init__(self):
    tk.Tk.__init__(self)
    rand_vec = os.urandom(4)
    self.rand_num = "%d%d%d%d" % (rand_vec[0], rand_vec[1],rand_vec[2],rand_vec[3])
    self.conversion_process = None
    self.title("WebM Maker")
    self.geometry("720x420")
    self.create_widgets()

  def open_file(self):
    self.input_val.set(tk.filedialog.askopenfilename(initialdir=os.getcwd()))

  def save_file(self):
    self.output.insert(0, tk.filedialog.asksaveasfilename(initialdir=os.getcwd(), filetypes=[('WebM files', '*.webm')]))

  def update_width_height_state(self):
    if self.video_width_height_enabled.get():
      self.video_width.configure(state='normal')
      self.video_height.configure(state='normal')
    else:
      self.video_width.configure(state='disabled')
      self.video_height.configure(state='disabled')

  def update_audio_state(self):
    if self.audio_enabled.get():
      self.qaudio.configure(state='normal')
    else:
      self.qaudio.configure(state='disabled')

  def update_crop_state(self):
    if self.video_crop_enabled.get():
      self.video_crop_size_x.configure(state='normal')
      self.video_crop_size_y.configure(state='normal')
      self.video_crop_offset_x.configure(state='normal')
      self.video_crop_offset_y.configure(state='normal')
    else:
      self.video_crop_size_x.configure(state='disabled')
      self.video_crop_size_y.configure(state='disabled')
      self.video_crop_offset_x.configure(state='disabled')
      self.video_crop_offset_y.configure(state='disabled')

  def update_times_state(self):
    if self.start_end_time_enabled.get():
      self.start_time.configure(state='normal')
      self.end_time.configure(state='normal')
    else:
      self.start_time.configure(state='disabled')
      self.end_time.configure(state='disabled')

  def create_widgets(self):
    self.columnconfigure(0, weight=2)
    self.columnconfigure(1, weight=10)
    self.columnconfigure(2, weight=10)
    self.columnconfigure(3, weight=1)
    cur_row = 0

    # Input
    self.rowconfigure(cur_row, weight=1)
    self.input_label = tk.Label(self, text="Input File")
    self.input_label.grid(column=0, row=cur_row, sticky='W')
    self.input_val = tk.StringVar()
    self.input = tk.Entry(self, textvariable=self.input_val)
    self.input.grid(column=1, row=0, columnspan=2, sticky='EW')
    self.input_browse = tk.Button(self, text="...", command=self.open_file)
    self.input_browse.grid(column=3, row=cur_row, sticky='EW')
    cur_row += 1

    # Video Width/Height
    self.rowconfigure(cur_row, weight=1)
    self.video_width_height_enabled = tk.IntVar()
    self.video_width_height_check = tk.Checkbutton(self, text="Video Width/Height", variable=self.video_width_height_enabled, command=self.update_width_height_state)
    self.video_width_height_check.grid(column=0, row=cur_row, sticky='W')
    self.video_width_val = tk.StringVar(value=-1)
    self.video_height_val = tk.StringVar(value=-1)
    self.video_width = tk.Spinbox(self, textvariable=self.video_width_val, from_=-1, to=3840)
    self.video_width.grid(column=1, row=cur_row, sticky='EW')
    self.video_height = tk.Spinbox(self, textvariable=self.video_height_val, from_=-1, to=2160)
    self.video_height.grid(column=2, row=cur_row, columnspan=2, sticky='EW')
    self.update_width_height_state()
    cur_row += 1

    # Video Crop
    self.rowconfigure(cur_row, weight=1)
    self.video_crop_frame = tk.Frame(self)
    self.video_crop_frame.rowconfigure(0, weight=1)
    self.video_crop_frame.columnconfigure(0, weight=8)
    self.video_crop_frame.columnconfigure(1, weight=21)
    self.video_crop_frame.columnconfigure(2, weight=21)
    self.video_crop_frame.columnconfigure(3, weight=21)
    self.video_crop_frame.columnconfigure(4, weight=21)
    self.video_crop_frame.grid(column=0, row=cur_row, columnspan=4, sticky='NSEW')
    self.video_crop_enabled = tk.IntVar()
    self.video_crop_check = tk.Checkbutton(self.video_crop_frame, text="Video Crop Size/Offset", variable=self.video_crop_enabled, command=self.update_crop_state)
    self.video_crop_check.grid(column=0, row=0, sticky='W')
    self.video_crop_size_x_val = tk.StringVar(value=100)
    self.video_crop_size_x = tk.Spinbox(self.video_crop_frame, textvariable=self.video_crop_size_x_val, from_=1, to=3840)
    self.video_crop_size_x.grid(column=1, row=0, sticky='EW')
    self.video_crop_size_y_val = tk.StringVar(value=100)
    self.video_crop_size_y = tk.Spinbox(self.video_crop_frame, textvariable=self.video_crop_size_y_val, from_=1, to=2160)
    self.video_crop_size_y.grid(column=2, row=0, sticky='EW')
    self.video_crop_offset_x_val = tk.StringVar(value=10)
    self.video_crop_offset_x = tk.Spinbox(self.video_crop_frame, textvariable=self.video_crop_offset_x_val, from_=0, to=2160)
    self.video_crop_offset_x.grid(column=3, row=0, sticky='EW')
    self.video_crop_offset_y_val = tk.StringVar(value=10)
    self.video_crop_offset_y = tk.Spinbox(self.video_crop_frame, textvariable=self.video_crop_offset_y_val, from_=0, to=2160)
    self.video_crop_offset_y.grid(column=4, row=0, sticky='EW')
    self.update_crop_state()
    cur_row += 1

    self.num_threads_val = tk.IntVar(value=1)
    self.qmin_val = tk.IntVar(value=0)
    self.qmax_val = tk.IntVar(value=63)

    if advanced_options:
      # Num Threads
      self.rowconfigure(cur_row, weight=1)
      self.num_threads_label = tk.Label(self, text="Num Threads")
      self.num_threads_label.grid(column=0, row=cur_row, sticky='W')
      self.num_threads = tk.Scale(self, orient='horizontal', from_=1, to=multiprocessing.cpu_count(), variable=self.num_threads_val)
      self.num_threads.grid(column=1, row=cur_row, columnspan=3, sticky='EW')
      cur_row += 1

      # QMin QMax
      self.rowconfigure(cur_row, weight=1)
      self.qmin_label = tk.Label(self, text="QMin QMax")
      self.qmin_label.grid(column=0, row=cur_row, sticky='W')
      self.qmin = tk.Scale(self, orient='horizontal', from_=0, to=63, variable=self.qmin_val)
      self.qmin.grid(column=1, row=cur_row, sticky='EW')
      self.qmax = tk.Scale(self, orient='horizontal', from_=0, to=63, variable=self.qmax_val)
      self.qmax.grid(column=2, row=cur_row, columnspan=2, sticky='EW')
      cur_row += 1

    # CRF
    self.rowconfigure(cur_row, weight=1)
    self.crf_label = tk.Label(self, text="CRF")
    self.crf_label.grid(column=0, row=cur_row, sticky='W')
    self.crf = tk.Scale(self, orient='horizontal', from_=0, to=63)
    self.crf.set(30)
    self.crf.grid(column=1, row=cur_row, columnspan=3, sticky='EW')
    cur_row += 1

    # Audio Quality
    self.rowconfigure(cur_row, weight=1)
    self.audio_enabled = tk.IntVar(value=1)
    self.qaudio_check = tk.Checkbutton(self, text="Audio Quality", variable=self.audio_enabled, command=self.update_audio_state)
    self.qaudio_check.grid(column=0, row=cur_row, sticky='W')
    self.qaudio = tk.Scale(self, orient='horizontal', from_=-1, to=10)
    self.qaudio.set(2)
    self.qaudio.grid(column=1, row=cur_row, columnspan=3, sticky='EW')
    self.update_audio_state()
    cur_row += 1

    # Start/End Time
    self.rowconfigure(cur_row, weight=1)
    self.start_end_time_enabled = tk.IntVar()
    self.start_end_time_check = tk.Checkbutton(self, text="Start/End Times", variable=self.start_end_time_enabled, command=self.update_times_state)
    self.start_end_time_check.grid(column=0, row=cur_row, sticky='W')
    self.start_time_val = tk.StringVar(value="00:00.0")
    self.start_time = tk.Entry(self, textvariable=self.start_time_val)
    self.start_time.grid(column=1, row=cur_row, sticky='EW')
    self.end_time_val = tk.StringVar(value="00:00.0")
    self.end_time = tk.Entry(self, textvariable=self.end_time_val)
    self.end_time.grid(column=2, row=cur_row, columnspan=2, sticky='EW')
    self.update_times_state()
    cur_row += 1

    # Output
    self.rowconfigure(cur_row, weight=1)
    self.output_label = tk.Label(self, text="Output File")
    self.output_label.grid(column=0, row=cur_row, sticky='W')
    self.output_val = tk.StringVar(value=os.path.join(os.getcwd(), 'output.webm'))
    self.output = tk.Entry(self, textvariable=self.output_val)
    self.output.grid(column=1, row=cur_row, columnspan=2, sticky='EW')
    self.output_browse = tk.Button(self, text="...", command=self.save_file)
    self.output_browse.grid(column=3, row=cur_row, sticky='EW')
    cur_row += 1

    # Make Button
    self.rowconfigure(cur_row, weight=1)
    self.make_button = tk.Button(self, text="Make WebM!", command=self.launch_makewebm)
    self.make_button.grid(column=0, row=cur_row, columnspan=3, sticky='NSEW')
    self.abort_button = tk.Button(self, text="X", command=self.abort_conversion)
    self.abort_button.grid(column=3, row=cur_row, sticky='NSEW')
    self.abort_button.configure(state='disabled')
    cur_row += 1

    # Log Output
    self.rowconfigure(cur_row, weight=1)
    self.output_message_val = tk.StringVar(value="Ready")
    self.output_message = tk.Entry(self, textvariable=self.output_message_val, state='disabled')
    self.output_message.grid(column=0, row=cur_row, columnspan=4, sticky='NSEW')
    cur_row += 1

  def gen_cmd(self, pass_num):
    cmd = ['ffmpeg', '-i', self.input.get()]

    if self.video_width_height_enabled.get() or self.video_crop_enabled.get():
      cmd.append('-vf')
      filters = []
      if self.video_crop_enabled.get():
        filters.append('crop={size_x}:{size_y}:{offset_x}:{offset_y}'.format(
          size_x=self.video_crop_size_x.get(),
          size_y=self.video_crop_size_y.get(),
          offset_x=self.video_crop_offset_x.get(),
          offset_y=self.video_crop_offset_y.get()
        ))
      if self.video_width_height_enabled.get():
        filters.append('scale={width}:{height}'.format(width=self.video_width.get(), height=self.video_height.get()))
      cmd.append(','.join(filters))

    cmd.extend([
      '-c:v', 'libvpx',
      '-deadline', 'good',
      '-cpu-used', '0',
      '-auto-alt-ref', '1',
      '-lag-in-frames', '16',
      '-arnr_max_frames', '2',
      '-threads', str(self.num_threads_val.get()),
      '-qmin', str(self.qmin_val.get()),
      '-qmax', str(self.qmax_val.get()),
      '-crf', str(self.crf.get()),
      '-passlogfile', 'tmp_passlogfile_{rand_num}'.format(rand_num=self.rand_num),
      '-pass', str(pass_num),
      '-b:v', '50M'
    ])

    if pass_num == 2 and self.audio_enabled.get():
      cmd.extend([
        '-c:a', 'libvorbis',
        '-qscale:a', str(self.qaudio.get()),
      ])
    else:
      cmd.append('-an')

    if self.start_end_time_enabled.get():
      cmd.extend([
        '-ss', self.start_time.get(),
        '-to', self.end_time.get()
      ])

    if pass_num == 1:
      cmd.extend(['-f', 'null', 'NUL'])
    else:
      cmd.append(self.output.get())

    return cmd

  def run_makewebm(self, cmds):
    self.make_button.configure(state='disabled')
    self.abort_button.configure(state='normal')
    self.output_message_val.set("Generating WebM...")

    # Remove output file if it already exists
    if os.path.isfile(self.output.get()):
      os.remove(self.output.get())

    for cmd in cmds:
      print(' '.join(cmd))
      sys.stdout.flush()
      self.conversion_process = subprocess.Popen(cmd)
      ret = self.conversion_process.wait()
      if ret != 0:
        break
    self.conversion_process = None

    for logfile in glob.glob(os.path.join(os.getcwd(), 'tmp_passlogfile_{rand_num}*.log'.format(rand_num=self.rand_num))):
      print("Removing {logfile}".format(logfile=logfile))
      os.remove(logfile)

    self.make_button.configure(state='normal')
    self.abort_button.configure(state='disabled')
    self.output_message_val.set("Ready")

  def launch_makewebm(self):
    Thread(target=self.run_makewebm, args=([self.gen_cmd(1), self.gen_cmd(2)],)).start()

  def abort_conversion(self):
    if (self.conversion_process):
      self.conversion_process.terminate()

if __name__ == "__main__":
  webm_app = WebmMakerApp()
  webm_app.mainloop()
