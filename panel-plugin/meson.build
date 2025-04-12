plugin_sources = [
  'xkb-cairo.c',
  'xkb-cairo.h',
  'xkb-dialog.c',
  'xkb-dialog.h',
  'xkb-keyboard.c',
  'xkb-keyboard.h',
  'xkb-modifier.c',
  'xkb-modifier.h',
  'xkb-plugin.c',
  'xkb-plugin.h',
  'xkb-properties.h',
  'xkb-util.c',
  'xkb-util.h',
  'xkb-xfconf.c',
  'xkb-xfconf.h',
  xfce_revision_h,
]

plugin_install_subdir = 'xfce4' / 'panel' / 'plugins'

plugin_lib = shared_module(
  'xkb',
  plugin_sources,
  gnu_symbol_visibility: 'hidden',
  c_args: [
    '-DG_LOG_DOMAIN="@0@"'.format('xfce4-xkb-plugin'),
    '-DFLAGSRELDIR="@0@"'.format('xfce4/xkb/flags'),
    '-DWNCK_I_KNOW_THIS_IS_UNSTABLE',
  ],
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    garcon,
    glib,
    gtk,
    libnotify,
    librsvg,
    libwnck,
    libxfce4panel,
    libxfce4ui,
    libxfce4util,
    libxklavier,
    x11,
    xfconf,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('libdir') / plugin_install_subdir,
)

i18n.merge_file(
  input: 'xkb.desktop.in',
  output: 'xkb.desktop',
  po_dir: '..' / 'po',
  type: 'desktop',
  install: true,
  install_dir: get_option('prefix') / get_option('datadir') / plugin_install_subdir,
)
