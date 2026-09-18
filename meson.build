project(
  'xenon',
  ['c', 'cpp'],
  version: '0.1.0',
  default_options: [
    'cpp_std=c++20',
    'warning_level=3',
  ],
)

GNOME = import('gnome')
GTK = dependency('gtk4', version: '>= 4.12')
ADWAITA = dependency('libadwaita-1', version: '>= 1.4')

subdir('data')
subdir('src')
subdir('tests')
