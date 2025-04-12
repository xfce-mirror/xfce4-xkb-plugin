sizes = [16, 32, 128]

foreach size : sizes
  install_data(
    '@0@x@0@'.format(size) / 'org.xfce.panel.xkb.png',
    install_dir: get_option('prefix') / get_option('datadir') / 'icons' / 'hicolor' / '@0@x@0@'.format(size) / 'apps',
  )
endforeach

install_data(
  'scalable' / 'org.xfce.panel.xkb.svg',
  install_dir: get_option('prefix') / get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps',
)
