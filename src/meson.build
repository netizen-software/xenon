XENON_INCLUDE_DIR = include_directories('.')

XENON_CORE = static_library(
  'xenon-core',
  'core/resources/resource-monitor.cpp',
  include_directories: XENON_INCLUDE_DIR,
)

executable(
  'xenon',
  [
    'main.cpp',
    'application.cpp',
    'main-window.cpp',
    'ui/resource-monitor-view.cpp',
    XENON_RESOURCES,
  ],
  dependencies: [GTK, ADWAITA],
  include_directories: XENON_INCLUDE_DIR,
  link_with: XENON_CORE,
  install: true,
)
