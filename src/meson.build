XENON_INCLUDE_DIR = include_directories('.')

XENON_CORE = static_library(
  'xenon-core',
  [
    'core/processes/process-manager.cpp',
    'core/resources/resource-monitor.cpp',
    'core/services/service-manager.cpp',
  ],
  dependencies: GIO,
  include_directories: XENON_INCLUDE_DIR,
)

executable(
  'xenon',
  [
    'main.cpp',
    'application.cpp',
    'main-window.cpp',
    'ui/process-manager-view.cpp',
    'ui/resource-monitor-view.cpp',
    'ui/services-manager-view.cpp',
    XENON_RESOURCES,
  ],
  dependencies: [GTK, ADWAITA],
  include_directories: XENON_INCLUDE_DIR,
  link_with: XENON_CORE,
  install: true,
)
