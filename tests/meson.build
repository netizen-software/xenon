if get_option('tests')
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

  test('process-manager', processManagerTest)
  test('resource-monitor', resourceMonitorTest)
endif
