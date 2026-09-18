APPLICATION_ID = 'io.github.netizen_software.Xenon'

desktopFile = configure_file(
  input: APPLICATION_ID + '.desktop.in',
  output: APPLICATION_ID + '.desktop',
  configuration: configuration_data(),
)

install_data(desktopFile, install_dir: get_option('datadir') / 'applications')
install_data(
  APPLICATION_ID + '.appdata.xml',
  install_dir: get_option('datadir') / 'metainfo',
)

XENON_RESOURCES = GNOME.compile_resources(
  'xenon-resources',
  'resources/xenon.gresource.xml',
  source_dir: 'resources',
  c_name: 'xenon',
)
