flags = [
  'ae',
  'al',
  'am',
  'ar',
  'at',
  'az',
  'ba',
  'ben',
  'be',
  'bg',
  'bh',
  'br',
  'by',
  'ca',
  'ch',
  'cu',
  'cz',
  'de',
  'dj',
  'dk',
  'dvorak',
  'dz',
  'ee',
  'eg',
  'el',
  'epo',
  'es',
  'fi',
  'fo',
  'fr',
  'gb',
  'ge',
  'gr',
  'hr',
  'hu',
  'ie',
  'il',
  'in',
  'iq',
  'ir',
  'is',
  'it',
  'jo',
  'jp',
  'kh',
  'km',
  'kp',
  'kr',
  'kw',
  'kz',
  'la',
  'lb',
  'lt',
  'lv',
  'ly',
  'ma',
  'me',
  'mk',
  'mm',
  'mn',
  'mt',
  'mx',
  'nl',
  'no',
  'om',
  'pl',
  'pt',
  'qa',
  'ro',
  'rs',
  'ru',
  'sa',
  'sd',
  'se',
  'si',
  'sk',
  'so',
  'sr',
  'sy',
  'th',
  'tn',
  'tr',
  'ua',
  'us',
  'uy',
  'uz',
  'vn',
  'ye',
]

# Silence warning about symlink copy
# https://gitlab.xfce.org/xfce/xfce4-power-manager/-/issues/268
if meson.version().version_compare('>= 1.3.0')
  foreach flag : flags
    install_data(
      '@0@.svg'.format(flag),
      install_dir: pkgdatadir / 'flags',
      follow_symlinks: true,
    )
  endforeach
else
  foreach flag : flags
    install_data(
      '@0@.svg'.format(flag),
      install_dir: pkgdatadir / 'flags',
    )
  endforeach
endif
