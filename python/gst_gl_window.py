#!/usr/bin/env python3
# Timestamp: 2026-02-28T15:50
# Not worked now

import gi
gi.require_version('Gst', '1.0')
gi.require_version('GstVideo', '1.0')
gi.require_version('Gtk', '3.0')
from gi.repository import Gst, GstVideo, Gtk, Gdk

Gst.init(None)
Gtk.init([])

# Создаём окно GTK
window = Gtk.Window()
window.set_default_size(1280, 720)   # Размер окна
window.move(100, 100)                # Позиция окна на экране
window.connect("destroy", Gtk.main_quit)
window.show_all()

# Получаем ID виджета для GStreamer
gdk_window = window.get_window()
window_id = gdk_window.get_xid() if gdk_window else 0

# Создаём пайплайн
pipeline = Gst.parse_launch(
    'filesrc location=/home/alex/Videos/Fujitsu_6mbit_Low_delay.mpg ! '
    'tsdemux ! queue ! h264parse ! v4l2h264dec ! glupload ! glimagesink name=sink'
)

# Устанавливаем widget property для glimagesink
sink = pipeline.get_by_name('sink')
sink.set_property('widget', window_id)

# Запускаем пайплайн
pipeline.set_state(Gst.State.PLAYING)

# GTK main loop
Gtk.main()

# Останавливаем пайплайн при выходе
pipeline.set_state(Gst.State.NULL)
