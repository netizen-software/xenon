if get_option('tests')
  systemCleanerTest = executable(
    'system-cleaner-test',
    'core/cleaner/system-cleaner-test.cpp',
    include_directories: include_directories('../src'),
    link_with: XENON_CORE,
  )

  processManagerTest = executable(
    'process-manager-test',
    'core/processes/process-manager-test.cpp',
    include_directories: include_directories('../src'),
    link_with: XENON_CORE,
  )

  resourceMonitorTest = executable(
    'resource-monitor-test',
    'core/resources/resource-monitor-test.cpp',
    include_directories: include_directories('../src'),
    link_with: XENON_CORE,
  )

  repositoryManagerTest = executable(
    'repository-manager-test',
    'core/repositories/repository-manager-test.cpp',
    dependencies: GIO,
    include_directories: include_directories('../src'),
    link_with: XENON_CORE,
  )

  serviceManagerTest = executable(
    'service-manager-test',
    'core/services/service-manager-test.cpp',
    dependencies: GIO,
    include_directories: include_directories('../src'),
    link_with: XENON_CORE,
  )

  test('process-manager', processManagerTest)
  test('repository-manager', repositoryManagerTest)
  test('resource-monitor', resourceMonitorTest)
  test('service-manager', serviceManagerTest)
  test('system-cleaner', systemCleanerTest)
endif
